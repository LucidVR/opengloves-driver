#pragma once

#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/EncodingManager.h"
#include "ForceFeedback.h"
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
      std::unique_ptr<CommunicationManager> communicationManager,
      std::shared_ptr<BoneAnimator> boneAnimator,
      std::string serialNumber,
      bool approximateThumb,
      VRDeviceConfiguration configuration);

  void HandleInput(VRInputData data) override;
  void SetupProps(vr::PropertyContainerHandle_t& props) override;
  void StartingDevice() override;
  void StoppingDevice() override;

 private:
  vr::VRInputComponentHandle_t inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::_Count)];
  vr::VRInputComponentHandle_t haptic_;
  std::unique_ptr<FFBListener> ffbProvider_;
  bool approximateThumb_;
};
