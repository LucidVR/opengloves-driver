#include <memory>

#include "Bones.h"
#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "openvr_driver.h"

class DeviceProvider : public vr::IServerTrackedDeviceProvider {
 public:
  /**
  Initialize and add your drivers to OpenVR here.
  **/
  vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);

  /**
  Called right before your driver is unloaded.
  **/
  virtual void Cleanup() override = 0;

  /**
  Returns version of the openVR interface this driver works with.
  **/
  virtual const char* const* GetInterfaceVersions() override = 0;

  /**
  Called every frame. Update your drivers here.
  **/
  virtual void RunFrame() override = 0;

  /**
  Return true if standby mode should be blocked. False otherwise.
  **/
  virtual bool ShouldBlockStandbyMode() override = 0;

  /**
  Called when OpenVR goes into stand-by mode, so you can tell your devices to go into stand-by mode
  **/
  virtual void EnterStandby() override = 0;

  /**
  Called when OpenVR leaves stand-by mode.
  **/
  virtual void LeaveStandby() override = 0;

 protected:
  virtual vr::EVRInitError Initialize() = 0;
  std::string GetDriverPath() const;
  std::unique_ptr<BoneAnimator> GetBoneAnimator(const VRDeviceConfiguration& configuration) const;
  std::unique_ptr<CommunicationManager> GetCommunicationManager(const VRDeviceConfiguration& configuration) const;

 private:
  std::string driverPath_;
};