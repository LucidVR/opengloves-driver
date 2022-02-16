#pragma once

#include <memory>

#include "ControllerPose.h"
#include "DeviceDriver/DeviceDriver.h"
#include "ForceFeedback.h"

// provides additional abstractions on top of a device driver. Physical is a bad name for this class, but means that it acts as a "real" controller in
// openvr, which submits poses, etc.
class PhysicalDeviceDriver : public DeviceDriver {
 public:
  PhysicalDeviceDriver(
      std::unique_ptr<CommunicationManager> communicationManager,
      std::unique_ptr<BoneAnimator> boneAnimator,
      std::string serialNumber,
      VRDeviceConfiguration configuration);
  void StartingDevice() override;
  void StoppingDevice() override;

  vr::DriverPose_t GetPose() override;

  void HandleInput(const VRInputData& data) override;
  virtual void OnInputUpdate(const VRInputData& data) = 0;

 private:
  void PoseUpdateThread();

  std::unique_ptr<ControllerPose> controllerPose_;

  std::thread poseUpdateThread_;
};