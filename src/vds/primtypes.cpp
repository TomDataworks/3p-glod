/******************************************************************************
 * Copyright 2004 David Luebke, Brenden Schubert                              *
 *                University of Virginia                                      *
 ******************************************************************************
 * This file is distributed as part of the VDSlib library, and, as such,      *
 * falls under the terms of the VDSlib public license. VDSlib is distributed  *
 * without any warranty, implied or otherwise. See the VDSlib license for     *
 * more details.                                                              *
 *                                                                            *
 * You should have recieved a copy of the VDSlib Open-Source License with     *
 * this copy of VDSlib; if not, please visit the VDSlib web page,             *
 * http://vdslib.virginia.edu/license for more information.                   *
 ******************************************************************************/
#ifdef _WIN32
//stop compiler from complaining about exception handling being turned off
#pragma warning(disable: 4530)
#endif

#include <string.h> // memset decl
#include "primtypes.h"

using namespace VDS;
using namespace std;


const Vec3 Vec3::i(1.0, 0.0, 0.0);
const Vec3 Vec3::j(0.0, 1.0, 0.0);
const Vec3 Vec3::k(0.0, 0.0, 1.0);

const Vec2 Vec2::i(1.0, 0.0);
const Vec2 Vec2::j(0.0, 1.0);

// POINT 2 CLASS*************************************
Point2::Point2()
{
}

Point2::Point2(Float x, Float y)
{
    X = x, Y = y;
}

Point2::Point2(const Point2 &p)
{
    X = p.X, Y = p.Y;
}

Point2::Point2(const Point3 &p)
{
    X = p.X, Y = p.Y;
}

Point2::Point2(const Vec2 &vec)
{
    X = vec.X, Y = vec.Y;
}

Point2 &Point2::operator =(const Point2 &p)
{
    X = p.X, Y = p.Y;
    return *this;
}

bool Point2::operator ==(const Point2 &p) const
{
    return X == p.X && Y == p.Y;
}

bool Point2::operator !=(const Point2 &p) const
{
    return !(p == *this);
}

Point2 &Point2::operator +=(const Vec2 &v)
{
    X += v.X, Y += v.Y;
    return *this;
}

Point2 &Point2::operator -=(const Vec2 &v)
{
    X -= v.X, Y -= v.Y;
    return *this;
}

Float Point2::DistanceToSquared(const Point2 &p) const
{
    return (X - p.X) * (X - p.X) + (Y - p.Y) * (Y - p.Y);
}

Float Point2::DistanceTo(const Point2 &p) const
{
    return sqrt(DistanceToSquared(p));
}

const Point2 Point2::AverageWith(const Point2 &p) const
{
    return Point2((X + p.X) / 2, (Y + p.Y) / 2);
}

void Point2::Set(Float x, Float y)
{
    X = x, Y = y;
}
//alternative access method
Float &Point2::operator [](unsigned char i)
{
    assert(i == 0 || i == 1);
    switch (i)
    {
    case 0:
        return X;
        break;
    case 1:
        return Y;
        break;
    default:
        return X; //arbitrary, shuts up compiler warning
        break;
    }
}
const Float &Point2::operator [](unsigned char i) const
{
    assert(i == 0 || i == 1);
    switch (i)
    {
    case 0:
        return X;
        break;
    case 1:
        return Y;
        break;
    default:
        return X; //arbitrary, shuts up compiler warning
        break;
    }
}

// VEC2 CLASS****************************************

Vec2::Vec2()
{
}

Vec2::Vec2(Float x, Float y)
{
    X = x, Y = y;
}

Vec2::Vec2(Point2 &p)
{
    X = p.X, Y = p.Y;
}

Vec2::Vec2(Vec3 &v)
{
    X = v.X, Y = v.Y;
}

//vector addition
Vec2 Vec2::operator +(const Vec2 &op) const
{
    return Vec2(X + op.X, Y + op.Y);
}

void Vec2::operator +=(const Vec2 &op)
{
    X += op.X, Y += op.Y;    
}

Vec2 Vec2::operator -(const Vec2 &op) const
{
    return Vec2(X - op.X, Y - op.Y);
}

void Vec2::operator -=(const Vec2 &op)
{
    X -= op.X, Y -= op.Y;    
}

Vec2 Vec2::operator -() const
{
    return Vec2(-X, -Y);    
}

//Dot Product
Float Vec2::operator *(const Vec2 &op) const
{
    return (X * op.X) + (Y * op.Y);
}

//Scalar Mult
Vec2 Vec2::operator *(const Float &scale) const
{
    return Vec2(X * scale, Y * scale);
}

Vec2 Vec2::operator *=(const Float &scale)
{
    X *= scale, Y *= scale;
    return *this;
}

//Scalar Div
Vec2 Vec2::operator /(const Float &scale) const
{
    return Vec2(X / scale, Y / scale);
}

Vec2 Vec2::operator /=(const Float &scale)
{
    X /= scale, Y /= scale;
    return *this;
}

//assignment
Vec2 &Vec2::operator =(const Vec2 &op)
{
    X = op.X, Y = op.Y;
    return *this;
}

//value comparison
bool Vec2::operator ==(const Vec2 &op) const
{
    return (X == op.X && Y == op.Y);
}

bool Vec2::operator !=(const Vec2 &op) const
{
    return !(op == *this);
}

//length comparisons
bool Vec2::operator >(const Vec2 &op) const
{
    return (Length() > op.Length());
}

bool Vec2::operator >=(const Vec2 &op) const
{
    return (Length() >= op.Length());
}

bool Vec2::operator <(const Vec2 &op) const
{
    return (Length() < op.Length());
}

