// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "hand_tracking.h"

#include <algorithm>

#include "util/driver_log.h"
#include "util/driver_math.h"

#define HAND_TRACKING_OPENVR_BONE_COUNT 31

static const float k_max_splay_angle = 20.0f;

static const std::array<float, 4> empty_rotation = {0.0f, 0.0f, 0.0f, 0.0f};
static const std::array<float, 3> empty_translation = {0.0f, 0.0f, 0.0f};

static float Lerp(const float& a, const float& b, const float& f) {
  return a + f * (b - a);
}

enum FingerIndex {
  kFingerIndex_Thumb = 0,
  kFingerIndex_IndexFinger,
  kFingerIndex_MiddleFinger,
  kFingerIndex_RingFinger,
  kFingerIndex_PinkyFinger,
  kFingerIndex_Unknown = -1
};

static bool IsBoneSplayable(const HandSkeletonBone& bone) {
  return bone == kHandSkeletonBone_Thumb0 || bone == kHandSkeletonBone_IndexFinger1 || bone == kHandSkeletonBone_MiddleFinger1 ||
         bone == kHandSkeletonBone_RingFinger1 || bone == kHandSkeletonBone_PinkyFinger1;
}

static FingerIndex GetFingerFromBoneIndex(const HandSkeletonBone& bone) {
  switch (bone) {
    case kHandSkeletonBone_Thumb0:
    case kHandSkeletonBone_Thumb1:
    case kHandSkeletonBone_Thumb2:
    case kHandSkeletonBone_AuxThumb:
      return kFingerIndex_Thumb;

    case kHandSkeletonBone_IndexFinger0:
    case kHandSkeletonBone_IndexFinger1:
    case kHandSkeletonBone_IndexFinger2:
    case kHandSkeletonBone_IndexFinger3:
    case kHandSkeletonBone_AuxIndexFinger:
      return kFingerIndex_IndexFinger;

    case kHandSkeletonBone_MiddleFinger0:
    case kHandSkeletonBone_MiddleFinger1:
    case kHandSkeletonBone_MiddleFinger2:
    case kHandSkeletonBone_MiddleFinger3:
    case kHandSkeletonBone_AuxMiddleFinger:
      return kFingerIndex_MiddleFinger;

    case kHandSkeletonBone_RingFinger0:
    case kHandSkeletonBone_RingFinger1:
    case kHandSkeletonBone_RingFinger2:
    case kHandSkeletonBone_RingFinger3:
    case kHandSkeletonBone_AuxRingFinger:
      return kFingerIndex_RingFinger;

    case kHandSkeletonBone_PinkyFinger0:
    case kHandSkeletonBone_PinkyFinger1:
    case kHandSkeletonBone_PinkyFinger2:
    case kHandSkeletonBone_PinkyFinger3:
    case kHandSkeletonBone_AuxPinkyFinger:
      return kFingerIndex_PinkyFinger;

    default:
      return kFingerIndex_Unknown;
  }
}

static HandSkeletonBone GetRootFingerBoneFromFingerIndex(const FingerIndex& finger) {
  switch (finger) {
    case kFingerIndex_Thumb:
      return kHandSkeletonBone_Thumb0;
    case kFingerIndex_IndexFinger:
      return kHandSkeletonBone_IndexFinger0;
    case kFingerIndex_MiddleFinger:
      return kHandSkeletonBone_MiddleFinger0;
    case kFingerIndex_RingFinger:
      return kHandSkeletonBone_RingFinger0;
    case kFingerIndex_PinkyFinger:
      return kHandSkeletonBone_PinkyFinger0;

    default:
      return kHandSkeletonBone_Unknown;
  }
}

static bool IsAuxBone(const HandSkeletonBone& boneIndex) {
  return boneIndex == kHandSkeletonBone_AuxThumb || boneIndex == kHandSkeletonBone_AuxIndexFinger || boneIndex == kHandSkeletonBone_AuxMiddleFinger ||
         boneIndex == kHandSkeletonBone_AuxRingFinger || boneIndex == kHandSkeletonBone_AuxPinkyFinger;
}

static void TransformLeftBone(vr::VRBoneTransform_t& bone, const HandSkeletonBone& bone_index) {
  switch (bone_index) {
    case kHandSkeletonBone_Root: {
      return;
    }
    case kHandSkeletonBone_Thumb0:
    case kHandSkeletonBone_IndexFinger0:
    case kHandSkeletonBone_MiddleFinger0:
    case kHandSkeletonBone_RingFinger0:
    case kHandSkeletonBone_PinkyFinger0: {
      const vr::HmdQuaternionf_t quat = bone.orientation;
      bone.orientation.w = -quat.x;
      bone.orientation.x = quat.w;
      bone.orientation.y = -quat.z;
      bone.orientation.z = quat.y;
      break;
    }
    case kHandSkeletonBone_Wrist:
    case kHandSkeletonBone_AuxIndexFinger:
    case kHandSkeletonBone_AuxThumb:
    case kHandSkeletonBone_AuxMiddleFinger:
    case kHandSkeletonBone_AuxRingFinger:
    case kHandSkeletonBone_AuxPinkyFinger: {
      bone.orientation.y *= -1;
      bone.orientation.z *= -1;
      break;
    }
    default: {
      bone.position.v[1] *= -1;
      bone.position.v[2] *= -1;
    }
  }

  bone.position.v[0] *= -1;
}

