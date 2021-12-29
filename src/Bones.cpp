#include "Bones.h"

#include <utility>

#include "DriverLog.h"
#include "Util/Quaternion.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

static const float c_maxSplayAngle = 10.0f;

static const std::array<float, 4> emptyRotation = {0.0f, 0.0f, 0.0f, 0.0f};
static const std::array<float, 3> emptyTranslation = {0.0f, 0.0f, 0.0f};

Transform::Transform() : rotation(emptyRotation), translation(emptyTranslation) {}
AnimationData::AnimationData() : startTime(0.0f), endTime(0.0f) {}

static float Lerp(const float& a, const float& b, const float& f) {
  return a + f * (b - a);
}

enum class FingerIndex : int { Thumb = 0, IndexFinger, MiddleFinger, RingFinger, PinkyFinger, Unknown = -1 };

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

static bool IsBoneSplayableBone(const HandSkeletonBone& bone) {
  return bone == HandSkeletonBone::Thumb0 || bone == HandSkeletonBone::IndexFinger1 || bone == HandSkeletonBone::MiddleFinger1 ||
         bone == HandSkeletonBone::RingFinger1 || bone == HandSkeletonBone::PinkyFinger1;
}

static FingerIndex GetFingerFromBoneIndex(const HandSkeletonBone& bone) {
  switch (bone) {
    case HandSkeletonBone::Thumb0:
    case HandSkeletonBone::Thumb1:
    case HandSkeletonBone::Thumb2:
    case HandSkeletonBone::Thumb3:
    case HandSkeletonBone::AuxThumb:
      return FingerIndex::Thumb;

    case HandSkeletonBone::IndexFinger0:
    case HandSkeletonBone::IndexFinger1:
    case HandSkeletonBone::IndexFinger2:
    case HandSkeletonBone::IndexFinger3:
    case HandSkeletonBone::IndexFinger4:
    case HandSkeletonBone::AuxIndexFinger:
      return FingerIndex::IndexFinger;

    case HandSkeletonBone::MiddleFinger0:
    case HandSkeletonBone::MiddleFinger1:
    case HandSkeletonBone::MiddleFinger2:
    case HandSkeletonBone::MiddleFinger3:
    case HandSkeletonBone::MiddleFinger4:
    case HandSkeletonBone::AuxMiddleFinger:
      return FingerIndex::MiddleFinger;

    case HandSkeletonBone::RingFinger0:
    case HandSkeletonBone::RingFinger1:
    case HandSkeletonBone::RingFinger2:
    case HandSkeletonBone::RingFinger3:
    case HandSkeletonBone::RingFinger4:
    case HandSkeletonBone::AuxRingFinger:
      return FingerIndex::RingFinger;

    case HandSkeletonBone::PinkyFinger0:
    case HandSkeletonBone::PinkyFinger1:
    case HandSkeletonBone::PinkyFinger2:
    case HandSkeletonBone::PinkyFinger3:
    case HandSkeletonBone::PinkyFinger4:
    case HandSkeletonBone::AuxPinkyFinger:
      return FingerIndex::PinkyFinger;

    default:
      return FingerIndex::Unknown;
  }
}

class GLTFModelManager : public IModelManager {
  tinygltf::Model model_;
  std::string fileName_;
  std::vector<Transform> initialTransforms_;
  std::vector<float> keyframeTimes_;
  std::vector<std::vector<Transform>> keyframeTransforms_;

 public:
  GLTFModelManager(std::string fileName) : fileName_(std::move(fileName)) {}

