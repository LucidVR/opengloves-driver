#pragma once

#include <memory>
#include <string>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "ControllerPose.h"
#include "DeviceConfiguration.h"
#include "ForceFeedback.h"
#include "openvr_driver.h"

class DeviceDriver : public vr::ITrackedDeviceServerDriver {
 public:
  DeviceDriver(
      std::unique_ptr<CommunicationManager> communicationManager, std::unique_ptr<BoneAnimator> boneAnimator, VRDeviceConfiguration configuration);

  vr::EVRInitError Activate(uint32_t unObjectId) override;
  void Deactivate() override;
  void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
  void EnterStandby() override;
  void* GetComponent(const char* pchComponentNameAndVersion) override;
  vr::DriverPose_t GetPose() override;

  virtual std::string GetSerialNumber();
  int32_t GetDeviceId() const;

  virtual bool IsActive();

  void OnEvent(vr::VREvent_t vrEvent) const;

 protected:
  virtual bool IsRightHand() const;
  virtual void StartDevice();

  virtual void HandleInput(VRInputData data) = 0;
  virtual void SetupProps(vr::PropertyContainerHandle_t& props) = 0;
  virtual void StartingDevice() = 0;
  virtual void StoppingDevice() = 0;
  void PoseUpdateThread() const;

  std::unique_ptr<CommunicationManager> communicationManager_;
  std::unique_ptr<BoneAnimator> boneAnimator_;

  VRDeviceConfiguration configuration_;
  std::string serialNumber_;

  std::unique_ptr<ControllerPose> controllerPose_;
  std::unique_ptr<FFBListener> ffbProvider_;

  vr::VRInputComponentHandle_t skeletalComponentHandle_;
  vr::VRInputComponentHandle_t haptic_;

  vr::VRBoneTransform_t handTransforms_[NUM_BONES];

  std::thread poseUpdateThread_;

  std::atomic<bool> hasActivated_;
  uint32_t deviceId_;
};