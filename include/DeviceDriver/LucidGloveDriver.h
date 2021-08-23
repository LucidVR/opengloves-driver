#pragma once
#include "openvr_driver.h"

#include <functional>
#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "ControllerPose.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/LegacyEncodingManager.h"

class LucidGloveDeviceDriver : public IDeviceDriver {
public:
	LucidGloveDeviceDriver(VRDeviceConfiguration_t configuration, std::unique_ptr<CommunicationManager> communicationManager, std::string serialNumber);

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
  vr::VRInputComponentHandle_t m_inputComponentHandles[15]{};

  vr::VRBoneTransform_t m_handTransforms[NUM_BONES];

	VRDeviceConfiguration_t m_configuration;
	std::unique_ptr<CommunicationManager> m_communicationManager;
	std::string m_serialNumber;

  std::unique_ptr<ControllerPose> m_controllerPose;
  std::shared_ptr<BoneAnimator> m_boneAnimator;
};
