#define _USE_MATH_DEFINES

#include "openvr_driver.h"
#include <iostream>
#include <cmath>

double DegToRad(int degrees);
double RadToDeg(double rad);

vr::HmdQuaternion_t GetRotation(const vr::HmdMatrix34_t matrix);
vr::HmdQuaternion_t MultiplyQuaternion(const vr::HmdQuaternion_t q, const vr::HmdQuaternion_t r);
vr::HmdQuaternion_t QuaternionFromAngle(const double& xx, const double& yy, const double& zz, const double& a);
vr::HmdMatrix33_t Get33Matrix(const vr::HmdMatrix34_t matrix);
vr::HmdVector3_t MultiplyMatrix(vr::HmdMatrix33_t matrix, const vr::HmdVector3_t vector);