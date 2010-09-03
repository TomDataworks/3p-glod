#ifndef _MATHLIB_H_
#define _MATHLIB_H_

#include <string.h>
#include <iostream>
#include <algorithm>
#include <math.h>

using namespace std;

#ifndef M_PI
#define M_PI		3.14159265359f
#endif

#ifndef EPSILON
#define	EPSILON		1.0e-4f
#endif

#ifndef INFINITY
#define INFINITY	1.0e+8f
#endif
	
class	Point;
class	Vector;
class	Matrix;
class	BBox;

class MatrixException {
public:
	char* mb_err;
	MatrixException(char* str="Error.")
	{
		mb_err = str;
	}
};

class Matrix {
public:
	Matrix() {
		m = n = 0;
		mb_element = NULL;
	}
	Matrix(int _m, int _n, float* element = NULL) {
		m = n = 0;
		mb_element = NULL;	
		create(_m, _n, element);
	}
	Matrix(const Matrix &mat) {
		m = n = 0;
		mb_element = NULL;
		create(mat.m, mat.n, mat.mb_element);
	}
	Matrix(const Matrix *mat) {
		m = n = 0;
		mb_element = NULL;
		create(mat->m, mat->n, mat->mb_element);
	}
	~Matrix() {
		if(mb_element)
			delete [] mb_element;
	}
    Matrix operator+(const Matrix &mat) const
	{
		if(!this->checkSameSize(mat))
			throw MatrixException("Adding matrices with different sizes.");
		int _m = this->m;
		int _n = this->n;
		Matrix tmp(_m, _n);
		for(int i=0; i<_m*_n; i++)
			tmp.mb_element[i] = this->mb_element[i] + mat.mb_element[i];
		return tmp;
	}
	Matrix &operator+=(const Matrix &mat)
	{
		if(!this->checkSameSize(mat))
			throw MatrixException("Adding matrices with different sizes.");
		int _m = this->m;
		int _n = this->n;
		for(int i=0; i<_m*_n; i++)
			this->mb_element[i] += mat.mb_element[i];
		return *this;		
	}
	Matrix operator-(const Matrix &mat) const
	{
		if(!this->checkSameSize(mat))
			throw MatrixException("Subtracting matrices with different sizes.");
		int _m = this->m;
		int _n = this->n;
		Matrix tmp(_m, _n);
		for(int i=0; i<_m*_n; i++)
			tmp.mb_element[i] = this->mb_element[i] - mat.mb_element[i];
		return tmp;
	}
	Matrix &operator-=(const Matrix &mat)
	{
		if(!this->checkSameSize(mat))
			throw MatrixException("Subtracting matrices with different sizes.");
		int _m = this->m;
		int _n = this->n;
		for(int i=0; i<_m*_n; i++)
			this->mb_element[i] -= mat.mb_element[i];
		return *this;
	}
    Matrix operator*(const Matrix &mat) const
	{
		int _m = this->m;
		int _n = this->n;
		if(_n != mat.m)
			throw MatrixException("Multiplying matrices with invalid sizes.");
		Matrix tmp(_m, mat.n);
		int i, j, k;
		for(i=0; i<_m; i++)
			for(j=0; j<mat.n; j++)
			{
				tmp[i][j] = 0;
				for(k=0; k<_n; k++)
					tmp[i][j] = tmp[i][j] + (this->mb_element[i*_n+k]) * (mat.mb_element[k*mat.n+j]);
			}
		return tmp;
	}
	bool operator==(const Matrix &mat) const {
		if(!this->checkSameSize(mat))
			throw MatrixException("Comparing matrix with different sizes.");
		int _m = this->m;
		int _n = this->n;
		for(int i=0; i<_m*_n; i++)
			if(this->mb_element[i] != mat.mb_element[i])
				return false;
		return true;
	}
	inline Matrix& operator=(const Matrix &mat) {
		create(mat.m, mat.n, mat.mb_element);
		return *this;
	}
	inline void resize(int _m, int _n) {
		create(_m, _n);
	}
	Matrix transpose();
	Matrix inv();
	float det();
	Matrix adj();
	float* operator[](int i) {
		if(i<0 || i>=m)		throw MatrixException("Incorrect index.");
		return mb_element+i*n;
	}
	inline int getm() const {
		return m;
	}
	inline int getn() const {
		return n;
	}
	float* svd(Matrix &,Matrix &);
	friend Point operator* (Matrix &m, Point &p);
	friend Vector operator* (Matrix &m, Vector &v);
	void print() {
		int i, j;
		for( j = 0; j < m; j ++) {
			for( i = 0; i < n; i ++) {
				printf("%8f ", mb_element[j * n + i]);
			}
			printf("\n");
		}
	}

protected:
	bool checkSameSize(const Matrix &mat) const {
		if( m == mat.m && n == mat.n )	return true;
		else	return false;
	}
	bool checkSquared() const {
		if( m == n )	return true;
		else	return false;
	}
	int m, n;
	float* mb_element;
	float idf;
	Matrix uppertriangle();
	void create(int, int, float* element = NULL);
	float pythag(float, float);
};

