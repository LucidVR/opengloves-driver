#include "lucidgloves_fw_discovery.h"

#include <chrono>
#include <utility>

#include "communication/encoding/alpha_encoding_service.h"
#include "communication/managers/hardware_communication_manager.h"
#include "communication/probers/bluetooth/prober_bluetooth.h"
#include "communication/probers/serial/prober_serial.h"
#include "communication/services/bluetooth/service_bluetooth.h"
#include "communication/services/serial/service_serial.h"
#include "devices/lucidgloves_device.h"
#include "opengloves_interface.h"

static og::Logger& logger = og::Logger::GetInstance();

static const std::vector<SerialProberIdentifier> lucidgloves_serial_ids = {
    {"10C4", "EA60"},  // cp2102
    {"7523", "7524"}   // ch340
};
static const std::vector<std::string> lucidgloves_bt_ids = {"lucidgloves", "lucidgloves-left", "lucidgloves-right"};

LucidglovesDeviceDiscoverer::LucidglovesDeviceDiscoverer(
    og::CommunicationConfiguration communication_configuration, std::vector<og::DeviceConfiguration> device_configurations)
    : device_configurations_(std::move(device_configurations)), communication_configuration_(communication_configuration) {}

void LucidglovesDeviceDiscoverer::StartDiscovery(std::function<void(std::unique_ptr<og::Device> device)> callback) {
  callback_ = callback;

  if (communication_configuration_.auto_probe) {
    // setup device probers
    std::vector<std::unique_ptr<ICommunicationProber>> probers;
    if (communication_configuration_.serial.enabled) {
      SerialProberConfiguration prober_configuration{lucidgloves_serial_ids};
      probers.emplace_back(std::make_unique<SerialCommunicationProber>(prober_configuration));
    }

    if (communication_configuration_.bluetooth.enabled) {
      BluetoothProberConfiguration prober_configuration{lucidgloves_bt_ids};
      probers.emplace_back(std::make_unique<BluetoothCommunicationProber>(prober_configuration));
    }

    for (auto& prober : probers) {
      prober_threads_.emplace_back(std::thread(&LucidglovesDeviceDiscoverer::ProberThread, this, std::move(prober)));
    }

  } else {
    // otherwise just immediately create the devices
    for (const auto& device_configuration : device_configurations_) {
      if (!device_configuration.enabled) continue;

      const og::DeviceCommunicationConfiguration& communication_configuration = device_configuration.device_communication;

      // setup encoding
      std::unique_ptr<IEncodingService> encoding_service;
      switch (communication_configuration.encoding_type) {
        default:
          logger.Log(og::kLoggerLevel_Warning, "Encoding type not set. Using alpha encoding");
        case og::kEncodingType_Alpha:
          logger.Log(og::kLoggerLevel_Info, "Encoding set to alpha encoding");
          encoding_service =
              std::make_unique<AlphaEncodingService>(std::get<og::DeviceAlphaEncodingConfiguration>(communication_configuration.encoding));
      }

      // setup communication
      std::unique_ptr<ICommunicationService> communication_service;
      switch (communication_configuration.communication_type) {
        case og::kCommunicationType_Bluetooth:
          logger.Log(og::kLoggerLevel_Info, "Communication set to bluetooth");
          communication_service = std::make_unique<BluetoothCommunicationService>(
              std::get<og::DeviceBluetoothCommunicationConfiguration>(communication_configuration.communication));
          break;
        default:
          logger.Log(og::kLoggerLevel_Warning, "Communication type not set. Using serial");
        case og::kCommunicationType_Serial:
          logger.Log(og::kLoggerLevel_Info, "Communication set to serial");
          communication_service = std::make_unique<SerialCommunicationService>(
              std::get<og::DeviceSerialCommunicationConfiguration>(communication_configuration.communication));
      }

      std::unique_ptr<ICommunicationManager> communication_manager =
          std::make_unique<HardwareCommunicationManager>(std::move(communication_service), std::move(encoding_service));

      callback_(std::make_unique<LucidglovesDevice>(device_configuration, std::move(communication_manager)));
    }
  }

  is_active_ = true;
}

