// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "anim_loader.h"

#include <map>
#include <utility>

#include "util/driver_log.h"
#include "tiny_gltf.h"

static const std::map<std::string, HandSkeletonBone> gltf_node_bone{
    {"REF:Root", kHandSkeletonBone_Root},
    {"REF:wrist_r", kHandSkeletonBone_Wrist},
    {"REF:finger_thumb_0_r", kHandSkeletonBone_Thumb0},
    {"REF:finger_thumb_1_r", kHandSkeletonBone_Thumb1},
    {"REF:finger_thumb_2_r", kHandSkeletonBone_Thumb2},
    {"REF:finger_thumb_r_end", kHandSkeletonBone_Thumb3},
    {"REF:finger_index_meta_r", kHandSkeletonBone_IndexFinger0},
    {"REF:finger_index_0_r", kHandSkeletonBone_IndexFinger1},
    {"REF:finger_index_1_r", kHandSkeletonBone_IndexFinger2},
    {"REF:finger_index_2_r", kHandSkeletonBone_IndexFinger3},
    {"REF:finger_index_r_end", kHandSkeletonBone_IndexFinger4},
    {"REF:finger_middle_meta_r", kHandSkeletonBone_MiddleFinger0},
    {"REF:finger_middle_0_r", kHandSkeletonBone_MiddleFinger1},
    {"REF:finger_middle_1_r", kHandSkeletonBone_MiddleFinger2},
    {"REF:finger_middle_2_r", kHandSkeletonBone_MiddleFinger3},
    {"REF:finger_middle_r_end", kHandSkeletonBone_MiddleFinger4},
    {"REF:finger_ring_meta_r", kHandSkeletonBone_RingFinger0},
    {"REF:finger_ring_0_r", kHandSkeletonBone_RingFinger1},
    {"REF:finger_ring_1_r", kHandSkeletonBone_RingFinger2},
    {"REF:finger_ring_2_r", kHandSkeletonBone_RingFinger3},
    {"REF:finger_ring_r_end", kHandSkeletonBone_RingFinger4},
    {"REF:finger_pinky_meta_r", kHandSkeletonBone_PinkyFinger0},
    {"REF:finger_pinky_0_r", kHandSkeletonBone_PinkyFinger1},
    {"REF:finger_pinky_1_r", kHandSkeletonBone_PinkyFinger2},
    {"REF:finger_pinky_2_r", kHandSkeletonBone_PinkyFinger3},
    {"REF:finger_pinky_r_end", kHandSkeletonBone_PinkyFinger4},
    {"REF:finger_thumb_r_aux", kHandSkeletonBone_AuxThumb},
    {"REF:finger_index_r_aux", kHandSkeletonBone_AuxIndexFinger},
    {"REF:finger_middle_r_aux", kHandSkeletonBone_AuxMiddleFinger},
    {"REF:finger_ring_r_aux", kHandSkeletonBone_AuxRingFinger},
    {"REF:finger_pinky_r_aux", kHandSkeletonBone_AuxPinkyFinger}};

static void map_right_transform(Transform& transform, const HandSkeletonBone& bone_index) {
  std::array<float, 4> quat = transform.rotation;
  switch (bone_index) {
    case kHandSkeletonBone_Root: {
      return;
    }

    case kHandSkeletonBone_IndexFinger0: {
      transform.rotation[0] *= -1;
      transform.rotation[1] *= -1;
      transform.rotation[2] *= -1;
      transform.rotation[3] *= -1;
      break;
    }
    case kHandSkeletonBone_AuxThumb:
      quat[0] *= -1;
      quat[1] *= -1;
      quat[2] *= -1;
      quat[3] *= -1;
    case kHandSkeletonBone_AuxIndexFinger:
    case kHandSkeletonBone_AuxMiddleFinger:
    case kHandSkeletonBone_AuxRingFinger:
    case kHandSkeletonBone_AuxPinkyFinger:
    case kHandSkeletonBone_Wrist: {
      transform.rotation[0] = -quat[2];
      transform.rotation[1] = quat[3];
      transform.rotation[2] = quat[0];
      transform.rotation[3] = -quat[1];

      transform.translation[0] *= -1;
      transform.translation[2] *= -1;

      break;
    }
    default:
      break;
  }
}

// convert from xyzw (gltf format) to wxyz (openvr format)
static void map_rotation(std::array<float, 4>& rotation) {
  float temp0 = rotation[0];
  rotation[0] = rotation[3];
  rotation[3] = rotation[2];
  rotation[2] = rotation[1];
  rotation[1] = temp0;
}

class GLTFModelManager::Impl {
 public:
  explicit Impl(std::string file_name) : file_name_(std::move(file_name)){};

  bool Load() {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    const bool ret = loader.LoadBinaryFromFile(&model_, &err, &warn, file_name_);

    if (!warn.empty()) {
      DriverLog("Warning parsing gltf file: %s", warn.c_str());
      return false;
    }

    if (!err.empty()) {
      DriverLog("Error parsing gltf file: %s", err.c_str());
      return false;
    }

    if (!ret) {
      DriverLog("Failed to parse gltf");
      return false;
    }

    initial_transforms_ = std::vector<Transform>(gltf_node_bone.size());
    keyframe_transforms_ = std::vector<std::vector<Transform>>(gltf_node_bone.size());

    LoadKeyframeTimes();
    LoadInitialTransforms();

    return true;
  }

