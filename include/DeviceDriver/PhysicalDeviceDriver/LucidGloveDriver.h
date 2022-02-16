#pragma once

#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/PhysicalDeviceDriver/PhysicalDeviceDriver.h"
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

class LucidGloveDeviceDriver : public PhysicalDeviceDriver {
 public:
  LucidGloveDeviceDriver(
      std::unique_ptr<CommunicationManager> communicationManager,
      std::unique_ptr<BoneAnimator> boneAnimator,
      const std::string& serialNumber,
      VRDeviceConfiguration configuration);

  void OnInputUpdate(const VRInputData& data) override;
  void SetupProps(vr::PropertyContainerHandle_t& props) override;

 private:
  vr::VRInputComponentHandle_t inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::_Count)];
};
