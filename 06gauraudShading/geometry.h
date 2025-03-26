#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <ostream>
#include <cassert>
#include <vector>

// ----------------------------------------------------------------------------
// Primary template for an N-dimensional vector of type T.
// By default, it just stores data_[] and provides operator[].
// We'll then provide partial specializations for 2D and 3D that add x,y,z
// and vector operations like cross, operator-, etc.
// ----------------------------------------------------------------------------

template <size_t DIM, typename T>
struct vec {
    T data_[DIM];

    vec() {
        for (size_t i = 0; i < DIM; i++) {
            data_[i] = T();
        }
    }

    // Variadic constructor for convenience (requires the exact number of args)
    template <typename... Args>
    vec(Args... args) : data_{static_cast<T>(args)...} {
        static_assert(sizeof...(args) == DIM,
            "Number of constructor arguments must match vector dimension");
    }

    T& operator[](size_t i) {
        assert(i < DIM);
        return data_[i];
    }

    const T& operator[](size_t i) const {
        assert(i < DIM);
        return data_[i];
    }
};

// ----------------------------------------------------------------------------
// 2D partial specialization: named fields x,y plus basic arithmetic operators.
// ----------------------------------------------------------------------------
template <typename T>
struct vec<2,T> {
    union {
        struct { T x, y; };
        T data_[2];
    };

    vec() : x(T()), y(T()) {}

    vec(T X, T Y) : x(X), y(Y) {}

    // Operator[]
    T& operator[](size_t i) {
        assert(i < 2);
        return (i==0 ? x : y);
    }
    const T& operator[](size_t i) const {
        assert(i < 2);
        return (i==0 ? x : y);
    }

    // Basic arithmetic
    vec<2,T> operator+(const vec<2,T> &other) const {
        return vec<2,T>(x + other.x, y + other.y);
    }
    vec<2,T> operator-(const vec<2,T> &other) const {
        return vec<2,T>(x - other.x, y - other.y);
    }
    vec<2,T> operator*(float f) const {
        return vec<2,T>(x*f, y*f);
    }
};

// ----------------------------------------------------------------------------
// 3D partial specialization: named fields x,y,z plus cross, dot, normalize, etc.
// ----------------------------------------------------------------------------
template <typename T>
struct vec<3,T> {
    union {
        struct { T x, y, z; };
        T data_[3];
    };

    vec() : x(T()), y(T()), z(T()) {}

    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}

    // Operator[]
    T& operator[](size_t i) {
        assert(i < 3);
        return (i == 0 ? x : (i == 1 ? y : z));
    }
    const T& operator[](size_t i) const {
        assert(i < 3);
        return (i == 0 ? x : (i == 1 ? y : z));
    }

    // Basic arithmetic
    vec<3,T> operator+(const vec<3,T> &v) const {
        return vec<3,T>(x + v.x, y + v.y, z + v.z);
    }
    vec<3,T> operator-(const vec<3,T> &v) const {
        return vec<3,T>(x - v.x, y - v.y, z - v.z);
    }
    vec<3,T> operator*(float f) const {
        return vec<3,T>(x*f, y*f, z*f);
    }

    // Dot product as operator*
    T operator*(const vec<3,T> &v) const {
        return x*v.x + y*v.y + z*v.z;
    }

    // Cross product
    vec<3,T> cross(const vec<3,T> &v) const {
        return vec<3,T>(
            y*v.z - z*v.y,
            z*v.x - x*v.z,
            x*v.y - y*v.x
        );
    }

    float norm() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    // Normalize in place, scaling length to 'l' (default 1)
    vec<3,T> & normalize(T l=1) {
        float length = norm();
        if (length > 1e-7) {
            float factor = l / length;
            x *= factor; y *= factor; z *= factor;
        }
        return *this;
    }
};

// ----------------------------------------------------------------------------
// 4D partial specialization: named fields x,y,z,w plus basic arithmetic
// ----------------------------------------------------------------------------
template <typename T>
struct vec<4, T> {
    union {
        struct { T x, y, z, w; };
        T data_[4];
    };

    vec() : x(T()), y(T()), z(T()), w(T()) {}

    vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}

    T& operator[](size_t i) {
        assert(i < 4);
        return data_[i];
    }

    const T& operator[](size_t i) const {
        assert(i < 4);
        return data_[i];
    }

    vec<4, T> operator+(const vec<4, T>& v) const {
        return vec<4, T>(x + v.x, y + v.y, z + v.z, w + v.w);
    }

    vec<4, T> operator-(const vec<4, T>& v) const {
        return vec<4, T>(x - v.x, y - v.y, z - v.z, w - v.w);
    }

    vec<4, T> operator*(float f) const {
        return vec<4, T>(x * f, y * f, z * f, w * f);
    }
};


