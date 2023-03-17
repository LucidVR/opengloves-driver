// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include <array>
#include <memory>

#include "anim_loader.h"
#include "opengloves_interface.h"
#include "openvr_driver.h"

class HandTracking {
 public:
  explicit HandTracking(const std::string& file_name);

  void LoadDefaultSkeletonByHand(vr::VRBoneTransform_t* bone_transforms, vr::ETrackedControllerRole role);

  void ComputeBoneTransforms(vr::VRBoneTransform_t* bone_transforms, const og::InputPeripheralData& data, vr::ETrackedControllerRole role);

  static float GetAverageFingerCurlValue(const std::array<float, 4>& joints);
 private:
  void SetTransformForBone(
      vr::VRBoneTransform_t& bone, const HandSkeletonBone& boneIndex, float curl, float splay, bool rightHand);

  bool model_loaded_;

  std::unique_ptr<IModelManager> model_manager_;

  std::vector<float> keyframe_times_;
};