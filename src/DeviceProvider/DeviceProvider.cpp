#include "DeviceProvider/DeviceProvider.h"

#include "Communication/BTSerialCommunicationManager.h"
#include "Communication/NamedPipeCommunicationManager.h"
#include "Communication/SerialCommunicationManager.h"
#include "Encode/AlphaEncodingManager.h"
#include "Encode/LegacyEncodingManager.h"
#include "Util/Windows.h"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "?"
#endif

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
  const vr::EVRInitError initError = InitServerDriverContext(pDriverContext);
  if (initError != vr::EVRInitError::VRInitError_None) return initError;

  VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext)
  InitDriverLog(vr::VRDriverLog());

  const std::string path = GetCurrentDirectoryDLL();
  DriverLog("Path to DLL: %s", path.c_str());

  const std::string unwanted = R"(\bin\win64\)";
  driverPath_ = path.substr(0, path.find_last_of("\\/")).erase(path.find(unwanted), unwanted.length());
  DriverLog("Root Driver Path: %s", driverPath_.c_str());

  const std::string commitHash = GIT_COMMIT_HASH;
  DriverLog("Built from: %s", commitHash.substr(0, 10).c_str());

  // Create background process for the overlay (used for finding controllers to bind to for tracking)
  if (!CreateBackgroundProcess(GetDriverPath() + R"(\bin\win64\openglove_overlay.exe)")) {
    DriverLog("Could not create background process: %c", GetLastErrorAsString().c_str());

    return vr::VRInitError_Init_FileNotFound;
  }

  // child initialization
  vr::EVRInitError err = Initialize();

  isInitialized_ = true;

  return err;
}

void DeviceProvider::RunFrame() {
  if (!isInitialized_) return;

  vr::VREvent_t vrEvent;
  while (vr::VRServerDriverHost()->PollNextEvent(&vrEvent, sizeof(vrEvent))) {
    DebugDriverLog("Event: %i", vrEvent.eventType);
    ProcessEvent(vrEvent);
  }
}

std::string DeviceProvider::GetDriverPath() const {
  return driverPath_;
}

std::unique_ptr<BoneAnimator> DeviceProvider::GetBoneAnimator(const VRDeviceConfiguration& deviceConfiguration) const {
  return std::make_unique<BoneAnimator>(GetDriverPath() + R"(\resources\anims\glove_anim.glb)");
}

std::unique_ptr<CommunicationManager> DeviceProvider::GetCommunicationManager(const VRDeviceConfiguration& configuration) const {
  std::unique_ptr<EncodingManager> encodingManager;

  const bool isRightHand = configuration.role == vr::TrackedControllerRole_RightHand;

  switch (configuration.encodingProtocol) {
    default:
      DriverLog("No encoding protocol set. Using legacy.");
    case VREncodingProtocol::Legacy: {
      const float maxAnalogValue = vr::VRSettings()->GetFloat(c_legacyEncodingSettingsSection, "max_analog_value");

      encodingManager = std::make_unique<LegacyEncodingManager>(maxAnalogValue);
      break;
    }
    case VREncodingProtocol::Alpha: {
      const float maxAnalogValue = vr::VRSettings()->GetFloat(c_alphaEncodingSettingsSection, "max_analog_value");

      encodingManager = std::make_unique<AlphaEncodingManager>(maxAnalogValue);
      break;
    }
  }

  switch (configuration.communicationProtocol) {
    case VRCommunicationProtocol::NamedPipe: {
      DriverLog("Communication set to Named Pipe");

      const std::string path = R"(\\.\pipe\vrapplication\input\glove\v1\)" + std::string(isRightHand ? "right" : "left");

      VRNamedPipeInputConfiguration namedPipeConfiguration(path);
      return std::make_unique<NamedPipeCommunicationManager>(namedPipeConfiguration, configuration);
    }

    case VRCommunicationProtocol::BtSerial: {
      DriverLog("Communication set to BTSerial");

      char name[248];
      vr::VRSettings()->GetString(c_btserialCommunicationSettingsSection, isRightHand ? "right_name" : "left_name", name, sizeof name);

      VRBTSerialConfiguration btSerialSettings(name);
      return std::make_unique<BTSerialCommunicationManager>(std::move(encodingManager), btSerialSettings, configuration);
    }

    default:
      DriverLog("No communication protocol set. Using USB Serial...");
    case VRCommunicationProtocol::Serial:
      DriverLog("Communication set to USB Serial");

      char port[16];
      vr::VRSettings()->GetString(c_serialCommunicationSettingsSection, isRightHand ? "right_port" : "left_port", port, sizeof port);
      const int baudRate = vr::VRSettings()->GetInt32(c_serialCommunicationSettingsSection, "baud_rate");

      VRSerialConfiguration serialSettings(port, baudRate);
      return std::make_unique<SerialCommunicationManager>(std::move(encodingManager), serialSettings, configuration);
  }
}