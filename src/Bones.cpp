#include "Bones.h"

#include <algorithm>
#include <utility>

#include "DriverLog.h"
#include "Util/Quaternion.h"

static const float c_maxSplayAngle = 10.0f;

static const std::array<float, 4> emptyRotation = {0.0f, 0.0f, 0.0f, 0.0f};
static const std::array<float, 3> emptyTranslation = {0.0f, 0.0f, 0.0f};

static float Lerp(const float& a, const float& b, const float& f) {
  return a + f * (b - a);
}

enum class FingerIndex : int { Thumb = 0, IndexFinger, MiddleFinger, RingFinger, PinkyFinger, Unknown = -1 };

static bool IsBoneSplayableBone(const HandSkeletonBone& bone) {
  return bone == HandSkeletonBone::Thumb0 || bone == HandSkeletonBone::IndexFinger1 || bone == HandSkeletonBone::MiddleFinger1 ||
         bone == HandSkeletonBone::RingFinger1 || bone == HandSkeletonBone::PinkyFinger1;
}

static FingerIndex GetFingerFromBoneIndex(const HandSkeletonBone& bone) {
  switch (bone) {
    case HandSkeletonBone::Thumb0:
    case HandSkeletonBone::Thumb1:
    case HandSkeletonBone::Thumb2:
    case HandSkeletonBone::AuxThumb:
      return FingerIndex::Thumb;

    case HandSkeletonBone::IndexFinger0:
    case HandSkeletonBone::IndexFinger1:
    case HandSkeletonBone::IndexFinger2:
    case HandSkeletonBone::IndexFinger3:
    case HandSkeletonBone::AuxIndexFinger:
      return FingerIndex::IndexFinger;

    case HandSkeletonBone::MiddleFinger0:
    case HandSkeletonBone::MiddleFinger1:
    case HandSkeletonBone::MiddleFinger2:
    case HandSkeletonBone::MiddleFinger3:
    case HandSkeletonBone::AuxMiddleFinger:
      return FingerIndex::MiddleFinger;

    case HandSkeletonBone::RingFinger0:
    case HandSkeletonBone::RingFinger1:
    case HandSkeletonBone::RingFinger2:
    case HandSkeletonBone::RingFinger3:
    case HandSkeletonBone::AuxRingFinger:
      return FingerIndex::RingFinger;

    case HandSkeletonBone::PinkyFinger0:
    case HandSkeletonBone::PinkyFinger1:
    case HandSkeletonBone::PinkyFinger2:
    case HandSkeletonBone::PinkyFinger3:
    case HandSkeletonBone::AuxPinkyFinger:
      return FingerIndex::PinkyFinger;

    default:
      return FingerIndex::Unknown;
  }
}

static HandSkeletonBone GetRootFingerBoneFromFingerIndex(const FingerIndex& finger) {
  switch (finger) {
    case FingerIndex::Thumb:
      return HandSkeletonBone::Thumb0;
    case FingerIndex::IndexFinger:
      return HandSkeletonBone::IndexFinger0;
    case FingerIndex::MiddleFinger:
      return HandSkeletonBone::MiddleFinger0;
    case FingerIndex::RingFinger:
      return HandSkeletonBone::RingFinger0;
    case FingerIndex::PinkyFinger:
      return HandSkeletonBone::PinkyFinger0;
  }
}

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

static bool IsAuxBone(const HandSkeletonBone& boneIndex) {
  return boneIndex == HandSkeletonBone::AuxThumb || boneIndex == HandSkeletonBone::AuxIndexFinger || boneIndex == HandSkeletonBone::AuxMiddleFinger ||
         boneIndex == HandSkeletonBone::AuxRingFinger || boneIndex == HandSkeletonBone::AuxPinkyFinger;
}

BoneAnimator::BoneAnimator(const std::string& fileName) : fileName_(fileName) {
  modelManager_ = std::make_unique<GLTFModelManager>(fileName);
  loaded_ = modelManager_->Load();
}

void BoneAnimator::ComputeSkeletonTransforms(vr::VRBoneTransform_t* skeleton, const VRInputData& inputData, const bool rightHand) {
  if (!loaded_) return;

  for (size_t i = 1; i < NUM_BONES; i++) {
    const FingerIndex finger = GetFingerFromBoneIndex(static_cast<HandSkeletonBone>(i));
    const int iFinger = static_cast<int>(finger);

    if (finger == FingerIndex::Unknown) continue;

    float curl;
    if (IsAuxBone(static_cast<HandSkeletonBone>(i)))
      curl = GetAverageCurlValue(inputData.flexion[iFinger]);
    else
      curl = inputData.flexion[iFinger][i - static_cast<int>(GetRootFingerBoneFromFingerIndex(finger))];

    const float splay = inputData.splay[iFinger];

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

  const float interp = std::clamp((curl - animationData.startTime) / (animationData.endTime - animationData.startTime), 0.0f, 1.0f);

  if (animationData.startTransform.rotation != emptyRotation) {
    bone.orientation.w = Lerp(animationData.startTransform.rotation[0], animationData.endTransform.rotation[0], interp);
    bone.orientation.x = Lerp(animationData.startTransform.rotation[1], animationData.endTransform.rotation[1], interp);
    bone.orientation.y = Lerp(animationData.startTransform.rotation[2], animationData.endTransform.rotation[2], interp);
    bone.orientation.z = Lerp(animationData.startTransform.rotation[3], animationData.endTransform.rotation[3], interp);
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

float BoneAnimator::GetAverageCurlValue(const std::array<float, 4>& joints) {
  float acc = 0;
  for (int i = 0; i < joints.size(); i++) {
    acc += joints[i];
  }

  return acc / static_cast<float>(joints.size());
}

void BoneAnimator::LoadDefaultSkeletonByHand(vr::VRBoneTransform_t* skeleton, const bool rightHand) {
  for (int i = 0; i < 31; i++) {
    Transform transform = modelManager_->GetTransformByBoneIndex((HandSkeletonBone)i);
    skeleton[i].orientation.w = transform.rotation[0];
    skeleton[i].orientation.x = transform.rotation[1];
    skeleton[i].orientation.y = transform.rotation[2];
    skeleton[i].orientation.z = transform.rotation[3];

    skeleton[i].position.v[0] = transform.translation[0];
    skeleton[i].position.v[1] = transform.translation[1];
    skeleton[i].position.v[2] = transform.translation[2];
    skeleton[i].position.v[3] = 1.0f;

    if (!rightHand) TransformLeftBone(skeleton[i], (HandSkeletonBone)i);
  }
}