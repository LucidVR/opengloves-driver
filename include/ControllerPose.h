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

  [[nodiscard]] vr::DriverPose_t UpdatePose() const;

  void StartCalibration(CalibrationMethod method) const;

  void CompleteCalibration(CalibrationMethod method);

  void CancelCalibration(CalibrationMethod method) const;

  [[nodiscard]] bool IsCalibrating() const;

 private:
  uint32_t _shadowControllerId = vr::k_unTrackedDeviceIndexInvalid;

  VRPoseConfiguration _poseConfiguration;

  vr::ETrackedControllerRole _shadowDeviceOfRole = vr::TrackedControllerRole_Invalid;

  std::string _thisDeviceManufacturer;

  [[nodiscard]] vr::TrackedDevicePose_t GetControllerPose() const;

  [[nodiscard]] bool IsRightHand() const;

  std::unique_ptr<ControllerDiscovery> _controllerDiscoverer;
  std::unique_ptr<NamedPipeListener<CalibrationDataIn>> _calibrationPipe;
  std::unique_ptr<Calibration> _calibration;
};