#pragma once

#include "openvr_driver.h"

#include <functional>
#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "ControllerPose.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/EncodingManager.h"

enum class LucidGloveDeviceComponentIndex : int {
  COMP_JOY_X = 0,
  COMP_JOY_Y,
  COMP_JOY_BTN,
  COMP_BTN_TRG,
  COMP_BTN_A,
  COMP_BTN_B,
  COMP_GES_GRAB,
  COMP_GES_PINCH,
  COMP_HAPTIC,
  COMP_TRG_THUMB,
  COMP_TRG_INDEX,
  COMP_TRG_MIDDLE,
  COMP_TRG_RING,
  COMP_TRG_PINKY,
  COMP_BTN_MENU,
  Count
};

class LucidGloveDeviceDriver : public DeviceDriver {
 public:
  LucidGloveDeviceDriver(std::unique_ptr<CommunicationManager> communicationManager, std::shared_ptr<BoneAnimator> boneAnimator, std::string serialNumber,
                         VRDeviceConfiguration configuration);

  void HandleInput(VRInputData datas);
  void SetupProps(vr::PropertyContainerHandle_t& props);
  void StartingDevice();
  void StoppingDevice();

 private:
  vr::VRInputComponentHandle_t m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::Count];
};
