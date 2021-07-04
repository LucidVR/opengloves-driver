#pragma once
#include <openvr_driver.h>
#include <memory>
#include "DeviceConfiguration.h"

class Calibration {
public:
    Calibration();

    void StartCalibration(vr::DriverPose_t maintainPose);

    void FinishCalibration(vr::TrackedDevicePose_t controllerPose, VRPoseConfiguration_t poseConfiguration, bool isRightHand);

    void CancelCalibration();

    bool isCalibrating();
    
    vr::DriverPose_t GetMaintainPose();

private:
    vr::DriverPose_t m_maintainPose;
    bool m_isCalibrating = false;
};