  [[nodiscard]] Transform GetTransformByBoneIndex(const HandSkeletonBone& bone_index) const {
    return initial_transforms_[static_cast<size_t>(bone_index)];
  }

  [[nodiscard]] AnimationData GetAnimationDataByBoneIndex(const HandSkeletonBone& bone_index, const float f) const {
    const size_t lower_keyframe_index = std::upper_bound(keyframe_times_.begin(), keyframe_times_.end(), f) - keyframe_times_.begin() - 1;
    const size_t upper_keyframe_index = lower_keyframe_index < keyframe_times_.size() - 1 ? lower_keyframe_index + 1 : lower_keyframe_index;

    AnimationData result;
    result.start_transform = keyframe_transforms_[static_cast<size_t>(bone_index)][lower_keyframe_index];
    result.start_time = keyframe_times_[lower_keyframe_index];
    result.end_transform = keyframe_transforms_[static_cast<size_t>(bone_index)][upper_keyframe_index];
    result.end_time = keyframe_times_[upper_keyframe_index];

    return result;
  }

 private:
  void LoadKeyframeTimes() {
    const tinygltf::Accessor accessor = model_.accessors[0];
    keyframe_times_.resize(accessor.count);

    const tinygltf::BufferView buffer_view = model_.bufferViews[accessor.bufferView];
    const std::vector<unsigned char>& buffer_data = model_.buffers[0].data;
    memcpy(&keyframe_times_[0], buffer_data.data() + buffer_view.byteOffset + accessor.byteOffset, accessor.count * sizeof(float));
  }

  void LoadInitialTransforms() {
    for (size_t node_index = 0; node_index < model_.nodes.size(); node_index++) {
      const tinygltf::Node& node = model_.nodes[node_index];

      try {
        const HandSkeletonBone bone = gltf_node_bone.at(node.name);
        const int bone_index = static_cast<int>(bone);

        Transform transform{};
        if (node.rotation.size() >= 4) {
          transform.rotation[0] = static_cast<float>(node.rotation[0]);
          transform.rotation[1] = static_cast<float>(node.rotation[1]);
          transform.rotation[2] = static_cast<float>(node.rotation[2]);
          transform.rotation[3] = static_cast<float>(node.rotation[3]);
          map_rotation(transform.rotation);
        }
        if (node.translation.size() >= 3) {
          transform.translation[0] = static_cast<float>(node.translation[0]);
          transform.translation[1] = static_cast<float>(node.translation[1]);
          transform.translation[2] = static_cast<float>(node.translation[2]);
        }
        map_right_transform(transform, (HandSkeletonBone)bone_index);

        initial_transforms_[bone_index] = transform;

        const tinygltf::Animation& animation = model_.animations[0];
        std::vector<Transform>& transforms = keyframe_transforms_[bone_index];

        transforms.resize(keyframe_times_.size());

        for (auto& channel : animation.channels) {
          if (channel.target_node != node_index) continue;

          const tinygltf::Accessor& accessor = model_.accessors[animation.samplers[channel.sampler].output];
          switch (accessor.type) {
            // rotation via quaternion
            case TINYGLTF_TYPE_VEC4: {
              std::vector<std::array<float, 4>> keyframes = GetVecN<4>(accessor);
              for (size_t i = 0; i < keyframes.size(); i++) {
                transforms[i].rotation = keyframes[i];
                map_rotation(transforms[i].rotation);
              };
              break;
            }
            // translation
            case TINYGLTF_TYPE_VEC3: {
              std::vector<std::array<float, 3>> keyframes = GetVecN<3>(accessor);
              for (size_t i = 0; i < keyframes.size(); i++) {
                transforms[i].translation = keyframes[i];
                map_right_transform(transforms[i], bone);
              };
              break;
            }
          }
        }

      } catch (const std::out_of_range&) {
        DriverLog("Not parsing node as it was not defined as a bone: %i", node_index);
        continue;
      }
    }
  }

  template <size_t N>
  [[nodiscard]] std::vector<std::array<float, N>> GetVecN(const tinygltf::Accessor& accessor) const {
    const tinygltf::BufferView buffer_view = model_.bufferViews[accessor.bufferView];
    const std::vector<unsigned char>& buffer_data = model_.buffers[0].data;

    std::vector<std::array<float, N>> res(accessor.count);
    memcpy(&res[0], buffer_data.data() + buffer_view.byteOffset + accessor.byteOffset, accessor.count * sizeof(float) * N);

    return res;
  }

  tinygltf::Model model_;
  std::string file_name_;
  std::vector<Transform> initial_transforms_;
  std::vector<float> keyframe_times_;
  std::vector<std::vector<Transform>> keyframe_transforms_;
};

GLTFModelManager::GLTFModelManager(std::string file_name) : pImpl_(std::make_unique<Impl>(std::move(file_name))){};

bool GLTFModelManager::Load() {
  return pImpl_->Load();
}

Transform GLTFModelManager::GetTransformByBoneIndex(const HandSkeletonBone& bone_index) const {
  return pImpl_->GetTransformByBoneIndex(bone_index);
}

AnimationData GLTFModelManager::GetAnimationDataByBoneIndex(const HandSkeletonBone& bone_index, const float f) const {
  return pImpl_->GetAnimationDataByBoneIndex(bone_index, f);
}

GLTFModelManager::~GLTFModelManager() = default;