#pragma once

#include <array>
#include <memory>

#include "openvr_driver.h"

const short NUM_BONES = 31;

struct Transform_t {
  Transform_t() : rotation({0.0f, 0.0f, 0.0f, 0.0f}), translation({0.0f, 0.0f, 0.0f}){};
  std::array<float, 4> rotation;
  std::array<float, 3> translation;
};

struct AnimationData_t {
  std::array<Transform_t, 2> transforms;
  std::array<float, 2> times;
};

extern vr::VRBoneTransform_t rightOpenPose[NUM_BONES];

extern vr::VRBoneTransform_t leftOpenPose[NUM_BONES];

class IModelManager {
 public:
  virtual bool Load() = 0;

  virtual AnimationData_t GetAnimationDataByNodeIndex(const size_t& boneIndex, float f) const = 0;
  virtual Transform_t GetTransformByNodeIndex(const size_t& boneIndex) const = 0;
};

class BoneAnimator {
 public:
  BoneAnimator(const std::string& fileName);
  void ComputeSkeletonTransforms(vr::VRBoneTransform_t* skeleton, const std::array<float, 5>& flexion, bool rightHand);

 private:
  vr::VRBoneTransform_t GetTransformForBone(const size_t boneIndex, const float f);

  std::string m_fileName;
  std::unique_ptr<IModelManager> m_modelManager;
  bool m_loaded;
  std::vector<float> m_keyframes;
};