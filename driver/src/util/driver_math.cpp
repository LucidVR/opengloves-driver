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

vr::HmdVector3d_t operator+(const vr::HmdVector3d_t& vec1, const vr::HmdVector3d_t& vec2) {
  return {vec1.v[0] + vec2.v[0], vec1.v[1] + vec2.v[1], vec1.v[2] + vec2.v[2]};
}
vr::HmdVector3d_t operator-(const vr::HmdVector3d_t& vec1, const vr::HmdVector3d_t& vec2) {
  return {vec1.v[0] - vec2.v[0], vec1.v[1] - vec2.v[1], vec1.v[2] - vec2.v[2]};
}

vr::HmdVector3d_t operator*(const vr::HmdVector3d_t& vec, const vr::HmdQuaternion_t& q) {
  const vr::HmdQuaternion_t qvec = {1.0, vec.v[0], vec.v[1], vec.v[2]};

  const vr::HmdQuaternion_t qResult = q * qvec * -q;

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