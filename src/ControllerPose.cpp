#include "ControllerPose.h"

#include <utility>

#include "DriverLog.h"
#include "Quaternion.h"


ControllerPose::ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole,
                               std::string thisDeviceManufacturer,
                               VRPoseConfiguration_t poseConfiguration) :
        m_shadowDeviceOfRole(shadowDeviceOfRole),
        m_thisDeviceManufacturer(std::move(thisDeviceManufacturer)),
        m_poseConfiguration(poseConfiguration) {}

vr::DriverPose_t ControllerPose::UpdatePose() {
    vr::DriverPose_t newPose = {0};
    newPose.qWorldFromDriverRotation.w = 1;
    newPose.qDriverFromHeadRotation.w = 1;

    if (m_shadowControllerId != vr::k_unTrackedDeviceIndexInvalid) {
        vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
        vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, trackedDevicePoses, vr::k_unMaxTrackedDeviceCount);

        if (trackedDevicePoses[m_shadowControllerId].bPoseIsValid) {

            //get the matrix that represents the position of the controller that we are shadowing
            vr::HmdMatrix34_t controllerMatrix = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking;

            //get only the rotation (3x3 matrix), as the 3x4 matrix also includes position
            vr::HmdMatrix33_t controllerRotationMatrix = GetRotationMatrix(controllerMatrix);

            //multiply the rotation matrix by the offset vector set that is the offset of the controller relative to the hand
            vr::HmdVector3_t vectorOffset = MultiplyMatrix(controllerRotationMatrix, m_poseConfiguration.offsetVector);

            //combine these positions to get the resultant position
            vr::HmdVector3_t newControllerPosition = CombinePosition(controllerMatrix, vectorOffset);

            newPose.vecPosition[0] = newControllerPosition.v[0];
            newPose.vecPosition[1] = newControllerPosition.v[1];
            newPose.vecPosition[2] = newControllerPosition.v[2];

            //Multiply rotation quaternions together, as the controller may be rotated relative to the hand
            newPose.qRotation = MultiplyQuaternion(GetRotation(controllerMatrix), m_poseConfiguration.angleOffsetQuaternion);

            //Copy other values from the controller that we want for this device
            newPose.vecAngularVelocity[0] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[0];
            newPose.vecAngularVelocity[1] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[1];
            newPose.vecAngularVelocity[2] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[2];

            newPose.vecVelocity[0] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[0];
            newPose.vecVelocity[1] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[1];
            newPose.vecVelocity[2] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[2];

            newPose.poseIsValid = true;
            newPose.deviceIsConnected = true;

            newPose.result = vr::TrackingResult_Running_OK;

            newPose.poseTimeOffset = m_poseConfiguration.poseOffset;
        } else {
            newPose.poseIsValid = false;
            newPose.deviceIsConnected = true;
            newPose.result = vr::TrackingResult_Uninitialized;
        }

    } else {
        newPose.result = vr::TrackingResult_Uninitialized;
        newPose.deviceIsConnected = false;
        DiscoverController();
    }

    return newPose;
}

void ControllerPose::DiscoverController() {
    //omit id 0, as this is always the headset pose
    if (!m_poseConfiguration.controllerOverrideEnabled) {
        for (int i = 1; i < vr::k_unMaxTrackedDeviceCount; i++) {
            vr::ETrackedPropertyError err;

            vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(i);

            std::string foundDeviceManufacturer = vr::VRProperties()->GetStringProperty(container,
                                                                                        vr::Prop_ManufacturerName_String,
                                                                                        &err);
            int32_t deviceControllerRole = vr::VRProperties()->GetInt32Property(container,
                                                                                vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32,
                                                                                &err);

            //We have a device which identifies itself as a tracked device that we want to be searching for, and that device is not this one.
            if (deviceControllerRole == m_shadowDeviceOfRole && foundDeviceManufacturer != m_thisDeviceManufacturer) {
                DebugDriverLog("Discovered a controller! Id: %i, Manufacturer: %s", i, foundDeviceManufacturer.c_str());
                m_shadowControllerId = i;
                break;
            }
        }
    } else {
        DebugDriverLog("Controller ID override set to id: %i", m_poseConfiguration.controllerIdOverride);
        m_shadowControllerId = m_poseConfiguration.controllerIdOverride;
    }
}