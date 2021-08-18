#pragma once
#pragma once
#include <openvr_driver.h>

#include <functional>
#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "ControllerPose.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/LegacyEncodingManager.h"
#include "ForceFeedback.h"

#define KNUCKLE_DEVICE_SETTINGS_SECTION_WITHOUT_PREFIX "device_knuckles"
#define KNUCKLE_DEVICE_SETTINGS_SECTION (OPENGLOVES_SECTION_PREFIX KNUCKLE_DEVICE_SETTINGS_SECTION_WITHOUT_PREFIX)

class KnuckleDeviceDriver : public IDeviceDriver {
 public:
  KnuckleDeviceDriver(VRDeviceConfiguration_t configuration, std::unique_ptr<ICommunicationManager> communicationManager, std::string serialNumber);

  vr::EVRInitError Activate(uint32_t unObjectId);
  void Deactivate();

  void EnterStandby();
  void* GetComponent(const char* pchComponentNameAndVersion);
  void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
  vr::DriverPose_t GetPose();
  void RunFrame();

  std::string GetSerialNumber();
  bool IsActive();

 private:
  void StartDevice();
  bool IsRightHand() const;

  bool m_hasActivated;
  uint32_t m_driverId;

  vr::VRInputComponentHandle_t m_skeletalComponentHandle{};
  vr::VRInputComponentHandle_t m_inputComponentHandles[23]{};

  vr::VRInputComponentHandle_t m_haptic{};

  vr::VRBoneTransform_t m_handTransforms[NUM_BONES];

  VRDeviceConfiguration_t m_configuration;
  std::unique_ptr<ICommunicationManager> m_communicationManager;
  std::string m_serialNumber;

  std::unique_ptr<ControllerPose> m_controllerPose;
  std::unique_ptr<FFBListener> m_ffbProvider;
};
