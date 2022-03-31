#include "openvr_driver.h"

double DegToRad(double degrees);
float DegToRad(const float degrees);
double RadToDeg(double rad);
float RadToDeg(const float rad);

namespace vr {
  struct HmdAxisAngle_t {
    double angle;
    vr::HmdVector3_t axis;
  };
}  // namespace vr

// get the quaternion for rotation from a matrix
vr::HmdQuaternion_t GetRotation(const vr::HmdMatrix34_t& matrix);
vr::HmdVector3_t GetPosition(const vr::HmdMatrix34_t& matrix);

vr::HmdQuaternion_t EulerToQuaternion(const double& yaw, const double& pitch, const double& roll);
vr::HmdVector3_t QuaternionToEuler(const vr::HmdQuaternion_t& q);

vr::HmdMatrix33_t GetRotationMatrix(const vr::HmdMatrix34_t& matrix);

vr::HmdMatrix33_t QuaternionToMatrix(const vr::HmdQuaternion_t& q);

double Dot(const vr::HmdVector3_t& vec1, const vr::HmdVector3_t& vec2);
vr::HmdVector3_t Cross(const vr::HmdVector3_t& vec1, const vr::HmdVector3_t& vec2);
double Magnitude(const vr::HmdVector3_t& vec);
vr::HmdVector3_t Normalise(const vr::HmdVector3_t& vec);

//calculates the axis angle between two vectors
vr::HmdAxisAngle_t VectorsToAxisAngle(const vr::HmdVector3_t& vec1, const vr::HmdVector3_t& vec2);

vr::HmdQuaternion_t AxisAngleToQuaternion(const vr::HmdAxisAngle_t& axisAngle);

vr::HmdQuaternion_t operator-(const vr::HmdQuaternion_t& q);

vr::HmdQuaternion_t operator*(const vr::HmdQuaternion_t& q, const vr::HmdQuaternion_t& r);
vr::HmdQuaternionf_t operator*(const vr::HmdQuaternionf_t& q, const vr::HmdQuaternion_t& r);
vr::HmdVector3_t operator+(const vr::HmdMatrix34_t& matrix, const vr::HmdVector3_t& vec);
vr::HmdVector3_t operator*(const vr::HmdMatrix33_t& matrix, const vr::HmdVector3_t& vec);
vr::HmdVector3_t operator-(const vr::HmdVector3_t& vec, const vr::HmdMatrix34_t& matrix);
vr::HmdVector3_t operator+(const vr::HmdVector3_t& vec1, const vr::HmdVector3_t& vec2);