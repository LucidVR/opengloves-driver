#include "Util/AnimLoader.h"

#include "DriverLog.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

static float Lerp(const float& a, const float& b, const float& f) {
  return a + f * (b - a);
}

static const std::array<float, 4> emptyRotation = {0.0f, 0.0f, 0.0f, 0.0f};
static const std::array<float, 3> emptyTranslation = {0.0f, 0.0f, 0.0f};

Transform::Transform() : rotation(emptyRotation), translation(emptyTranslation) {}
AnimationData::AnimationData() : startTime(0.0f), endTime(0.0f) {}

static const std::map<std::string, HandSkeletonBone> GLTFNodeBoneMap{
    {"REF:Root", HandSkeletonBone::Root},
    {"REF:wrist_r", HandSkeletonBone::Wrist},
    {"REF:finger_thumb_0_r", HandSkeletonBone::Thumb0},
    {"REF:finger_thumb_1_r", HandSkeletonBone::Thumb1},
    {"REF:finger_thumb_2_r", HandSkeletonBone::Thumb2},
    {"REF:finger_thumb_r_end", HandSkeletonBone::Thumb3},
    {"REF:finger_index_meta_r", HandSkeletonBone::IndexFinger0},
    {"REF:finger_index_0_r", HandSkeletonBone::IndexFinger1},
    {"REF:finger_index_1_r", HandSkeletonBone::IndexFinger2},
    {"REF:finger_index_2_r", HandSkeletonBone::IndexFinger3},
    {"REF:finger_index_r_end", HandSkeletonBone::IndexFinger4},
    {"REF:finger_middle_meta_r", HandSkeletonBone::MiddleFinger0},
    {"REF:finger_middle_0_r", HandSkeletonBone::MiddleFinger1},
    {"REF:finger_middle_1_r", HandSkeletonBone::MiddleFinger2},
    {"REF:finger_middle_2_r", HandSkeletonBone::MiddleFinger3},
    {"REF:finger_middle_r_end", HandSkeletonBone::MiddleFinger4},
    {"REF:finger_ring_meta_r", HandSkeletonBone::RingFinger0},
    {"REF:finger_ring_0_r", HandSkeletonBone::RingFinger1},
    {"REF:finger_ring_1_r", HandSkeletonBone::RingFinger2},
    {"REF:finger_ring_2_r", HandSkeletonBone::RingFinger3},
    {"REF:finger_ring_r_end", HandSkeletonBone::RingFinger4},
    {"REF:finger_pinky_meta_r", HandSkeletonBone::PinkyFinger0},
    {"REF:finger_pinky_0_r", HandSkeletonBone::PinkyFinger1},
    {"REF:finger_pinky_1_r", HandSkeletonBone::PinkyFinger2},
    {"REF:finger_pinky_2_r", HandSkeletonBone::PinkyFinger3},
    {"REF:finger_pinky_r_end", HandSkeletonBone::PinkyFinger4},
    {"REF:finger_thumb_r_aux", HandSkeletonBone::AuxThumb},
    {"REF:finger_index_r_aux", HandSkeletonBone::AuxIndexFinger},
    {"REF:finger_middle_r_aux", HandSkeletonBone::AuxMiddleFinger},
    {"REF:finger_ring_r_aux", HandSkeletonBone::AuxRingFinger},
    {"REF:finger_pinky_r_aux", HandSkeletonBone::AuxPinkyFinger}};

static void MapRightTransform(Transform& transform, const HandSkeletonBone& boneIndex) {
  std::array<float, 4> quat = transform.rotation;
  switch (boneIndex) {
    case HandSkeletonBone::Root: {
      return;
    }

    case HandSkeletonBone::IndexFinger0: {
      transform.rotation[0] *= -1;
      transform.rotation[1] *= -1;
      transform.rotation[2] *= -1;
      transform.rotation[3] *= -1;
      break;
    }
    case HandSkeletonBone::AuxThumb:
      quat[0] *= -1;
      quat[1] *= -1;
      quat[2] *= -1;
      quat[3] *= -1;
    case HandSkeletonBone::AuxIndexFinger:
    case HandSkeletonBone::AuxMiddleFinger:
    case HandSkeletonBone::AuxRingFinger:
    case HandSkeletonBone::AuxPinkyFinger:
    case HandSkeletonBone::Wrist: {
      transform.rotation[0] = -quat[2];
      transform.rotation[1] = quat[3];
      transform.rotation[2] = quat[0];
      transform.rotation[3] = -quat[1];

      transform.translation[0] *= -1;
      transform.translation[2] *= -1;

      break;
    }
  }
}