bool Vec2::operator <=(const Vec2 &op) const
{
    return (Length() <= op.Length());
}

Float Vec2::LengthSquared() const
{
    return X*X + Y*Y;
}

Float Vec2::Length() const
{
    return sqrt(LengthSquared());
}

void Vec2::Normalize()
{
    *this /= Length();
}

Vec2 Vec2::Normalized()
{
    return *this / Length();
}


Float Vec2::AngleBetween(const Vec2 &op) const
{
    return (*this / Length()) * (op / op.Length());
}

void Vec2::Set(Float x, Float y)
{
    X = x, Y = y;
}
//alternative access method
Float &Vec2::operator [](unsigned char i)
{
    assert((i == 0) || (i == 1));
    switch (i)
    {
    case 0:            
        return X;
        break;
    case 1:            
        return Y;
        break;
    default:
        return Y;  //arbitrary, silences compiler warnings
        break;
    }
}

const Float &Vec2::operator [](unsigned char i) const
{
    assert((i == 0) || (i == 1));
    switch (i)
    {
    case 0:            
        return X;
        break;
    case 1:            
        return Y;
        break;
    default:
        return Y;  //arbitrary, silences compiler warnings
        break;
    }
}

// POINT CLASS************************************************
// moved back to primtypes for inlining and performanc reasons
Point3::Point3(const Vec3 &vec) { 
    X = vec.X, Y = vec.Y, Z = vec.Z; 
}
Point3::Point3(const Vec4 &vec) { 
    X = vec.X/vec.W, Y = vec.Y/vec.W, Z = vec.Z/vec.W; 
}

Point3& Point3::operator +=(const Vec3 &v) {
  X += v.X, Y += v.Y, Z += v.Z;
  return *this;
}
Point3 &Point3::operator -=(const Vec3 &v) {
  X -= v.X, Y -= v.Y, Z -= v.Z;
  return *this;
}
Float Point3::operator *(const Plane3 &plane) const {
  return plane.A * X + plane.B * Y + plane.C * Z - plane.D;				
}
Float Point3::DistanceToSquared(const Point3 &p) const {
  return	(X - p.X) * (X - p.X) + 
    (Y - p.Y) * (Y - p.Y) + 
    (Z - p.Z) * (Z - p.Z);
}
Float Point3::DistanceTo(const Point3 &p) const {
  return sqrt(DistanceToSquared(p));
}

const Point3 Point3::AverageWith(const Point3 &p) const {
  return Point3((X + p.X) / 2.0, (Y + p.Y) / 2.0, (Z + p.Z) / 2.0);				
}



// Vec3 CLASS****************************************
Vec3::Vec3() : X(0), Y(0), Z(0)
{
}

Vec3::Vec3(Float x, Float y, Float z) : X(x), Y(y), Z(z)
{
}

//create a Vec3 going from one point to another
Vec3::Vec3(const Point3 &src, const Point3 &dest) : X(dest.X - src.X), Y(dest.Y - src.Y), Z(dest.Z - src.Z)
{
}

Vec3::Vec3(const Vec3 &vec)
{
    X = vec.X, Y = vec.Y, Z  = vec.Z; 
}

Vec3::Vec3(const Vec4 &vec)
{
    X = vec.X / vec.W, Y = vec.Y / vec.W, Z = vec.Z / vec.W;
}

Vec3::Vec3(const Point3 &point)
{
    X = point.X, Y = point.Y, Z = point.Z;
}

//vector addition
Vec3 Vec3::operator +(const Vec3 &op) const
{
	return Vec3(X + op.X, Y + op.Y, Z + op.Z);
}

void Vec3::operator +=(const Vec3 &op)
{
	X += op.X, Y += op.Y, Z += op.Z;
}

Vec3 Vec3::operator -(const Vec3 &op) const
{
	return Vec3(X - op.X, Y - op.Y, Z - op.Z);
}

void Vec3::operator -=(const Vec3 &op)
{
	X -= op.X, Y -= op.Y, Z -= op.Z;
}

Vec3 Vec3::operator -() const
{
    return Vec3(-X, -Y, -Z);
}

//Dot Product
Float Vec3::operator *(const Vec3 &op) const
{
	return X * op.X + Y * op.Y + Z * op.Z;
}

//Scalar Mult
Vec3 Vec3::operator *(const Float &scale) const
{
	return Vec3(X * scale, Y * scale, Z * scale);
}

Vec3 Vec3::operator *=(const Float &scale)
{
	X *= scale, Y *= scale, Z *= scale;
	return *this;
}

//Scalar Div
Vec3 Vec3::operator /(const Float &scale) const
{
	return Vec3(X / scale, Y / scale, Z / scale);
}

Vec3 Vec3::operator /=(const Float &scale)
{
	X /= scale, Y /= scale, Z /= scale;
	return *this;
}

//Cross Product
Vec3 Vec3::operator %(const Vec3 &op) const
{
	return Vec3(Y * op.Z - Z * op.Y,
				Z * op.X - X * op.Z,
				X * op.Y - Y * op.X);		
}

Vec3 Vec3::operator %=(const Vec3 &op)
{
	Float newX = Y * op.Z - Z * op.Y;
	Float newY = Z * op.X - X * op.Z;
	Float newZ = X * op.Y - Y * op.X;
	X = newX, Y = newY, Z = newZ;
	return *this;
}

//assignment
Vec3 &Vec3::operator =(const Vec3 &op)
{
	X = op.X;
	Y = op.Y;
	Z = op.Z;
	return *this;
}

bool Vec3::operator ==(const Vec3 &op) const
{
	return (X == op.X) && (Y == op.Y) && (Z == op.Z);
}

