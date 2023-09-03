#pragma once

#include <RayFlow/rayflow.h>
#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Std/optional.h>

#include <cassert>

namespace rayflow {
template <int N>
RAYFLOW_CPU_GPU inline void InitMatrix(Float m[N][N], int i, int j) {}

template <int N, typename... Args>
RAYFLOW_CPU_GPU inline void InitMatrix(Float m[N][N], int i, int j, Float v, Args... args) {
	m[i][j] = v;
	if (++j == N) {
		++i;
		j = 0;
	}
	InitMatrix<N>(m, i, j, args...);
}

template <int N>
RAYFLOW_CPU_GPU inline void SetMatrixDiag(Float m[N][N], int i) {}

template <int N, typename... Args>
RAYFLOW_CPU_GPU inline void SetMatrixDiag(Float m[N][N], int i, Float v, Args... args) {
	m[i][i] == v;
	SetMatrixDiag<N>(m, i + 1, args...);
}

template <int N>
class MatrixNxN {
public:
	RAYFLOW_CPU_GPU MatrixNxN() {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				m[i][j] = (i == j) ? 1 : 0;
			}
		}
	}

	RAYFLOW_CPU_GPU MatrixNxN(const Float mat[N][N]) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				m[i][j] = mat[i][j];
			}
		}
	}

	template <typename... Args>
	RAYFLOW_CPU_GPU MatrixNxN(Float v, Args... args) {
		static_assert(1 + sizeof...(args) == N * N, "Incorrect number of values provided to matrix constructor");
		InitMatrix<N>(m, 0, 0, v, args...);
	}

	RAYFLOW_CPU_GPU bool operator==(const MatrixNxN& other) const {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				if (m[i][j] != other.m[i][j]) {
					return false;
				}
			}
		}
		return true;
	}

	RAYFLOW_CPU_GPU bool operator!=(const MatrixNxN& other) const {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				if (m[i][j] != other.m[i][j]) {
					return true;
				}
			}
		}
		return false;
	}

	RAYFLOW_CPU_GPU MatrixNxN operator+(const MatrixNxN& other) const {
		MatrixNxN result;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				result.m[i][j] = m[i][j] + other.m[i][j];
			}
		}
		return result;
	}

	RAYFLOW_CPU_GPU MatrixNxN operator*(const MatrixNxN& other) const {
		MatrixNxN result;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				result.m[i][j] = 0;
				for (int k = 0; k  < N; ++k) {
					result.m[i][j] += m[i][k] * other.m[k][j];
				}
			}
		}
		return result;
	}

	RAYFLOW_CPU_GPU MatrixNxN& operator+=(const MatrixNxN& other) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				m[i][j] += other.m[i][j];
			}
		}
		return *this;
	}

	RAYFLOW_CPU_GPU MatrixNxN operator-(const MatrixNxN& other) const {
		MatrixNxN result;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				result.m[i][j] = m[i][j] - other.m[i][j];
			}
		}
		return result;
	}

	RAYFLOW_CPU_GPU MatrixNxN& operator-=(const MatrixNxN& other) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				m[i][j] -= other.m[i][j];
			}
		}
		return *this;
	}

	RAYFLOW_CPU_GPU MatrixNxN operator*(Float u) const {
		MatrixNxN result;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				result.m[i][j] = u * m[i][j];
			}
		}
		return result;
	}

	RAYFLOW_CPU_GPU MatrixNxN& operator*=(Float u) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				m[i][j] *= u;
			}
		}
		return *this;
	}

	RAYFLOW_CPU_GPU MatrixNxN operator/(Float u) const {
		MatrixNxN result;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				result.m[i][j] = m[i][j] / u;
			}
		}
		return result;
	}

	RAYFLOW_CPU_GPU MatrixNxN& operator/=(Float u) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				m[i][j] /= u;
			}
		}
		return *this;
	}

	RAYFLOW_CPU_GPU static MatrixNxN Zero() {
		MatrixNxN result;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				result.m[i][j] = 0;
			}
		}
		return result;
	}

	RAYFLOW_CPU_GPU static MatrixNxN Indentity() {
		MatrixNxN result;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				result.m[i][j] = (i == j) ? 1 : 0;
			}
		}
		return result;
	}

	template <typename... Args>
	RAYFLOW_CPU_GPU static MatrixNxN Diag(Float v, Args... args) {
		static_assert(1 + sizeof...(args) == N, "Incorrect number of values provided to construct diagonal matrix");
		MatrixNxN result;
		SetMatrixDiag<N>(result.m, 0, v, args...);
		return result;
	}
	Float m[N][N];

	
};


RAYFLOW_CPU_GPU inline Vector2 operator*(const MatrixNxN<2>& m, const Vector2& v) {
	return { m.m[0][0] * v.x + m.m[0][1] * v.y,
	         m.m[1][0] * v.x + m.m[1][1] * v.y };
}

RAYFLOW_CPU_GPU inline Vector3 operator*(const MatrixNxN<3>& m, const Vector3& v) {
	return { m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z, 
	         m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
			 m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z };
}


template <int N>
RAYFLOW_CPU_GPU inline MatrixNxN<N> operator*(Float u, const MatrixNxN<N>& m) {
	return m * u;
}

template <int N>
RAYFLOW_CPU_GPU inline MatrixNxN<N> Transpose(const MatrixNxN<N>& m) {
	MatrixNxN<N> result;
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

RAYFLOW_CPU_GPU inline Float Determinant(const MatrixNxN<1>& m) {
	return m.m[0][0];
}

RAYFLOW_CPU_GPU inline Float Determinant(const MatrixNxN<2>& m) {
	return m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
}

RAYFLOW_CPU_GPU inline Float Determinant(const MatrixNxN<3>& m) {
	Float sub01 = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];
	Float sub02 = m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0];
	Float sub03 = m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0];
	return m.m[0][0] * sub01 - m.m[0][1] * sub02 + m.m[0][2] * sub03;
}

