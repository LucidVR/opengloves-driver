#pragma once
#include <openvr_driver.h>

const int NUM_BONES = 31;
extern vr::VRBoneTransform_t right_open_hand_pose[NUM_BONES];
extern vr::VRBoneTransform_t right_fist_pose[NUM_BONES];

extern vr::VRBoneTransform_t left_open_hand_pose[NUM_BONES];
extern vr::VRBoneTransform_t left_fist_pose[NUM_BONES];

/**
*Computes the bone transforms for the flexion and extension of fingers. Applied to one BoneTransform_t.
**/
void ComputeBoneFlexion(vr::VRBoneTransform_t* bone_transform, float transform, int bone_index, bool right_hand);

/**
*Computes the bone transforms for the abduction and adduction of fingers. Applied to one BoneTransform_t.
**/
void ComputeBoneSplay(vr::VRBoneTransform_t bone_transform, float transform, int bone_index, bool right_hand);

/**
* Computes bone transforms for the entire hand
**/
void ComputeEntireHand(vr::VRBoneTransform_t bone_transforms[NUM_BONES], float flexion[5], float splay[5], bool right_hand);

/**
*Linear interpolation between a and b.
**/
float Lerp(float a, float b, float f);

/**
*Returns an int for which finger a bone is in
**/
int FingerFromBone(vr::BoneIndex_t bone);

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