bool Vec3::operator !=(const Vec3 &op) const
{
	return !(*this == op);
}

//length comparisons
bool Vec3::operator >(const Vec3 &op) const
{
	return LengthSquared() > op.LengthSquared();
}

bool Vec3::operator >=(const Vec3 &op) const
{
	return LengthSquared() >= op.LengthSquared();
}

bool Vec3::operator <(const Vec3 &op) const
{
	return LengthSquared() < op.LengthSquared();
}

bool Vec3::operator <=(const Vec3 &op) const
{
	return LengthSquared() <= op.LengthSquared();
}

Float Vec3::LengthSquared() const
{
	return X*X + Y*Y + Z*Z;
}

Float Vec3::Length() const
{
	return sqrt(LengthSquared());
}

void Vec3::Normalize()
{
	*this /= Length();
}

Vec3 Vec3::Normalized()
{
	return *this / Length();
}


Float Vec3::AngleBetween(const Vec3 &op) const
{
    return (*this / Length()) * (op / op.Length());
}

void Vec3::Set(Float x, Float y, Float z)
{
    X = x;
    Y = y;
    Z = z;
}

// Mat3 CLASS****************************************

// Constructors

Mat3::Mat3()
{
	rows[0] = Vec3(1, 0, 0);
	rows[1] = Vec3(0, 1, 0);
	rows[2] = Vec3(0, 0, 1);
}

Mat3::Mat3(float a, float b, float c, float d, float e, float f, float g, float h, float i)
{
	rows[0] = Vec3(a,b,c);
	rows[1] = Vec3(d,e,f);
	rows[2] = Vec3(g,h,i);
}

Mat3::Mat3(const Vec3 &row0, const Vec3 &row1, const Vec3 &row2)
{
    rows[0] = row0;
    rows[1] = row1;
    rows[2] = row2;
}

Mat3::Mat3(const Mat3 &mat)
{
    rows[0] = mat.rows[0];
    rows[1] = mat.rows[1];
    rows[2] = mat.rows[2];
}

Mat3::Mat3(const Vec3 &vec)
{
	rows[0] = Vec3(vec.X*vec.X, vec.X*vec.Y, vec.X*vec.Z);
	rows[1] = Vec3(vec.Y*vec.X, vec.Y*vec.Y, vec.Y*vec.Z);
	rows[2] = Vec3(vec.Z*vec.X, vec.Z*vec.Y, vec.Z*vec.Z);
}

// Assignment VDS::operators	
Mat3& Mat3::operator =(const Mat3 &mat)
{
    rows[0] = mat.rows[0];
    rows[1] = mat.rows[1];
    rows[2] = mat.rows[2];
    return *this;
}

Mat3 Mat3::operator +(const Mat3 &mat) const
{
    return Mat3(rows[0] + mat.rows[0],
                rows[1] + mat.rows[1],
                rows[2] + mat.rows[2]);
}

Mat3 &Mat3::operator +=(const Mat3& mat)
{
    rows[0] += mat.rows[0];
    rows[1] += mat.rows[1];
    rows[2] += mat.rows[2];
    return *this;
}

Mat3 Mat3::operator -(const Mat3 &mat) const
{
    return Mat3(rows[0] - mat.rows[0],
                rows[1] - mat.rows[1],
                rows[2] - mat.rows[2]);
}

Mat3 &Mat3::operator -=(const Mat3 &mat)
{
    rows[0] -= mat.rows[0];
    rows[1] -= mat.rows[1];
    rows[2] -= mat.rows[2];
    return *this;
}

Mat3 Mat3::operator -() const
{
    return Mat3(-rows[0], -rows[1], -rows[2]);
}

Mat3 Mat3::operator *(const Mat3 &mat) const
{        
    Mat3 result;
    unsigned char col, row;
    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
        {
            result[row][col] = rows[row][0] * mat.rows[0][col]
                             + rows[row][1] * mat.rows[1][col]
                             + rows[row][2] * mat.rows[2][col];
        }
    }
    return result;
}

Mat3 Mat3::operator *(const float d) const
{
    return Mat3(rows[0] * d,
                rows[1] * d,
                rows[2] * d);
}

Mat3& Mat3::operator *=(const float d)
{
    rows[0] *= d;
    rows[1] *= d;
    rows[2] *= d;
    return *this;
}

Vec3 Mat3::operator *(const Vec3 &v) const
{
    return Vec3(rows[0][0] * v[0] + rows[0][1] * v[1] + rows[0][2] * v[2],
                rows[1][0] * v[0] + rows[1][1] * v[1] + rows[1][2] * v[2],
                rows[2][0] * v[0] + rows[2][1] * v[1] + rows[2][2] * v[2]);
}

Point3 Mat3::operator *(const Point3 &point) const
{        
    Vec3 tmp = (*this * Vec3(point));        
    return Point3(tmp.X, tmp.Y, tmp.Z);
}

Mat3 Mat3::operator /(const float d) const
{
    return Mat3(rows[0] / d,
                rows[1] / d,
                rows[2] / d);
}

Mat3 &Mat3::operator /=( const float d)
{
    rows[0] /= d;
    rows[1] /= d;
    rows[2] /= d;
    return *this;
}

bool Mat3::operator ==(const Mat3 &mat) const
{
    if (rows[0] == mat.rows[0] && rows[1] == mat.rows[1] && rows[2] == mat.rows[2])
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Mat3::operator !=(const Mat3 &mat) const
{
    return !(mat == *this);
}

Mat3 Mat3::Transpose() const
{
    Mat3 result;        
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result.rows[i][j] = rows[j][i];
        }
    }
    return result;
}

