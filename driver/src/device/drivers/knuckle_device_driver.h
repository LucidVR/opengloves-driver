#pragma once

#include "openvr_driver.h"

class KnuckleDeviceDriver : vr::ITrackedDeviceServerDriver {
  virtual vr::EVRInitError Activate(uint32_t unObjectId) = 0;

  void Deactivate() override;

  void EnterStandby() override;

  void *GetComponent(const char *pchComponentNameAndVersion) override;

  void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override;

  vr::DriverPose_t GetPose() override;
};