#include "openvr_driver.h"

#include "configuration/device_configuration.h"

enum class CalibrationMethod {
  Hardware,
  Ui,
  None,
};


class PoseCalibration {
 public:
  void StartCalibration(vr::DriverPose_t maintainPose, CalibrationMethod method);

  VRPoseConfiguration CompleteCalibration(
      vr::TrackedDevicePose_t controllerPose, VRPoseConfiguration poseConfiguration, bool isRightHand, CalibrationMethod method);

  void CancelCalibration(CalibrationMethod method);

  bool IsCalibrating() const;

  vr::DriverPose_t GetMaintainPose() const;

 private:
  vr::DriverPose_t maintain_pose_;
  bool is_calibrating_;
  CalibrationMethod calibration_method_;
};