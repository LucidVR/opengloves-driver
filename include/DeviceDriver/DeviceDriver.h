#pragma once

#include "openvr_driver.h"

#include <memory>
#include <string>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "ControllerPose.h"
#include "DeviceConfiguration.h"

class DeviceDriver : public vr::ITrackedDeviceServerDriver {
 public:
  DeviceDriver(std::unique_ptr<CommunicationManager> communicationManager, std::shared_ptr<BoneAnimator> boneAnimator, std::string serialNumber,
               VRDeviceConfiguration configuration);

  virtual vr::EVRInitError Activate(uint32_t unObjectId);
  virtual void Deactivate();
  virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
  virtual void EnterStandby();
  virtual void* GetComponent(const char* pchComponentNameAndVersion);
  virtual vr::DriverPose_t GetPose();
  virtual std::string GetSerialNumber();
  virtual bool IsActive();
  virtual void RunFrame();

 protected:
  virtual bool IsRightHand() const;
  virtual void StartDevice();

  virtual void HandleInput(VRInputData datas) = 0;
  virtual void SetupProps(vr::PropertyContainerHandle_t& props) = 0;
  virtual void StartingDevice() = 0;
  virtual void StoppingDevice() = 0;

  std::unique_ptr<CommunicationManager> m_communicationManager;
  std::shared_ptr<BoneAnimator> m_boneAnimator;
  VRDeviceConfiguration m_configuration;
  std::string m_serialNumber;

  std::unique_ptr<ControllerPose> m_controllerPose;
  vr::VRInputComponentHandle_t m_skeletalComponentHandle;
  vr::VRBoneTransform_t m_handTransforms[NUM_BONES];

  bool m_hasActivated;
  uint32_t m_driverId;
};