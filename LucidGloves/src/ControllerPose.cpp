#pragma once
#include "ControllerPose.h"

ControllerPose::ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer, VRDeviceConfiguration_t configuration, uint32_t driverId) : m_configuration(configuration), m_shadowDeviceOfRole(shadowDeviceOfRole), m_driverId(driverId), m_thisDeviceManufacturer(thisDeviceManufacturer) {

	m_pose.deviceIsConnected = m_shadowControllerId != -1;
	m_pose.poseIsValid = true;
	m_pose.result = vr::TrackingResult_Running_OK;
	m_pose.deviceIsConnected = true;
	m_pose.qWorldFromDriverRotation.w = 1;
	m_pose.qWorldFromDriverRotation.x = 0;
	m_pose.qWorldFromDriverRotation.y = 0;
	m_pose.qWorldFromDriverRotation.z = 0;
	m_pose.qDriverFromHeadRotation.w = 1;
	m_pose.qDriverFromHeadRotation.x = 0;
	m_pose.qDriverFromHeadRotation.y = 0;
	m_pose.qDriverFromHeadRotation.z = 0;
	//This will be configurable in settings
	m_pose.poseTimeOffset = 0.05f;
}

vr::DriverPose_t ControllerPose::UpdatePose() {
	if (m_shadowControllerId != -1) {
		vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
		vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, trackedDevicePoses, vr::k_unMaxTrackedDeviceCount);

		if (trackedDevicePoses[m_shadowControllerId].bPoseIsValid)
		{
			m_pose.poseIsValid = true;
			m_pose.deviceIsConnected = true;

			//const vr::HmdMatrix34_t shadowControllerMatrix = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking;

			////As we need to account for rotation for the offset, multiply 
			//const vr::HmdVector3_t vectorOffset = MultiplyMatrix(Get33Matrix(shadowControllerMatrix), m_configuration.offsetVector);

			m_pose.vecPosition[0] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[0][3];
			m_pose.vecPosition[1] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[1][3];
			m_pose.vecPosition[2] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[2][3];

			vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_driverId, m_pose, sizeof(vr::DriverPose_t));

			//m_pose.vecPosition[0] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[0][3] + vectorOffset.v[0];
			//m_pose.vecPosition[1] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[1][3] + vectorOffset.v[1];
			//m_pose.vecPosition[2] = trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking.m[2][3] + vectorOffset.v[2]; //- forward

			////We want to produce a quaternion rotation of the controllers that accounts for any rotation the controller has on the hand.
			//m_pose.qRotation = MultiplyQuaternion(GetRotation(shadowControllerMatrix), QuaternionFromAngle(1, 0, 0, DegToRad(-90)));

			//m_pose.result = vr::TrackingResult_Running_OK;

			//m_pose.vecAngularVelocity[0] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[0];
			//m_pose.vecAngularVelocity[1] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[1];
			//m_pose.vecAngularVelocity[2] = trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[2];

			//m_pose.vecVelocity[0] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[0];
			//m_pose.vecVelocity[1] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[1];
			//m_pose.vecVelocity[2] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[2];

			//set the pose

		}
		else {
			DebugDriverLog("pose is not valid");
		}
	} else {
		DebugDriverLog("Discovering controller");
		DiscoverController();
	}

	return m_pose;
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
	if (m_shadowControllerId == -1) {
		DebugDriverLog("did not find a controller");
	}
	//We didn't find a controller
}