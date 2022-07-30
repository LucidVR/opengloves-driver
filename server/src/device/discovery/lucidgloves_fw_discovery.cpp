#include "lucidgloves_fw_discovery.h"

#include <chrono>

#include "encoding/alpha/alpha_encoding_service.h"
#include "managers/communication_manager.h"
#include "probers/bluetooth/prober_bluetooth.h"
#include "probers/serial/prober_serial.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

static const std::vector<SerialProberSearchParam> lucidgloves_serial_ids = {
    {"10C4", "EA60"},  // cp2102
    {"7523", "7524"}   // ch340
};
static const std::vector<std::string> lucidgloves_bt_ids = {"lucidgloves", "lucidgloves-left", "lucidgloves-right"};

LucidglovesDeviceDiscoverer::LucidglovesDeviceDiscoverer(const og::DeviceDefaultConfiguration& default_configuration) {
  default_configuration_ = default_configuration;
}

void LucidglovesDeviceDiscoverer::StartDiscovery(std::function<void(std::unique_ptr<og::Device> device)> callback) {
  callback_ = callback;

  // setup queryable device discoverers
  queryable_probers_.emplace_back(std::make_unique<SerialCommunicationProber>(lucidgloves_serial_ids));
  queryable_probers_.emplace_back(std::make_unique<BluetoothCommunicationProber>(lucidgloves_bt_ids));

  for (auto& prober : queryable_probers_) {
    std::thread prober_thread = std::thread(&LucidglovesDeviceDiscoverer::QueryableProberThread, this, prober.get());
  }
}

void LucidglovesDeviceDiscoverer::QueryableProberThread(ICommunicationProber* prober) {
  while (is_active_) {
    std::vector<std::unique_ptr<ICommunicationService>> found_services;

    for (auto& service : found_services) {
      logger.Log(kLoggerLevel_Info, "Device discovered with prober: %s", prober->GetName().c_str());

      OnQueryableDeviceFound(std::move(service));
    }
  }
}

void LucidglovesDeviceDiscoverer::OnQueryableDeviceFound(std::unique_ptr<ICommunicationService> communication_service) {
  std::lock_guard<std::mutex> lock(device_found_mutex_);

  std::unique_ptr<IEncodingService> encoding_service = std::make_unique<AlphaEncodingService>(default_configuration_.encoding_configuration);

  Output fetch_info_output = {
      .type = kOutputDataType_FetchInfo,
      .data = {.fetch_info = {.get_info = true}},
  };

  std::string soutput = encoding_service->EncodePacket(fetch_info_output) + "\n";

  // try to retrieve information from the device
  int retries = 0;
  bool fw_is_valid = false;

  // try to fill this struct
  InputInfoData device_info{};
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
      Output device_start_stream = {.type = kOutputDataType_FetchInfo, .data = {.fetch_info = {.start_streaming = true}}};
      communication_service->RawWrite(encoding_service->EncodePacket(device_start_stream));

      break;

    } else if (received.type == og::kInputDataType_Peripheral) {
      fw_is_valid = true;
    } else {
      logger.Log(og::kLoggerLevel_Warning, "Device discovery received an invalid packet.");
    }

    retries++;
  } while (retries < 5);

  if (!fw_is_valid) {
    logger.Log(og::kLoggerLevel_Warning, "Device was not a compatible opengloves device.");

    return;
  }

  // eventually we want this to be able to perform custom device-based logic
  switch (device_info.device_type) {
    default:
      logger.Log(og::kLoggerLevel_Warning, "Device does not support fetching info from lucidgloves firmware. setting device type to lucidgloves.");
    case og::kGloveType_lucidgloves: {
      logger.Log(og::kLoggerLevel_Info, "Setting up lucidgloves device");

      std::unique_ptr<CommunicationManager> communication_manager =
          std::make_unique<CommunicationManager>(std::move(communication_service), std::move(encoding_service));

      // callback_(std::make_unique<og::Device>());
    }
  }
}

void LucidglovesDeviceDiscoverer::StopDiscovery() {
  if (is_active_.exchange(false)) {
    for (unsigned int i = 0; i < queryable_probers_.size(); i++) {
      const std::string prober_name = queryable_probers_[i]->GetName();
      logger.Log(kLoggerLevel_Info, "Attempting to clean up prober: %s", prober_name.c_str());

      queryable_prober_threads_[i].join();
      queryable_probers_[i].reset();

      logger.Log(kLoggerLevel_Info, "Cleaned up prober: %s", prober_name.c_str());
    }
  }
}

LucidglovesDeviceDiscoverer::~LucidglovesDeviceDiscoverer() {
  StopDiscovery();
}