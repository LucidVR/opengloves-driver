#pragma once
#include <openvr_driver.h>

#include <memory>

#include "Calibration.h"
#include "ControllerDiscovery.h"
#include "DeviceConfiguration.h"
#include "Util/NamedPipeListener.h"

struct CalibrationDataIn {
  uint8_t start;
};

class ControllerPose {
 public:
  ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer, VRPoseConfiguration poseConfiguration);
  ~ControllerPose();

  vr::DriverPose_t UpdatePose() const;

  void StartCalibration(CalibrationMethod method) const;

  void CompleteCalibration(CalibrationMethod method);

  void CancelCalibration(CalibrationMethod method) const;

  bool IsCalibrating() const;

 private:
  uint32_t shadowControllerId_ = vr::k_unTrackedDeviceIndexInvalid;

  VRPoseConfiguration poseConfiguration_;

  vr::ETrackedControllerRole shadowDeviceOfRole_ = vr::TrackedControllerRole_Invalid;

  std::string thisDeviceManufacturer_;

  vr::TrackedDevicePose_t GetControllerPose() const;

  bool IsRightHand() const;

  std::unique_ptr<ControllerDiscovery> controllerDiscoverer_;
  std::unique_ptr<NamedPipeListener<CalibrationDataIn>> calibrationPipe_;
  std::unique_ptr<Calibration> calibration_;
};