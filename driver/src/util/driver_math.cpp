// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "driver_math.h"

vr::HmdQuaternion_t EulerToQuaternion(const double& yaw, const double& pitch, const double& roll) {
  const double cy = cos(yaw * 0.5);
  const double sy = sin(yaw * 0.5);
  const double cp = cos(pitch * 0.5);
  const double sp = sin(pitch * 0.5);
  const double cr = cos(roll * 0.5);
  const double sr = sin(roll * 0.5);

  vr::HmdQuaternion_t q{};
  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;

  return q;
}

vr::HmdVector3d_t QuaternionToEuler(const vr::HmdQuaternion_t& q) {
  vr::HmdVector3d_t result{};

  // roll (x-axis rotation)
  const double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
  const double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
  result.v[2] = std::atan2(sinr_cosp, cosr_cosp);

  // pitch (y-axis rotation)
  const double sinp = 2 * (q.w * q.y - q.z * q.x);
  if (std::abs(sinp) >= 1)
    result.v[1] = std::copysign(M_PI / 2, sinp);  // use 90 degrees if out of range
  else
    result.v[1] = std::asin(sinp);

  // yaw (z-axis rotation)
  const double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
  const double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
  result.v[0] = std::atan2(siny_cosp, cosy_cosp);

  return result;
}

vr::HmdVector3d_t MatrixToPosition(const vr::HmdMatrix34_t& matrix) {
  return {matrix.m[0][3], matrix.m[1][3], matrix.m[2][3]};
}

vr::HmdQuaternion_t MatrixToOrientation(const vr::HmdMatrix34_t& matrix) {
  vr::HmdQuaternion_t q{};

  q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
  q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
  q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
  q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;

  q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
  q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
  q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);

  return q;
}

vr::HmdQuaternion_t operator-(const vr::HmdQuaternion_t& q) {
  return {q.w, -q.x, -q.y, -q.z};
}

vr::HmdQuaternion_t operator*(const vr::HmdQuaternion_t& q1, const vr::HmdQuaternion_t& q2) {
  return {
      -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w,
      q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x,
      -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y,
      q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z,
  };
}

vr::HmdQuaternionf_t operator*(const vr::HmdQuaternionf_t& q1, const vr::HmdQuaternion_t& q2) {
  return {
      static_cast<float>(-q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w),
      static_cast<float>(q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x),
      static_cast<float>(-q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y),
      static_cast<float>(q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z),
  };
}

vr::HmdVector3_t operator+(const vr::HmdMatrix34_t& matrix, const vr::HmdVector3_t& vec) {
  vr::HmdVector3_t vector{};

  vector.v[0] = matrix.m[0][3] + vec.v[0];
  vector.v[1] = matrix.m[1][3] + vec.v[1];
  vector.v[2] = matrix.m[2][3] + vec.v[2];

  return vector;
}

vr::HmdVector3_t operator*(const vr::HmdMatrix33_t& matrix, const vr::HmdVector3_t& vec) {
  vr::HmdVector3_t result{};

  result.v[0] = matrix.m[0][0] * vec.v[0] + matrix.m[0][1] * vec.v[1] + matrix.m[0][2] * vec.v[2];
  result.v[1] = matrix.m[1][0] * vec.v[0] + matrix.m[1][1] * vec.v[1] + matrix.m[1][2] * vec.v[2];
  result.v[2] = matrix.m[2][0] * vec.v[0] + matrix.m[2][1] * vec.v[1] + matrix.m[2][2] * vec.v[2];

  return result;
}

vr::HmdVector3_t operator-(const vr::HmdVector3_t& vec, const vr::HmdMatrix34_t& matrix) {
  return {vec.v[0] - matrix.m[0][3], vec.v[1] - matrix.m[1][3], vec.v[2] - matrix.m[2][3]};
}

vr::HmdVector3d_t operator+(const vr::HmdVector3d_t& vec1, const vr::HmdVector3d_t& vec2) {
  return {vec1.v[0] + vec2.v[0], vec1.v[1] + vec2.v[1], vec1.v[2] + vec2.v[2]};
}
vr::HmdVector3d_t operator-(const vr::HmdVector3d_t& vec1, const vr::HmdVector3d_t& vec2) {
  return {vec1.v[0] - vec2.v[0], vec1.v[1] - vec2.v[1], vec1.v[2] - vec2.v[2]};
}

vr::HmdVector3d_t operator*(const vr::HmdVector3d_t& vec, const vr::HmdQuaternion_t& q) {
  const vr::HmdQuaternion_t qvec = {0.0, vec.v[0], vec.v[1], vec.v[2]};

  const vr::HmdQuaternion_t qResult = (q * qvec) * (-q);

  return {qResult.x, qResult.y, qResult.z};
}

vr::HmdVector3_t operator*(const vr::HmdVector3_t& vec, const vr::HmdQuaternion_t& q) {
  const vr::HmdQuaternion_t qvec = {1.0, vec.v[0], vec.v[1], vec.v[2]};

  const vr::HmdQuaternion_t qResult = q * qvec * -q;

  return {static_cast<float>(qResult.x), static_cast<float>(qResult.y), static_cast<float>(qResult.z)};
}

bool operator==(const vr::HmdVector3d_t& v1, const vr::HmdVector3d_t& v2) {
  return v1.v[0] == v2.v[0] && v1.v[1] == v2.v[1] && v1.v[2] == v2.v[2];
}

bool operator==(const vr::HmdQuaternion_t& q1, const vr::HmdQuaternion_t& q2) {
  return q1.w == q2.w && q1.x == q2.x && q1.y == q2.y && q1.z == q2.z;
}