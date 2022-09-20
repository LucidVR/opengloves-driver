#pragma once

#include <atomic>
#include <string>
#include <thread>

#include "device/pose/device_pose.h"
#include "opengloves_interface.h"
#include "openvr_driver.h"

enum KnuckleDeviceComponentIndex {
  kKnuckleDeviceComponentIndex_SystemClick = 0,
  kKnuckleDeviceComponentIndex_SystemTouch,
  kKnuckleDeviceComponentIndex_TriggerClick,
  kKnuckleDeviceComponentIndex_TriggerValue,
  kKnuckleDeviceComponentIndex_TrackpadX,
  kKnuckleDeviceComponentIndex_TrackpadY,
  kKnuckleDeviceComponentIndex_TrackpadTouch,
  kKnuckleDeviceComponentIndex_TrackpadForce,
  kKnuckleDeviceComponentIndex_GripTouch,
  kKnuckleDeviceComponentIndex_GripForce,
  kKnuckleDeviceComponentIndex_GripValue,
  kKnuckleDeviceComponentIndex_ThumbstickClick,
  kKnuckleDeviceComponentIndex_ThumbstickTouch,
  kKnuckleDeviceComponentIndex_ThumbstickX,
  kKnuckleDeviceComponentIndex_ThumbstickY,
  kKnuckleDeviceComponentIndex_AClick,
  kKnuckleDeviceComponentIndex_ATouch,
  kKnuckleDeviceComponentIndex_BClick,
  kKnuckleDeviceComponentIndex_BTouch,
  kKnuckleDeviceComponentIndex_FingerIndex,
  kKnuckleDeviceComponentIndex_FingerMiddle,
  kKnuckleDeviceComponentIndex_FingerRing,
  kKnuckleDeviceComponentIndex_FingerPinky,
  kKnuckleDeviceComponentIndex_Count
};

class KnuckleDeviceDriver : public vr::ITrackedDeviceServerDriver {
 public:
  KnuckleDeviceDriver(std::unique_ptr<og::IDevice> device);

  vr::EVRInitError Activate(uint32_t unObjectId) override;

  void Deactivate() override;

  void EnterStandby() override;

  void *GetComponent(const char *pchComponentNameAndVersion) override;

  void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override;

  vr::DriverPose_t GetPose() override;

  std::string GetSerialNumber();

  ~KnuckleDeviceDriver();

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  std::atomic<uint32_t> device_id_;

  [[nodiscard]] bool IsRightHand() const;
  [[nodiscard]] vr::ETrackedControllerRole GetRole() const;

  void PoseThread();

  std::unique_ptr<og::IDevice> ogdevice_;

  vr::VRBoneTransform_t skeleton_[31]{};
  std::array<vr::VRInputComponentHandle_t, kKnuckleDeviceComponentIndex_Count> input_components_{};
  vr::VRInputComponentHandle_t skeleton_handle_{};

  std::unique_ptr<DevicePose> pose_;

  std::atomic<bool> is_active_;
  std::thread pose_thread_;
};