HandTracking::HandTracking(const std::string& file_name) {
  model_manager_ = std::make_unique<GLTFModelManager>(file_name);

  model_loaded_ = model_manager_->Load();

  if (!model_loaded_) DriverLog("hand tracking failed to load due to failing to load animation file");
}

void HandTracking::LoadDefaultSkeletonByHand(vr::VRBoneTransform_t* bone_transforms, vr::ETrackedControllerRole role) {
  if (!model_loaded_) return;

  for (int i = 0; i < HAND_TRACKING_OPENVR_BONE_COUNT; i++) {
    Transform transform = model_manager_->GetTransformByBoneIndex((HandSkeletonBone)i);
    bone_transforms[i].orientation.w = transform.rotation[0];
    bone_transforms[i].orientation.x = transform.rotation[1];
    bone_transforms[i].orientation.y = transform.rotation[2];
    bone_transforms[i].orientation.z = transform.rotation[3];

    bone_transforms[i].position.v[0] = transform.translation[0];
    bone_transforms[i].position.v[1] = transform.translation[1];
    bone_transforms[i].position.v[2] = transform.translation[2];
    bone_transforms[i].position.v[3] = 1.0f;

    if (role == vr::TrackedControllerRole_LeftHand) TransformLeftBone(bone_transforms[i], (HandSkeletonBone)i);
  }
}

void HandTracking::SetTransformForBone(vr::VRBoneTransform_t& bone, const HandSkeletonBone& boneIndex, float curl, float splay, bool rightHand) {
  // We don't clamp this, as chances are if it's invalid we don't really want to use it anyway.
  if (curl < 0.0f || curl > 1.0f) return;

  const Transform nodeTransform = model_manager_->GetTransformByBoneIndex(boneIndex);
  bone.orientation.x = nodeTransform.rotation[0];
  bone.orientation.y = nodeTransform.rotation[1];
  bone.orientation.z = nodeTransform.rotation[2];
  bone.orientation.w = nodeTransform.rotation[3];
  bone.position.v[0] = nodeTransform.translation[0];
  bone.position.v[1] = nodeTransform.translation[1];
  bone.position.v[2] = nodeTransform.translation[2];

  const AnimationData animationData = model_manager_->GetAnimationDataByBoneIndex(boneIndex, curl);

  const float interp = std::clamp((curl - animationData.start_time) / (animationData.end_time - animationData.start_time), 0.0f, 1.0f);

  if (animationData.start_transform.rotation != empty_rotation) {
    bone.orientation.w = Lerp(animationData.start_transform.rotation[0], animationData.end_transform.rotation[0], interp);
    bone.orientation.x = Lerp(animationData.start_transform.rotation[1], animationData.end_transform.rotation[1], interp);
    bone.orientation.y = Lerp(animationData.start_transform.rotation[2], animationData.end_transform.rotation[2], interp);
    bone.orientation.z = Lerp(animationData.start_transform.rotation[3], animationData.end_transform.rotation[3], interp);
  }

  if (animationData.start_transform.translation != empty_translation) {
    bone.position.v[0] = Lerp(animationData.start_transform.translation[0], animationData.end_transform.translation[0], interp);
    bone.position.v[1] = Lerp(animationData.start_transform.translation[1], animationData.end_transform.translation[1], interp);
    bone.position.v[2] = Lerp(animationData.start_transform.translation[2], animationData.end_transform.translation[2], interp);
  }
  bone.position.v[3] = 1.0f;

  if (splay >= -1.0f && splay <= 1.0f) {
    // only splay one bone (all the rest are done relative to this one)
    if (IsBoneSplayable(boneIndex)) bone.orientation = bone.orientation * EulerToQuaternion(0.0, DEG_TO_RAD(splay * k_max_splay_angle), 0.0);
  }

  // we're guaranteed to have updated the bone, so we can safely apply a transformation
  if (!rightHand) TransformLeftBone(bone, boneIndex);
};

float HandTracking::GetAverageFingerCurlValue(const std::array<float, 4>& joints) {
  float acc = 0;
  for (float joint : joints) {
    acc += joint;
  }

  return acc / static_cast<float>(joints.size());
}

void HandTracking::ComputeBoneTransforms(
    vr::VRBoneTransform_t* bone_transforms, const og::InputPeripheralData& data, vr::ETrackedControllerRole role) {
  if (!model_loaded_) return;

  for (size_t i = 1; i < HAND_TRACKING_OPENVR_BONE_COUNT; i++) {
    const FingerIndex finger = GetFingerFromBoneIndex(static_cast<HandSkeletonBone>(i));
    const int iFinger = static_cast<int>(finger);

    if (finger == kFingerIndex_Unknown) continue;

    float curl;
    if (IsAuxBone(static_cast<HandSkeletonBone>(i)))
      curl = GetAverageFingerCurlValue(data.flexion[iFinger]);
    else
      curl = data.flexion[iFinger][i - static_cast<int>(GetRootFingerBoneFromFingerIndex(finger))];

    const float splay = data.splay[iFinger];

    SetTransformForBone(bone_transforms[i], static_cast<HandSkeletonBone>(i), curl, splay, role == vr::TrackedControllerRole_RightHand);
  }
}