//Mat3 Inverse(); //implement if needed	

void Mat3::Set(const Vec3& row0, const Vec3& row1, const Vec3& row2)
{
    rows[0] = row0;
    rows[1] = row1;
    rows[2] = row2;
}

//return a row from the matrix
Vec3 &Mat3::operator [](unsigned char i)
{
    assert ((i == 0) || (i == 1) || (i == 2));
    return rows[i];
}

const Vec3 &Mat3::operator [](unsigned char i) const
{
    assert ((i == 0) || (i == 1) || (i == 2));
    return rows[i];
}

// Vec4 CLASS****************************************
//create a Vec4 going from one point to another
// moved back to primtypes.h for performance reasons

// Plane3 CLASS**************************************
Plane3::Plane3()
{
	A = 0; B = 0; C = 0; D = 0;
}

Plane3::Plane3(Float a, Float b, Float c, Float d)
{
	A = a; B = b; C = c; D = d;
}

Plane3::Plane3(const Plane3 &plane)
{
	A = plane.A;  B = plane.B;  C = plane.C;  D = plane.D;
}

void Plane3::Set(Float a, Float b, Float c, Float d)
{
	A = a;  B = b;  C = c;  D = d;
}

void Plane3::SetNormal(const Vec3 &n)
{
	float length = n.Length();

	A = n[0]/length;  B = n[1]/length;  C = n[2]/length;
}

void Plane3::SetD(const float &d)
{
	D = d;
}

void Plane3::SetPointOnPlane(const Point3 &p)
{
	Vec3 normal(A, B, C);

	assert(!(A==0 && B==0 && C==0) /* All coefs zero; has normal been set yet? */);
	D = normal * (Vec3) p;
}

Float Plane3::operator *(const Point3 &p) const
{
	return A * p.X + B * p.Y + C * p.Z - D;
}

Point3 Plane3::Intersect3Planes(const Plane3 &p1, const Plane3 &p2)
{
	VDS::Vec3 N1(p1.A, p1.B, p1.C);
	float d1 = p1.D;
	VDS::Vec3 N2(p2.A, p2.B, p2.C);
	float d2 = p2.D;
	VDS::Vec3 N3(this->A, this->B, this->C);
	float d3 = this->D;

	VDS::Vec3 intvec = ((N2%N3) * d1 + (N3%N1) * d2 + (N1%N2) * d3) / (N1*(N2%N3));
	VDS::Point3 intersection(intvec.X, intvec.Y, intvec.Z);
	return intersection;
}

// Mat4 CLASS****************************************

#if 0
Mat4::Mat4(const float m[16])
{
/*	rows[0][0] = m[0];
	rows[0][1] = m[1];
	rows[0][2] = m[2];
	rows[0][3] = m[3];

	rows[1][0] = m[4];
	rows[1][1] = m[5];
	rows[1][2] = m[6];
	rows[1][3] = m[7];

	rows[2][0] = m[8];
	rows[2][1] = m[9];
	rows[2][2] = m[10];
	rows[2][3] = m[11];

	rows[3][0] = m[12];
	rows[3][1] = m[13];
	rows[3][2] = m[14];
	rows[3][3] = m[15];
*/
// COLUMN-major, not row-major!
}

void Mat4::Set(const float m[16]) { // col major
#endif

// Assignment VDS::operators	
Mat4& Mat4::operator =(const Mat4 &mat)
{
    memcpy(this->cells, mat.cells, sizeof(float) * 16);
    return *this;
}

Mat4 Mat4::operator +(const Mat4 &mat) const
{
    // Vec4 addition does homogeneous vector addition
    Mat4 ret;
    for (int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
            ret.cells[i][j] = cells[i][j] + mat.cells[i][j];
    }
    return ret;
}

Mat4& Mat4::operator +=(const Mat4& mat)
{
    // Vec4 addition does homogeneous vector addition
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cells[i][j] += mat.cells[i][j];
        }
    }
    return *this;
}

Mat4 Mat4::operator -(const Mat4 &mat) const
{
    // Vec4 subtraction does homogeneous vector subtraction
    Mat4 ret;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ret.cells[i][j] = cells[i][j] - mat.cells[i][j];
        }
    }
    return ret;
}

Mat4 &Mat4::operator -=(const Mat4 &mat)
{
    // Vec4 subtraction does homogeneous vector subtraction
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cells[i][j] -= mat.cells[i][j];
        }
    }
    return *this;
}

Mat4 Mat4::operator -() const
{
    // Vec4 negation does not negate W
    Mat4 ret;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ret.cells[i][j] = -cells[i][j];
        }
    }
    return ret;
}

Mat4 Mat4::operator *(const Mat4 &mat) const
{        
    Mat4 result;
    unsigned char col, row;
    for (row = 0; row < 4; row++)
    {
        for (col = 0; col < 4; col++)
        {
            result.cells[row][col] = cells[row][0] * mat.cells[0][col]
                + cells[row][1] * mat.cells[1][col]
                + cells[row][2] * mat.cells[2][col]
                + cells[row][3] * mat.cells[3][col];
        }
    }
    return result;
}

Mat4 Mat4::operator *(const float d) const
{
    // Vec4 scaling does homogeneous vector scaling
    Mat4 ret;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ret.cells[i][j] = cells[i][j] * d;
        }
    }
    return ret;
}

Mat4& Mat4::operator *=(const float d)
{
    // Vec4 scaling does homogeneous vector scaling
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cells[i][j] *= d;
        }
    }
    return *this;
}

Point3 Mat4::operator *(const Point3 &point) const
{        
    Vec4 tmp = (*this * Vec4(point));
    tmp.Homogonize();
    return Point3(tmp.X / tmp.W, tmp.Y / tmp. W, tmp.Z / tmp.W);
}