void LucidglovesDeviceDiscoverer::ProberThread(std::unique_ptr<ICommunicationProber> prober) {
  while (is_active_) {
    std::vector<std::unique_ptr<ICommunicationService>> found_services{};

    og::CommunicationType type = prober->InquireDevices(found_services);

    for (auto& service : found_services) {
      logger.Log(og::kLoggerLevel_Info, "Device discovered with prober: %s", prober->GetName().c_str());

      OnDeviceFound(std::move(service), type);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
}

void LucidglovesDeviceDiscoverer::OnDeviceFound(
    std::unique_ptr<ICommunicationService> communication_service, og::CommunicationType communication_type) {
  std::lock_guard<std::mutex> lock(device_found_mutex_);

  og::DeviceAlphaEncodingConfiguration default_encoding_configuration{};
  std::unique_ptr<IEncodingService> encoding_service = std::make_unique<AlphaEncodingService>(default_encoding_configuration);

  std::string soutput = encoding_service->EncodePacket({
      .type = og::kOutputDataType_FetchInfo,
      .data = {.fetch_info = {.get_info = true}},
  });

  // try to retrieve information from the device
  int retries = 0;
  bool fw_is_valid = false;

  // try to fill this struct
  og::InputInfoData device_info{};
  do {
    std::string buff;

    // flush any extra packets we might have
    communication_service->PurgeBuffer();

    communication_service->RawWrite(soutput);
    communication_service->ReceiveNextPacket(buff);

    og::Input received = encoding_service->DecodePacket(buff);

    // we have a firmware version that is giving us info
    if (received.type == og::kInputDataType_Info) {
      fw_is_valid = true;

      device_info = received.data.info;

      // start streaming data
      og::Output device_start_stream = {.type = og::kOutputDataType_FetchInfo, .data = {.fetch_info = {.start_streaming = true}}};
      communication_service->RawWrite(encoding_service->EncodePacket(device_start_stream));

      logger.Log(og::kLoggerLevel_Info, "Lucidgloves device found and provides information");

      break;

    } else if (received.type == og::kInputDataType_Peripheral) {
      logger.Log(og::kLoggerLevel_Info, "Lucidgloves device found but does not provide information");
      fw_is_valid = true;
    } else {
      logger.Log(og::kLoggerLevel_Warning, "Device discovery received an invalid packet");
    }

    retries++;
  } while (retries < 5);

  if (!fw_is_valid) {
    logger.Log(og::kLoggerLevel_Warning, "Device was not a compatible opengloves device.");

    return;
  }

  logger.Log(og::kLoggerLevel_Info, "Found a compatible opengloves device. Initialising device...");

  // eventually we want this to be able to perform custom device-based logic
  switch (device_info.device_type) {
    default:
      logger.Log(og::kLoggerLevel_Warning, "Device does not support fetching info from lucidgloves firmware. setting device type to lucidgloves");
    case og::kDeviceType_lucidgloves: {
      logger.Log(og::kLoggerLevel_Info, "Setting up lucidgloves device");

      std::unique_ptr<ICommunicationManager> communication_manager =
          std::make_unique<HardwareCommunicationManager>(std::move(communication_service), std::move(encoding_service));

      og::DeviceConfiguration configuration {
        .enabled = true, .device_type = og::kDeviceType_lucidgloves,
        .device_communication = {
            .communication_type = communication_type,
            .encoding_type = og::kEncodingType_Alpha,
        }
      };

      callback_(std::make_unique<LucidglovesDevice>(configuration, std::move(communication_manager)));
    }
  }
}

void LucidglovesDeviceDiscoverer::StopDiscovery() {
  if (is_active_.exchange(false)) {
    logger.Log(og::kLoggerLevel_Info, "Attempting to clean up queryable device probers...");
    for (auto& prober_thread : prober_threads_) {
      prober_thread.join();
    }

    logger.Log(og::kLoggerLevel_Info, "Cleaned up queryable device probers");
  }
}

LucidglovesDeviceDiscoverer::~LucidglovesDeviceDiscoverer() {
  StopDiscovery();
}