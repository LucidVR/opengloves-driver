#pragma once
#include <openvr_driver.h>
#include <array>

const int NUM_ANIMATION_FRAMES = 2;
const int KEYFRAME_OPEN = 0;
const int KEYFRAME_CLOSED = NUM_ANIMATION_FRAMES - 1;

const int NUM_BONES = 31;

extern float animationFrameTimes[NUM_ANIMATION_FRAMES];
extern vr::VRBoneTransform_t rightAnimationFrames[NUM_ANIMATION_FRAMES][NUM_BONES];
extern vr::VRBoneTransform_t leftAnimationFrames[NUM_ANIMATION_FRAMES][NUM_BONES];

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

void ComputeHand(vr::VRBoneTransform_t* skeleton, const std::array<float, 5>& flexion, bool isRightHand);