#include "lucidgloves_fw_discovery.h"

#include <chrono>
#include <utility>

#include "communication/encoding/alpha_encoding_service.h"
#include "communication/managers/hardware_communication_manager.h"
#include "communication/probers/prober_bluetooth_win.h"
#include "communication/probers/prober_serial_identifiers_win.h"
#include "communication/services/service_bluetooth.h"
#include "communication/services/service_serial.h"
#include "device/lucidgloves/lucidgloves_device.h"
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

void LucidglovesDeviceDiscoverer::StartDiscovery(std::function<void(std::unique_ptr<og::IDevice> device)> callback) {
  callback_ = callback;

  if (communication_configuration_.auto_probe) {
    logger.Log(og::kLoggerLevel_Warning, "Auto probing is currently not implemented.");

    return;
  }

  if(communication_configuration_.bluetooth.enabled) {
    logger.Log(og::kLoggerLevel_Info, "Using bluetooth configuration.");



  } else {
    if(!communication_configuration_.serial.enabled) logger.Log(og::kLoggerLevel_Warning, "No communication type set. using serial");

    logger.Log(og::kLoggerLevel_Info, "Using serial configuration.");
  }

  is_active_ = true;
}

void LucidglovesDeviceDiscoverer::ConnectableProberThread(std::unique_ptr<ICommunicationProber> prober) {
  while (is_active_) {
    std::vector<std::unique_ptr<ICommunicationService>> found_services;

    bool has_devices = prober->InquireDevices(found_services);

    for (auto& service : found_services) {
      logger.Log(og::kLoggerLevel_Info, "Device discovered with identifier: %s", service->GetIdentifier().c_str());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
}

void LucidglovesDeviceDiscoverer::OnDeviceFound(const og::DeviceConfiguration& configuration, std::unique_ptr<ICommunicationService> service) {
  std::lock_guard<std::mutex> lock(device_found_mutex_);

  std::unique_ptr<IEncodingService> encoding_service;
  switch (configuration.communication.encoding_type) {
    default:
      logger.Log(og::kLoggerLevel_Warning, "No encoding type set. Using alpha encoding.");
    case og::kEncodingType_Alpha:
      encoding_service = std::make_unique<AlphaEncodingService>(std::get<og::DeviceAlphaEncodingConfiguration>(configuration.communication.encoding));
      break;
  }

  std::unique_ptr<ICommunicationManager> communication_manager;
  switch (configuration.type) {
    default:
      logger.Log(og::kLoggerLevel_Warning, "Unknown or unset device type. Using hardware communication manager.");
    case og::kDeviceType_lucidgloves:
      communication_manager = std::make_unique<HardwareCommunicationManager>(std::move(service), std::move(encoding_service));
      break;
  }

  std::unique_ptr<og::IDevice> lucidgloves_device = std::make_unique<LucidglovesDevice>(configuration, std::move(communication_manager));

  callback_(std::move(lucidgloves_device));
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