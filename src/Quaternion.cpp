#include "Quaternion.h"
#include <cmath>

double DegToRad(int degrees) {
	return degrees * M_PI / 180;
}
double RadToDeg(double rad) {
	return rad * 180 / M_PI;
}

vr::HmdVector3_t GetPosition(const vr::HmdMatrix34_t& matrix) {
	vr::HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3];
	vector.v[1] = matrix.m[1][3];
	vector.v[2] = matrix.m[2][3];

	return vector;
}

vr::HmdVector3_t CombinePosition(const vr::HmdMatrix34_t& matrix, const vr::HmdVector3_t& vec) {
	vr::HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3] + vec.v[0];
	vector.v[1] = matrix.m[1][3] + vec.v[1];
	vector.v[2] = matrix.m[2][3] + vec.v[2];

	return vector;
}

vr::HmdQuaternion_t GetRotation(const vr::HmdMatrix34_t& matrix) {
	vr::HmdQuaternion_t q;

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
	vr::HmdMatrix33_t result = { {
		{matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]},
		{matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]},
		{matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]}
		} };

	return result;
}
vr::HmdVector3_t MultiplyMatrix(const vr::HmdMatrix33_t& matrix, const vr::HmdVector3_t& vector) {
	vr::HmdVector3_t result;

	result.v[0] = matrix.m[0][0] * vector.v[0] + matrix.m[0][1] * vector.v[1] + matrix.m[0][2] * vector.v[2];
	result.v[1] = matrix.m[1][0] * vector.v[0] + matrix.m[1][1] * vector.v[1] + matrix.m[1][2] * vector.v[2];
	result.v[2] = matrix.m[2][0] * vector.v[0] + matrix.m[2][1] * vector.v[1] + matrix.m[2][2] * vector.v[2];

	return result;
}

vr::HmdQuaternion_t QuaternionFromAngle(const double& xx, const double& yy, const double& zz, const double& a) {
	double factor = sin(a / 2.0);

	// Calculate the x, y and z of the quaternion
	double x = xx * factor;
	double y = yy * factor;
	double z = zz * factor;

	double w = cos(a / 2.0);

	double n = std::sqrt(x * x + y * y + z * z + w * w);
	x /= n;
	y /= n;
	z /= n;
	w /= n;

	vr::HmdQuaternion_t quat = { w,x,y,z };

	return quat;
}

vr::HmdMatrix33_t QuaternionToMatrix(const vr::HmdQuaternion_t q) {
	
	vr::HmdMatrix33_t result = { {
		{1 - 2*q.y*q.y - 2*q.z*q.z, 2*q.x*q.y - 2*q.z*q.w, 2*q.x*q.z + 2*q.y*q.w},
		{2*q.x*q.y + 2*q.z*q.w, 1 - 2*q.x*q.x - 2 * q.z*q.z, 2*q.y*q.z - 2*q.x*q.w},
		{2*q.x*q.z - 2*q.y*q.w, 2*q.y*q.z + 2*q.x*q.w, 1 - 2*q.x*q.x - 2*q.y*q.y}
		} };

	return result;
}

double QuatNorm(const vr::HmdQuaternion_t q) {
	return sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

vr::HmdQuaternion_t QuatConjugate(const vr::HmdQuaternion_t q) {
	vr::HmdQuaternion_t quat = { q.w,-q.x,-q.y,-q.z };
	return quat;
}

vr::HmdQuaternion_t MultiplyQuaternion(const vr::HmdQuaternion_t& q, const vr::HmdQuaternion_t& r) {
	vr::HmdQuaternion_t result;

	result.w = (r.w * q.w - r.x * q.x - r.y * q.y - r.z * q.z);
	result.x = (r.w * q.x + r.x * q.w - r.y * q.z + r.z * q.y);
	result.y = (r.w * q.y + r.x * q.z + r.y * q.w - r.z * q.x);
	result.z = (r.w * q.z - r.x * q.y + r.y * q.x + r.z * q.w);

	return result;
}

vr::HmdQuaternion_t EulerToQuaternion(const double& yaw, const double& pitch, const double& roll) {
	// Abbreviations for the various angular functions
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);
	double cp = cos(pitch * 0.5);
	double sp = sin(pitch * 0.5);
	double cr = cos(roll * 0.5);
	double sr = sin(roll * 0.5);

	vr::HmdQuaternion_t q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;

	return q;
}
vr::HmdVector3_t QuaternionToEuler(vr::HmdQuaternion_t q) {
	vr::HmdVector3_t angles;

	// roll (x-axis rotation)
	double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.v[0] = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = 2 * (q.w * q.y - q.z * q.x);
	if (std::abs(sinp) >= 1)
		angles.v[1] = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
	else
		angles.v[1] = std::asin(sinp);

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.v[2] = std::atan2(siny_cosp, cosy_cosp);
	vr::HmdVector3_t result = { RadToDeg(angles.v[2]), RadToDeg(angles.v[1]), RadToDeg(angles.v[0]) };
	return result;
}