  bool Load() override {
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

  AnimationData GetAnimationDataByBoneIndex(const HandSkeletonBone& boneIndex, const float f) const override {
    const float smallest = keyframeTimes_.at(0);
    const float largest = keyframeTimes_.at(keyframeTimes_.size() - 1);
    const float fScaled = Lerp(smallest, largest, f);

    const size_t lowerKeyframeIndex = std::upper_bound(keyframeTimes_.begin(), keyframeTimes_.end(), fScaled) - keyframeTimes_.begin() - 1;
    const size_t upperKeyframeIndex = lowerKeyframeIndex < keyframeTimes_.size() - 1 ? lowerKeyframeIndex + 1 : lowerKeyframeIndex;

    AnimationData result;
    result.startTransform = keyframeTransforms_[static_cast<size_t>(boneIndex)][lowerKeyframeIndex];
    result.startTime = keyframeTimes_[lowerKeyframeIndex];
    result.endTransform = keyframeTransforms_[static_cast<size_t>(boneIndex)][upperKeyframeIndex];
    result.endTime = keyframeTimes_[upperKeyframeIndex];
    result.fScaled = fScaled;

    return result;
  }

  Transform GetTransformByBoneIndex(const HandSkeletonBone& boneIndex) const override {
    return initialTransforms_[static_cast<size_t>(boneIndex)];
  }

 private:
  void LoadInitialTransforms() {
    for (size_t nodeIndex = 0; nodeIndex < model_.nodes.size(); nodeIndex++) {
      const tinygltf::Node& node = model_.nodes[nodeIndex];

      try {
        const int boneIndex = static_cast<int>(GLTFNodeBoneMap.at(node.name));

        Transform transform;
        if (node.rotation.size() >= 4) {
          transform.rotation[0] = static_cast<float>(node.rotation[0]);
          transform.rotation[1] = static_cast<float>(node.rotation[1]);
          transform.rotation[2] = static_cast<float>(node.rotation[2]);
          transform.rotation[3] = static_cast<float>(node.rotation[3]);
        }
        if (node.translation.size() >= 3) {
          transform.translation[0] = static_cast<float>(node.translation[0]);
          transform.translation[1] = static_cast<float>(node.translation[1]);
          transform.translation[2] = static_cast<float>(node.translation[2]);
        }

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
              for (size_t i = 0; i < keyframes.size(); i++) transforms[i].rotation = keyframes[i];
              break;
            }
            // translation
            case TINYGLTF_TYPE_VEC3: {
              std::vector<std::array<float, 3>> keyframes = GetVecN<3>(accessor);
              for (size_t i = 0; i < keyframes.size(); i++) transforms[i].translation = keyframes[i];
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

  void LoadKeyframeTimes() {
    const tinygltf::Accessor accessor = model_.accessors[0];
    keyframeTimes_.resize(accessor.count);

    const tinygltf::BufferView bufferView = model_.bufferViews[accessor.bufferView];
    const std::vector<unsigned char>& bufData = model_.buffers[0].data;
    memcpy(&keyframeTimes_[0], bufData.data() + bufferView.byteOffset + accessor.byteOffset, accessor.count * sizeof(float));
  }

  template <size_t N>
  std::vector<std::array<float, N>> GetVecN(const tinygltf::Accessor& accessor) const {
    const tinygltf::BufferView bufferView = model_.bufferViews[accessor.bufferView];
    const std::vector<unsigned char>& bufData = model_.buffers[0].data;

    std::vector<std::array<float, N>> res(accessor.count);
    memcpy(&res[0], bufData.data() + bufferView.byteOffset + accessor.byteOffset, accessor.count * sizeof(float) * N);

    return res;
  }
};

BoneAnimator::BoneAnimator(const std::string& fileName) : fileName_(fileName) {
  modelManager_ = std::make_unique<GLTFModelManager>(fileName);
  loaded_ = modelManager_->Load();
}

void BoneAnimator::ComputeSkeletonTransforms(vr::VRBoneTransform_t* skeleton, const VRInputData& inputData, const bool rightHand) {
  if (!loaded_) return;

  for (size_t i = 1; i < NUM_BONES; i++) {
    const FingerIndex finger = GetFingerFromBoneIndex(static_cast<HandSkeletonBone>(i));
    if (finger == FingerIndex::Unknown) continue;

    const float curl = inputData.flexion[static_cast<int>(finger)];
    const float splay = inputData.splay[static_cast<int>(finger)];

    SetTransformForBone(skeleton[i], static_cast<HandSkeletonBone>(i), curl, splay, rightHand);
  }
}

// splay asssumesthat there is a valid curl value for the finger
void BoneAnimator::SetTransformForBone(
    vr::VRBoneTransform_t& bone, const HandSkeletonBone& boneIndex, const float curl, const float splay, const bool rightHand) const {
  // We don't clamp this, as chances are if it's invalid we don't really want to use it anyway.
  if (curl < 0.0f || curl > 1.0f) return;

  const Transform nodeTransform = modelManager_->GetTransformByBoneIndex(boneIndex);
  bone.orientation.x = nodeTransform.rotation[0];
  bone.orientation.y = nodeTransform.rotation[1];
  bone.orientation.z = nodeTransform.rotation[2];
  bone.orientation.w = nodeTransform.rotation[3];
  bone.position.v[0] = nodeTransform.translation[0];
  bone.position.v[1] = nodeTransform.translation[1];
  bone.position.v[2] = nodeTransform.translation[2];

  const AnimationData animationData = modelManager_->GetAnimationDataByBoneIndex(boneIndex, curl);

  // start and end time can be the same (if we've reached the max keyframe), so make sure we only do the lerp if not
  const float diff = animationData.endTime - animationData.startTime;
  const float interp = diff != 0.0f ? (animationData.fScaled - animationData.startTime) / diff : 1.0f;

  if (animationData.startTransform.rotation != emptyRotation) {
    bone.orientation.x = Lerp(animationData.startTransform.rotation[0], animationData.endTransform.rotation[0], interp);
    bone.orientation.y = Lerp(animationData.startTransform.rotation[1], animationData.endTransform.rotation[1], interp);
    bone.orientation.z = Lerp(animationData.startTransform.rotation[2], animationData.endTransform.rotation[2], interp);
    bone.orientation.w = Lerp(animationData.startTransform.rotation[3], animationData.endTransform.rotation[3], interp);
  }

  if (animationData.startTransform.translation != emptyTranslation) {
    bone.position.v[0] = Lerp(animationData.startTransform.translation[0], animationData.endTransform.translation[0], interp);
    bone.position.v[1] = Lerp(animationData.startTransform.translation[1], animationData.endTransform.translation[1], interp);
    bone.position.v[2] = Lerp(animationData.startTransform.translation[2], animationData.endTransform.translation[2], interp);
  }
  bone.position.v[3] = 1.0f;

  if (splay >= -1.0f && splay <= 1.0f) {
    // only splay one bone (all the rest are done relative to this one)
    if (IsBoneSplayableBone(boneIndex))
      bone.orientation = MultiplyQuaternion(bone.orientation, EulerToQuaternion(0.0f, static_cast<float>(DegToRad(splay * c_maxSplayAngle)), 0.0f));
  }

  // we're guaranteed to have updated the bone, so we can safely apply a transformation
  if (!rightHand) TransformLeftBone(bone, boneIndex);
};

void BoneAnimator::TransformLeftBone(vr::VRBoneTransform_t& bone, const HandSkeletonBone& boneIndex) {
  switch (boneIndex) {
    case HandSkeletonBone::Root: {
      return;
    }
    case HandSkeletonBone::Thumb0:
    case HandSkeletonBone::IndexFinger0:
    case HandSkeletonBone::MiddleFinger0:
    case HandSkeletonBone::RingFinger0:
    case HandSkeletonBone::PinkyFinger0: {
      const vr::HmdQuaternionf_t quat = bone.orientation;
      bone.orientation.w = -quat.x;
      bone.orientation.x = quat.w;
      bone.orientation.y = -quat.z;
      bone.orientation.z = quat.y;
      break;
    }
    case HandSkeletonBone::Wrist:
    case HandSkeletonBone::AuxIndexFinger:
    case HandSkeletonBone::AuxThumb:
    case HandSkeletonBone::AuxMiddleFinger:
    case HandSkeletonBone::AuxRingFinger:
    case HandSkeletonBone::AuxPinkyFinger: {
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

// Initial values for the right/left poses
vr::VRBoneTransform_t rightOpenPose[NUM_BONES] = {
    {{0.000000f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
    {{0.034038f, 0.036503f, 0.164722f, 1.000000f}, {-0.055147f, -0.078608f, 0.920279f, -0.379296f}},
    {{0.012083f, 0.028070f, 0.025050f, 1.000000f}, {0.567418f, -0.464112f, 0.623374f, -0.272106f}},
    {{-0.040406f, -0.000000f, 0.000000f, 1.000000f}, {0.994838f, 0.082939f, 0.019454f, 0.055130f}},
    {{-0.032517f, -0.000000f, -0.000000f, 1.000000f}, {0.974793f, -0.003213f, 0.021867f, -0.222015f}},
    {{-0.030464f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
    {{-0.000632f, 0.026866f, 0.015002f, 1.000000f}, {0.421979f, -0.644251f, 0.422133f, 0.478202f}},
    {{-0.074204f, 0.005002f, -0.000234f, 1.000000f}, {0.995332f, 0.007007f, -0.039124f, 0.087949f}},
    {{-0.043930f, 0.000000f, 0.000000f, 1.000000f}, {0.997891f, 0.045808f, 0.002142f, -0.045943f}},
    {{-0.028695f, -0.000000f, -0.000000f, 1.000000f}, {0.999649f, 0.001850f, -0.022782f, -0.013409f}},
    {{-0.022821f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, 0.000000f, -0.000000f}},
    {{-0.002177f, 0.007120f, 0.016319f, 1.000000f}, {0.541276f, -0.546723f, 0.460749f, 0.442520f}},
    {{-0.070953f, -0.000779f, -0.000997f, 1.000000f}, {0.980294f, -0.167261f, -0.078959f, 0.069368f}},
    {{-0.043108f, -0.000000f, -0.000000f, 1.000000f}, {0.997947f, 0.018493f, 0.013192f, 0.059886f}},
    {{-0.033266f, -0.000000f, -0.000000f, 1.000000f}, {0.997394f, -0.003328f, -0.028225f, -0.066315f}},
    {{-0.025892f, 0.000000f, -0.000000f, 1.000000f}, {0.999195f, -0.000000f, 0.000000f, 0.040126f}},
    {{-0.000513f, -0.006545f, 0.016348f, 1.000000f}, {0.550143f, -0.516692f, 0.429888f, 0.495548f}},
    {{-0.065876f, -0.001786f, -0.000693f, 1.000000f}, {0.990420f, -0.058696f, -0.101820f, 0.072495f}},
    {{-0.040697f, -0.000000f, -0.000000f, 1.000000f}, {0.999545f, -0.002240f, 0.000004f, 0.030081f}},
    {{-0.028747f, 0.000000f, 0.000000f, 1.000000f}, {0.999102f, -0.000721f, -0.012693f, 0.040420f}},
    {{-0.022430f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
    {{0.002478f, -0.018981f, 0.015214f, 1.000000f}, {0.523940f, -0.526918f, 0.326740f, 0.584025f}},
    {{-0.062878f, -0.002844f, -0.000332f, 1.000000f}, {0.986609f, -0.059615f, -0.135163f, 0.069132f}},
    {{-0.030220f, -0.000000f, -0.000000f, 1.000000f}, {0.994317f, 0.001896f, -0.000132f, 0.106446f}},
    {{-0.018187f, -0.000000f, -0.000000f, 1.000000f}, {0.995931f, -0.002010f, -0.052079f, -0.073526f}},
    {{-0.018018f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
    {{0.006059f, 0.056285f, 0.060064f, 1.000000f}, {0.737238f, 0.202745f, -0.594267f, -0.249441f}},
    {{0.040416f, -0.043018f, 0.019345f, 1.000000f}, {-0.290331f, 0.623527f, 0.663809f, 0.293734f}},
    {{0.039354f, -0.075674f, 0.047048f, 1.000000f}, {-0.187047f, 0.678062f, 0.659285f, 0.265683f}},
    {{0.038340f, -0.090987f, 0.082579f, 1.000000f}, {-0.183037f, 0.736793f, 0.634757f, 0.143936f}},
    {{0.031806f, -0.087214f, 0.121015f, 1.000000f}, {-0.003659f, 0.758407f, 0.639342f, 0.126678f}},
};

vr::VRBoneTransform_t leftOpenPose[NUM_BONES] = {
    {{0.000000f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
    {{-0.034038f, 0.036503f, 0.164722f, 1.000000f}, {-0.055147f, -0.078608f, -0.920279f, 0.379296f}},
    {{-0.012083f, 0.028070f, 0.025050f, 1.000000f}, {0.464112f, 0.567418f, 0.272106f, 0.623374f}},
    {{0.040406f, 0.000000f, -0.000000f, 1.000000f}, {0.994838f, 0.082939f, 0.019454f, 0.055130f}},
    {{0.032517f, 0.000000f, 0.000000f, 1.000000f}, {0.974793f, -0.003213f, 0.021867f, -0.222015f}},
    {{0.030464f, -0.000000f, -0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
    {{0.000632f, 0.026866f, 0.015002f, 1.000000f}, {0.644251f, 0.421979f, -0.478202f, 0.422133f}},
    {{0.074204f, -0.005002f, 0.000234f, 1.000000f}, {0.995332f, 0.007007f, -0.039124f, 0.087949f}},
    {{0.043930f, -0.000000f, -0.000000f, 1.000000f}, {0.997891f, 0.045808f, 0.002142f, -0.045943f}},
    {{0.028695f, 0.000000f, 0.000000f, 1.000000f}, {0.999649f, 0.001850f, -0.022782f, -0.013409f}},
    {{0.022821f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, -0.000000f, 0.000000f, -0.000000f}},
    {{0.002177f, 0.007120f, 0.016319f, 1.000000f}, {0.546723f, 0.541276f, -0.442520f, 0.460749f}},
    {{0.070953f, 0.000779f, 0.000997f, 1.000000f}, {0.980294f, -0.167261f, -0.078959f, 0.069368f}},
    {{0.043108f, 0.000000f, 0.000000f, 1.000000f}, {0.997947f, 0.018493f, 0.013192f, 0.059886f}},
    {{0.033266f, 0.000000f, 0.000000f, 1.000000f}, {0.997394f, -0.003328f, -0.028225f, -0.066315f}},
    {{0.025892f, -0.000000f, 0.000000f, 1.000000f}, {0.999195f, -0.000000f, 0.000000f, 0.040126f}},
    {{0.000513f, -0.006545f, 0.016348f, 1.000000f}, {0.516692f, 0.550143f, -0.495548f, 0.429888f}},
    {{0.065876f, 0.001786f, 0.000693f, 1.000000f}, {0.990420f, -0.058696f, -0.101820f, 0.072495f}},
    {{0.040697f, 0.000000f, 0.000000f, 1.000000f}, {0.999545f, -0.002240f, 0.000004f, 0.030081f}},
    {{0.028747f, -0.000000f, -0.000000f, 1.000000f}, {0.999102f, -0.000721f, -0.012693f, 0.040420f}},
    {{0.022430f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
    {{-0.002478f, -0.018981f, 0.015214f, 1.000000f}, {0.526918f, 0.523940f, -0.584025f, 0.326740f}},
    {{0.062878f, 0.002844f, 0.000332f, 1.000000f}, {0.986609f, -0.059615f, -0.135163f, 0.069132f}},
    {{0.030220f, 0.000000f, 0.000000f, 1.000000f}, {0.994317f, 0.001896f, -0.000132f, 0.106446f}},
    {{0.018187f, 0.000000f, 0.000000f, 1.000000f}, {0.995931f, -0.002010f, -0.052079f, -0.073526f}},
    {{0.018018f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
    {{-0.006059f, 0.056285f, 0.060064f, 1.000000f}, {0.737238f, 0.202745f, 0.594267f, 0.249441f}},
    {{-0.040416f, -0.043018f, 0.019345f, 1.000000f}, {-0.290331f, 0.623527f, -0.663809f, -0.293734f}},
    {{-0.039354f, -0.075674f, 0.047048f, 1.000000f}, {-0.187047f, 0.678062f, -0.659285f, -0.265683f}},
    {{-0.038340f, -0.090987f, 0.082579f, 1.000000f}, {-0.183037f, 0.736793f, -0.634757f, -0.143936f}},
    {{-0.031806f, -0.087214f, 0.121015f, 1.000000f}, {-0.003659f, 0.758407f, -0.639342f, -0.126678f}},
};