#include "Bones.h"

#include "DriverLog.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

static const std::array<float, 4> emptyRotation = {0.0f, 0.0f, 0.0f, 0.0f};
static const std::array<float, 3> emptyTranslation = {0.0f, 0.0f, 0.0f};

static float Lerp(const float& a, const float& b, const float& f) { return a + f * (b - a); }

enum HandSkeletonBone : vr::BoneIndex_t {
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

static short GetFingerFromBoneIndex(vr::BoneIndex_t bone) {
  switch (bone) {
    case eBone_Thumb0:
    case eBone_Thumb1:
    case eBone_Thumb2:
    case eBone_Thumb3:
    case eBone_Aux_Thumb:
      return 0;
    case eBone_IndexFinger0:
    case eBone_IndexFinger1:
    case eBone_IndexFinger2:
    case eBone_IndexFinger3:
    case eBone_IndexFinger4:
    case eBone_Aux_IndexFinger:
      return 1;
    case eBone_MiddleFinger0:
    case eBone_MiddleFinger1:
    case eBone_MiddleFinger2:
    case eBone_MiddleFinger3:
    case eBone_MiddleFinger4:
    case eBone_Aux_MiddleFinger:
      return 2;
    case eBone_RingFinger0:
    case eBone_RingFinger1:
    case eBone_RingFinger2:
    case eBone_RingFinger3:
    case eBone_RingFinger4:
    case eBone_Aux_RingFinger:
      return 3;
    case eBone_PinkyFinger0:
    case eBone_PinkyFinger1:
    case eBone_PinkyFinger2:
    case eBone_PinkyFinger3:
    case eBone_PinkyFinger4:
    case eBone_Aux_PinkyFinger:
      return 4;

    default:
      return -1;
  }
}

class GLTFModelManager : public IModelManager {
 public:
  GLTFModelManager(const std::string& fileName) : m_fileName(fileName){};

  bool Load() {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadBinaryFromFile(&m_model, &err, &warn, m_fileName);

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

    m_keyframes = GetFloat(GetAccessorByIndex(0));

    return true;
  }

  AnimationData_t GetAnimationDataByNodeIndex(const size_t& boneIndex, const float& f) const {
    AnimationData_t result{};

    std::vector<float> timeKeyframes = GetKeyframes();
    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), f);

    const size_t lowerKeyframeIndex = it - m_keyframes.begin();
    const size_t upperKeyframeIndex = std::next(it, 1) - m_keyframes.begin();

    result.times = {timeKeyframes[lowerKeyframeIndex], timeKeyframes[upperKeyframeIndex]};

    std::vector<tinygltf::AnimationChannel> channels = GetAnimationChannelsByNodeIndex(0, boneIndex);
    for (tinygltf::AnimationChannel const& channel : channels) {
      tinygltf::Accessor accessor = GetAccessorByAnimationChannel(0, channel);

      switch (accessor.type) {
        // quaternion
        case TINYGLTF_TYPE_VEC4: {
          std::vector<std::array<float, 4>> rotationKeyframes = GetVec4(accessor);

          auto it = std::lower_bound(timeKeyframes.begin(), timeKeyframes.end(), f);

          std::array<float, 4> lowerKeyframe = rotationKeyframes[lowerKeyframeIndex];
          std::array<float, 4> upperKeyframe = rotationKeyframes[upperKeyframeIndex];

          result.transforms[0].rotation = lowerKeyframe;
          result.transforms[1].rotation = upperKeyframe;
          break;
        }
        // translation
        case TINYGLTF_TYPE_VEC3: {
          std::vector<std::array<float, 3>> translationKeyframes = GetVec3(accessor);

          std::array<float, 3> lowerKeyframe = translationKeyframes[lowerKeyframeIndex];
          std::array<float, 3> upperKeyframe = translationKeyframes[upperKeyframeIndex];

          result.transforms[0].translation = lowerKeyframe;
          result.transforms[1].translation = upperKeyframe;
          break;
        }
      }
    }