// convert from xyzw (gltf format) to wxyz (openvr format)
static void MapRotation(std::array<float, 4>& rotation) {
  float temp0 = rotation[0];
  rotation[0] = rotation[3];
  rotation[3] = rotation[2];
  rotation[2] = rotation[1];
  rotation[1] = temp0;
}

bool GLTFModelManager::Load() {
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  const bool ret = loader.LoadBinaryFromFile(&model_, &err, &warn, fileName_);

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

  initialTransforms_ = std::vector<Transform>(GLTFNodeBoneMap.size());
  keyframeTransforms_ = std::vector<std::vector<Transform>>(GLTFNodeBoneMap.size());

  LoadKeyframeTimes();
  LoadInitialTransforms();

  return true;
}

void GLTFModelManager::LoadKeyframeTimes() {
  const tinygltf::Accessor accessor = model_.accessors[0];
  keyframeTimes_.resize(accessor.count);

  const tinygltf::BufferView bufferView = model_.bufferViews[accessor.bufferView];
  const std::vector<unsigned char>& bufData = model_.buffers[0].data;
  memcpy(&keyframeTimes_[0], bufData.data() + bufferView.byteOffset + accessor.byteOffset, accessor.count * sizeof(float));
}

void GLTFModelManager::LoadInitialTransforms() {
  for (size_t nodeIndex = 0; nodeIndex < model_.nodes.size(); nodeIndex++) {
    const tinygltf::Node& node = model_.nodes[nodeIndex];

    try {
      const HandSkeletonBone bone = GLTFNodeBoneMap.at(node.name);
      const int boneIndex = static_cast<int>(bone);

      Transform transform;
      if (node.rotation.size() >= 4) {
        transform.rotation[0] = static_cast<float>(node.rotation[0]);
        transform.rotation[1] = static_cast<float>(node.rotation[1]);
        transform.rotation[2] = static_cast<float>(node.rotation[2]);
        transform.rotation[3] = static_cast<float>(node.rotation[3]);
        MapRotation(transform.rotation);
      }
      if (node.translation.size() >= 3) {
        transform.translation[0] = static_cast<float>(node.translation[0]);
        transform.translation[1] = static_cast<float>(node.translation[1]);
        transform.translation[2] = static_cast<float>(node.translation[2]);
      }
      MapRightTransform(transform, (HandSkeletonBone)boneIndex);

      initialTransforms_[boneIndex] = transform;

      const tinygltf::Animation& animation = model_.animations[0];
      std::vector<Transform>& transforms = keyframeTransforms_[boneIndex];

      transforms.resize(keyframeTimes_.size());

      for (auto& channel : animation.channels) {
        if (channel.target_node != nodeIndex) continue;

        const tinygltf::Accessor& accessor = model_.accessors[animation.samplers[channel.sampler].output];
        switch (accessor.type) {
          // rotation via quaternion
          case TINYGLTF_TYPE_VEC4: {
            std::vector<std::array<float, 4>> keyframes = GetVecN<4>(accessor);
            for (size_t i = 0; i < keyframes.size(); i++) {
              transforms[i].rotation = keyframes[i];
              MapRotation(transforms[i].rotation);
            };
            break;
          }
          // translation
          case TINYGLTF_TYPE_VEC3: {
            std::vector<std::array<float, 3>> keyframes = GetVecN<3>(accessor);
            for (size_t i = 0; i < keyframes.size(); i++) {
              transforms[i].translation = keyframes[i];
              MapRightTransform(transforms[i], bone);
            };
            break;
          }
        }
      }

    } catch (const std::out_of_range&) {
      DriverLog("Not parsing node as it was not defined as a bone: %i", nodeIndex);
      continue;
    }
  }
}

Transform GLTFModelManager::GetTransformByBoneIndex(const HandSkeletonBone& boneIndex) const {
  return initialTransforms_[static_cast<size_t>(boneIndex)];
}

AnimationData GLTFModelManager::GetAnimationDataByBoneIndex(const HandSkeletonBone& boneIndex, const float f) const {
  const size_t lowerKeyframeIndex = std::upper_bound(keyframeTimes_.begin(), keyframeTimes_.end(), f) - keyframeTimes_.begin() - 1;
  const size_t upperKeyframeIndex = lowerKeyframeIndex < keyframeTimes_.size() - 1 ? lowerKeyframeIndex + 1 : lowerKeyframeIndex;

  AnimationData result;
  result.startTransform = keyframeTransforms_[static_cast<size_t>(boneIndex)][lowerKeyframeIndex];
  result.startTime = keyframeTimes_[lowerKeyframeIndex];
  result.endTransform = keyframeTransforms_[static_cast<size_t>(boneIndex)][upperKeyframeIndex];
  result.endTime = keyframeTimes_[upperKeyframeIndex];

  return result;
}