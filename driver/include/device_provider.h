#pragma once

#include "openvr_driver.h"
#include <map>

class DeviceProvider : public vr::IServerTrackedDeviceProvider {
 public:
  vr::EVRInitError Init(vr::IVRDriverContext* context) override;

  void RunFrame() override;

  void EnterStandby() override;

  void LeaveStandby() override;

  void Cleanup() override;

  bool ShouldBlockStandbyMode() override;
  const char* const* GetInterfaceVersions() override;

 private:
  std::map<std::string, >
};