#pragma once

#include <atomic>

#include "ControllerDiscovery.h"
#include "DeviceProvider/DeviceProvider.h"
#include "Hooks/HookReceiver.h"
#include "openvr_driver.h"

// A device provider that hooks into an existing controller to override skeletal input
class HookingDeviceProvider : public DeviceProvider, IHookReceiver {
 public:
  vr::EVRInitError Initialize() override;

  void Cleanup() override;

  const char* const* GetInterfaceVersions() override;

  void ProcessEvent(const vr::VREvent_t& vrEvent) override;

  bool ShouldBlockStandbyMode() override;

  void EnterStandby() override;

  void LeaveStandby() override;

 private:
  vr::IVRDriverContext* thisDriverContext_;

  std::unique_ptr<ControllerDiscovery> leftControllerDiscoverer_;
  std::unique_ptr<ControllerDiscovery> rightControllerDiscoverer_;

  std::atomic<int> leftHookingControllerId_;
  std::atomic<int> rightHookingControllerId_;
};