    return result;
  }

  Transform_t GetTransformByNodeIndex(const size_t& boneIndex) const {
    tinygltf::Node node = GetNodeByIndex(boneIndex);

    Transform_t transform;

    for (short i = 0; i < node.rotation.size(); i++) {
      transform.rotation[i] = node.rotation[i];
    }

    for (short i = 0; i < node.translation.size(); i++) {
      transform.translation[i] = node.translation[i];
    }

    return transform;
  }

 private:
  std::vector<std::array<float, 4>> GetVec4(const tinygltf::Accessor& accessor) const {
    tinygltf::BufferView bufferView = m_model.bufferViews[accessor.bufferView];
    std::vector<unsigned char> bufIn = GetDataFromBuffer(bufferView);

    std::vector<std::array<float, 4>> res(accessor.count);
    memcpy(&res[0], bufIn.data(), accessor.count * sizeof(float) * 4);

    return res;
  }

  std::vector<std::array<float, 3>> GetVec3(const tinygltf::Accessor& accessor) const {
    tinygltf::BufferView bufferView = m_model.bufferViews[accessor.bufferView];
    std::vector<unsigned char> bufIn = GetDataFromBuffer(bufferView);

    std::vector<std::array<float, 3>> res(accessor.count);
    memcpy(&res[0], bufIn.data(), accessor.count * sizeof(float) * 3);

    return res;
  }

  std::vector<float> GetFloat(const tinygltf::Accessor& accessor) const {
    tinygltf::BufferView bufferView = m_model.bufferViews[accessor.bufferView];
    std::vector<unsigned char> bufData = m_model.buffers[0].data;

    std::vector<unsigned char>::const_iterator viewBegin = bufData.begin() + bufferView.byteOffset;
    std::vector<unsigned char> bufIn(viewBegin, viewBegin + bufferView.byteLength);

    std::vector<float> res(accessor.count);

    memcpy(&res[0], bufIn.data(), accessor.count * sizeof(float));
    return res;
  }

  tinygltf::Node GetNodeByIndex(const size_t& nodeIndex) const {
    tinygltf::Node node = m_model.nodes[nodeIndex];

    return node;
  }

  std::vector<tinygltf::AnimationChannel> GetAnimationChannelsByNodeIndex(const size_t& animationIndex, const size_t& nodeIndex) const {
    std::vector<tinygltf::AnimationChannel> channels = m_model.animations[animationIndex].channels;

    std::vector<tinygltf::AnimationChannel> result;
    for (const tinygltf::AnimationChannel& channel : channels) {
      if (channel.target_node == nodeIndex) {
        result.push_back(channel);
        if (result.size() == 2) return result;
      }
    }

    return result;
  }
  tinygltf::Accessor GetAccessorByAnimationChannel(const size_t& animationIndex, const tinygltf::AnimationChannel& channel) const {
    tinygltf::AnimationSampler sampler = m_model.animations[animationIndex].samplers[channel.sampler];
    tinygltf::Accessor accessor = m_model.accessors[sampler.output];

    return accessor;
  }

  tinygltf::Accessor GetAccessorByIndex(const size_t& accessorIndex) const {
    tinygltf::Accessor accessor = m_model.accessors[accessorIndex];
    return accessor;
  }

  std::vector<float> GetKeyframes() const { return m_keyframes; }

  std::vector<unsigned char> GetDataFromBuffer(const tinygltf::BufferView& bufferView) const {
    std::vector<unsigned char> bufData = m_model.buffers[0].data;

    std::vector<unsigned char>::const_iterator viewBegin = bufData.cbegin() + bufferView.byteOffset;
    std::vector<unsigned char> result(viewBegin, viewBegin + bufferView.byteLength);

    return result;
  }

  tinygltf::Model m_model;
  std::vector<float> m_keyframes;
  std::string m_fileName;
};

BoneAnimator::BoneAnimator(const std::string& fileName) : m_fileName(fileName) {
  m_modelManager = std::make_unique<GLTFModelManager>(fileName);
  m_loaded = m_modelManager->Load();
}

void BoneAnimator::ComputeSkeletonTransforms(vr::VRBoneTransform_t* skeleton, const std::array<float, 5>& flexion, bool rightHand) {
  if (!m_loaded) return;

  for (size_t i = 0; i < NUM_BONES; i++) {
    size_t finger = GetFingerFromBoneIndex(i);
    if (finger != -1) skeleton[i] = GetTransformForBone(i, flexion[finger]);
  }
}

vr::VRBoneTransform_t BoneAnimator::GetTransformForBone(const size_t boneIndex, const float f) {
  vr::VRBoneTransform_t result;

  Transform_t nodeTransform = m_modelManager->GetTransformByNodeIndex(boneIndex+1);
  result.orientation.z = nodeTransform.rotation[0];
  result.orientation.w = nodeTransform.rotation[1];
  result.orientation.x = nodeTransform.rotation[2];
  result.orientation.y = nodeTransform.rotation[3];
  std::copy(nodeTransform.translation.begin(), nodeTransform.translation.end(), result.position.v);

  AnimationData_t animationData = m_modelManager->GetAnimationDataByNodeIndex(boneIndex+1, f);

  if (animationData.transforms[0].rotation != emptyRotation) {
    result.orientation.x = animationData.transforms[0].rotation[0];
    result.orientation.y = animationData.transforms[0].rotation[1];
    result.orientation.z = animationData.transforms[0].rotation[2];
    result.orientation.w = animationData.transforms[0].rotation[3];
  }

  if (animationData.transforms[0].translation != emptyTranslation) {
    result.position.v[0] = animationData.transforms[0].translation[0];
    result.position.v[1] = animationData.transforms[0].translation[1];
    result.position.v[2] = animationData.transforms[0].translation[2];
  }

   const float interp = Lerp(animationData.times[0], animationData.times[1], f);

   if (animationData.transforms[0].rotation != emptyRotation) {
    result.orientation.x = Lerp(animationData.transforms[0].rotation[0], animationData.transforms[1].rotation[0], interp);
    result.orientation.y = Lerp(animationData.transforms[0].rotation[1], animationData.transforms[1].rotation[1], interp);
    result.orientation.z = Lerp(animationData.transforms[0].rotation[2], animationData.transforms[1].rotation[2], interp);
    result.orientation.w = Lerp(animationData.transforms[0].rotation[3], animationData.transforms[1].rotation[3], interp);
  }

   if (animationData.transforms[0].translation != emptyTranslation) {
    result.position.v[0] = Lerp(animationData.transforms[0].translation[0], animationData.transforms[1].translation[0], interp);
    result.position.v[1] = Lerp(animationData.transforms[0].translation[1], animationData.transforms[1].translation[1], interp);
    result.position.v[2] = Lerp(animationData.transforms[0].translation[2], animationData.transforms[1].translation[2], interp);
  }
  int boneIndext = boneIndex;
  result.position.v[3] = 1.0f;
  return result;
};

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