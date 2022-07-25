#include "device_discovery.h"

#include "encoding/alpha/alpha_encoding_service.h"
#include "encoding/encoding_service.h"
#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

DeviceDiscovery::DeviceDiscovery(
    const og::LegacyConfiguration& legacy_configuration, std::function<void(std::unique_ptr<og::Device> device)>& callback) {
  callback_ = callback;

  prober_manager_ = std::make_unique<ProberManager>(
      [&](std::unique_ptr<ICommunicationService> communication_service) { OnDeviceDiscovered(std::move(communication_service)); });

  legacy_configuration_ = legacy_configuration;
}

void DeviceDiscovery::OnDeviceDiscovered(std::unique_ptr<ICommunicationService> communication_service) {
  // assume alpha encoding
  std::unique_ptr<IEncodingService> encoding_service = std::make_unique<AlphaEncodingService>(legacy_configuration_.encoding_configuration);

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

  switch (device_info.device_type) {
    default:
      logger.Log(og::kLoggerLevel_Warning, "Device does not support fetching info. setting device type to lucidgloves.");
    case og::kGloveType_lucidgloves: {
      logger.Log(og::kLoggerLevel_Info, "Setting up lucidgloves device");

      callback_(std::make_unique<og::Device>());
    }
  }
}