#pragma once

#include <RayFlow/rayflow.h>
#include <RayFlow/Util/matrix.h>
#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Core/ray.h>

namespace rayflow {

class Transform {
public:
	RAYFLOW_CPU_GPU Transform() = default;
	RAYFLOW_CPU_GPU Transform(const MatrixNxN<4>& mat, const MatrixNxN<4>& matInv) : m(mat), mInv(matInv) {}
	RAYFLOW_CPU_GPU explicit Transform(const MatrixNxN<4>& mat) : m(mat) {
		auto inv = Inverse(m);
		if (inv.has_value()) {
			mInv = inv.value();
		}
		else {
			Float NaN = std::numeric_limits<Float>::has_signaling_NaN
				? std::numeric_limits<Float>::signaling_NaN()
				: std::numeric_limits<Float>::quiet_NaN();
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mInv.m[i][j] = NaN;
				}
			}
		}
	}
	RAYFLOW_CPU_GPU Transform(const Float mat[4][4]) : Transform(MatrixNxN<4>(mat)) {}
	RAYFLOW_CPU_GPU explicit Transform(const Quaternion& q) {
		Float xx = q.v.x * q.v.x, yy = q.v.y * q.v.y, zz = q.v.z * q.v.z;
		Float xy = q.v.x * q.v.y, xz = q.v.x * q.v.z, yz = q.v.y * q.v.z;
		Float wx = q.v.x * q.w, wy = q.v.y * q.w, wz = q.v.z * q.w;

		m.m[0][0] = 1 - 2 * (yy + zz);
		m.m[0][1] = 2 * (xy - wz);
		m.m[0][2] = 2 * (xz + wy);
		m.m[1][0] = 2 * (xy + wz);
		m.m[1][1] = 1 - 2 * (xx + zz);
		m.m[1][2] = 2 * (yz - wx);
		m.m[2][0] = 2 * (xz - wy);
		m.m[2][1] = 2 * (yz + wx);
		m.m[2][2] = 1 - 2 * (xx + yy);

		mInv = Transpose(m);
	}
	// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/transform.cpp
	RAYFLOW_CPU_GPU operator Quaternion() const {
		Float trace = m.m[0][0] + m.m[1][1] + m.m[2][2];
		Quaternion quat;
		if (trace > 0.f) {
			// Compute w from matrix trace, then xyz
			// 4w^2 = m[0][0] + m[1][1] + m[2][2] + m[3][3] (but m[3][3] == 1)
			Float s = std::sqrt(trace + 1.0f);
			quat.w = s / 2.0f;
			s = 0.5f / s;
			quat.v.x = (m.m[2][1] - m.m[1][2]) * s;
			quat.v.y = (m.m[0][2] - m.m[2][0]) * s;
			quat.v.z = (m.m[1][0] - m.m[0][1]) * s;
		}
		else {
			// Compute largest of $x$, $y$, or $z$, then remaining components
			const int nxt[3] = { 1, 2, 0 };
			Float q[3];
			int i = 0;
			if (m.m[1][1] > m.m[0][0])
				i = 1;
			if (m.m[2][2] > m.m[i][i])
				i = 2;
			int j = nxt[i];
			int k = nxt[j];
			Float s = SafeSqrt((m.m[i][i] - (m.m[j][j] + m.m[k][k])) + 1.0f);
			q[i] = s * 0.5f;
			if (s != 0.f)
				s = 0.5f / s;
			quat.w = (m.m[k][j] - m.m[j][k]) * s;
			q[j] = (m.m[j][i] + m.m[i][j]) * s;
			q[k] = (m.m[k][i] + m.m[i][k]) * s;
			quat.v.x = q[0];
			quat.v.y = q[1];
			quat.v.z = q[2];
		}
		return quat;
	}

	RAYFLOW_CPU_GPU bool operator==(const Transform& trans) const { return m == trans.m; }
	RAYFLOW_CPU_GPU bool operator!=(const Transform& trans) const { return m != trans.m; }

	RAYFLOW_CPU_GPU Transform operator*(const Transform& other) const {
		return Transform(m * other.m, other.mInv * mInv);
	}

	RAYFLOW_CPU_GPU Point3 operator()(const Point3& p) const {
		Float x = (m.m[0][0] * p.x + m.m[0][1] * p.y) + (m.m[0][2] * p.z + m.m[0][3]);
		Float y = (m.m[1][0] * p.x + m.m[1][1] * p.y) + (m.m[1][2] * p.z + m.m[1][3]);
		Float z = (m.m[2][0] * p.x + m.m[2][1] * p.y) + (m.m[2][2] * p.z + m.m[2][3]);
		Float w = (m.m[3][0] * p.x + m.m[3][1] * p.y) + (m.m[3][2] * p.z + m.m[3][3]);
		Point3 result(x, y, z);
		return result / w;
	}

	RAYFLOW_CPU_GPU Vector3 operator()(const Vector3& v) const {
		return Vector3(m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
			           m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
			           m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z);
	}

	RAYFLOW_CPU_GPU Normal3 operator()(const Normal3& n) const {
		return Normal3(mInv.m[0][0] * n.x + mInv.m[1][0] * n.y + mInv.m[2][0] * n.z,
			           mInv.m[0][1] * n.x + mInv.m[1][1] * n.y + mInv.m[2][1] * n.z,
			           mInv.m[0][2] * n.x + mInv.m[1][2] * n.y + mInv.m[2][2] * n.z);
	}

	RAYFLOW_CPU_GPU Ray operator()(const Ray& ray) const {
		return Ray((*this)(ray.o), (*this)(ray.d));
	}

	RAYFLOW_CPU_GPU AABB3 operator()(const AABB3& box) const {
		AABB3 result;
		for (int i = 0; i < 8; ++i) {
			result = Union(result, (*this)(box.Corner(i)));
		}
		return result;
	}

	MatrixNxN<4> m;
	MatrixNxN<4> mInv;
};

