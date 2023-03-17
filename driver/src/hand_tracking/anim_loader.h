// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

enum HandSkeletonBone {
  kHandSkeletonBone_Root = 0,
  kHandSkeletonBone_Wrist,
  kHandSkeletonBone_Thumb0,
  kHandSkeletonBone_Thumb1,
  kHandSkeletonBone_Thumb2,
  kHandSkeletonBone_Thumb3,
  kHandSkeletonBone_IndexFinger0,
  kHandSkeletonBone_IndexFinger1,
  kHandSkeletonBone_IndexFinger2,
  kHandSkeletonBone_IndexFinger3,
  kHandSkeletonBone_IndexFinger4,
  kHandSkeletonBone_MiddleFinger0,
  kHandSkeletonBone_MiddleFinger1,
  kHandSkeletonBone_MiddleFinger2,
  kHandSkeletonBone_MiddleFinger3,
  kHandSkeletonBone_MiddleFinger4,
  kHandSkeletonBone_RingFinger0,
  kHandSkeletonBone_RingFinger1,
  kHandSkeletonBone_RingFinger2,
  kHandSkeletonBone_RingFinger3,
  kHandSkeletonBone_RingFinger4,
  kHandSkeletonBone_PinkyFinger0,
  kHandSkeletonBone_PinkyFinger1,
  kHandSkeletonBone_PinkyFinger2,
  kHandSkeletonBone_PinkyFinger3,
  kHandSkeletonBone_PinkyFinger4,
  kHandSkeletonBone_AuxThumb,
  kHandSkeletonBone_AuxIndexFinger,
  kHandSkeletonBone_AuxMiddleFinger,
  kHandSkeletonBone_AuxRingFinger,
  kHandSkeletonBone_AuxPinkyFinger,
  kHandSkeletonBone_Unknown = -1,
  kHandSkeletonBone_Count
};

struct Transform {
  std::array<float, 4> rotation;
  std::array<float, 3> translation;
};

struct AnimationData {
  Transform start_transform;
  float start_time;
  Transform end_transform;
  float end_time;
};

class IModelManager {
 public:
  virtual bool Load() = 0;

  [[nodiscard]] virtual AnimationData GetAnimationDataByBoneIndex(const HandSkeletonBone& bone_index, float f) const = 0;
  [[nodiscard]] virtual Transform GetTransformByBoneIndex(const HandSkeletonBone& bone_index) const = 0;

  virtual ~IModelManager() = default;
};

class GLTFModelManager : public IModelManager {
 public:
  explicit GLTFModelManager(std::string file_name);

  bool Load() override;

  [[nodiscard]] AnimationData GetAnimationDataByBoneIndex(const HandSkeletonBone& bone_index, float f) const override;
  [[nodiscard]] Transform GetTransformByBoneIndex(const HandSkeletonBone& bone_index) const override;

  ~GLTFModelManager() override;
 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;
};
