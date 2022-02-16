#include "DeviceProvider/PhysicalDeviceProvider.h"

#include <algorithm>
#include <string>

#include "DeviceDriver/KnuckleDriver.h"
#include "DeviceDriver/LucidGloveDriver.h"
#include "DriverLog.h"

vr::EVRInitError PhysicalDeviceProvider::Initialize() {
  const VRDeviceConfiguration leftConfiguration = GetDeviceConfiguration(vr::TrackedControllerRole_LeftHand);
  const VRDeviceConfiguration rightConfiguration = GetDeviceConfiguration(vr::TrackedControllerRole_RightHand);

  const auto boneAnimator = std::make_shared<BoneAnimator>(GetDriverPath() + R"(\resources\anims\glove_anim.glb)");

  if (leftConfiguration.enabled) {
    leftHand_ = InstantiateDeviceDriver(leftConfiguration);
    vr::VRServerDriverHost()->TrackedDeviceAdded(leftHand_->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, leftHand_.get());
  }

  if (rightConfiguration.enabled) {
    rightHand_ = InstantiateDeviceDriver(rightConfiguration);
    vr::VRServerDriverHost()->TrackedDeviceAdded(rightHand_->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, rightHand_.get());
  }

  return vr::VRInitError_None;
}

std::unique_ptr<DeviceDriver> PhysicalDeviceProvider::InstantiateDeviceDriver(const VRDeviceConfiguration& configuration) const {
  const bool isRightHand = configuration.role == vr::TrackedControllerRole_RightHand;

  std::unique_ptr<BoneAnimator> boneAnimator = GetBoneAnimator(configuration);
  std::unique_ptr<CommunicationManager> communicationManager = GetCommunicationManager(configuration);

  switch (configuration.deviceDriver) {
    case VRDeviceDriver::EmulatedKnuckles: {
      DriverLog("Using Emulated Knuckles device");

      char serialNumber[32];
      vr::VRSettings()->GetString(
          c_knuckleDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof serialNumber);

      bool approximateThumb = vr::VRSettings()->GetBool(c_knuckleDeviceSettingsSection, "approximate_thumb");

      return std::make_unique<KnuckleDeviceDriver>(
          std::move(communicationManager), std::move(boneAnimator), serialNumber, approximateThumb, configuration);
    }

    default:
      DriverLog("No device driver set. Using lucidgloves...");
    case VRDeviceDriver::LucidGloves: {
      DriverLog("Using lucidgloves device");
      char serialNumber[32];
      vr::VRSettings()->GetString(
          c_lucidGloveDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof serialNumber);

      return std::make_unique<LucidGloveDeviceDriver>(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration);
    }
  }
}

void PhysicalDeviceProvider::Cleanup() {}

const char* const* PhysicalDeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

void PhysicalDeviceProvider::RunFrame() {
  if (leftHand_ && leftHand_->IsActive()) leftHand_->RunFrame();
  if (rightHand_ && rightHand_->IsActive()) rightHand_->RunFrame();
}

bool PhysicalDeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

void PhysicalDeviceProvider::EnterStandby() {}

void PhysicalDeviceProvider::LeaveStandby() {}