Mat4 Mat4::operator /(const float d) const
{
    // Vec4 scaling does homogeneous vector scaling
    Mat4 ret;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ret.cells[i][j] = cells[i][j] / d;
        }
    }
    return ret;
}

Mat4 &Mat4::operator /=( const float d)
{
    // Vec4 scaling does homogeneous vector scaling
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cells[i][j] /= d;
        }
    }
    return *this;
}

bool Mat4::operator ==(const Mat4 &mat) const
{
    //Vec4 equality does homogeneous equality
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (cells[i][j] != mat.cells[i][j])
            {
                return false;
            }
        }
    }
    return true;
}

bool Mat4::operator !=(const Mat4 &mat) const
{
    return !(mat == *this);
}

Mat4 Mat4::Transpose() const
{
    Mat4 result;        
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.cells[i][j] = cells[j][i];
        }
    }
    return result;
}
/*
Mat4 Mat4::Inverse() const //code taken from Paul Rademacher's algebra3 code
{
    Mat4 a, b;        
    int i, j, k, i1;
    Vec4 temp;

    b.rows[0][0] = b.rows[1][1] = b.rows[2][2] = b.rows[3][3] = 1.0;
    b.rows[1][0] = b.rows[2][0] = b.rows[3][0] = 0.0;
    b.rows[0][1] = b.rows[2][1] = b.rows[3][1] = 0.0;
    b.rows[0][2] = b.rows[1][2] = b.rows[3][2] = 0.0;
    b.rows[0][3] = b.rows[1][3] = b.rows[2][3] = 0.0;
    a = *this;
    // Loop over cols of a from left to right, eliminating above and below diag
    for (j=0; j<4; j++)
    {   // Find largest pivot in column j among rows j..3
        i1 = j;		    // Row with largest pivot candidate
        for (i=j+1; i<4; i++)
        {
    	    if (fabs(a.rows[i][j]) > fabs(a.rows[i1][j]))
            {
            	i1 = i;
            }
        }

        // Swap rows i1 and j in a and b to put pivot on diagonal            
        temp = a.rows[i1];
        a.rows[i1] = a.rows[j];
        a.rows[j] = temp;

        temp = b.rows[i1];
        b.rows[i1] = b.rows[j];
        b.rows[j] = temp;


        // Scale row j to have a unit diagonal
        if (a.rows[j][j]==0.0)
        {
            std::cerr << "Tried to invert singular Mat4" << std::endl;
            return b;
        }
        //can't use vector division b/c it is homogenous subtraction
        for (k = 0; k < 4; k++)
        {
            b.rows[j][k] /= a.rows[j][j];
            a.rows[j][k] /= a.rows[j][j];
        }

        // Eliminate off-diagonal elems in col j of a, doing identical ops to b
        for (i = 0; i < 4; i++)
        {
	        if (i != j) 
            {
                //can't use vector subtraction b/c it is homogenous subtraction
                for (k = 0; k < 4; k++)
                {
	                b.rows[i][k] -=  b.rows[j][k] * a.rows[i][j];
	                a.rows[i][k] -=  a.rows[j][k] * a.rows[i][j];
                }
	        }
        }
    }
    return b;
}
*/

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}
Mat4 Mat4::Inverse() const //code taken from Paul Rademacher's algebra3 code
{
    Mat4 a;
    float indxc[4] = {0, 0, 0, 0};
    float indxr[4] = {0, 0, 0, 0};
    float ipiv[4] = {0, 0, 0, 0};
	int i, icol, irow, j, k, l, ll;
	float big, dum, pivinv, temp;

    a = *this;

	for (i = 0; i < 4; ++i)
	{
		big = 0.0;
		for (j = 0; j < 4; ++j)
		{
			if (ipiv[j] != 1)
			{
				for (k = 0; k < 4; ++k)
				{
					if (ipiv[k] == 0)
					{
						if (fabs(a.cells[j][k]) >= big)
						{
							big = fabs(a.cells[j][k]);
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1)
					{
						std::cerr << "Error: Tried to invert singular Mat4" << std::endl;
					    memset(&a, 0, sizeof(a));
						return a;
					}
				}
			}
		}
		++(ipiv[icol]);

		if (irow != icol)
		{
            for (l = 0; l < 4; ++l) {               
                SWAP(a.cells[irow][l],a.cells[icol][l])
            }
		}
		indxr[i] = (VDS::Float)irow;
		indxc[i] = (VDS::Float)icol;
		if (fabs(a.cells[icol][icol])<.0001)
		{
			//std::cerr << "Malfunction: Tried to invert singular Mat4" << std::endl;
		    memset(&a, 0, sizeof(a));
		    return a;
		}
		pivinv = 1.0 / a.cells[icol][icol];
		a.cells[icol][icol] = 1.0;
		for (l = 0; l < 4; ++l)
			a.cells[icol][l] *= pivinv;
		for (ll = 0; ll < 4; ++ll)
		{
			if (ll != icol)
			{
				dum = a.cells[ll][icol];
				a.cells[ll][icol] = 0.0;
				for (l = 0; l < 4; ++l)
					a.cells[ll][l] -= a.cells[icol][l] * dum;
			}
		}
	}
    
	for (l = 3; l >= 0; --l)
	{
		if (indxr[l] != indxc[l])
		{
			for (k = 0; k < 4; ++k)
				SWAP(a.cells[k][(char)(indxr[l])],a.cells[k][(char)indxc[l]])
		}
	}
	
	return a;
}

void Mat4::Set(const Vec4& row0, const Vec4& row1, const Vec4& row2, const Vec4& row3)
{
    SetRow(0, row0);
    SetRow(1, row1);
    SetRow(2, row2);
    SetRow(3, row3);
}

//return a row from the matrix
//this is kind of dangerous b/c allows homogenous vector operations on a matrix row.
/*Vec4 &Mat4::operator [](unsigned char i)
{
#ifdef _DEBUG
    assert ((i == 0) || (i == 1) || (i == 2) || (i == 3));
#endif
    return rows[i];
}

const Vec4 &Mat4::operator [](unsigned char i) const
{
#ifdef _DEBUG
    assert ((i == 0) || (i == 1) || (i == 2) || (i == 3));
#endif
    return rows[i];
}*/


// FloatColor CLASS****************************************


FloatColor::FloatColor(Float red, Float green, Float blue)
{
    R = red;
    G = green;
    B = blue;
}

FloatColor::FloatColor()
{
    R = 0.0;
    G = 0.0;
    B = 0.0;
}

FloatColor::FloatColor(const FloatColor &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
}

FloatColor::FloatColor(const FloatColorA &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
}

FloatColor::FloatColor(const ByteColor &color)
{
    R = (Float) color.R / 255.0;
    G = (Float) color.G / 255.0;
    B = (Float) color.B / 255.0;
}

FloatColor::FloatColor(const ByteColorA &color)
{
    R = (Float) color.R / 255.0;
    G = (Float) color.G / 255.0;
    B = (Float) color.B / 255.0;
}

FloatColor &FloatColor::operator =(const FloatColor &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
    return *this;
}

bool FloatColor::operator ==(const FloatColor &color) const
{
    return ((R == color.R) && (G == color.G) && (B == color.B));
}

bool FloatColor::operator !=(const FloatColor &color) const
{
    return !(*this == color);
}

//alternative access
void FloatColor::Clamp()
{
   if (R > 1.0)
   {
       R = 1.0;
   }
   else if (R < 0.0)
   {
       R = 0.0;
   }
   if (G > 1.0)
   {
       G = 1.0;
   }
   else if (G < 0.0)
   {
       G = 0.0;
   }
   if (B > 1.0)
   {
       B = 1.0;
   }
   else if (B < 0.0)
   {
       B = 0.0;
   }
}


//alternative access
Float &FloatColor::operator [](unsigned int i)
{
    assert (i == 0 || i == 1 || i == 2);
    switch(i)
    {
    case 0:
        return R;
        break;
    case 1:
        return G;
        break;
    case 2:
        return B;
        break;
    default:
        return R;  //arbitrary, silences compiler warnings
        break;
    }
}

const Float &FloatColor::operator [](unsigned int i) const
{
    assert (i == 0 || i == 1 || i == 2);
    switch(i)
    {
    case 0:
        return R;
        break;
    case 1:
        return G;
        break;
    case 2:
        return B;
        break;
    default:
        return R;  //arbitrary, silences compiler warnings
        break;
    }
}

// ByteColor CLASS****************************************


ByteColor::ByteColor(Byte red, Byte green, Byte blue)
{
    R = red;
    G = green;
    B = blue;
}

ByteColor::ByteColor()
{
    R = 0;
    G = 0;
    B = 0;
}

ByteColor::ByteColor(const ByteColor &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
}

ByteColor::ByteColor(const ByteColorA &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
}

ByteColor::ByteColor(const FloatColor &color)
{
    R = (Byte)(color.R * 255.0);
    G = (Byte)(color.G * 255.0);
    B = (Byte)(color.B * 255.0);
}

ByteColor::ByteColor(const FloatColorA &color)
{
    R = (Byte)(color.R * 255.0);
    G = (Byte)(color.G * 255.0);
    B = (Byte)(color.B * 255.0);
}

ByteColor &ByteColor::operator =(const ByteColor &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
    return *this;
}

bool ByteColor::operator ==(const ByteColor &color) const
{
    return ((R == color.R) && (G == color.G) && (B == color.B));
}

bool ByteColor::operator !=(const ByteColor &color) const
{
    return !(*this == color);
}

//alternative access
Byte &ByteColor::operator [](unsigned int i)
{
    assert (i == 0 || i == 1 || i == 2);
    switch(i)
    {
    case 0:
        return R;
        break;
    case 1:
        return G;
        break;
    case 2:
        return B;
        break;
    default:
        return R;  //arbitrary, silences compiler warnings
        break;
    }
}

const Byte &ByteColor::operator [](unsigned int i) const
{
    assert (i == 0 || i == 1 || i == 2);
    switch(i)
    {
    case 0:
        return R;
        break;
    case 1:
        return G;
        break;
    case 2:
        return B;
        break;
    default:
        return R;  //arbitrary, silences compiler warnings
        break;
    }
}

// FloatColorA CLASS****************************************

FloatColorA::FloatColorA(Float red, Float green, Float blue, Float alpha)
{
    R = red;
    G = green;
    B = blue;
    A = alpha;
}

FloatColorA::FloatColorA()
{
    R = 0.0;
    G = 0.0;
    B = 0.0;
    A = 1.0;
}

FloatColorA::FloatColorA(const FloatColorA &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
    A = color.A;
}

FloatColorA::FloatColorA(const FloatColor &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
    A = 1.0;
}

FloatColorA::FloatColorA(const ByteColor &color)
{
    R = (Float) color.R / 255.0;
    G = (Float) color.G / 255.0;
    B = (Float) color.B / 255.0;
    A = 1.0;
}

FloatColorA::FloatColorA(const ByteColorA &color)
{
    R = (Float) color.R / 255.0;
    G = (Float) color.G / 255.0;
    B = (Float) color.B / 255.0;
    A = (Float) color.A / 255.0;
}

FloatColorA &FloatColorA::operator =(const FloatColorA &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
    A = color.A;
    return *this;
}

bool FloatColorA::operator ==(const FloatColorA &color) const
{
    return ((R == color.R) && (G == color.G) && (B == color.B) & (A == color.A));
}

bool FloatColorA::operator !=(const FloatColorA &color) const
{
    return !(*this == color);
}

//alternative access
void FloatColorA::Clamp()
{
   if (R > 1.0)
   {
       R = 1.0;
   }
   else if (R < 0.0)
   {
       R = 1.0;
   }
   if (G > 1.0)
   {
       G = 1.0;
   }
   else if (G < 0.0)
   {
       G = 0.0;
   }
   if (B > 1.0)
   {
       B = 1.0;
   }
   else if (B < 0.0)
   {
       B = 0.0;
   }
   if (A > 1.0)
   {
       A = 1.0;
   }
   else if (A < 0.0)
   {
       A = 0.0;
   }
}


//alternative access
Float &FloatColorA::operator [](unsigned int i)
{
    assert (i == 0 || i == 1 || i == 2 || i == 3);
    switch(i)
    {
    case 0:
        return R;
        break;
    case 1:
        return G;
        break;
    case 2:
        return B;
        break;
    case 3:
        return A;
        break;
    default:
        return R;  //arbitrary, silences compiler warnings
        break;
    }
}

const Float &FloatColorA::operator [](unsigned int i) const
{
    assert (i == 0 || i == 1 || i == 2 || i == 3);
    switch(i)
    {
    case 0:
        return R;
        break;
    case 1:
        return G;
        break;
    case 2:
        return B;
        break;
    case 3:
        return A;
        break;
    default:
        return R;  //arbitrary, silences compiler warnings
        break;
    }
}

// ByteColor CLASS****************************************


ByteColorA::ByteColorA(Byte red, Byte green, Byte blue, Byte alpha)
{
    R = red;
    G = green;
    B = blue;
    A = alpha;
}

ByteColorA::ByteColorA()
{
    R = 0;
    G = 0;
    B = 0;
    A = 255;
}

ByteColorA::ByteColorA(const ByteColorA &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
    A = color.A;
}

ByteColorA::ByteColorA(const ByteColor &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
    A = 255;
}

ByteColorA::ByteColorA(const FloatColorA &color)
{
    R = (Byte)(color.R * 255.0);
    G = (Byte)(color.G * 255.0);
    B = (Byte)(color.B * 255.0);
    A = (Byte)(color.A * 255.0);
}

ByteColorA::ByteColorA(const FloatColor &color)
{
    R = (Byte)(color.R * 255.0);
    G = (Byte)(color.G * 255.0);
    B = (Byte)(color.B * 255.0);
    A = 255;
}

ByteColorA &ByteColorA::operator =(const ByteColorA &color)
{
    R = color.R;
    G = color.G;
    B = color.B;
    A = color.A;
    return *this;
}

bool ByteColorA::operator ==(const ByteColorA &color) const
{
    return ((R == color.R) && (G == color.G) && (B == color.B) && (A == color.A));
}

bool ByteColorA::operator !=(const ByteColorA &color) const
{
    return !(*this == color);
}

//alternative access
Byte &ByteColorA::operator [](unsigned int i)
{
    assert (i == 0 || i == 1 || i == 2 || i == 3);
    switch(i)
    {
    case 0:
        return R;
        break;
    case 1:
        return G;
        break;
    case 2:
        return B;
        break;
    case 3:
        return A;
        break;
    default:
        return R;  //arbitrary, silences compiler warnings
        break;
    }
}

const Byte &ByteColorA::operator [](unsigned int i) const
{
    assert (i == 0 || i == 1 || i == 2 || i == 3);
    switch(i)
    {
    case 0:
        return R;
        break;
    case 1:
        return G;
        break;
    case 2:
        return B;
        break;
    case 3:
        return A;
        break;
    default:
        return R;  //arbitrary, silences compiler warnings
        break;
    }
}

Point2 VDS::operator +(const Vec2 &vec, const Point2 &point)
{
    return Point2(vec.X + point.X, vec.Y + point.Y);
}

Point2 VDS::operator +(const Point2 &point, const Vec2 &vec)
{
    return Point2(vec.X + point.X, vec.Y + point.Y);
}

Point2 VDS::operator -(const Point2 &point, const Vec2 &vec)
{
    return Point2(point.X - vec.X, point.Y - vec.Y);
}

Vec2 VDS::operator -(const Point2 &point1, const Point2 &point2)
{
    return Vec2(point1.X - point2.X, point1.Y - point2.Y);
}

Point3 VDS::operator +(const Vec3 &vec, const Point3 &point)
{
	return Point3(vec.X + point.X, vec.Y + point.Y, vec.Z + point.Z);
}

Point3 VDS::operator +(const Point3 &point, const Vec3 &vec)
{
	return Point3(vec.X + point.X, vec.Y + point.Y, vec.Z + point.Z);
}

Point3 VDS::operator -(const Point3 &point, const Vec3 &vec)
{
	return Point3(point.X - vec.X, point.Y - vec.Y, point.Z - vec.Z);
}

Vec3 VDS::operator -(const Point3 &point1, const Point3 &point2)
{
    return Vec3(point1.X - point2.X, point1.Y - point2.Y, point1.Z - point2.Z);
}

Point3 VDS::operator +(const Vec4 &vec, const Point3 &point)
{
	return Point3(vec.X/vec.W + point.X, vec.Y/vec.W + point.Y, vec.Z/vec.W +point.Z);
}

Point3 VDS::operator +(const Point3 &point, const Vec4 &vec)
{
	return Point3(vec.X/vec.W + point.X, vec.Y/vec.W + point.Y, vec.Z/vec.W +point.Z);
}

Point3 VDS::operator -(const Point3 &point, const Vec4 &vec)
{
	return Point3(point.X - vec.X/vec.W, point.Y - vec.Y/vec.W, point.Z - vec.Z/vec.W);
}

std::ostream &VDS::operator <<(std::ostream &out, const Point2 &point)
{
    out << point.X << " " << point.Y << " ";
    return out;
}

std::ostream &VDS::operator <<(std::ostream &out, const Vec2 &vec)
{
    out << vec.X << " " << vec.Y << " ";
    return out;
}

std::ostream &VDS::operator <<(std::ostream &out, const Point3 &point)
{
    out << point.X << " " << point.Y << " " << point.Z << " ";
    return out;
}

std::ostream &VDS::operator <<(std::ostream &out, const Vec3 &vec)
{
    out << vec.X << " " << vec.Y << " " << vec.Z << " ";
    return out;
}

std::ostream& VDS::operator << (std::ostream &out, Mat3 &mat)
{
    out << mat.rows[0] << mat.rows[1] << mat.rows[2];
    return out;
}

std::ostream& VDS::operator << (std::ostream &out, Vec4 &vec)
{
    out << vec.X << vec.Y << vec.Z << vec.W;
    return out;
}

std::ostream& VDS::operator << (std::ostream &out, Mat4 &mat)
{
    out << mat.cells[0][0] << mat.cells[0][1] << mat.cells[0][2] << mat.cells[0][3]
        << mat.cells[1][0] << mat.cells[1][1] << mat.cells[1][2] << mat.cells[1][3]
        << mat.cells[2][0] << mat.cells[2][1] << mat.cells[2][2] << mat.cells[2][3]
        << mat.cells[3][0] << mat.cells[3][1] << mat.cells[3][2] << mat.cells[3][3];
    return out;
}

std::ostream &VDS::operator <<(std::ostream &out, const FloatColor &color)
{
    out << color.R << " " << color.G << " " << color.B << " ";
    return out;
}

std::ostream &VDS::operator <<(std::ostream &out, const FloatColorA &color)
{
    out << color.R << " " << color.G << " " << color.B << " " << color.A << " ";
    return out;
}

std::ostream &VDS::operator <<(std::ostream &out, const ByteColor &color)
{
    out << color.R << " " << color.G << " " << color.B << " ";
    return out;
}

std::ostream &VDS::operator <<(std::ostream &out, const ByteColorA &color)
{
    out << color.R << " " << color.G << " " << color.B << " " << color.A << " ";
    return out;
}

std::istream &VDS::operator >>(std::istream &in, Point2 &point)
{
    in >> point.X;
    in >> point.Y;    
    return in;
}

std::istream &VDS::operator >>(std::istream &in, Vec2 &vec)
{
    in >> vec.X;
    in >> vec.Y;    
    return in;
}

std::istream &VDS::operator >>(std::istream &in, Point3 &point)
{
    in >> point.X;
    in >> point.Y;
    in >> point.Z;
    return in;
}

std::istream &VDS::operator >>(std::istream &in, Vec3 &vec)
{
    in >> vec.X;
    in >> vec.Y;
    in >> vec.Z;
    return in;
}

std::istream& VDS::operator >> (std::istream &in, Mat3 &mat)
{
    in >> mat.rows[0] >> mat.rows[1] >> mat.rows[2];
    return in;

}

std::istream &VDS::operator >>(std::istream &in, Vec4 &vec)
{
    in >> vec.X;
    in >> vec.Y;
    in >> vec.Z;
    in >> vec.W;
    return in;
}

std::istream& VDS::operator >> (std::istream &in, Mat4 &mat)
{
    in  >> mat.cells[0][0] >> mat.cells[0][1] >> mat.cells[0][2] >> mat.cells[0][3]
        >> mat.cells[1][0] >> mat.cells[1][1] >> mat.cells[1][2] >> mat.cells[1][3]
        >> mat.cells[2][0] >> mat.cells[2][1] >> mat.cells[2][2] >> mat.cells[2][3]
        >> mat.cells[3][0] >> mat.cells[3][1] >> mat.cells[3][2] >> mat.cells[3][3];

    return in;
}

std::istream &VDS::operator >>(std::istream &in, FloatColor &color)
{
    in >> color.R;
    in >> color.G;
    in >> color.B;
    return in;
}

std::istream &VDS::operator >>(std::istream &in, FloatColorA &color)
{
    in >> color.R;
    in >> color.G;
    in >> color.B;
    in >> color.A;
    return in;
}

std::istream &VDS::operator >>(std::istream &in, ByteColor &color)
{
	unsigned int r, g, b;
	in >> r >> g >> b;
    color.R = r;
    color.G = g;
    color.B = b;
    return in;
}

std::istream &VDS::operator >>(std::istream &in, ByteColorA &color)
{
	unsigned int r, g, b, a;
	in >> r >> g >> b >> a;
    color.R = r;
    color.G = g;
    color.B = b;
    color.A = a;
    return in;
}

Vec3 VDS::operator *(float scale, const Vec3 &vec)
{
    return vec * scale;
}

Mat3 VDS::operator *(const float d, const Mat3 &mat)
{
    return mat * d;
}

Vec4 VDS::operator*(float scale, const Vec4 &vec)
{
    return vec * scale;
}

Mat4 VDS::operator *(const float d, const Mat4 &mat)
{
    return mat * d;
}
