#pragma once

#include <functional>
#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/EncodingManager.h"
#include "ForceFeedback.h"
#include "openvr_driver.h"

enum class KnuckleDeviceComponentIndex : int {
  SYSTEM_CLICK = 0,
  SYSTEM_TOUCH,
  TRIGGER_CLICK,
  TRIGGER_VALUE,
  TRACKPAD_X,
  TRACKPAD_Y,
  TRACKPAD_TOUCH,
  TRACKPAD_FORCE,
  GRIP_TOUCH,
  GRIP_FORCE,
  GRIP_VALUE,
  THUMBSTICK_CLICK,
  THUMBSTICK_TOUCH,
  THUMBSTICK_X,
  THUMBSTICK_Y,
  A_CLICK,
  A_TOUCH,
  B_CLICK,
  B_TOUCH,
  FINGER_INDEX,
  FINGER_MIDDLE,
  FINGER_RING,
  FINGER_PINKY,
  Count
};

class KnuckleDeviceDriver : public DeviceDriver {
 public:
  KnuckleDeviceDriver(
      std::unique_ptr<CommunicationManager> communicationManager,
      std::shared_ptr<BoneAnimator> boneAnimator,
      std::string serialNumber,
      VRDeviceConfiguration configuration);

  void HandleInput(VRInputData datas) override;
  void SetupProps(vr::PropertyContainerHandle& props) override;
  void StartingDevice() override;
  void StoppingDevice() override;

 private:
  vr::VRInputComponentHandle_t m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::Count];
  vr::VRInputComponentHandle_t m_haptic;
  std::unique_ptr<FFBListener> m_ffbProvider;
};
