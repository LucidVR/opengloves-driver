#include "DeviceProvider/HookingDeviceProvider.h"
#include "Hooks/InterfaceHookInjector.h"
#include "DeviceConfiguration.h"

vr::EVRInitError HookingDeviceProvider::Init(vr::IVRDriverContext* pDriverContext){
  InjectHooks(this, thisDriverContext_);

  VRDeviceConfiguration leftConfiguration = GetDeviceConfiguration(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
  VRDeviceConfiguration rightConfiguration = GetDeviceConfiguration(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

  return vr::EVRInitError::VRInitError_None;
};

void HookingDeviceProvider::RunFrame() {
	//vrevent
}

void HookingDeviceProvider::Cleanup() {

}

const char* const* HookingDeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

bool HookingDeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

void HookingDeviceProvider::EnterStandby() {}

void HookingDeviceProvider::LeaveStandby() {}