// ----------------------------------------------------------------------------
// Example operator*(vec<DIM,T>, vec<DIM,T>) for the general case (dot product).
// For 2D or 3D, you may prefer the specialized versions above, but you can keep
// this around if you want a quick dot-product for higher DIM vectors.
// ----------------------------------------------------------------------------
template<size_t DIM,typename T>
T operator*(const vec<DIM,T>& lhs, const vec<DIM,T>& rhs) {
    T ret = T();
    for (size_t i = 0; i < DIM; i++) {
        ret += lhs[i] * rhs[i];
    }
    return ret;
}

// ----------------------------------------------------------------------------
// 'embed' helper: embed a DIM-vector into a larger LEN-vector (fill with 'fill').
// ----------------------------------------------------------------------------
template<size_t LEN, size_t DIM, typename T>
vec<LEN, T> embed(const vec<DIM, T> &v, T fill = 1) {
    static_assert(LEN >= DIM, "Embedding must go to a larger or equal dimension");
    vec<LEN, T> ret;
    for (size_t i = 0; i < LEN; i++) {
        ret[i] = (i < DIM ? v[i] : fill);
    }
    return ret;
}

// template<size_t LEN, size_t DIM, typename T> vec<LEN, T> proj(const vec<DIM,T> &v) {
// 	vec<LEN, T> ret;
// 	for (size_t i = LEN; i--; ret[i] = v[i]);
// 	return ret;
// }

template <size_t LEN, size_t DIM, typename T>
vec<LEN, T> proj(const vec<DIM, T> &v) {
    static_assert(LEN <= DIM,
        "Cannot project to a bigger dimension than the source");
    vec<LEN, T> result;
    for (size_t i = 0; i < LEN; i++) {
        result[i] = v[i]; // copy the first LEN components
    }
    return result;
}

template <size_t LEN, typename T, size_t DIM>
vec<LEN, T> proj(const vec<DIM, T> &v) {
    static_assert(LEN <= DIM, "Cannot project to a bigger dimension than the source");
    vec<LEN, T> result;
    for (size_t i = 0; i < LEN; i++) {
        result[i] = v[i];
    }
    return result;
}


// ----------------------------------------------------------------------------
// Typedefs for convenience:
// ----------------------------------------------------------------------------
typedef vec<2, float>  Vec2f;
typedef vec<2, int>    Vec2i;
typedef vec<3, float>  Vec3f;
typedef vec<3, int>    Vec3i;
typedef vec<4, float>  Vec4f;
typedef vec<4, int>    Vec4i;

// ----------------------------------------------------------------------------
// Stream output for 2D and 3D. For higher dims, you can write another or just use
// an indexing loop.
// ----------------------------------------------------------------------------
#include <iostream>

template <typename T>
std::ostream& operator<<(std::ostream& out, const vec<2,T>& v) {
    out << "(" << v.x << ", " << v.y << ")";
    return out;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const vec<3,T>& v) {
    out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return out;
}

// ----------------------------------------------------------------------------
// A basic Matrix class (4x4 by default), matching your original code
// ----------------------------------------------------------------------------
class Matrix {
private:
    std::vector<std::vector<float>> m;
    int rows, cols;

    float determinantRecursive(const std::vector<std::vector<float>>& mat) const;
    Matrix getCofactorMatrix(int p, int q) const;

public:
    Matrix(int rows = 4, int cols = 4);
    Matrix(const std::vector<std::vector<float>>& values);

    int getRows() const;
    int getCols() const;

    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const;
    Matrix operator*(float scalar) const;

    std::vector<float>& operator[](int index);
    const std::vector<float>& operator[](int index) const;

    Matrix transpose() const;
    float determinant() const;
    Matrix inverse() const;
    static Matrix identity(int dimensions);
    void display() const;
    Matrix rotation_x(float cos, float sin);
    Matrix rotation_y(float cos, float sin);
    Matrix rotation_z(float cos, float sin);
    Matrix zoom(float factor);
    Matrix translation(Vec3f v);
    Vec4f operator*(const Vec4f &vec) const;

	void set_col(int col, const Vec4f& v);
	Matrix inverse_transpose() const;
};

#endif // __GEOMETRY_H__
