// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "knuckle_device_driver.h"

#include "device/pose/device_pose.h"
#include "hand_tracking/hand_tracking.h"
#include "nlohmann/json.hpp"
#include "services/driver_external.h"
#include "util/file_path.h"

static DriverExternalServer &external_server = DriverExternalServer::GetInstance();

class KnuckleDeviceDriver::Impl {
 public:
  explicit Impl(vr::ETrackedControllerRole role)
      : role_(role),
        pose_(std::make_unique<DevicePose>(role_)),
        hand_tracking_(std::make_unique<HandTracking>(GetDriverRootPath() + R"(\resources\anims\glove_anim.glb)")) {}

  void SetDeviceDriver(std::unique_ptr<og::IDevice> device) {
    device_ = std::move(device);

    device_->ListenForInput([&](og::InputPeripheralData data) {
      // clang-format off
      hand_tracking_->ComputeBoneTransforms(skeleton_, data, IsRightHand() ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand);
      vr::VRDriverInput()->UpdateSkeletonComponent(input_components_[kKnuckleDeviceComponentIndex_Skeleton],  vr::VRSkeletalMotionRange_WithController, skeleton_, 31);
      vr::VRDriverInput()->UpdateSkeletonComponent(input_components_[kKnuckleDeviceComponentIndex_Skeleton], vr::VRSkeletalMotionRange_WithoutController, skeleton_, 31);

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

      vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_FingerIndex], hand_tracking_->GetAverageFingerCurlValue(data.flexion[1]), 0);
      vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_FingerMiddle], hand_tracking_->GetAverageFingerCurlValue(data.flexion[2]), 0);
      vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_FingerRing], hand_tracking_->GetAverageFingerCurlValue(data.flexion[3]), 0);
      vr::VRDriverInput()->UpdateScalarComponent(input_components_[kKnuckleDeviceComponentIndex_FingerPinky], hand_tracking_->GetAverageFingerCurlValue(data.flexion[4]), 0);
      // clang-format on

      if (data.calibrate.pressed) {
        if (!pose_->IsCalibrating()) {
          pose_->StartCalibration(kCalibrationMethod_Hardware);
        }
      }
      else if (pose_->IsCalibrating()) {
        pose_->CompleteCalibration(kCalibrationMethod_Hardware);
      }
    });

    external_server.RegisterFunctionCallback("force_feedback/" + std::string(IsRightHand() ? "right" : "left"), [&](const std::string &data) {
      const nlohmann::json json = nlohmann::json::parse(data);

      int16_t thumb = json["thumb"];
      int16_t index = json["index"];
      int16_t middle = json["middle"];
      int16_t ring = json["ring"];
      int16_t pinky = json["pinky"];

      og::Output output{};
      output.type = og::kOutputData_Type_ForceFeedback;

      og::OutputForceFeedbackData force_feedback_data = {thumb, index, middle, ring, pinky};
      output.data.force_feedback_data = force_feedback_data;

      device_->Output(output);

      return true;
    });
  }

  vr::EVRInitError Activate(uint32_t device_id) {
    device_id_ = device_id;

    // clang-format off
    const vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(device_id_);

    vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, IsRightHand() ? "LHR-E217CD01" : "LHR-E217CD00");
    vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, "LucidVR");

    vr::VRProperties()->SetFloatProperty(props, vr::Prop_DeviceBatteryPercentage_Float, 1.f);
    vr::VRProperties()->SetBoolProperty(props, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);

    vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
    vr::VRProperties()->SetBoolProperty(props, vr::Prop_Identifiable_Bool, true);
    vr::VRProperties()->SetInt32Property(
        props, vr::Prop_ControllerRoleHint_Int32, IsRightHand() ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand);

    vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, 2147483647);

    vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, "knuckles");

    vr::VRProperties()->SetStringProperty(props, vr::Prop_RenderModelName_String, IsRightHand() ? "{indexcontroller}valve_controller_knu_1_0_right" : "{indexcontroller}valve_controller_knu_1_0_left");

    vr::VRDriverInput()->CreateSkeletonComponent(
        props,
        IsRightHand() ? "/input/skeleton/right" : "/input/skeleton/left",
        IsRightHand() ? "/skeleton/hand/right" : "/skeleton/hand/left",
        "/pose/raw",
        vr::EVRSkeletalTrackingLevel::VRSkeletalTracking_Full,
        skeleton_,
        31,
        &input_components_[kKnuckleDeviceComponentIndex_Skeleton]);

    //update the skeleton so steamvr knows we have an active skeletal input device
    hand_tracking_->LoadDefaultSkeletonByHand(skeleton_, IsRightHand() ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand);
    vr::VRDriverInput()->UpdateSkeletonComponent(input_components_[kKnuckleDeviceComponentIndex_Skeleton], vr::VRSkeletalMotionRange_WithController, skeleton_, 31);
    vr::VRDriverInput()->UpdateSkeletonComponent(input_components_[kKnuckleDeviceComponentIndex_Skeleton], vr::VRSkeletalMotionRange_WithoutController, skeleton_, 31);

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

    is_active_ = true;

    pose_thread_ = std::thread(&Impl::PoseThread, this);

    return vr::VRInitError_None;
  }

  ~Impl() {
    if (is_active_.exchange(false)) {
      pose_thread_.join();
      device_ = nullptr;
    }
  }

 private:
  bool IsRightHand() {
    return role_ == vr::TrackedControllerRole_RightHand;
  }

  void PoseThread() {
    while (is_active_) {
      vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_id_, pose_->UpdatePose(), sizeof(vr::DriverPose_t));

      std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
  }

  std::atomic<uint32_t> device_id_;
  std::atomic<bool> is_active_;
  vr::ETrackedControllerRole role_;

  vr::VRBoneTransform_t skeleton_[31]{};
  std::array<vr::VRInputComponentHandle_t, kKnuckleDeviceComponentIndex_Count> input_components_{};

  std::thread pose_thread_;

  std::unique_ptr<og::IDevice> device_;

  std::unique_ptr<DevicePose> pose_;
  std::unique_ptr<HandTracking> hand_tracking_;
};

KnuckleDeviceDriver::KnuckleDeviceDriver(vr::ETrackedControllerRole role) : pImpl_(std::make_unique<Impl>(role)), role_(role){};

void KnuckleDeviceDriver::SetDeviceDriver(std::unique_ptr<og::IDevice> device) {
  pImpl_->SetDeviceDriver(std::move(device));
}

vr::EVRInitError KnuckleDeviceDriver::Activate(uint32_t unObjectId) {
  is_active_ = true;

  return pImpl_->Activate(unObjectId);
}

void KnuckleDeviceDriver::EnterStandby() {}

void *KnuckleDeviceDriver::GetComponent(const char *pchComponentNameAndVersion) {
  return nullptr;
}

void KnuckleDeviceDriver::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) {}

void KnuckleDeviceDriver::Deactivate() {
  is_active_ = false;
  pImpl_ = nullptr;
}

vr::DriverPose_t KnuckleDeviceDriver::GetPose() {
  vr::DriverPose_t pose{};

  pose.poseIsValid = true;
  return pose;
}

std::string KnuckleDeviceDriver::GetSerialNumber() {
  return role_ == vr::TrackedControllerRole_RightHand ? "LHR-E217CD01" : "LHR-E217CD00";
}

bool KnuckleDeviceDriver::IsActive() {
  return is_active_;
}

KnuckleDeviceDriver::~KnuckleDeviceDriver() = default;