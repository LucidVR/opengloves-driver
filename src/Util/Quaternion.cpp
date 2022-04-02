#define _USE_MATH_DEFINES

#include "Util/Quaternion.h"

#include <cmath>

double DegToRad(const double degrees) {
  return degrees * M_PI / 180.0;
}
float DegToRad(const float degrees) {
  return degrees * M_PI / 180.0;
}

double RadToDeg(const double rad) {
  return rad * 180.0 / M_PI;
}

float RadToDeg(const float rad) {
  return rad * 180.0 / M_PI;
}

vr::HmdVector3_t GetPosition(const vr::HmdMatrix34_t& matrix) {
  vr::HmdVector3_t vector{};

  vector.v[0] = matrix.m[0][3];
  vector.v[1] = matrix.m[1][3];
  vector.v[2] = matrix.m[2][3];

  return vector;
}

vr::HmdQuaternion_t GetRotation(const vr::HmdMatrix34_t& matrix) {
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

vr::HmdMatrix33_t GetRotationMatrix(const vr::HmdMatrix34_t& matrix) {
  return {
      {{matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]},
       {matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]},
       {matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]}}};
}

vr::HmdMatrix33_t QuaternionToMatrix(const vr::HmdQuaternion_t& q) {
  return {
      {{static_cast<float>(1 - 2 * q.y * q.y - 2 * q.z * q.z),
        static_cast<float>(2 * q.x * q.y - 2 * q.z * q.w),
        static_cast<float>(2 * q.x * q.z + 2 * q.y * q.w)},
       {static_cast<float>(2 * q.x * q.y + 2 * q.z * q.w),
        static_cast<float>(1 - 2 * q.x * q.x - 2 * q.z * q.z),
        static_cast<float>(2 * q.y * q.z - 2 * q.x * q.w)},
       {static_cast<float>(2 * q.x * q.z - 2 * q.y * q.w),
        static_cast<float>(2 * q.y * q.z + 2 * q.x * q.w),
        static_cast<float>(1 - 2 * q.x * q.x - 2 * q.y * q.y)}}};
}

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

vr::HmdVector3_t QuaternionToEuler(const vr::HmdQuaternion_t& q) {
  vr::HmdVector3_t result;

  // roll (x-axis rotation)
  double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
  double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
  result.v[2] = std::atan2(sinr_cosp, cosr_cosp);

  // pitch (y-axis rotation)
  double sinp = 2 * (q.w * q.y - q.z * q.x);
  if (std::abs(sinp) >= 1)
    result.v[1] = std::copysign(M_PI / 2, sinp);  // use 90 degrees if out of range
  else
    result.v[1] = std::asin(sinp);

  // yaw (z-axis rotation)
  double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
  double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
  result.v[0] = std::atan2(siny_cosp, cosy_cosp);

  return result;
}

vr::HmdQuaternion_t operator-(const vr::HmdQuaternion_t& q) {
  return {q.w, -q.x, -q.y, -q.z};
}

vr::HmdQuaternion_t operator*(const vr::HmdQuaternion_t& q, const vr::HmdQuaternion_t& r) {
  vr::HmdQuaternion_t result{};

  result.w = r.w * q.w - r.x * q.x - r.y * q.y - r.z * q.z;
  result.x = r.w * q.x + r.x * q.w - r.y * q.z + r.z * q.y;
  result.y = r.w * q.y + r.x * q.z + r.y * q.w - r.z * q.x;
  result.z = r.w * q.z - r.x * q.y + r.y * q.x + r.z * q.w;

  return result;
}

vr::HmdQuaternionf_t operator*(const vr::HmdQuaternionf_t& q, const vr::HmdQuaternion_t& r) {
  vr::HmdQuaternionf_t result{};

  result.w = r.w * q.w - r.x * q.x - r.y * q.y - r.z * q.z;
  result.x = r.w * q.x + r.x * q.w - r.y * q.z + r.z * q.y;
  result.y = r.w * q.y + r.x * q.z + r.y * q.w - r.z * q.x;
  result.z = r.w * q.z - r.x * q.y + r.y * q.x + r.z * q.w;

  return result;
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

vr::HmdVector3_t operator+(const vr::HmdVector3_t& vec1, const vr::HmdVector3_t& vec2) {
  return {vec1.v[0] + vec2.v[0], vec1.v[1] + vec2.v[1], vec1.v[2] + vec2.v[2]};
}

vr::HmdVector3_t operator*(const vr::HmdVector3_t& vec, const vr::HmdQuaternion_t& q) {
  const vr::HmdQuaternion_t qVec = {1.0, vec.v[0], vec.v[1], vec.v[2]};

  const vr::HmdQuaternion_t qResult = q * qVec * -q;

  return {static_cast<float>(qResult.x), static_cast<float>(qResult.y), static_cast<float>(qResult.z)};
}