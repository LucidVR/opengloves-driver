#pragma once

#include "DeviceConfiguration.h"
#include "openvr_driver.h"

enum class CalibrationMethod {
  Hardware,
  Ui,
  None,
};

class Calibration {
 public:
  Calibration();

  void StartCalibration(vr::DriverPose_t maintainPose, CalibrationMethod method);

  VRPoseConfiguration CompleteCalibration(
      vr::TrackedDevicePose_t controllerPose, VRPoseConfiguration poseConfiguration, bool isRightHand, CalibrationMethod method);

  void CancelCalibration(CalibrationMethod method);

  bool IsCalibrating() const;

  vr::DriverPose_t GetMaintainPose() const;

 private:
  vr::DriverPose_t maintainPose_;
  bool isCalibrating_;
  CalibrationMethod calibratingMethod_;
};