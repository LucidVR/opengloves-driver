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
  virtual bool IsRightHand() const;
  virtual void StartDevice();

  virtual void HandleInput(VRInputData data) = 0;
  virtual void SetupProps(vr::PropertyContainerHandle_t& props) = 0;
  virtual void StartingDevice() = 0;
  virtual void StoppingDevice() = 0;
  void PoseUpdateThread();

  std::unique_ptr<CommunicationManager> communicationManager_;
  std::shared_ptr<BoneAnimator> boneAnimator_;
  VRDeviceConfiguration configuration_;
  std::string serialNumber_;

  std::unique_ptr<ControllerPose> controllerPose_;
  vr::VRInputComponentHandle_t skeletalComponentHandle_;
  vr::VRBoneTransform_t handTransforms_[NUM_BONES];

  std::thread poseUpdateThread_;

  std::atomic<bool> hasActivated_;
  uint32_t driverId_;
};