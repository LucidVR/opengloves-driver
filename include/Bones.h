#pragma once

#include <array>
#include <memory>

#include "Encode/EncodingManager.h"
#include "openvr_driver.h"
#include "Util/AnimLoader.h"

const short NUM_BONES = static_cast<short>(HandSkeletonBone::_Count);

class BoneAnimator {
 public:
  explicit BoneAnimator(const std::string& fileName);
  void ComputeSkeletonTransforms(vr::VRBoneTransform_t* skeleton, const VRInputData& inputData, const bool rightHand);
  static void TransformLeftBone(vr::VRBoneTransform_t& bone, const HandSkeletonBone& boneIndex);
  static float GetAverageCurlValue(const std::array<float, 4>& joints);
  void LoadDefaultSkeletonByHand(vr::VRBoneTransform_t* skeleton, const bool rightHand);

 private:
  void SetTransformForBone(
      vr::VRBoneTransform_t& bone, const HandSkeletonBone& boneIndex, const float curl, const float splay, const bool rightHand) const;

  std::string fileName_;
  std::unique_ptr<IModelManager> modelManager_;
  bool loaded_;
  std::vector<float> keyframes_;
};