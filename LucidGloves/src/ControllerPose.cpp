#pragma once
#include "ControllerPose.h"

ControllerPose::ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer, VRDeviceConfiguration_t configuration, uint32_t driverId) : m_configuration(configuration), m_shadowDeviceOfRole(shadowDeviceOfRole), m_driverId(driverId), m_thisDeviceManufacturer(thisDeviceManufacturer) {

	m_pose.poseIsValid = true;
	m_pose.result = vr::TrackingResult_Running_OK;
	m_pose.deviceIsConnected = true;
	m_pose.qWorldFromDriverRotation.w = 1;
	m_pose.qDriverFromHeadRotation.w = 1;
	//This will be configurable in settings
	m_pose.poseTimeOffset = configuration.poseOffset;
	const vr::HmdVector3_t angleOffset = m_configuration.angleOffsetVector;
	m_offsetQuaternion = EulerToQuaternion(DegToRad(angleOffset.v[0]), DegToRad(angleOffset.v[1]), DegToRad(angleOffset.v[2]));
	DebugDriverLog("Offset calculated! {%.2f, %.2f, %.2f, %.2f}", m_offsetQuaternion.w, m_offsetQuaternion.x, m_offsetQuaternion.y, m_offsetQuaternion.z);
}

vr::DriverPose_t ControllerPose::UpdatePose() {
	vr::DriverPose_t newPose = { 0 };
	if (m_shadowControllerId != -1) {
		vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
		vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, trackedDevicePoses, vr::k_unMaxTrackedDeviceCount);

		if (trackedDevicePoses[m_shadowControllerId].bPoseIsValid)
		{
			newPose.qWorldFromDriverRotation.w = 1;
			newPose.qDriverFromHeadRotation.w = 1;

			const vr::HmdMatrix34_t shadowControllerMatrix = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking;

			////As we need to account for rotation for the offset, multiply 
			const vr::HmdVector3_t vectorOffset = MultiplyMatrix(Get33Matrix(shadowControllerMatrix), m_configuration.offsetVector);
			
			newPose.vecPosition[0] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[0][3] + vectorOffset.v[0];
			newPose.vecPosition[1] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[1][3] + vectorOffset.v[1];
			newPose.vecPosition[2] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[2][3] + vectorOffset.v[2]; //- forward

			//vr::HmdQuaternion_t offset_quaternion = QuaternionFromAngle(1, 0, 0, DegToRad(-45));
			
			//merge rotation
			newPose.qRotation = MultiplyQuaternion(GetRotation(shadowControllerMatrix), m_offsetQuaternion);

			newPose.vecAngularVelocity[0] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[0];
			newPose.vecAngularVelocity[1] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[1];
			newPose.vecAngularVelocity[2] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[2];

			newPose.vecVelocity[0] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[0];
			newPose.vecVelocity[1] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[1];
			newPose.vecVelocity[2] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[2];

			newPose.poseIsValid = true;
			newPose.deviceIsConnected = true;
			newPose.result = vr::TrackingResult_Running_OK;

		} else {
			//DebugDriverLog("pose %d is not valid", m_shadowControllerId);
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

	for (int i = 1; i < vr::k_unMaxTrackedDeviceCount; i++) //omit id 0, as this is always the headset pose
	{
		vr::ETrackedPropertyError err;

		vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(i);

		std::string foundDeviceManufacturer = vr::VRProperties()->GetStringProperty(container, vr::Prop_ManufacturerName_String, &err);
		int32_t deviceControllerRole = vr::VRProperties()->GetInt32Property(container, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32, &err);

		//We have a device which identifies itself as a tracked device that we want to be searching for, and that device is not this one.
		if (deviceControllerRole == m_shadowDeviceOfRole && foundDeviceManufacturer != m_thisDeviceManufacturer) {
			DebugDriverLog("Discovered a controller! Id: %i, Manufacturer: %s", i, foundDeviceManufacturer.c_str());
			m_shadowControllerId = i;
			break;
		}
	}
}