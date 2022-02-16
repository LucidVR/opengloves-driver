#pragma once

#include "openvr_driver.h"
#include "Hooks/HookReceiver.h"

// A device provider that hooks into an existing controller to override skeletal input
class HookingDeviceProvider : public vr::IServerTrackedDeviceProvider, IHookReceiver {
 public:
  vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;

  void Cleanup() override;

  const char* const* GetInterfaceVersions() override;

  void RunFrame() override;

  bool ShouldBlockStandbyMode() override;

  void EnterStandby() override;

  void LeaveStandby() override;

private:
  vr::IVRDriverContext* thisDriverContext_;
};