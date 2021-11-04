#pragma once

#include <array>
#include <memory>

#include "openvr_driver.h"

enum class HandSkeletonBone : vr::BoneIndex_t {
  eBone_Root = 0,
  eBone_Wrist,
  eBone_Thumb0,
  eBone_Thumb1,
  eBone_Thumb2,
  eBone_Thumb3,
  eBone_IndexFinger0,
  eBone_IndexFinger1,
  eBone_IndexFinger2,
  eBone_IndexFinger3,
  eBone_IndexFinger4,
  eBone_MiddleFinger0,
  eBone_MiddleFinger1,
  eBone_MiddleFinger2,
  eBone_MiddleFinger3,
  eBone_MiddleFinger4,
  eBone_RingFinger0,
  eBone_RingFinger1,
  eBone_RingFinger2,
  eBone_RingFinger3,
  eBone_RingFinger4,
  eBone_PinkyFinger0,
  eBone_PinkyFinger1,
  eBone_PinkyFinger2,
  eBone_PinkyFinger3,
  eBone_PinkyFinger4,
  eBone_Aux_Thumb,
  eBone_Aux_IndexFinger,
  eBone_Aux_MiddleFinger,
  eBone_Aux_RingFinger,
  eBone_Aux_PinkyFinger,
  eBone_Count
};

const short NUM_BONES = (short)HandSkeletonBone::eBone_Count;

extern vr::VRBoneTransform_t rightOpenPose[NUM_BONES];
extern vr::VRBoneTransform_t leftOpenPose[NUM_BONES];

struct Transform_t {
  Transform_t();
  std::array<float, 4> rotation;
  std::array<float, 3> translation;
};

struct AnimationData_t {
  AnimationData_t();
  Transform_t startTransform;
  float startTime;
  Transform_t endTransform;
  float endTime;
};

class IModelManager {
 public:
  virtual bool Load() = 0;

  virtual AnimationData_t GetAnimationDataByBoneIndex(const HandSkeletonBone& boneIndex, float f) const = 0;
  virtual Transform_t GetTransformByBoneIndex(const HandSkeletonBone& boneIndex) const = 0;
};

class BoneAnimator {
 public:
  BoneAnimator(const std::string& fileName);
  void ComputeSkeletonTransforms(vr::VRBoneTransform_t* skeleton, const std::array<float, 5>& flexion, const bool rightHand);
  void TransformLeftBone(vr::VRBoneTransform_t& bone, const HandSkeletonBone& boneIndex);

 private:
  vr::VRBoneTransform_t GetTransformForBone(const HandSkeletonBone& boneIndex, const float f, const bool rightHand);

  std::string m_fileName;
  std::unique_ptr<IModelManager> m_modelManager;
  bool m_loaded;
  std::vector<float> m_keyframes;
};