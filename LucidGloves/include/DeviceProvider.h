#pragma once
#include <openvr_driver.h>
#include <windows.h>
#include <memory>
#include "DeviceConfiguration.h"
#include "DriverLog.h"

#include "Communication/CommunicationManager.h"
#include "Communication/SerialCommunicationManager.h"

#include "Encode/EncodingManager.h"
#include "Encode/LegacyEncodingManager.h"

#include "DeviceDriver/DeviceDriver.h"
#include "DeviceDriver/LucidGloveDriver.h"
#include "DeviceDriver/KnuckleDriver.h"

/**
This class instantiates all the device drivers you have, meaning if you've
created multiple drivers for multiple different controllers, this class will
create instances of each of those and inform OpenVR about all of your devices.

Take a look at the comment blocks for all the methods in IServerTrackedDeviceProvider
too.
**/
class DeviceProvider : public vr::IServerTrackedDeviceProvider {
public:

	/**
	Initiailze and add your drivers to OpenVR here.
	**/
	vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);

	/**
	Called right before your driver is unloaded.
	**/
	void Cleanup();

	/**
	Returns version of the openVR interface this driver works with.
	**/
	const char* const* GetInterfaceVersions();

	/**
	Called every frame. Update your drivers here.
	**/
	void RunFrame();

	/**
	Return true if standby mode should be blocked. False otherwise.
	**/
	bool ShouldBlockStandbyMode();

	/**
	Called when OpenVR goes into stand-by mode, so you can tell your devices to go into stand-by mode
	**/
	void EnterStandby();

	/**
	Called when OpenVR leaves stand-by mode.
	**/
	void LeaveStandby();

private:
	std::unique_ptr<IDeviceDriver> m_leftHand;
	std::unique_ptr<IDeviceDriver> m_rightHand;
	/**
	* returns the configuration set in VRSettings for the device role given
	**/
	std::unique_ptr<VRDeviceConfiguration_t> GetConfiguration(vr::ETrackedControllerRole role);

	std::unique_ptr<IDeviceDriver> InstantiateDeviceDriver(std::unique_ptr<VRDeviceConfiguration_t> configuration);
};