class Vector {
public:
	// Vector Methods
	Vector(float _x=0, float _y=0, float _z=0)
		: x(_x), y(_y), z(_z) {
	}
	operator Point();
	inline Vector operator+(const Vector &v) const {
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	
	inline Vector& operator+=(const Vector &v) {
		x += v.x; y += v.y; z += v.z;
		return *this;
	}
	inline Vector operator-(const Vector &v) const {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	
	inline Vector& operator-=(const Vector &v) {
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}
	inline Vector operator*(float f) const {
		return Vector(f*x, f*y, f*z);
	}
	
	inline Vector &operator*=(float f) {
		x *= f; y *= f; z *= f;
		return *this;
	}
	inline Vector operator/(float f) const {
		float inv = 1.f / f;
		return Vector(x * inv, y * inv, z * inv);
	}
	inline Vector &operator/=(float f) {
		float inv = 1.f / f;
		x *= inv; y *= inv; z *= inv;
		return *this;
	}
	inline Vector operator-() const {
		return Vector(-x, -y, -z);
	}
	inline float operator[](int i) const { return (&x)[i]; }
	inline float &operator[](int i) { return (&x)[i]; }
	inline float lenSquared() const { return x*x + y*y + z*z; }
	inline float len() const { return sqrtf( lenSquared() ); }
	inline Vector hat() const { return (*this)/len(); }
	// Vector Public Data
	float x, y, z;
};

class Point {
public:
	// Point Methods
	Point(float _x=0, float _y=0, float _z=0)
		: x(_x), y(_y), z(_z) {
	}
	operator Vector() {
		return Vector(x, y, z);
	}
	inline Point operator+(const Vector &v) const {
		return Point(x + v.x, y + v.y, z + v.z);
	}
	inline Point &operator+=(const Vector &v) {
		x += v.x; y += v.y; z += v.z;
		return *this;
	}
	inline Vector operator-(const Point &p) const {
		return Vector(x - p.x, y - p.y, z - p.z);
	}
	inline Point operator-(const Vector &v) const {
		return Point(x - v.x, y - v.y, z - v.z);
	}
	inline Point &operator-=(const Vector &v) {
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}
	inline Point &operator+=(const Point &p) {
		x += p.x; y += p.y; z += p.z;
		return *this;
	}
	inline Point operator+(const Point &p) {
		return Point(x + p.x, y + p.y, z + p.z);
	}
	inline Point operator* (float f) const {
		return Point(f*x, f*y, f*z);
	}
	inline Point &operator*=(float f) {
		x *= f; y *= f; z *= f;
		return *this;
	}
	inline Point operator/ (float f) const {
		float inv = 1.f/f;
		return Point(inv*x, inv*y, inv*z);
	}
	inline Point &operator/=(float f) {
		float inv = 1.f/f;
		x *= inv; y *= inv; z *= inv;
		return *this;
	}
	inline float len() { return sqrtf(lenSquared()); }
	inline float lenSquared() { return (x * x + y * y + z * z); }
	inline float operator[](int i) const { return (&x)[i]; }
	inline float &operator[](int i) { return (&x)[i]; }
	// Point Public Data
	float x,y,z;
};


class Ray {
public:
    // Ray Constructor Declarations
    Ray(): mint(EPSILON), maxt(INFINITY), time(0.) {}
    Ray(const Point &origin, const Vector &direction,
        float start = EPSILON, float end = INFINITY, float t = 0.)
        : O(origin), D(direction), mint(start), maxt(end), time(t) {
    }
    // Ray Method Declarations
    Point operator()(float t) const { return O + D * t; }
    // Ray Public Data 
    Point O;
    Vector D;
    float mint, maxt; 
    float time;
};  

class BBox {
public:
    // BBox Constructors
    BBox() {
        pMin = Point( INFINITY,  INFINITY,  INFINITY);
        pMax = Point(-INFINITY, -INFINITY, -INFINITY);
    }
    BBox(const Point &p) : pMin(p), pMax(p) { }
    BBox(const Point &p1, const Point &p2) {
        pMin = Point(min(p1.x, p2.x),
                     min(p1.y, p2.y),
                     min(p1.z, p2.z));
        pMax = Point(max(p1.x, p2.x),
                     max(p1.y, p2.y),
                     max(p1.z, p2.z));
    }
    // BBox Method Declarations
    friend BBox Union(const BBox &b, const Point &p);
    friend BBox Union(const BBox &b, const BBox &b2);
    friend BBox Intersection(const BBox &b1, const BBox &b2);
    bool Overlaps(const BBox &b) const {
        bool x = (pMax.x >= b.pMin.x) && (pMin.x <= b.pMax.x);
        bool y = (pMax.y >= b.pMin.y) && (pMin.y <= b.pMax.y);
        bool z = (pMax.z >= b.pMin.z) && (pMin.z <= b.pMax.z);
        return (x && y && z);
    }
    bool Inside(const Point &pt) const {
        return (pt.x >= pMin.x && pt.x <= pMax.x &&
                pt.y >= pMin.y && pt.y <= pMax.y &&
                pt.z >= pMin.z && pt.z <= pMax.z);
    }
    void Expand(float delta) {
        pMin -= Vector(delta, delta, delta);
        pMax += Vector(delta, delta, delta);
    }
    float volume() const {
        Vector d = pMax - pMin;
        return d.x * d.y * d.z;
    }
    bool IntersectP(const Ray &ray, float *hitt0 = NULL, float *hitt1 = NULL) const;
    // BBox Public Data
    Point pMin, pMax;
};

inline Vector::operator Point()
{
	return Point(x, y, z);
}

inline Vector operator*(float f, const Vector &v) { return v*f; }

inline float Dot(const Vector &v1, const Vector &v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline float AbsDot(const Vector &v1, const Vector &v2) {
	return fabsf(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

inline Vector Cross(const Vector &v1, const Vector &v2) {
	return Vector((v1.y * v2.z) - (v1.z * v2.y),
                  (v1.z * v2.x) - (v1.x * v2.z),
                  (v1.x * v2.y) - (v1.y * v2.x));
}

inline void CoordinateSystem(const Vector &v1, Vector *v2, Vector *v3) {
	//these are boned
	/*if (fabsf(v1.x) > fabsf(v1.y)) {
		float invLen = 1.f / sqrtf(v1.x*v1.x + v1.z*v1.z);
		*v2 = Vector(-v1.z * invLen, 0.f, v1.x * invLen);
	}
	else {
		float invLen = 1.f / sqrtf(v1.y*v1.y + v1.z*v1.z);
		*v2 = Vector(0.f, v1.z * invLen, -v1.y * invLen);
	}
	*v3 = Cross(v1, *v2);*/
	
	
	/*if(v1.y != 1 && v1.y != -1)
	{
		*v2 = Vector(v1.z, 0, -v1.x).hat();
		*v3 = Cross(v1, *v2);
	}
	else
	{
		*v2 = Vector(1, 0, 0);
		*v3 = Vector(0, 0, 1);
	}*/
	
	
	/* third try: using stereographi projection parametrization */
	/* notice it could be the singular point (0, -1, 0) */
	
	if((v1.y + 1.f) < 1.0e-8)
		*v2 = Vector(0, 0, 0);
	else
		*v2 = Vector((v1.y + 1.f) * (v1.y + 1.f) + v1.z * v1.z - v1.x * v1.x,
					 -2.f * v1.x * (v1.y + 1.f),
					 -2.f * v1.x * v1.z).hat();
	*v3 = Cross(v1, *v2);
}

inline float Distance(const Point &p1, const Point &p2) {
	return (p1 - p2).len();
}

inline float DistanceSquared(const Point &p1, const Point &p2) {
	return (p1 - p2).lenSquared();
}

inline Point operator*(float f, const Point &p) { return p*f; }

inline Vector SphericalDirection(float sintheta, float costheta,
		float phi) {
	return Vector(sintheta * cosf(phi),
		sintheta * sinf(phi), costheta);
}

inline Vector SphericalDirection(float sintheta, float costheta,
		float phi, const Vector &x, const Vector &y,
		const Vector &z) {
	return sintheta * cosf(phi) * x +
		sintheta * sinf(phi) * y + costheta * z;
}

inline float SphericalTheta(const Vector &v) {
	return acosf(v.z);
}

inline float SphericalPhi(const Vector &v) {
	return atan2f(v.y, v.x) + (float)M_PI;
}

Point operator* (Matrix &m, Point &p);
Vector operator* (Matrix &m, Vector &v);

bool read_mat4(char *fname, Matrix &m);

#endif


