#pragma once

#include <array>
#include <memory>

#include "openvr_driver.h"

enum class HandSkeletonBone : vr::BoneIndex_t {
  Root = 0,
  Wrist,
  Thumb0,
  Thumb1,
  Thumb2,
  Thumb3,
  IndexFinger0,
  IndexFinger1,
  IndexFinger2,
  IndexFinger3,
  IndexFinger4,
  MiddleFinger0,
  MiddleFinger1,
  MiddleFinger2,
  MiddleFinger3,
  MiddleFinger4,
  RingFinger0,
  RingFinger1,
  RingFinger2,
  RingFinger3,
  RingFinger4,
  PinkyFinger0,
  PinkyFinger1,
  PinkyFinger2,
  PinkyFinger3,
  PinkyFinger4,
  AuxThumb,
  AuxIndexFinger,
  AuxMiddleFinger,
  AuxRingFinger,
  AuxPinkyFinger,
  _Count
};

const short NUM_BONES = static_cast<short>(HandSkeletonBone::_Count);

extern vr::VRBoneTransform_t rightOpenPose[NUM_BONES];
extern vr::VRBoneTransform_t leftOpenPose[NUM_BONES];

struct Transform {
  Transform();
  std::array<float, 4> rotation;
  std::array<float, 3> translation;
};

struct AnimationData {
  AnimationData();
  Transform startTransform;
  float startTime;
  Transform endTransform;
  float endTime;
  float fScaled;
};

class IModelManager {
 public:
  virtual bool Load() = 0;

  virtual AnimationData GetAnimationDataByBoneIndex(const HandSkeletonBone& boneIndex, float f) const = 0;
  virtual Transform GetTransformByBoneIndex(const HandSkeletonBone& boneIndex) const = 0;
};

class BoneAnimator {
 public:
  explicit BoneAnimator(const std::string& fileName);
  void ComputeSkeletonTransforms(vr::VRBoneTransform_t* skeleton, const std::array<float, 5>& flexion, const bool rightHand);
  static void TransformLeftBone(vr::VRBoneTransform_t& bone, const HandSkeletonBone& boneIndex);

 private:
  vr::VRBoneTransform_t GetTransformForBone(const HandSkeletonBone& boneIndex, const float f, const bool rightHand) const;

  std::string fileName_;
  std::unique_ptr<IModelManager> modelManager_;
  bool loaded_;
  std::vector<float> keyframes_;
};