#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <ostream>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <size_t DIM, typename T> struct vec {
    vec() { for (size_t i=DIM; i--; data_[i] = T()); }
          T& operator[](const size_t i)       { assert(i<DIM); return data_[i]; }
    const T& operator[](const size_t i) const { assert(i<DIM); return data_[i]; }
private:
    T data_[DIM];
};



template <class t> struct Vec2 {
	union {
		struct {t u, v;};
		struct {t x, y;};
		t raw[2];
	};
	Vec2() : u(0), v(0) {}
	Vec2(t _u, t _v) : u(_u),v(_v) {}
	inline Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(u+V.u, v+V.v); }
	inline Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(u-V.u, v-V.v); }
	inline Vec2<t> operator *(float f)          const { return Vec2<t>(u*f, v*f); }
	inline t& operator[](int index) {
		return raw[index];
	}
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
	union {
		struct {t x, y, z;};
		struct { t ivert, iuv, inorm; };
		t raw[3];
	};
	Vec3() : x(0), y(0), z(0) {}
	Vec3(t _x, t _y, t _z) : x(_x),y(_y),z(_z) {}
	inline Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
	inline Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
	inline Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
	inline Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
	inline t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
	inline t& operator[](int index) {
		return raw[index];
	}
	float norm () const { return std::sqrt(x*x+y*y+z*z); }
	Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);

	// Vec3<t> operator ^(const Vec3<t>& v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
    // friend Vec3<t> cross(Vec3<t> v1, Vec3<t> v2) { return v1^v2; }

	inline Vec3<t> cross(const Vec3<t>& V) const {
        return Vec3<t>(
            y * V.z - z * V.y, // x component
            z * V.x - x * V.z, // y component
            x * V.y - y * V.x  // z component
        );
    }
};

typedef Vec2<float>   Vec2f;
typedef Vec2<int>     Vec2i;
typedef Vec3<float>   Vec3f;
typedef Vec3<int>     Vec3i;
typedef vec<4, float> Vec4f;
typedef vec<4, int>   Vec4i;

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

class Matrix {
	private:
		std::vector<std::vector<float>> m;
		int rows, cols; // Renamed for clarity

		float determinantRecursive(const std::vector<std::vector<float>>& mat) const;
    	Matrix getCofactorMatrix(int p, int q) const;
	
	public:
		Matrix(int rows = 4, int cols = 4);
		Matrix(const std::vector<std::vector<float>>& values);
	
		// Accessors
		int getRows() const;
		int getCols() const;
	
		// Operators
		Matrix operator+(const Matrix& other) const;
		Matrix operator-(const Matrix& other) const;
		Matrix operator*(const Matrix& other) const;
		Matrix operator*(float scalar) const;
		
		std::vector<float>& operator[](int index);
		const std::vector<float>& operator[](int index) const;
	
		// Methods
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
};

#endif //__GEOMETRY_H__
