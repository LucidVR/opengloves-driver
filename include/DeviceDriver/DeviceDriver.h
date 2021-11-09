#pragma once

#include <memory>
#include <string>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "ControllerPose.h"
#include "DeviceConfiguration.h"
#include "openvr_driver.h"

class DeviceDriver : public vr::ITrackedDeviceServerDriver {
 public:
  DeviceDriver(
      std::unique_ptr<CommunicationManager> communicationManager,
      std::shared_ptr<BoneAnimator> boneAnimator,
      std::string serialNumber,
      VRDeviceConfiguration configuration);

  vr::EVRInitError Activate(uint32_t unObjectId) override;
  void Deactivate() override;
  void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
  void EnterStandby() override;
  void* GetComponent(const char* pchComponentNameAndVersion) override;
  vr::DriverPose_t GetPose() override;
  virtual std::string GetSerialNumber();
  virtual bool IsActive();
  virtual void RunFrame();

 protected:
  [[nodiscard]] virtual bool IsRightHand() const;
  virtual void StartDevice();

  virtual void HandleInput(VRInputData data) = 0;
  virtual void SetupProps(vr::PropertyContainerHandle_t& props) = 0;
  virtual void StartingDevice() = 0;
  virtual void StoppingDevice() = 0;

  std::unique_ptr<CommunicationManager> _communicationManager;
  std::shared_ptr<BoneAnimator> _boneAnimator;
  VRDeviceConfiguration _configuration;
  std::string _serialNumber;

  std::unique_ptr<ControllerPose> _controllerPose;
  vr::VRInputComponentHandle_t _skeletalComponentHandle;
  vr::VRBoneTransform_t _handTransforms[NUM_BONES];

  bool _hasActivated;
  uint32_t _driverId;
};