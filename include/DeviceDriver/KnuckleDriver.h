#pragma once

#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/EncodingManager.h"
#include "openvr_driver.h"

enum class KnuckleDeviceComponentIndex : int {
  SystemClick = 0,
  SystemTouch,
  TriggerClick,
  TriggerValue,
  TrackpadX,
  TrackpadY,
  TrackpadTouch,
  TrackpadForce,
  GripTouch,
  GripForce,
  GripValue,
  ThumbstickClick,
  ThumbstickTouch,
  ThumbstickX,
  ThumbstickY,
  AClick,
  ATouch,
  BClick,
  BTouch,
  FingerIndex,
  FingerMiddle,
  FingerRing,
  FingerPinky,
  _Count
};

class KnuckleDeviceDriver : public DeviceDriver {
 public:
  KnuckleDeviceDriver(
      std::unique_ptr<CommunicationManager> communicationManager, std::unique_ptr<BoneAnimator> boneAnimator, const VRDeviceConfiguration& configuration);

  void HandleInput(VRInputData data) override;
  void SetupProps(vr::PropertyContainerHandle_t& props) override;
  void StartingDevice() override;
  void StoppingDevice() override;

 private:
  vr::VRInputComponentHandle_t inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::_Count)];

  VRDeviceKnucklesConfiguration knucklesConfiguration_;
};
