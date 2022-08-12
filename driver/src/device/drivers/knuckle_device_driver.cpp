#include "knuckle_device_driver.h"

#include "hand_tracking/hand_tracking.h"
#include "util/file_path.h"

class KnuckleDeviceDriver::Impl {
 public:
  explicit Impl(const std::string &anim_file_path) : hand_tracking_(std::make_unique<HandTracking>(anim_file_path)) {}

  void SetDefaultSkeleton(vr::VRBoneTransform_t *bone_transforms, vr::ETrackedControllerRole role) {
    hand_tracking_->LoadDefaultSkeletonByHand(bone_transforms, role);
  }

  void SetSkeleton(vr::VRBoneTransform_t *bone_transforms, const og::InputPeripheralData &data, vr::ETrackedControllerRole role) {
    hand_tracking_->ComputeBoneTransforms(bone_transforms, data, role);
  }

  float GetAverageFingerCurlValue(const std::array<float, 4> &joints) {
    return hand_tracking_->GetAverageFingerCurlValue(joints);
  }

 private:
  std::unique_ptr<HandTracking> hand_tracking_;
};

KnuckleDeviceDriver::KnuckleDeviceDriver(std::unique_ptr<og::Device> device)
    : pImpl_(std::make_unique<Impl>(GetDriverRootPath() + "\\resources\\anims\\glove_anim.glb")),
      ogdevice_(std::move(device)),
      pose_(std::make_unique<DevicePose>(GetRole())) {}

vr::EVRInitError KnuckleDeviceDriver::Activate(uint32_t unObjectId) {
  device_id_ = unObjectId;

  // clang-format off
  const vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);

  vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, IsRightHand() ? "LHR-E217CD01" : "LHR-E217CD00");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, "LucidVR");

  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, 2000000000);
  vr::VRProperties()->SetFloatProperty(props, vr::Prop_DeviceBatteryPercentage_Float, 1.f);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);

  vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Identifiable_Bool, true);
  vr::VRProperties()->SetInt32Property(
      props, vr::Prop_ControllerRoleHint_Int32, IsRightHand() ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, "knuckles");

  vr::VRDriverInput()->CreateSkeletonComponent(
      props,
      IsRightHand() ? "/input/skeleton/right" : "/input/skeleton/left",
      IsRightHand() ? "/skeleton/hand/right" : "/skeleton/hand/left",
      "/pose/raw",
      vr::EVRSkeletalTrackingLevel::VRSkeletalTracking_Full,
      skeleton_,
      31,
      &skeleton_handle_);

  //update the skeleton so steamvr knows we have an active skeletal input device
  pImpl_->SetDefaultSkeleton(skeleton_, IsRightHand() ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand);
  vr::VRDriverInput()->UpdateSkeletonComponent(skeleton_handle_, vr::VRSkeletalMotionRange_WithController, skeleton_, 31);
  vr::VRDriverInput()->UpdateSkeletonComponent(skeleton_handle_, vr::VRSkeletalMotionRange_WithoutController, skeleton_, 31);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/click", &input_components_[kKnuckleDeviceComponentIndex_SystemClick]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/touch", &input_components_[kKnuckleDeviceComponentIndex_SystemTouch]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &input_components_[kKnuckleDeviceComponentIndex_TriggerClick]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trigger/value", &input_components_[kKnuckleDeviceComponentIndex_TriggerValue], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/x", &input_components_[kKnuckleDeviceComponentIndex_TrackpadX], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/y", &input_components_[kKnuckleDeviceComponentIndex_TrackpadY], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trackpad/touch", &input_components_[kKnuckleDeviceComponentIndex_TrackpadTouch]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/force", &input_components_[kKnuckleDeviceComponentIndex_TrackpadForce], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grip/touch", &input_components_[kKnuckleDeviceComponentIndex_GripTouch]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/grip/force", &input_components_[kKnuckleDeviceComponentIndex_GripForce], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/grip/value", &input_components_[kKnuckleDeviceComponentIndex_GripValue], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/thumbstick/click", &input_components_[kKnuckleDeviceComponentIndex_ThumbstickClick]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/thumbstick/touch", &input_components_[kKnuckleDeviceComponentIndex_ThumbstickTouch]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/thumbstick/x", &input_components_[kKnuckleDeviceComponentIndex_ThumbstickX], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/thumbstick/y", &input_components_[kKnuckleDeviceComponentIndex_ThumbstickY], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/a/click", &input_components_[kKnuckleDeviceComponentIndex_AClick]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/a/touch", &input_components_[kKnuckleDeviceComponentIndex_ATouch]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/b/click", &input_components_[kKnuckleDeviceComponentIndex_BClick]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/b/touch", &input_components_[kKnuckleDeviceComponentIndex_BTouch]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/index", &input_components_[kKnuckleDeviceComponentIndex_FingerIndex], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/middle", &input_components_[kKnuckleDeviceComponentIndex_FingerMiddle], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/ring", &input_components_[kKnuckleDeviceComponentIndex_FingerRing], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/pinky", &input_components_[kKnuckleDeviceComponentIndex_FingerPinky], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  // clang-format on

  ogdevice_->ListenForInput([&](og::InputPeripheralData data) {
    // clang-format off
    pImpl_->SetSkeleton(skeleton_, data, IsRightHand() ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand);
    vr::VRDriverInput()->UpdateSkeletonComponent(skeleton_handle_, vr::VRSkeletalMotionRange_WithController, skeleton_, 31);
    vr::VRDriverInput()->UpdateSkeletonComponent(skeleton_handle_, vr::VRSkeletalMotionRange_WithoutController, skeleton_, 31);

    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_ThumbstickX], data.joystick.x, 0);
    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_ThumbstickY], data.joystick.y, 0);

    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_ThumbstickClick], data.joystick.pressed, 0);
    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_ThumbstickTouch], data.joystick.pressed, 0);

    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_TriggerClick], data.trigger.pressed, 0);
    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_TriggerValue], data.trigger.value, 0);

    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_AClick], data.A.pressed, 0);
    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_ATouch], data.A.value > 0.f, 0);

    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_BClick], data.B.pressed, 0);
    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_BTouch], data.B.value > 0.f, 0);

    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_GripTouch], data.grab.activated, 0);
    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_GripForce], data.grab.activated, 0);
    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_GripValue], data.grab.activated, 0);

    vr::VRDriverInput()->UpdateBooleanComponent(input_components_[kKnuckleDeviceComponentIndex_SystemClick], data.menu.pressed, 0);

    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_FingerIndex], pImpl_->GetAverageFingerCurlValue(data.flexion[1]), 0);
    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_FingerMiddle], pImpl_->GetAverageFingerCurlValue(data.flexion[2]), 0);
    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_FingerRing], pImpl_->GetAverageFingerCurlValue(data.flexion[3]), 0);
    vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_FingerPinky], pImpl_->GetAverageFingerCurlValue(data.flexion[4]), 0);
    // clang-format on
  });

  pose_thread_ = std::thread(&KnuckleDeviceDriver::PoseThread, this);

  is_active_ = true;

  return vr::VRInitError_None;
}

