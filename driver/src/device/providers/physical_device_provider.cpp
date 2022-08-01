#include "physical_device_provider.h"

#include "configuration/device_configuration.h"
#include "driver_log.h"

vr::EVRInitError PhysicalDeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
  VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

  if (!InitDriverLog(vr::VRDriverLog())) {
    DriverLog("Failed to initialise driver logs!");

    return vr::VRInitError_Driver_NotLoaded;
  }

  DebugDriverLog("OpenGloves is running in DEBUG MODE");

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

    DriverLog("server %s: %s", str_level.c_str(), message.c_str());
  });

  // initialise opengloves
  ogserver_ = std::make_unique<og::Server>();

  ogserver_->SetDefaultConfiguration(GetDriverLegacyConfiguration(vr::TrackedControllerRole_LeftHand));

  ogserver_->StartProber([&](std::unique_ptr<og::Device> found_device) {
    DriverLog("Physical device provider found a device, hand: %s", found_device->GetInfo().hand == og::kHandLeft ? "Left" : "Right");
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

void PhysicalDeviceProvider::Cleanup() {}