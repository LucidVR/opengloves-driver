#pragma once

#include <string>

#include "openvr_driver.h"
#include "opengloves_interface.h"

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
  KnuckleDeviceDriver(std::unique_ptr<og::Device> device);

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

  void HandleInput(const og::InputPeripheralData& data);

  [[nodiscard]] bool IsRightHand() const;

  std::unique_ptr<og::Device> ogdevice_;

  std::array<vr::VRInputComponentHandle_t, kKnuckleDeviceComponentIndex_Count> input_components_{};
};