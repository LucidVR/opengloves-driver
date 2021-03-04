#pragma once
#include <openvr_driver.h>

const int NUM_BONES = 31;
extern vr::VRBoneTransform_t rightOpenPose[NUM_BONES];
extern vr::VRBoneTransform_t rightFistPose[NUM_BONES];

extern vr::VRBoneTransform_t leftOpenPose[NUM_BONES];
extern vr::VRBoneTransform_t leftFistPose[NUM_BONES];

void ComputeBoneTransform(vr::VRBoneTransform_t bone_transform[NUM_BONES], const float transform, const int startBoneIndex, const bool isRightHand);

/**
*Linear interpolation between a and b.
**/
float Lerp(const float a, const float b, const float f);