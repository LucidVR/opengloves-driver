#pragma once
#pragma once
#include <openvr_driver.h>
#include <windows.h>
#include <thread>
#include <functional>
#include "driverlog.h"
#include "bones.h"

#include "Communication/SerialCommunicationManager.h"
#include "Encode/LegacyEncodingManager.h"
#include "DeviceDriver/DeviceDriver.h"

#include "ControllerPose.h"
#include "DeviceConfiguration.h"

class KnuckleDeviceDriver : public IDeviceDriver {
public:
	KnuckleDeviceDriver(std::unique_ptr<VRDeviceConfiguration_t> configuration);

	vr::EVRInitError Activate(uint32_t unObjectId);
	void Deactivate();

	void EnterStandby();
	void* GetComponent(const char* pchComponentNameAndVersion);
	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
	vr::DriverPose_t GetPose();
	void RunFrame();

	std::string GetSerialNumber();
	bool IsActive();
private:
	void StartDevice();
	bool IsRightHand() const;

	bool m_hasActivated;
	uint32_t m_driverId;

	vr::VRInputComponentHandle_t m_skeletalComponentHandle{};
	vr::VRInputComponentHandle_t m_inputComponentHandles[23]{};

	vr::VRInputComponentHandle_t m_haptic{};

	vr::VRBoneTransform_t m_handTransforms[NUM_BONES];

	uint32_t m_shadowControllerId = vr::k_unTrackedDeviceIndexInvalid;

	std::unique_ptr<VRDeviceConfiguration_t> m_configuration;

	std::unique_ptr<ControllerPose> m_controllerPose;
};
