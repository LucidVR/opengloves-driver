#pragma once
#include <openvr_driver.h>

const int NUM_BONES = 31;
extern vr::VRBoneTransform_t rightOpenPose[NUM_BONES];
extern vr::VRBoneTransform_t rightFistPose[NUM_BONES];

extern vr::VRBoneTransform_t leftOpenPose[NUM_BONES];
extern vr::VRBoneTransform_t leftFistPose[NUM_BONES];

enum HandSkeletonBone : vr::BoneIndex_t
{
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

void ComputeBoneFlexion(vr::VRBoneTransform_t *bone_transform, float transform, int index, const bool isRightHand);
void ComputeBoneSplay(vr::VRBoneTransform_t *bone_transform, const float transform, int index, const bool isRightHand);
vr::HmdQuaternionf_t CalculateOrientation(const float transform, const int boneIndex, const vr::VRBoneTransform_t* openPose, const vr::VRBoneTransform_t* fistPose);
vr::HmdVector4_t CalculatePosition(const float transform, const int boneIndex, const vr::VRBoneTransform_t* openPose, const vr::VRBoneTransform_t* fistPose);
int FingerFromBone(vr::BoneIndex_t bone);
/**
*Linear interpolation between a and b.
**/
float Lerp(const float a, const float b, const float f);