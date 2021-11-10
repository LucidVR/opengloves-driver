#pragma once

#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/EncodingManager.h"
#include "openvr_driver.h"

enum class LucidGloveDeviceComponentIndex : int {
  JoyX = 0,
  JoyY,
  JoyBtn,
  BtnTrg,
  BtnA,
  BtnB,
  GesGrab,
  GesPinch,
  Haptic,
  TrgThumb,
  TrgIndex,
  TrgMiddle,
  TrgRing,
  TrgPinky,
  BtnMenu,
  _Count
};

class LucidGloveDeviceDriver : public DeviceDriver {
 public:
  LucidGloveDeviceDriver(
      std::unique_ptr<CommunicationManager> communicationManager,
      std::shared_ptr<BoneAnimator> boneAnimator,
      const std::string& serialNumber,
      VRDeviceConfiguration configuration);

  void HandleInput(VRInputData data) override;
  void SetupProps(vr::PropertyContainerHandle_t& props) override;
  void StartingDevice() override;
  void StoppingDevice() override;

 private:
  vr::VRInputComponentHandle_t inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::_Count)];
};