void KnuckleDeviceDriver::PoseThread() {
  while (is_active_) {
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_id_, pose_->UpdatePose(), sizeof vr::DriverPose_t);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void KnuckleDeviceDriver::EnterStandby() {}

void *KnuckleDeviceDriver::GetComponent(const char *pchComponentNameAndVersion) {
  return nullptr;
}

void KnuckleDeviceDriver::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) {}

void KnuckleDeviceDriver::Deactivate() {
  if (is_active_.exchange(false)) {
    ogdevice_.reset();
    pose_thread_.join();
  }
}

vr::DriverPose_t KnuckleDeviceDriver::GetPose() {
  vr::DriverPose_t pose{};

  pose.poseIsValid = true;
  return pose;
}

std::string KnuckleDeviceDriver::GetSerialNumber() {
  return IsRightHand() ? "LHR-E217CD01" : "LHR-E217CD00";
}

bool KnuckleDeviceDriver::IsRightHand() const {
  return is_active_ && ogdevice_->GetInfo().hand == og::kHandRight;
}

vr::ETrackedControllerRole KnuckleDeviceDriver::GetRole() const {
  switch (ogdevice_->GetInfo().hand) {
    case og::kHandRight:
      return vr::TrackedControllerRole_RightHand;
    case og::kHandLeft:
      return vr::TrackedControllerRole_LeftHand;
    default:
      return vr::TrackedControllerRole_Invalid;
  }
}

KnuckleDeviceDriver::~KnuckleDeviceDriver() = default;