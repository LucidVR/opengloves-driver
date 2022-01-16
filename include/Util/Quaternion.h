#define _USE_MATH_DEFINES

#include "openvr_driver.h"

double DegToRad(double degrees);
double RadToDeg(double rad);

// get the quaternion for rotation from a matrix
vr::HmdQuaternion_t GetRotation(const vr::HmdMatrix34_t& matrix);
vr::HmdVector3_t GetPosition(const vr::HmdMatrix34_t& matrix);
vr::HmdVector3_t CombinePosition(const vr::HmdMatrix34_t& matrix, const vr::HmdVector3_t& vec);

// returns the result of multiplying two quaternions, effectively applying a rotation on a quaternion
vr::HmdQuaternion_t MultiplyQuaternion(const vr::HmdQuaternion_t& q, const vr::HmdQuaternion_t& r);
vr::HmdQuaternionf_t MultiplyQuaternion(const vr::HmdQuaternionf_t& q, const vr::HmdQuaternionf_t& r);

vr::HmdQuaternionf_t EulerToQuaternion(const float& yaw, const float& pitch, const float& roll);
vr::HmdQuaternion_t EulerToQuaternion(const double& yaw, const double& pitch, const double& roll);

vr::HmdMatrix33_t GetRotationMatrix(const vr::HmdMatrix34_t& matrix);

vr::HmdVector3_t MultiplyMatrix(const vr::HmdMatrix33_t& matrix, const vr::HmdVector3_t& vector);

vr::HmdMatrix33_t QuaternionToMatrix(const vr::HmdQuaternion_t& q);

vr::HmdQuaternion_t QuatConjugate(const vr::HmdQuaternion_t& q);

vr::HmdVector3_t QuaternionToEuler(const vr::HmdQuaternion_t& q);