#pragma once
#include "openvr_driver.h"

#include <memory>

#include "DeviceConfiguration.h"

enum class CalibrationMethod { HARDWARE, UI, NONE };

class Calibration {
 public:
  Calibration();

  void StartCalibration(vr::DriverPose_t maintainPose, CalibrationMethod method);

  VRPoseConfiguration CompleteCalibration(vr::TrackedDevicePose_t controllerPose, VRPoseConfiguration poseConfiguration, bool isRightHand, CalibrationMethod method);

  void CancelCalibration(CalibrationMethod method);

  bool isCalibrating();

  vr::DriverPose_t GetMaintainPose();

 private:
  vr::DriverPose_t m_maintainPose;
  bool m_isCalibrating;
  CalibrationMethod m_calibratingMethod;
};