// https://www.geometrictools.com/Documentation/LaplaceExpansionTheorem.pdf
RAYFLOW_CPU_GPU inline Float Determinant(const MatrixNxN<4>& m) {
	Float s0 = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
	Float s1 = m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0];
	Float s2 = m.m[0][0] * m.m[1][3] - m.m[0][3] * m.m[1][0];
	Float s3 = m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1];
	Float s4 = m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1];
	Float s5 = m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2];

	Float c0 = m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0];
	Float c1 = m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0];
	Float c2 = m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0];
	Float c3 = m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1];
	Float c4 = m.m[2][1] * m.m[3][3] - m.m[2][2] * m.m[3][1];
	Float c5 = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];
	return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
}

RAYFLOW_CPU_GPU inline rstd::optional<MatrixNxN<2>> Inverse(const MatrixNxN<2>& m) {
	Float determinant = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
	if (determinant == 0) {
		return {};
	}
	return MatrixNxN<2>(m.m[1][1], -m.m[0][1], -m.m[1][0], m.m[0][0]) / determinant;
}

RAYFLOW_CPU_GPU inline rstd::optional<MatrixNxN<3>> Inverse(const MatrixNxN<3>& m) {
	Float determinant = Determinant(m);
	if (determinant == 0) {
		return {};
	}
	Float invDet = 1 / determinant;
	MatrixNxN<3> result;
	result.m[0][0] = invDet * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]);
	result.m[0][1] = invDet * (m.m[0][2] * m.m[2][1] - m.m[0][1] * m.m[2][2]);
	result.m[0][2] = invDet * (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]);

	result.m[1][0] = invDet * (m.m[1][2] * m.m[2][0] - m.m[1][0] * m.m[2][2]);
	result.m[1][1] = invDet * (m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0]);
	result.m[1][2] = invDet * (m.m[0][2] * m.m[1][0] - m.m[0][0] * m.m[1][2]);

	result.m[2][0] = invDet * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
	result.m[2][1] = invDet * (m.m[0][1] * m.m[2][0] - m.m[0][0] * m.m[2][1]);
	result.m[2][2] = invDet * (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]);
	return result;
}

// https://www.geometrictools.com/Documentation/LaplaceExpansionTheorem.pdf
RAYFLOW_CPU_GPU inline rstd::optional<MatrixNxN<4>> Inverse(const MatrixNxN<4>& m) {
	Float s0 = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
	Float s1 = m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0];
	Float s2 = m.m[0][0] * m.m[1][3] - m.m[0][3] * m.m[1][0];
	Float s3 = m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1];
	Float s4 = m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1];
	Float s5 = m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2];

	Float c0 = m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0];
	Float c1 = m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0];
	Float c2 = m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0];
	Float c3 = m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1];
	Float c4 = m.m[2][1] * m.m[3][3] - m.m[2][2] * m.m[3][1];
	Float c5 = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];

	Float determinant = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;;
	if (determinant == 0) {
		return {};
	}
	Float invDet = 1 / determinant;
	MatrixNxN<4> result;
	result.m[0][0] = invDet * (m.m[1][1] * c5 - m.m[1][2] * c4 + m.m[1][3] * c3);
	result.m[0][1] = invDet * (-m.m[0][1] * c5 + m.m[0][2] * c4 - m.m[0][3] * c3);
	result.m[0][2] = invDet * (m.m[3][1] * s5 - m.m[3][2] * s4 + m.m[3][3] * s3);
	result.m[0][3] = invDet * (-m.m[2][1] * s5 + m.m[2][2] * s4 - m.m[2][3] * s3);

	result.m[1][0] = invDet * (-m.m[1][0] * c5 + m.m[1][2] * c2 - m.m[1][3] * c1);
	result.m[1][1] = invDet * (m.m[0][0] * c5 - m.m[0][2] * c2 + m.m[0][3] * c1);
	result.m[1][2] = invDet * (-m.m[3][0] * s5 + m.m[3][2] * s2 - m.m[3][3] * s1);
	result.m[1][3] = invDet * (m.m[2][0] * s5 - m.m[2][2] * s2 + m.m[2][3] * s1);

	result.m[2][0] = invDet * (m.m[1][0] * c4 - m.m[1][1] * c2 + m.m[1][3] * c0);
	result.m[2][1] = invDet * (-m.m[0][0] * c4 + m.m[0][1] * c2 - m.m[0][3] * c0);
	result.m[2][2] = invDet * (m.m[3][0] * s4 - m.m[3][1] * s2 + m.m[3][3] * s0);
	result.m[2][3] = invDet * (-m.m[2][0] * s4 + m.m[2][1] * s2 - m.m[2][3] * s0);

	result.m[3][0] = invDet * (-m.m[1][0] * c3 + m.m[1][1] * c1 - m.m[1][2] * c0);
	result.m[3][1] = invDet * (m.m[0][0] * c3 - m.m[0][1] * c1 + m.m[0][2] * c0);
	result.m[3][2] = invDet * (-m.m[3][0] * s3 + m.m[3][1] * s1 - m.m[3][2] * s0);
	result.m[3][3] = invDet * (m.m[2][0] * s3 - m.m[2][1] * s1 + m.m[2][2] * s0);
	return result;
}

extern template class MatrixNxN<2>;
extern template class MatrixNxN<3>;
extern template class MatrixNxN<4>;
}