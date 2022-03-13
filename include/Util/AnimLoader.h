#pragma once

#include <array>

#include "openvr_driver.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

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
};

class IModelManager {
 public:
  virtual bool Load() = 0;

  virtual AnimationData GetAnimationDataByBoneIndex(const HandSkeletonBone& boneIndex, float f) const = 0;
  virtual Transform GetTransformByBoneIndex(const HandSkeletonBone& boneIndex) const = 0;
};

class GLTFModelManager : public IModelManager {
 public:
  GLTFModelManager(std::string fileName) : fileName_(std::move(fileName)) {}

  bool Load() override;

  AnimationData GetAnimationDataByBoneIndex(const HandSkeletonBone& boneIndex, const float f) const override;
  Transform GetTransformByBoneIndex(const HandSkeletonBone& boneIndex) const;

 private:
  void LoadInitialTransforms();
  void LoadKeyframeTimes();

  template <size_t N>
  std::vector<std::array<float, N>> GetVecN(const tinygltf::Accessor& accessor) const {
    const tinygltf::BufferView bufferView = model_.bufferViews[accessor.bufferView];
    const std::vector<unsigned char>& bufData = model_.buffers[0].data;

    std::vector<std::array<float, N>> res(accessor.count);
    memcpy(&res[0], bufData.data() + bufferView.byteOffset + accessor.byteOffset, accessor.count * sizeof(float) * N);

    return res;
  }

  tinygltf::Model model_;
  std::string fileName_;
  std::vector<Transform> initialTransforms_;
  std::vector<float> keyframeTimes_;
  std::vector<std::vector<Transform>> keyframeTransforms_;
};