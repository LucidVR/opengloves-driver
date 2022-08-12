#include "physical_device_provider.h"

#include "device/configuration/device_configuration.h"
#include "device/drivers/knuckle_device_driver.h"
#include "external_services/driver_internal.h"
#include "util/driver_log.h"
#include "util/file_path.h"

static bool InitialiseExternalServices() {
  // spin up the tracking discovery service before the process so we know we have a server running before the client
  DriverInternalServer::GetInstance();

  const std::string bin_path = GetDriverBinPath();

  DriverLog("Binary path located: %s", bin_path.c_str());

  return CreateBackgroundProcess(bin_path, "opengloves_overlay.exe");
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
  ogserver_ = std::make_unique<og::Server>();

  ogserver_->SetDefaultConfiguration(GetDriverLegacyConfiguration(vr::TrackedControllerRole_LeftHand));

  ogserver_->StartProber([&](std::unique_ptr<og::Device> found_device) {
    DriverLog("Physical device provider found a device, hand: %s", found_device->GetInfo().hand == og::kHandLeft ? "Left" : "Right");

    switch (found_device->GetInfo().device_type) {
      default:
        DriverLog("Physical device provider was given an unknown device type");
      case og::kGloveType_lucidgloves:
        DriverLog("Physical device provider activating lucidgloves");

        std::unique_ptr<KnuckleDeviceDriver> device_driver = std::make_unique<KnuckleDeviceDriver>(std::move(found_device));

        vr::VRServerDriverHost()->TrackedDeviceAdded(
            device_driver->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, device_driver.get());

        device_drivers_.emplace_back(std::move(device_driver));
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
}