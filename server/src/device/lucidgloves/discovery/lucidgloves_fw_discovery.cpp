// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "lucidgloves_fw_discovery.h"

#include <chrono>
#include <utility>

#include "communication/encoding/alpha_encoding_service.h"
#include "communication/managers/hardware_communication_manager.h"
#include "communication/probers/prober_bluetooth_connectable.h"
#include "communication/probers/prober_serial_connectable.h"
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

  if (communication_configuration_.bluetooth.enabled) {
    logger.Log(og::kLoggerLevel_Info, "Setting up bluetooth probers...");

    for (const auto& device_configuration : device_configurations_) {
      const og::DeviceBluetoothCommunicationConfiguration& configuration = device_configuration.communication.bluetooth;
      BluetoothPortProberConfiguration prober_configuration{configuration.name};

      prober_threads_.emplace_back(
          &LucidglovesDeviceDiscoverer::ProberThread,
          this,
          std::make_unique<BluetoothPortProber>(prober_configuration),
          [=](std::unique_ptr<ICommunicationService> service) { OnDeviceFound(device_configuration, std::move(service)); }
          );
    }
  } else {
    logger.Log(og::kLoggerLevel_Info, "Not probing for bluetooth devices as it was disabled in settings");
  }

  if (communication_configuration_.serial.enabled) {
    logger.Log(og::kLoggerLevel_Info, "Setting up serial probers...");

    for (const auto& device_configuration : device_configurations_) {
      const og::DeviceSerialCommunicationConfiguration& configuration = device_configuration.communication.serial;

      SerialPortProberConfiguration prober_configuration{configuration.port_name};
      prober_threads_.emplace_back(std::thread(
          &LucidglovesDeviceDiscoverer::ProberThread,
          this,
          std::make_unique<SerialPortProber>(prober_configuration),
          [=](std::unique_ptr<ICommunicationService> service) { OnDeviceFound(device_configuration, std::move(service)); }));
    }
  } else {
    logger.Log(og::kLoggerLevel_Info, "Not probing for serial devices as it was disabled in settings");
  }

  is_active_ = true;
}

void LucidglovesDeviceDiscoverer::ProberThread(
    std::unique_ptr<ICommunicationProber> prober, const std::function<void(std::unique_ptr<ICommunicationService> service)>& callback) {
  while (is_active_) {
    std::vector<std::unique_ptr<ICommunicationService>> found_services;

    bool has_devices = prober->InquireDevices(found_services);
    for (auto& service : found_services) {
      logger.Log(og::kLoggerLevel_Info, "Device discovered with identifier: %s", service->GetIdentifier().c_str());
      callback(std::move(service));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
}

void LucidglovesDeviceDiscoverer::OnDeviceFound(const og::DeviceConfiguration& configuration, std::unique_ptr<ICommunicationService> service) {
  std::lock_guard<std::mutex> lock(device_found_mutex_);

  std::unique_ptr<IEncodingService> encoding_service = std::make_unique<AlphaEncodingService>(configuration.communication.encoding);

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