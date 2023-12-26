// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "physical_device_provider.h"

#include "device/configuration/device_configuration.h"
#include "device/drivers/knuckle_device_driver.h"
#include "nlohmann/json.hpp"
#include "services/driver_external.h"
#include "services/driver_internal.h"
#include "util/driver_log.h"
#include "util/file_path.h"

static bool InitialiseExternalServices() {
  // spin up the tracking discovery service before the process so we know we have a server running before the client
  DriverInternalServer::GetInstance();
  DriverExternalServer::GetInstance();

  const std::string bin_path = GetDriverBinPath();

  DriverLog("Binary path located: %s", bin_path.c_str());

  return CreateBackgroundProcess(bin_path, "opengloves_overlay.exe");
}

static og::ServerConfiguration CreateServerConfiguration() {
  auto driver_configuration = GetDriverConfigurationMap();
  auto serial_configuration = GetSerialConfigurationMap();
  auto bluetooth_configuration = GetBluetoothSerialConfigurationMap();
  auto encoding_configuration = GetAlphaEncodingConfigurationMap();
  auto namedpipe_configuration = GetNamedPipeConfigurationMap();

  std::vector<og::DeviceConfiguration> device_configurations;

  if (const bool left_enabled = std::get<bool>(driver_configuration["left_enabled"])) {
    device_configurations.push_back({
        .enabled = left_enabled,
        .hand = og::kHandLeft,
        .type = og::kDeviceType_lucidgloves,
        .communication =
            {
                .serial =
                    {
                        .port_name = std::get<std::string>(serial_configuration["left_port"]),
                    },
                .bluetooth =
                    {
                        .name = std::get<std::string>(bluetooth_configuration["left_name"]),
                    },
                .encoding =
                    {
                        .max_analog_value = static_cast<unsigned int>(std::get<int>(encoding_configuration["max_analog_value"])),
                    },
            },
    });
  }

  ;
  if (const bool right_enabled = std::get<bool>(driver_configuration["right_enabled"])) {
    device_configurations.push_back({
        .enabled = right_enabled,
        .hand = og::kHandRight,
        .type = og::kDeviceType_lucidgloves,
        .communication =
            {
                .serial =
                    {
                        .port_name = std::get<std::string>(serial_configuration["right_port"]),
                    },
                .bluetooth =
                    {
                        .name = std::get<std::string>(bluetooth_configuration["right_name"]),
                    },
                .encoding =
                    {
                        .max_analog_value = static_cast<unsigned int>(std::get<int>(encoding_configuration["max_analog_value"])),
                    },
            },
    });
  }

  og::ServerConfiguration result = {
      .communication =
          {
              .serial =
                  {
                      .enabled = std::get<bool>(serial_configuration["enabled"]),
                  },
              .bluetooth =
                  {
                      .enabled = std::get<bool>(bluetooth_configuration["enabled"]),
                  },
              .named_pipe =
                  {
                      .enabled = std::get<bool>(namedpipe_configuration["enabled"]),
                  },
          },
      .devices = device_configurations,
  };

  return result;
}

vr::EVRInitError PhysicalDeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
  VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

  if (!InitDriverLog(vr::VRDriverLog())) {
    DriverLog("Failed to initialise driver logs!");

    return vr::VRInitError_Driver_NotLoaded;
  }

  DebugDriverLog("OpenGloves is running in DEBUG MODE");

  if (!InitialiseExternalServices()) {
    DriverLog("Failed to initialise external services. Exiting..");
    return vr::VRInitError_Init_FileNotFound;
  }

  static og::Logger& logger = og::Logger::GetInstance();
  logger.SubscribeToLogger([&](const std::string& message, og::LoggerLevel log_level) {
    std::string str_level;
    switch (log_level) {
      case og::LoggerLevel::kLoggerLevel_Info:
        str_level = "Info";
        break;
      case og::LoggerLevel::kLoggerLevel_Warning:
        str_level = "Warning";
        break;
      case og::LoggerLevel::kLoggerLevel_Error:
        str_level = "ERROR";
        break;
    }

    DriverLog("OpenGloves Server %s: %s", str_level.c_str(), message.c_str());
  });

  // initialise opengloves
  ogserver_ = std::make_unique<og::Server>(CreateServerConfiguration());

  const auto driver_configuration = GetDriverConfigurationMap();
  if (std::get<bool>(driver_configuration["left_enabled"])) {
    device_drivers_[vr::TrackedControllerRole_LeftHand] = std::make_unique<KnuckleDeviceDriver>(vr::TrackedControllerRole_LeftHand);
    vr::VRServerDriverHost()->TrackedDeviceAdded(
        device_drivers_[vr::TrackedControllerRole_LeftHand]->GetSerialNumber().c_str(),
        vr::TrackedDeviceClass_Controller,
        device_drivers_[vr::TrackedControllerRole_LeftHand].get());
  }

  if (std::get<bool>(driver_configuration["right_enabled"])) {
    device_drivers_[vr::TrackedControllerRole_RightHand] = std::make_unique<KnuckleDeviceDriver>(vr::TrackedControllerRole_RightHand);
    vr::VRServerDriverHost()->TrackedDeviceAdded(
        device_drivers_[vr::TrackedControllerRole_RightHand]->GetSerialNumber().c_str(),
        vr::TrackedDeviceClass_Controller,
        device_drivers_[vr::TrackedControllerRole_RightHand].get());
  }

  ogserver_->StartProber([&](std::unique_ptr<og::IDevice> found_device) {
    DriverLog("Physical device provider found a device, hand: %s", found_device->GetConfiguration().hand == og::kHandLeft ? "Left" : "Right");

    vr::ETrackedControllerRole role = vr::TrackedControllerRole_Invalid;
    switch (found_device->GetConfiguration().hand) {
      case og::kHandLeft:
        role = vr::TrackedControllerRole_LeftHand;
        break;
      case og::kHandRight:
        role = vr::TrackedControllerRole_RightHand;
        break;
    }

    if (role == vr::TrackedControllerRole_Invalid) {
      DriverLog("Received device with unknown role, aborting.");
      return;
    }

    if (device_drivers_[role] != nullptr) {
      device_drivers_[role]->SetDeviceDriver(std::move(found_device));
    } else {
      DriverLog("Discovered a device but no steamvr device was found to use it!");
    }
  });

  return vr::VRInitError_None;
}

const char* const* PhysicalDeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

void PhysicalDeviceProvider::RunFrame() {
  // do nothing
}

void PhysicalDeviceProvider::EnterStandby() {}

void PhysicalDeviceProvider::LeaveStandby() {}

bool PhysicalDeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

void PhysicalDeviceProvider::Cleanup() {
  ogserver_->StopProber();
  DriverInternalServer::GetInstance().Stop();
  DriverExternalServer::GetInstance().Stop();
}