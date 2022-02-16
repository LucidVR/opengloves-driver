#include "DeviceProvider/HookingDeviceProvider.h"

#include "DeviceConfiguration.h"
#include "Hooks/InterfaceHookInjector.h"

vr::EVRInitError HookingDeviceProvider::Initialize() {
  InjectHooks(this, thisDriverContext_);

  // The current "active" controllers will be listened to. This uses the same method we get the controller to track from for physical devices.
  leftControllerDiscoverer_ = std::make_unique<ControllerDiscovery>(
      vr::ETrackedControllerRole::TrackedControllerRole_LeftHand,
      [&](ControllerDiscoveryPipeData data) { leftHookingControllerId_ = data.controllerId; });

  rightControllerDiscoverer_ = std::make_unique<ControllerDiscovery>(
      vr::ETrackedControllerRole::TrackedControllerRole_RightHand,
      [&](ControllerDiscoveryPipeData data) { rightHookingControllerId_ = data.controllerId; });

  VRDeviceConfiguration leftConfiguration = GetDeviceConfiguration(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
  VRDeviceConfiguration rightConfiguration = GetDeviceConfiguration(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

  return vr::EVRInitError::VRInitError_None;
};

void HookingDeviceProvider::ProcessEvent(const vr::VREvent_t& vrEvent) {}

void HookingDeviceProvider::Cleanup() {}

const char* const* HookingDeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

bool HookingDeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

void HookingDeviceProvider::EnterStandby() {}

void HookingDeviceProvider::LeaveStandby() {}