RAYFLOW_CPU_GPU inline Transform Inverse(const Transform& trans) {
	return { trans.mInv, trans.m };
}
// right hand
RAYFLOW_CPU_GPU inline Transform Rotate(const Vector3& axis, Float theta) {
	Float sinTheta = std::sin(theta);
	Float cosTheta = std::cos(theta);
	Vector3 a = Normalize(axis);
	Float xx = a.x * a.x;
	Float yy = a.y * a.y;
	Float zz = a.z * a.z;
	Float xy = a.x * a.y;
	Float xz = a.x * a.z;
	Float yz = a.y * a.z;
	MatrixNxN<4> m(cosTheta + (1 - cosTheta) * xx, (1 - cosTheta) * xy - sinTheta * a.z, (1 - cosTheta) * xz + sinTheta * a.y, 0,
		           (1 - cosTheta) * xy + sinTheta * a.z, cosTheta + (1 - cosTheta) * yy, (1 - cosTheta) * yz - sinTheta * a.x, 0,
		           (1 - cosTheta) * xz - sinTheta * a.y, (1 - cosTheta) * yz + sinTheta * a.z, cosTheta + ( 1- cosTheta) * zz, 0,
		            0, 0, 0, 1);
	return Transform(m, Transpose(m));
}

RAYFLOW_CPU_GPU inline Transform Translate(const Vector3& offset) {
	MatrixNxN<4> m(1, 0, 0, offset.x,
		           0, 1, 0, offset.y,
		           0, 0, 1, offset.z,
		           0, 0, 0, 1);
	MatrixNxN<4> mInv(1, 0, 0, -offset.x,
					  0, 1, 0, -offset.y,
					  0, 0, 1, -offset.z,
					  0, 0, 0, 1);

	return Transform(m, mInv);
}

RAYFLOW_CPU_GPU inline Transform Scale(const Vector3& s) {
	MatrixNxN<4> m(s.x, 0, 0, 0,
		           0, s.y, 0, 0,
		           0, 0, s.z, 0,
		           0, 0, 0, 1);
	MatrixNxN<4> mInv(1 / s.x, 0, 0, 0,
		              0, 1 / s.y, 0, 0,
		              0, 0, 1 / s.z, 0,
		              0, 0, 0, 1);
	return Transform(m, mInv);
}

RAYFLOW_CPU_GPU inline Transform LookAt(const Point3& p, const Point3& target, const Vector3& up) {
	Vector3 out = Normalize(target - p);
	Vector3 normUp = Normalize(up);
	Vector3 left = Normalize(Cross(normUp, out));
	normUp = Normalize(Cross(out, left));
	MatrixNxN<4> mInv(left.x, left.y, left.z, p.x,
		           normUp.x, normUp.y, normUp.z, p.y,
		           out.x, out.y, out.z, p.z,
		           0, 0, 0, 1);
	return Transform(*(Inverse(mInv)), mInv);
}

RAYFLOW_CPU_GPU inline Transform Orthographic(Float zNear, Float zFar) {
	MatrixNxN<4> m(1, 0, 0, 0,
		           0, 1, 0, 0,
		           0, 0, 1 / (zFar - zNear), -zNear / (zFar - zNear),
		           0, 0, 0, 1);
	MatrixNxN<4> mInv(1, 0, 0 , 0,
		              0, 1, 0, 0,
		              0, 0, zFar - zNear, zNear,
		              0, 0, 0, 1);
	return Transform(m, mInv);
}

RAYFLOW_CPU_GPU inline Transform Perspective(Float fov, Float zNear, Float zFar) {
	Float tanHalfFov = std::tan(Radians(fov) / 2);
	MatrixNxN<4> m(1 / tanHalfFov, 0, 0, 0,
		           0, 1 / tanHalfFov, 0, 0,
		           0, 0, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
		           0, 0, 1, 0);
	MatrixNxN<4> mInv(tanHalfFov, 0, 0, 0,
		              0, tanHalfFov, 0, 0,
		              0, 0, 0, 1,
		              0, 0, (zNear - zFar) / (zNear * zFar), 1 / zNear);
	return Transform(m, mInv);
}

}