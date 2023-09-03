#pragma once


#include <RayFlow/rayflow.h>
#include <RayFlow/Util/math.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <string>

namespace rayflow {

template <typename T>
struct TupleLength {
	using type = Float;
};

template <>
struct TupleLength<double> {
	using type = double;
};

template <>
struct TupleLength<long double> {
	using type = long double;
};

template <template <typename> class Derived, typename T>
class Tuple2 {
public:
	RAYFLOW_CPU_GPU Tuple2() = default;
	RAYFLOW_CPU_GPU Tuple2(T x, T y) : x(x), y(y) {}
	
	RAYFLOW_CPU_GPU bool operator==(const Derived<T>& d) const { return x == d.x && y == d.y; }
	RAYFLOW_CPU_GPU bool operator!=(const Derived<T>& d) const { return x != d.x || y != d.y; }

	RAYFLOW_CPU_GPU Derived<T> operator-() const {
		return { -x, -y };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator+(const Derived<U>& d) const ->Derived<decltype(T{} + U{})> {
		return { x + d.x, y + d.y };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator-(const Derived<U>& d) const ->Derived<decltype(T{} - U{})> {
		return { x - d.x, y - d.y };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator*(U u) const -> Derived<decltype(T{} * U{})> {
		return { u * x, u * y };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator/(U u) const -> Derived<decltype(T{} / U{})> {
		return { x / u, y / u };
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator+=(const Derived<U>& d) {
		x += d.x;
		y += d.y;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator-=(const Derived<U>& d) {
		x -= d.x;
		y -= d.y;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator*=(U u) {
		x *= u;
		y *= u;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator/=(U u) {
		x /= u;
		y /= u;
		return static_cast<Derived<T>&>(*this);
	}

	RAYFLOW_CPU_GPU T operator[](int i) const {
		return (i == 0) ? x : y;
	}

	RAYFLOW_CPU_GPU T& operator[](int i) {
		return (i == 0) ? x : y;
	}

	T x{};
	T y{};
};

template <template <typename> class Derived, typename T, typename U>
RAYFLOW_CPU_GPU auto operator*(U u, const Tuple2<Derived, T>& tup)->Derived<decltype(T{} * U{})> {
	return tup * u;
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Abs(const Tuple2<Derived, T>& tup) {
	using std::abs;
	return { abs(tup.x), abs(tup.y) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Min(const Tuple2<Derived, T>& lhs, const Tuple2<Derived, T>& rhs) {
	using std::min;
	return { min(lhs.x, rhs.x), min(lhs.y, rhs.y) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Max(const Tuple2<Derived, T>& lhs, const Tuple2<Derived, T>& rhs) {
	using std::max;
	return { max(lhs.x, rhs.x), max(lhs.y, rhs.y) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline T MinComponentVal(const Tuple2<Derived, T>& tup) {
	using std::min;
	return min(tup.x, tup.y);
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline T MinComponentIndex(const Tuple2<Derived, T>& tup) {
	return tup.x < tup.y ? 0 : 1;
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline T MaxComponentVal(const Tuple2<Derived, T>& tup) {
	using std::max;
	return max(tup.x, tup.y);
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline T MaxComponentIndex(const Tuple2<Derived, T>& tup) {
	return tup.x < tup.y ? 1 : 0;
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Lerp(const Tuple2<Derived, T>& lhs, const Tuple2<Derived, T>& rhs, Float t) {
	return (1.0 - t) * lhs + t * rhs;
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Permute(const Tuple2<Derived, T>& tup, int ix, int iy) {
	return { tup[ix], tup[iy] };
}

template <template <typename> class Derived, typename T>
class Tuple3 {
public:
	RAYFLOW_CPU_GPU Tuple3() = default;
	RAYFLOW_CPU_GPU Tuple3(T x, T y, T z) : x(x), y(y), z(z) {}

	RAYFLOW_CPU_GPU bool operator==(const Derived<T>& d) const { return x == d.x && y == d.y && z == d.z; }
	RAYFLOW_CPU_GPU bool operator!=(const Derived<T>& d) const { return x != d.x || y != d.y || z != d.z; }

	RAYFLOW_CPU_GPU Derived<T> operator-() const {
		return { -x, -y, -z };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator+(const Derived<U>& d) const->Derived<decltype(T{} + U{}) > {
		return { x + d.x, y + d.y, z + d.z };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator-(const Derived<U>& d) const->Derived<decltype(T{} - U{}) > {
		return { x - d.x, y - d.y, z - d.z };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator*(U u) const -> Derived<decltype(T{} *U{}) > {
		return { u * x, u * y , u * z };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator/(U u) const -> Derived<decltype(T{} / U{}) > {
		return { x / u, y / u, z / u };
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator+=(const Derived<U>& d) {
		x += d.x;
		y += d.y;
		z += d.z;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator-=(const Derived<U>& d) {
		x -= d.x;
		y -= d.y;
		z -= d.z;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator*=(U u) {
		x *= u;
		y *= u;
		z *= u;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator/=(U u) {
		x /= u;
		y /= u;
		z /= u;
		return static_cast<Derived<T>&>(*this);
	}

	RAYFLOW_CPU_GPU T operator[](int i) const {
		if (i == 0) {
			return x;
		}
		else if (i == 1) {
			return y;
		}
		return z;
	}

	RAYFLOW_CPU_GPU T& operator[](int i) {
		if (i == 0) {
			return x;
		}
		else if (i == 1) {
			return y;
		}
		return z;
	}

	T x{};
	T y{};
	T z{};
};

template <template <typename> class Derived, typename T, typename U>
RAYFLOW_CPU_GPU inline auto operator*(U u, const Tuple3<Derived, T>& tup)->Derived<decltype(T{} * U{}) > {
	return tup * u;
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Abs(const Tuple3<Derived, T>& tup) {
	using std::abs;
	return { abs(tup.x), abs(tup.y), abs(tup.z) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Min(const Tuple3<Derived, T>& lhs, const Tuple3<Derived, T>& rhs) {
	using std::min;
	return { min(lhs.x, rhs.x), min(lhs.y, rhs.y), min(lhs.z, rhs.z) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Max(const Tuple3<Derived, T>& lhs, const Tuple3<Derived, T>& rhs) {
	using std::max;
	return { max(lhs.x, rhs.x), max(lhs.y, rhs.y), max(lhs.z, rhs.z) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline T MinComponentVal(const Tuple3<Derived, T>& tup) {
	using std::min;
	return min(min(tup.x, tup.y), tup.z);
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline T MinComponentIndex(const Tuple3<Derived, T>& tup) {
	using std::min;
	return (tup.x < tup.y) ? (tup.x < tup.z ? 0 : 2) : (tup.y < tup.z ? 1 : 2);
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline T MaxComponentVal(const Tuple3<Derived, T>& tup) {
	using std::max;
	return max(max(tup.x, tup.y), tup.z);
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline T MaxComponentIndex(const Tuple3<Derived, T>& tup) {
	return (tup.x > tup.y) ? (tup.x > tup.z ? 0 : 2) : (tup.y > tup.z ? 1 : 2);
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Lerp(const Tuple3<Derived, T>& lhs, const Tuple3<Derived, T>& rhs, Float t) {
	return (1.0 - t) * lhs + t * rhs;
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Permute(const Tuple3<Derived, T>& tup, int ix, int iy, int iz) {
	return { tup[ix], tup[iy], tup[iz] };
}

template <template <typename> class Derived, typename T>
class Tuple4 {
public:
	RAYFLOW_CPU_GPU Tuple4() = default;
	RAYFLOW_CPU_GPU Tuple4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

	RAYFLOW_CPU_GPU bool operator==(const Derived<T>& d) const { return x == d.x && y == d.y && z == d.z && w == d.w; }
	RAYFLOW_CPU_GPU bool operator!=(const Derived<T>& d) const { return x != d.x || y != d.y || z != d.z || w != d.w; }

	RAYFLOW_CPU_GPU Derived<T> operator-() const {
		return { -x, -y, -z, -w };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator+(const Derived<U>& d) const->Derived<decltype(T{} + U{}) > {
		return { x + d.x, y + d.y, z + d.z, w + d.w };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator-(const Derived<U>& d) const->Derived<decltype(T{} - U{}) > {
		return { x - d.x, y - d.y, z - d.z, w - d.w };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator*(U u) const -> Derived<decltype(T{} *U{}) > {
		return { u * x, u * y , u * z, u * w };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator/(U u) const -> Derived<decltype(T{} / U{}) > {
		return { x / u, y / u, z / u, w / u };
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator+=(const Derived<U>& d) {
		x += d.x;
		y += d.y;
		z += d.z;
		w += d.w;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator-=(const Derived<U>& d) {
		x -= d.x;
		y -= d.y;
		z -= d.z;
		w -= d.w;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator*=(U u) {
		x *= u;
		y *= u;
		z *= u;
		w *= u;
		return static_cast<Derived<T>&>(*this);
	}

	template <typename U>
	RAYFLOW_CPU_GPU Derived<T>& operator/=(U u) {
		x /= u;
		y /= u;
		z /= u;
		w /= u;
		return static_cast<Derived<T>&>(*this);
	}

	RAYFLOW_CPU_GPU T operator[](int i) const {
		if (i == 0) {
			return x;
		}
		else if (i == 1) {
			return y;
		}
		else if (i == 2) {
			return z;
		}
		return w;
	}

	RAYFLOW_CPU_GPU T& operator[](int i) {
		if (i == 0) {
			return x;
		}
		else if (i == 1) {
			return y;
		}
		else if (i == 2) {
			return z;
		}
		return w;
	}

	T x{};
	T y{};
	T z{};
	T w{};
};

template <template <typename> class Derived, typename T, typename U>
RAYFLOW_CPU_GPU auto operator*(U u, const Tuple4<Derived, T>& tup)->Derived<decltype(T{} *U{}) > {
	return tup * u;
} 

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Abs(const Tuple4<Derived, T>& tup) {
	using std::abs;
	return { abs(tup.x), abs(tup.y), abs(tup.z), abs(tup.w) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Min(const Tuple4<Derived, T>& lhs, const Tuple4<Derived, T>& rhs) {
	using std::min;
	return { min(lhs.x, rhs.x), min(lhs.y, rhs.y), min(lhs.z, rhs.z), min(lhs.w, rhs.w) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Max(const Tuple4<Derived, T>& lhs, const Tuple4<Derived, T>& rhs) {
	using std::max;
	return { max(lhs.x, rhs.x), max(lhs.y, rhs.y), max(lhs.z, rhs.z), max(lhs.w, rhs.w) };
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Lerp(const Tuple4<Derived, T>& lhs, const Tuple4<Derived, T>& rhs, Float t) {
	return (1.0 - t) * lhs + t * rhs;
}

template <template <typename> class Derived, typename T>
RAYFLOW_CPU_GPU inline Derived<T> Permute(const Tuple4<Derived, T>& tup, int ix, int iy, int iz, int iw) {
	return { tup[ix], tup[iy], tup[iz], tup[iw] };
}

template <typename T>
class TVector2 : public Tuple2<TVector2, T> {
public:
	RAYFLOW_CPU_GPU TVector2() = default;
	RAYFLOW_CPU_GPU TVector2(T x, T y) : Tuple2<TVector2, T>(x, y) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TVector2(const TVector2<U>& v) : Tuple2<TVector2, T>(T(v.x), T(v.y)) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TVector2(const TPoint2<U>& p);

};

template <typename T>
class TVector3 : public Tuple3<TVector3, T> {
public:

	RAYFLOW_CPU_GPU TVector3() = default;
	RAYFLOW_CPU_GPU TVector3(T x, T y, T z) : Tuple3<TVector3, T>(x, y, z) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TVector3(const TVector3<U>&v) : Tuple3<TVector3, T>(T(v.x), T(v.y), T(v.z)) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TVector3(const TPoint3<U>&p);
	template <typename U>
	RAYFLOW_CPU_GPU explicit TVector3(const TNormal3<U>& n);
};

template <typename T>
class TPoint2 : public Tuple2<TPoint2, T> {
public:
	using Tuple2<TPoint2, T>::x;
	using Tuple2<TPoint2, T>::y;
	using Tuple2<TPoint2, T>::operator*;
	using Tuple2<TPoint2, T>::operator+;
	using Tuple2<TPoint2, T>::operator+=;

	RAYFLOW_CPU_GPU TPoint2() { x = y = 0; }
	TPoint2(T x, T y) : Tuple2<TPoint2, T>(x, y) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TPoint2(const TPoint2<U>& p) : Tuple2<TPoint2, T>(T(p.x), T(p.y)) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TPoint2(const TVector2<U>& v) : Tuple2<TPoint2, T>(T(v.x), T(v.y)) {}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator+(const TVector2<U>& v) const -> TPoint2<decltype(T{} + U{})> {
		return { x + v.x, y + v.y };
	}

	template <typename U>
	RAYFLOW_CPU_GPU TPoint2<T> &operator+=(const TVector2<U>& v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator-(const TPoint2<U>& p) const->TVector2<decltype(T{} - U{}) > {
		return { x - p.x, y - p.y };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator-(const TVector2<U>& v) const->TPoint2<decltype(T{} - U{}) > {
		return { x - v.x, y - v.y };
	}

	template <typename U>
	RAYFLOW_CPU_GPU TPoint2<T> &operator-=(const TVector2<U>& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
};

template <typename T>
class TPoint3 : public Tuple3<TPoint3, T> {
public:
	using Tuple3<TPoint3, T>::x;
	using Tuple3<TPoint3, T>::y;
	using Tuple3<TPoint3, T>::z;
	using Tuple3<TPoint3, T>::operator+;
	using Tuple3<TPoint3, T>::operator+=;

	RAYFLOW_CPU_GPU TPoint3() { x = y = z = 0; }
	RAYFLOW_CPU_GPU TPoint3(T x, T y, T z) : Tuple3<TPoint3, T>(x, y, z) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TPoint3(const TPoint3<U>& p) : Tuple3<TPoint3, T>(T(p.x), T(p.y), T(p.z)) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TPoint3(const TVector3<U>& v) : Tuple3<TPoint3, T>(T(v.x), T(v.y), T(v.z)) {}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator+(const TVector3<U>& v) const->TPoint3<decltype(T{} + U{}) > {
		return { x + v.x, y + v.y, z + v.z };
	}

	template <typename U>
	RAYFLOW_CPU_GPU TPoint3<T>& operator+=(const TVector3<U>& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator-(const TPoint3<U>& p) const->TVector3<decltype(T{} - U{}) > {
		return { x - p.x, y - p.y, z - p.z };
	}

	template <typename U>
	RAYFLOW_CPU_GPU auto operator-(const TVector3<U>& v) const->TPoint3<decltype(T{} - U{}) > {
		return { x - v.x, y - v.y, z - v.z };
	}

	template <typename U>
	RAYFLOW_CPU_GPU TPoint3<T>& operator-=(const TVector3<U>& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
};

template <typename T>
class TNormal3 : public Tuple3<TNormal3, T> {
public:
	using Tuple3<TNormal3, T>::x;
	using Tuple3<TNormal3, T>::y;
	using Tuple3<TNormal3, T>::z;

	RAYFLOW_CPU_GPU TNormal3() = default;
	RAYFLOW_CPU_GPU TNormal3(T x, T y, T z) : Tuple3<TNormal3, T>(x, y, z) {}

	template <typename U>
	RAYFLOW_CPU_GPU explicit TNormal3(const TNormal3<U>& n) : Tuple3<TNormal3, T>(T(n.x), T(n.y), T(n.z)) {}

	template <typename U>
	RAYFLOW_CPU_GPU explicit TNormal3(const TVector3<U>& v) : Tuple3<TNormal3, T>(T(v.x), T(v.y), T(v.z)) {}
};

template <typename T>
template <typename U>
TVector2<T>::TVector2(const TPoint2<U>& p) : Tuple2<TVector2, T>(T(p.x), T(p.y)) {}

template <typename T, typename U>
RAYFLOW_CPU_GPU inline TVector2<T> operator*(U u, const TVector2<T>& v) {
	return v * u;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto LengthSquare(const TVector2<T>& v)-> typename TupleLength<T>::type {
	return v.x * v.x + v.y * v.y;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Length(const TVector2<T>& v)-> typename TupleLength<T>::type {
	using std::sqrt;
	return sqrt(LengthSquare(v));
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Dot(const TVector2<T>& lhs, const TVector2<T>& rhs)-> typename TupleLength<T>::type {
	return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto AbsDot(const TVector2<T>& lhs, const TVector2<T>& rhs)-> typename TupleLength<T>::type {
	using std::abs;
	return abs(lhs.x * rhs.x + lhs.y * rhs.y);
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Normalize(const TVector2<T>& v) {
	return v / Length(v);
}

template <typename T>
RAYFLOW_CPU_GPU inline auto DistanceSquare(const TPoint2<T>& lhs, const TPoint2<T>& rhs) {
	return LengthSquare(lhs - rhs);
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Distance(const TPoint2<T>& lhs, const TPoint2<T>& rhs) {
	return Length(lhs - rhs);
}

template <typename T>
RAYFLOW_CPU_GPU inline auto DistanceSquare(const TPoint3<T>& lhs, const TPoint3<T>& rhs) {
	return LengthSquare(lhs - rhs);
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Distance(const TPoint3<T>& lhs, const TPoint3<T>& rhs) {
	return Length(lhs - rhs);
}

template <typename T>
template <typename U>
TVector3<T>::TVector3(const TPoint3<U>& p) : Tuple3<TVector3, T>(T(p.x), T(p.y), T(p.z)) {}

template <typename T, typename U>
RAYFLOW_CPU_GPU inline TVector3<T> operator*(U u, const TVector3<T>& v) {
	return v * u;
}
template <typename T>
template <typename U>
TVector3<T>::TVector3(const TNormal3<U>& n) : Tuple3<TVector3, T>(T(n.x), T(n.y), T(n.z)) {}

template <typename T>
RAYFLOW_CPU_GPU inline auto LengthSquare(const TVector3<T>& v)-> typename TupleLength<T>::type {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Length(const TVector3<T>& v)-> typename TupleLength<T>::type {
	using std::sqrt;
	return sqrt(LengthSquare(v));
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Dot(const TVector3<T>& lhs, const TVector3<T>& rhs) -> typename TupleLength<T>::type {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
};

template <typename T>
RAYFLOW_CPU_GPU inline auto AbsDot(const TVector3<T>& lhs, const TVector3<T>& rhs) -> typename TupleLength<T>::type {
	using std::abs;
	return abs(lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z);
};

template <typename T>
RAYFLOW_CPU_GPU inline TVector3<T> Cross(const TVector3<T>& v1, const TVector3<T>& v2) {
	return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
}

template <typename T>
RAYFLOW_CPU_GPU inline TVector3<T> Cross(const TVector3<T>& v, const TNormal3<T>& n) {
	return { v.y * n.z - v.z * n.y, v.z * n.x - v.x * n.z, v.x * n.y - v.y * n.x };
}

template <typename T>
RAYFLOW_CPU_GPU inline TVector3<T> Cross(const TNormal3<T>& n, const TVector3<T>& v) {
	return { n.y * v.z - n.z * v.y, n.z * v.x - n.x * v.z, n.x * v.y - n.y * v.x };
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Normalize(const TVector3<T>& v) {
	return v / Length(v);
}

template <typename T>
RAYFLOW_CPU_GPU inline TVector3<T> GramSchmidt(const TVector3<T>& v, const TVector3<T>& w) {
	return v - Dot(v, w) * w;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto LengthSquare(const TNormal3<T>& n)-> typename TupleLength<T>::type {
	return n.x * n.x + n.y * n.y + n.z * n.z;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Length(const TNormal3<T>& n)-> typename TupleLength<T>::type {
	using std::sqrt;
	return sqrt(LengthSquare(n));
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Dot(const TNormal3<T>& n, const TVector3<T>& v) -> typename TupleLength<T>::type {
	return n.x * v.x + n.y * v.y + n.z * v.z;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Dot(const TVector3<T>& v, const TNormal3<T>& n) -> typename TupleLength<T>::type {
	return n.x * v.x + n.y * v.y + n.z * v.z;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Dot(const TNormal3<T>& n1, const TNormal3<T>& n2) -> typename TupleLength<T>::type {
	return n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
}

template <typename T>
RAYFLOW_CPU_GPU inline auto AbsDot(const TNormal3<T>& n, const TVector3<T>& v) -> typename TupleLength<T>::type {
	using std::abs;
	return abs(n.x * v.x + n.y * v.y + n.z * v.z);
}

template <typename T>
RAYFLOW_CPU_GPU inline auto AbsDot(const TVector3<T>& v, const TNormal3<T>& n) -> typename TupleLength<T>::type {
	using std::abs;
	return abs(n.x * v.x + n.y * v.y + n.z * v.z);
}

template <typename T>
RAYFLOW_CPU_GPU inline auto AbsDot(const TNormal3<T>& n1, const TNormal3<T>& n2) -> typename TupleLength<T>::type {
	using std::abs;
	return abs(n1.x * n2.x + n1.y * n2.y + n1.z * n2.z);
}

template <typename T>
RAYFLOW_CPU_GPU inline void CoordinateSystem(const TVector3<T>& v1, TVector3<T>* v2, TVector3<T>* v3) {
	using std::abs;
	using std::sqrt;
	if (abs(v1.x) < abs(v1.y)) {
		*v2 = TVector3<T>(0, -v1.z, v1.y) / sqrt(v1.y * v1.y + v1.z * v1.z);
	}
	else {
		*v2 = TVector3<T>(-v1.z, 0, v1.x) / sqrt(v1.x * v1.x + v1.z * v1.z);
	}
	*v3 = Cross(v1, *v2);
}

template <typename T>
RAYFLOW_CPU_GPU inline void CoordinateSystem(const TNormal3<T>& v1, TVector3<T>* v2, TVector3<T>* v3) {
	using std::abs;
	using std::sqrt;
	if (abs(v1.x) < abs(v1.y)) {
		*v2 = TVector3<T>(0, -v1.z, v1.y) / sqrt(v1.y * v1.y + v1.z * v1.z);
	}
	else {
		*v2 = TVector3<T>(-v1.z, 0, v1.x) / sqrt(v1.x * v1.x + v1.z * v1.z);
	}
	*v3 = Cross(v1, *v2);
}

template <typename T>
RAYFLOW_CPU_GPU inline auto Normalize(const TNormal3<T>& n) {
	return n / Length(n);
}

// http://www.plunk.org/~hatch/rightway.html
template <typename T>
RAYFLOW_CPU_GPU inline Float AngleBetween(const TVector3<T>& v1, const TVector3<T>& v2) {
	if (Dot(v1, v2) < 0) {
		return Pi - 2 * SafeAsin(Length(v1 + v2) / 2);
	} 
	else {
		return 2 * SafeAsin(Length(v1 - v2) / 2);
	}
}

// http://www.plunk.org/~hatch/rightway.html
template <typename T>
RAYFLOW_CPU_GPU inline Float AngleBetween(const TNormal3<T>& n1, const TNormal3<T>& n2) {
	if (Dot(v1, v2) < 0) {
		return Pi - 2 * SafeAsin(Length(n1 + n2) / 2);
	}
	else {
		return 2 * SafeAsin(Length(n1 - n2) / 2);
	}
}

template <typename T>
RAYFLOW_CPU_GPU inline TNormal3<T> FaceForward(const TNormal3<T>& n, const TVector3<T>& v) {
	return (Dot(n, v) < 0) ? -n : n;
}

template <typename T>
RAYFLOW_CPU_GPU inline TNormal3<T> FaceForward(const TNormal3<T>& n1, const TNormal3<T>& n2) {
	return (Dot(n1, n2) < 0) ? -n1 : n1;
}

template <typename T>
RAYFLOW_CPU_GPU inline TVector3<T> FaceForward(const TVector3<T>& v1, const TVector3<T>& v2) {
	return (Dot(v1, v2) < 0) ? -v1 : v1;
}

template <typename T>
RAYFLOW_CPU_GPU inline TVector3<T> FaceForward(const TVector3<T>& v, const TNormal3<T>& n) {
	return (Dot(v, n) < 0) ? -v : n;
}

template <typename T>
class TAABB2 {
public:
	RAYFLOW_CPU_GPU TAABB2() :
		pMin(std::numeric_limits<T>::max(), std::numeric_limits<T>::max()),
		pMax(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()) {}
	RAYFLOW_CPU_GPU explicit TAABB2(const TPoint2<T>& p) : pMin(p), pMax(p) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TAABB2(const TAABB2<U>& box) {
		if (box.IsEmpty()) {
			*this = TAABB2<T>();
		}
		else {
			pMin = TPoint2<T>(box.pMin);
			pMax = TPoint2<T>(box.pMax);
		}
	}
	RAYFLOW_CPU_GPU TAABB2(const TPoint2<T>& p1, const TPoint2<T>& p2) : pMin(Min(p1, p2)), pMax(Max(p1, p2)) {}

	RAYFLOW_CPU_GPU bool IsEmpty() const {
		return pMin.x >= pMax.x || pMin.y >= pMax.y;
	}

	RAYFLOW_CPU_GPU TVector2<T> Diagonal() const {
		return pMax - pMin;
	}

	RAYFLOW_CPU_GPU bool IsDegenerate() const {
		return pMin.x > pMax.x || pMin.y > pMax.y;
	}

	RAYFLOW_CPU_GPU int MaxDimension() const {
		TVector2<T> diag = Diagonal();
		if (diag.x > diag.y) {
			return 0;
		}
		else {
			return 1;
		}
	}

	RAYFLOW_CPU_GPU TPoint2<T> Corner(int idx) const {
		return TPoint2<T>((*this)[idx & 1].x, (*this)[(idx & 2) ? 1 : 0].y);
	}

	RAYFLOW_CPU_GPU T Area() const {
		TVector2<T> diag = Diagonal();
		return diag.x * diag.y;
	}

	RAYFLOW_CPU_GPU bool operator==(const TAABB2<T>& box) const {
		return pMin == box.pMin && pMax == box.pMax;
	}

	RAYFLOW_CPU_GPU bool operator!=(const TAABB2<T>& box) const {
		return pMin != box.pMin || pMax != box.pMax;
	}

	RAYFLOW_CPU_GPU TPoint2<T> operator[](int i) const {
		return i == 0 ? pMin : pMax;
	}

	RAYFLOW_CPU_GPU TPoint2<T>& operator[](int i) {
		return i == 0 ? pMin : pMax;
	}

	TPoint2<T> pMin;
	TPoint2<T> pMax;
};

template <typename T>
RAYFLOW_CPU_GPU inline TAABB2<T> Union(const TAABB2<T>& box, const TPoint2<T>& p) {
	TAABB2<T> result;
	result.pMin = Min(box.pMin, p);
	result.pMax = Max(box.pMax, p);
	return result;
}

template <typename T>
RAYFLOW_CPU_GPU inline TAABB2<T> Union(const TAABB2<T>& box1, const TAABB2<T>& box2) {
	TAABB2<T> result;
	result.pMin = Min(box1.pMin, box2.pMin);
	result.pMax = Max(box1.pMax, box2.pMax);
	return result;
}

template <typename T>
RAYFLOW_CPU_GPU inline bool Inside(const TPoint2<T>& p, const TAABB2<T>& box) {
	return box.pMin.x <= p.x && p.x <= box.pMax.x &&
		box.pMin.y <= p.y && p.y <= box.pMax.y;
}

template <typename T>
RAYFLOW_CPU_GPU inline bool InsideExclusive(const TPoint2<T>& p, const TAABB2<T>& box) {
	return box.pMin.x <= p.x && p.x <= box.pMax.x &&
		box.pMin.y <= p.y && p.y < box.pMax.y;
}

template <typename T>
RAYFLOW_CPU_GPU inline TAABB2<T> Intersect(const TAABB2<T>& box1, const TAABB2<T>& box2) {
	TAABB2<T> result;
	result.pMin = Max(box1.pMin, box2.pMin);
	result.pMax = Min(box1.pMax, box2.pMax);
	return result;
}

template <typename T>
RAYFLOW_CPU_GPU inline bool Overlaps(const TAABB2<T>& box1, const TAABB2<T>& box2) {
	return box1.pMax.x >= box2.pMin.x && box1.pMin.x <= box2.pMax.x &&
		box1.pMax.y >= box2.pMin.y && box1.pMin.y <= box2.pMax.y;
}

template <typename T>
class TAABB3 {
public:
	RAYFLOW_CPU_GPU TAABB3() :
		pMin(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max()),
		pMax(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()) {}
	RAYFLOW_CPU_GPU explicit TAABB3(const TPoint3<T>& p) : pMin(p), pMax(p) {}
	template <typename U>
	RAYFLOW_CPU_GPU explicit TAABB3(const TAABB3<U>& box) {
		if (box.IsEmpty()) {
			*this = TAABB3<T>();
		}
		else {
			pMin = TPoint3<T>(box.pMin);
			pMax = TPoint3<T>(box.pMax);
		}
	}
	RAYFLOW_CPU_GPU TAABB3(const TPoint3<T>& p1, const TPoint3<T>& p2) : pMin(Min(p1, p2)), pMax(Max(p1, p2)) {}

	RAYFLOW_CPU_GPU bool IsEmpty() const {
		return pMin.x >= pMax.x || pMin.y >= pMax.y || pMin.z >= pMax.z;
	}

	RAYFLOW_CPU_GPU TVector3<T> Diagonal() const {
		return pMax - pMin;
	}

	RAYFLOW_CPU_GPU bool IsDegenerate() const {
		return pMin.x > pMax.x || pMin.y > pMax.y || pMin.z > pMax.z;
	}

	RAYFLOW_CPU_GPU int MaxDimension() const {
		TVector3<T> diag = Diagonal();
		return (diag.x > diag.y) ? (diag.x > diag.z ? 0 : 2) : (diag.y > diag.z ? 1 : 2);
	}

	RAYFLOW_CPU_GPU TPoint3<T> Corner(int idx) const {
		return TPoint3<T>((*this)[idx & 1].x, (*this)[(idx & 2) ? 1 : 0].y, (*this)[(idx & 4) ? 1 : 0].z);
	}

	RAYFLOW_CPU_GPU T SurfaceArea() const {
		TVector3<T> diag = Diagonal();
		return 2 * (diag.x * diag.y + diag.x * diag.z + diag.y * diag.z);
	}

	RAYFLOW_CPU_GPU T Volume() const {
		TVector3<T> diag = Diagonal();
		return diag.x * diag.y * diag.z;
	}

	RAYFLOW_CPU_GPU bool operator==(const TAABB3<T>& box) const {
		return pMin == box.pMin && pMax == box.pMax;
	}

	RAYFLOW_CPU_GPU bool operator!=(const TAABB3<T>& box) const {
		return pMin != box.pMin || pMax != box.pMax;
	}

	RAYFLOW_CPU_GPU TPoint3<T> operator[](int i) const {
		return i == 0 ? pMin : pMax;
	}

	RAYFLOW_CPU_GPU TPoint3<T>& operator[](int i) {
		return i == 0 ? pMin : pMax;
	}

	TPoint3<T> pMin;
	TPoint3<T> pMax;
};

template <typename T>
RAYFLOW_CPU_GPU inline TAABB3<T> Union(const TAABB3<T>& box, const TPoint3<T>& p) {
	TAABB3<T> result;
	result.pMin = Min(box.pMin, p);
	result.pMax = Max(box.pMax, p);
	return result;
}

template <typename T>
RAYFLOW_CPU_GPU inline TAABB3<T> Union(const TAABB3<T>& box1, const TAABB3<T>& box2) {
	TAABB3<T> result;
	result.pMin = Min(box1.pMin, box2.pMin);
	result.pMax = Max(box1.pMax, box2.pMax);
	return result;
}

template <typename T>
RAYFLOW_CPU_GPU inline bool Inside(const TPoint3<T>& p, const TAABB3<T>& box) {
	return box.pMin.x <= p.x && p.x <= box.pMax.x &&
		box.pMin.y <= p.y && p.y <= box.pMax.y && 
		box.pMin.z <= p.z && p.z <= box.pMax.z;
}

template <typename T>
RAYFLOW_CPU_GPU inline bool InsideExclusive(const TPoint3<T>& p, const TAABB3<T>& box) {
	return box.pMin.x < p.x && p.x < box.pMax.x &&
		box.pMin.y < p.y && p.y < box.pMax.y &&
		box.pMin.z < p.z && p.z < box.pMax.z;
}

template <typename T>
RAYFLOW_CPU_GPU inline bool Overlaps(const TAABB3<T>& box1, const TAABB3<T>& box2) {
	return box1.pMax.x >= box2.pMin.x && box1.pMin.x <= box2.pMax.x &&
		box1.pMax.y >= box2.pMin.y && box1.pMin.y <= box2.pMax.y && 
		box1.pMax.z >= box2.pMin.z && box1.pMin.z <= box2.pMax.z;
}

template <typename T>
RAYFLOW_CPU_GPU inline bool Intersect(const TAABB3<T>& box, const TPoint3<T>& o, const TVector3<T>& dir, const TVector3<T>& invDir, const int isDirInv[3], Float rayTMax) {
	Float tMinX = (box[isDirInv[0]].x - o.x) * invDir.x;
	Float tMaxX = (box[1 - isDirInv[0]].x - o.x) * invDir.x;
	Float tMinY = (box[isDirInv[1]].y - o.y) * invDir.y;
	Float tMaxY = (box[1 - isDirInv[1]].y - o.y) * invDir.y;
	Float tMinZ = (box[isDirInv[2]].z - o.z) * invDir.z;
	Float tMaxZ = (box[1 - isDirInv[2]].z - o.z) * invDir.z;
	Float tMin = std::max(std::max(tMinX, tMinY), tMinZ);
	Float tMax = std::min(std::min(tMaxX, tMaxY), tMaxZ);
	return (tMin <= tMax) && (tMax > 0) && (tMin < rayTMax);
}

template <typename T>
RAYFLOW_CPU_GPU inline TAABB3<T> Intersect(const TAABB3<T>& box1, const TAABB3<T>& box2) {
	TAABB2<T> result;
	result.pMin = Max(box1.pMin, box2.pMin);
	result.pMax = Min(box1.pMax, box2.pMax);
	return result;
}

RAYFLOW_CPU_GPU inline Point2 CartesianToSphere(const Vector3& p) {
	const auto& cosTheta = Clamp(p.z, -1, 1);
	Float sinTheta = ::sqrt(1 - cosTheta * cosTheta);
	Float phi = ::atan2(p.x, p.y);
	return { phi < 0 ? (phi + 2 * Pi) : phi, ::acos(cosTheta) };
}

RAYFLOW_CPU_GPU inline Vector3 SphereToCartesian(const Point2& p) {
	const auto& phi = p.x;
	const auto& theta = p.y;

	return { ::cos(phi) * ::sin(theta), ::sin(phi) * ::sin(theta), ::cos(theta) };
}

template <typename T>
RAYFLOW_CPU_GPU inline TNormal3<T> Faceforward(const TNormal3<T> &n, const TVector3<T> &v) {
    return (Dot(n, v) < 0.f) ? -n : n;
}

template <typename T>
RAYFLOW_CPU_GPU inline TNormal3<T> Faceforward(const TNormal3<T> &n, const TNormal3<T> &n2) {
    return (Dot(n, n2) < 0.f) ? -n : n;
}

template <typename T>
RAYFLOW_CPU_GPU inline TVector3<T> Faceforward(const TVector3<T> &v, const TVector3<T> &v2) {
    return (Dot(v, v2) < 0.f) ? -v : v;
}

template <typename T>
RAYFLOW_CPU_GPU inline TVector3<T> Faceforward(const TVector3<T> &v, const TNormal3<T> &n2) {
    return (Dot(v, n2) < 0.f) ? -v : v;
}

class Quaternion {
public:
	RAYFLOW_CPU_GPU Quaternion() = default;
	RAYFLOW_CPU_GPU Quaternion(const Vector3& v, Float w) : v(v), w(w) {}
	RAYFLOW_CPU_GPU Quaternion(Float x, Float y, Float z, Float w) : v(x, y, z), w(w) {}

	RAYFLOW_CPU_GPU bool operator==(const Quaternion& q) const {
		return v == q.v && w == q.w;
	}

	RAYFLOW_CPU_GPU bool operator!=(const Quaternion& q) const {
		return v != q.v || w != q.w;
	}

	RAYFLOW_CPU_GPU Quaternion operator-() const {
		return { -v, -w };
	}

	RAYFLOW_CPU_GPU Quaternion operator+(const Quaternion& q) const {
		return { v + q.v, w + q.w };
	}

	RAYFLOW_CPU_GPU Quaternion& operator+=(const Quaternion& q) {
		v += q.v;
		w += q.w;
		return *this;
	}

	RAYFLOW_CPU_GPU Quaternion operator-(const Quaternion& q) const {
		return { v - q.v, w - q.w };
	}

	RAYFLOW_CPU_GPU Quaternion& operator-=(const Quaternion& q) {
		v -= q.v;
		w -= q.w;
		return *this;
	}

	RAYFLOW_CPU_GPU Quaternion operator*(const Quaternion& q) const {
		return { Cross(v, q.v) - w * q.v - q.w * v, w * q.w - Dot(v, q.v) };
	}

	RAYFLOW_CPU_GPU Quaternion& operator*=(const Quaternion& q) {
		v = Cross(v, q.v) - w * q.v - q.w * v;
		w = w * q.w - Dot(v, q.v);
		return *this;
	}
	
	RAYFLOW_CPU_GPU Quaternion operator*(Float u) const {
		return { v * u, w * u };
	}

	RAYFLOW_CPU_GPU Quaternion operator*=(Float u) {
		v *= u;
		w *= u;
		return *this;
	}

	RAYFLOW_CPU_GPU Quaternion operator/(Float u) const {
		return { v / u, w / u };
	}

	RAYFLOW_CPU_GPU Quaternion operator/=(Float u) {
		v /= u;
		w /= u;
		return *this;
	}
	
	Vector3 v;
	Float w{};
};

RAYFLOW_CPU_GPU inline Quaternion operator*(Float u, const Quaternion& q) {
	return q * u;
}

RAYFLOW_CPU_GPU inline Float LengthSquare(const Quaternion& q) {
	return q.v.x * q.v.x + q.v.y * q.v.y + q.v.z * q.v.z + q.w * q.w;
}

RAYFLOW_CPU_GPU inline Float Length(const Quaternion& q) {
	using std::sqrt;
	return sqrt(LengthSquare(q));
}

RAYFLOW_CPU_GPU inline Quaternion Conjugate(const Quaternion& q) {
	return { -q.v, q.w };
}

RAYFLOW_CPU_GPU inline Quaternion Inverse(const Quaternion& q) {
	return Conjugate(q) / Length(q);
}

RAYFLOW_CPU_GPU inline Float Dot(const Quaternion& q1, const Quaternion& q2) {
	return q1.v.x * q2.v.x + q1.v.y * q2.v.y + q1.v.z * q2.v.z + q1.w * q2.w;
}

// http://www.plunk.org/~hatch/rightway.html
RAYFLOW_CPU_GPU inline Float AngleBetween(const Quaternion& q1, const Quaternion& q2) {
	if (Dot(q1, q2) < 0) {
		return Pi - 2 * SafeAsin(Length(q1 + q2) / 2);
	}
	else {
		return 2 * SafeAsin(Length(q1 - q2) / 2);
	}
}

// http://www.plunk.org/~hatch/rightway.html
RAYFLOW_CPU_GPU inline Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, Float t) {
	Float theta = AngleBetween(q1, q2);
	Float sinThetaOverTheta = SinXOverX(theta);
	return q1 * (1 - t) * SinXOverX((1 - t) * theta) / sinThetaOverTheta +
		q2 * t * SinXOverX(t * theta) / sinThetaOverTheta;
}

class Frame {
public:
	RAYFLOW_CPU_GPU Frame() : s(1, 0, 0), t(0, 1, 0), n(0, 0, 1) {}

	RAYFLOW_CPU_GPU static Frame FromX(const Vector3& v) {
		Frame result;
		result.s = v;
		CoordinateSystem(v, &result.t, &result.n);
		return result;
	}

	RAYFLOW_CPU_GPU static Frame FromY(const Vector3& v) {
		Frame result;
		result.t = v;
		CoordinateSystem(v, &result.s, &result.n);
		return result;
	}

	RAYFLOW_CPU_GPU static Frame FromZ(const Vector3& v) {
		Frame result;
		result.n = v;
		CoordinateSystem(v, &result.s, &result.t);
		return result;
	}

	RAYFLOW_CPU_GPU static Frame FromX(const Normal3& v) {
		Frame result;
		result.s = Vector3(v);
		CoordinateSystem(v, &result.t, &result.n);
		return result;
	}

	RAYFLOW_CPU_GPU static Frame FromY(const Normal3& v) {
		Frame result;
		result.t = Vector3(v);
		CoordinateSystem(v, &result.s, &result.n);
		return result;
	}

	RAYFLOW_CPU_GPU static Frame FromZ(const Normal3& v) {
		Frame result;
		result.n = Vector3(v);
		CoordinateSystem(v, &result.s, &result.t);
		return result;
	}

	RAYFLOW_CPU_GPU inline Vector3 FromLocal(const Vector3& v) const {
		return v.x * s + v.y * t + v.z * n;
	}

	RAYFLOW_CPU_GPU inline Normal3 FromLocal(const Normal3& v) const {
		return Normal3(v.x * s + v.y * t + v.z * n);
	}

	RAYFLOW_CPU_GPU inline Vector3 ToLocal(const Vector3& v) const {
		return Vector3(Dot(v, s), Dot(v, t), Dot(v, n));
	}

	RAYFLOW_CPU_GPU inline Normal3 ToLocal(const Normal3& v) const {
		return Normal3(Dot(v, s), Dot(v, t), Dot(v, n));
	}

	
	Vector3 s;
	Vector3 t;
	Vector3 n;
};

RAYFLOW_CPU_GPU inline Float CosTheta(const Vector3& v) { return v.z; }

RAYFLOW_CPU_GPU inline Float AbsCosTheta(const Vector3& v) { return ::abs(v.z); }

RAYFLOW_CPU_GPU inline Float Cos2Theta(const Vector3& v) { return v.z * v.z; }

RAYFLOW_CPU_GPU inline Float Sin2Theta(const Vector3& v) { return std::max<Float>(0, 1 - Cos2Theta(v)); }

RAYFLOW_CPU_GPU inline Float SinTheta(const Vector3& v) { return std::sqrt(Sin2Theta(v)); }

RAYFLOW_CPU_GPU inline Float TanTheta(const Vector3& v) { return SinTheta(v) / CosTheta(v); }

RAYFLOW_CPU_GPU inline Float Tan2Theta(const Vector3& v) { return Sin2Theta(v) / Cos2Theta(v); }

RAYFLOW_CPU_GPU inline Float SinPhi(const Vector3& v) {
	Float sinTheta = SinTheta(v);
	return (sinTheta == 0) ? 0 : Clamp(v.y / sinTheta, -1, 1);
}

RAYFLOW_CPU_GPU inline Float CosPhi(const Vector3& v) {
	Float sinTheta = SinTheta(v);
	return (sinTheta == 0) ? 0 : Clamp(v.x / sinTheta, -1, 1);
}

RAYFLOW_CPU_GPU inline Float Sin2Phi(const Vector3& v) {
	Float sinPhi = SinPhi(v);
	return sinPhi * sinPhi;
}

RAYFLOW_CPU_GPU inline Float Cos2Phi(const Vector3& v) {
	Float cosPhi = CosPhi(v);
	return cosPhi * cosPhi;
}

}