// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <cmath>

#include "openvr_driver.h"

#define DEG_TO_RAD(degrees) ((degrees)*M_PI / 180.0)
#define RAD_TO_DEG(radians) ((radians)*180.0 / M_PI)

vr::HmdQuaternion_t EulerToQuaternion(const double& yaw, const double& pitch, const double& roll);
vr::HmdVector3d_t QuaternionToEuler(const vr::HmdQuaternion_t& q);

vr::HmdVector3d_t MatrixToPosition(const vr::HmdMatrix34_t& matrix);
vr::HmdQuaternion_t MatrixToOrientation(const vr::HmdMatrix34_t& matrix);

vr::HmdQuaternion_t operator-(const vr::HmdQuaternion_t& q);
vr::HmdQuaternion_t operator*(const vr::HmdQuaternion_t& q1, const vr::HmdQuaternion_t& q2);
vr::HmdQuaternionf_t operator*(const vr::HmdQuaternionf_t& q1, const vr::HmdQuaternion_t& q2);
vr::HmdVector3_t operator+(const vr::HmdMatrix34_t& matrix, const vr::HmdVector3_t& vec);
vr::HmdVector3_t operator*(const vr::HmdMatrix33_t& matrix, const vr::HmdVector3_t& vec);
vr::HmdVector3_t operator-(const vr::HmdVector3_t& vec, const vr::HmdMatrix34_t& matrix);
vr::HmdVector3d_t operator+(const vr::HmdVector3d_t& vec1, const vr::HmdVector3d_t& vec2);
vr::HmdVector3d_t operator-(const vr::HmdVector3d_t& vec1, const vr::HmdVector3d_t& vec2);
vr::HmdVector3d_t operator*(const vr::HmdVector3d_t& vec, const vr::HmdQuaternion_t& q);
vr::HmdVector3_t operator*(const vr::HmdVector3_t& vec, const vr::HmdQuaternion_t& q);