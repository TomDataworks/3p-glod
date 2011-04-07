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
#ifndef PRIMTYPES_H
#define PRIMTYPES_H

//A couple of useful lightweight geometry type classes, nothing specific to VDS here

#include <cmath>
#include <cassert>
#include <iostream>

#ifdef __APPLE__
#include <string.h>
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
#ifndef DEGREES_TO_RADIANS
#define DEGREES_TO_RADIANS 0.017453292519943295769236907684886
#endif

namespace VDS
{
    
    typedef float Float;
    typedef unsigned char Byte;
    
    class Vec2;
    class Point3;
    class Vec3;
    class Mat3;
    class Plane3;
    class Vec4;
    class Mat4;
    class FloatColor;
    class ByteColor;
    class FloatColorA;
    class ByteColorA;
    
#pragma pack(push,1)
    
    class Point2
    {
    public:
        Float X, Y;
        Point2();
        Point2(Float x, Float y);
        Point2(const Point2 &p);
        explicit Point2(const Point3 &p);
        explicit Point2(const Vec2 &vec);
        Point2 &operator =(const Point2 &p);
        bool operator ==(const Point2 &p) const;
        bool operator !=(const Point2 &p) const; 
        Point2 &operator +=(const Vec2 &v);
        Point2 &operator -=(const Vec2 &v);
        Float DistanceToSquared(const Point2 &p) const;
        Float DistanceTo(const Point2 &p) const;
        const Point2 AverageWith(const Point2 &p) const;
        void Set(Float x, Float y);
        //alternative access method
        Float &operator [](unsigned char i);
        const Float &operator [](unsigned char i) const;    
    };
    
    class Vec2
    {
    public:
        Float X, Y;
        static const Vec2 i, j;
        Vec2();	
        Vec2(Float x, Float y);	
        explicit Vec2(Point2 &p);
        explicit Vec2(Vec3 &v);
        //vector addition
        Vec2 operator +(const Vec2 &op) const;
        void operator +=(const Vec2 &op);
        Vec2 operator -(const Vec2 &op) const;
        void operator -=(const Vec2 &op);
        Vec2 operator -() const;
        //Dot Product
        Float operator *(const Vec2 &op) const;	
        //Scalar Mult
        Vec2 operator *(const Float &scale) const;
        Vec2 operator *=(const Float &scale);
        //Scalar Div
        Vec2 operator /(const Float &scale) const;
        Vec2 operator /=(const Float &scale);
        //assignment
        Vec2 &operator =(const Vec2 &op);
        //value comparison
        bool operator ==(const Vec2 &op) const;
        bool operator !=(const Vec2 &op) const;
        //length comparisons
        bool operator >(const Vec2 &op) const;
        bool operator >=(const Vec2 &op) const;
        bool operator <(const Vec2 &op) const;
        bool operator <=(const Vec2 &op) const;
        Float LengthSquared() const;
        Float Length() const;
        void Normalize();
        Vec2 Normalized();
        Float AngleBetween(const Vec2 &op) const;
        void Set(Float x, Float y);    
        //alternative access method
        Float &operator [](unsigned char i);
        const Float &operator [](unsigned char i) const;
    };
    
    class Point3
    {
    public:
        Float X, Y, Z;
        Point3() {}
        Point3(Float x, Float y, Float z)  : X(x), Y(y), Z(z) {}
        Point3(const Point3 &p) : X(p.X), Y(p.Y), Z(p.Z) { }
        explicit Point3(const Vec3 &vec);
        explicit Point3(const Vec4 &vec);
        Point3 &operator =(const Point3 &p) {
            X = p.X, Y = p.Y, Z = p.Z;
            return *this;
        }
        Point3& operator +=(const Vec3& v);
        Point3& operator -=(const Vec3& v);
        
        bool operator ==(const Point3 &p) const {
            return X == p.X && Y == p.Y && Z == p.Z;				
        }
        bool operator !=(const Point3 &p) const {
            return !(p == *this);
        }
        // evaluate a point wrt to plane
        Float operator *(const Plane3 &plane) const;
        Float DistanceToSquared(const Point3 &p) const;
        Float DistanceTo(const Point3 &p) const;
        const Point3 AverageWith(const Point3 &p) const;
        
        void Set(Float x, Float y, Float z) {
            X = x;
            Y = y;
            Z = z;
        }
        //alternative access method
        Float &operator [](unsigned char i) {
#ifdef _DEBUG
            assert((i == 0) || (i == 1) || (i == 2));
#endif
            switch (i)
            {
            case 0:            
                return X;
                break;
            case 1:            
                return Y;
                break;
            case 2:            
                return Z;
                break;
            default:
                return Z;  //arbitrary, silences compiler warnings
                break;
            }
        }
        const Float &operator [](unsigned char i) const {
#ifdef _DEBUG
            assert((i == 0) || (i == 1) || (i == 2));
#endif
            switch (i)
            {
            case 0:            
                return X;
                break;
            case 1:            
                return Y;
                break;
            case 2:            
                return Z;
                break;
            default:
                return Z;  //arbitrary, silences compiler warnings
                break;
            }
        }
    };
    
    class Vec3
    {
    public:
        Float X, Y, Z;
        static const Vec3 i, j, k;
        Vec3();	
        Vec3(Float x, Float y, Float z);	
        //create a Vec3 going from one point to another
        Vec3(const Point3 &src, const Point3 &dest);	
        Vec3(const Vec3 &vec);    
        explicit Vec3(const Point3 &point);
        explicit Vec3(const Vec4 &vec);
        //vector addition
        Vec3 operator +(const Vec3 &op) const;
        void operator +=(const Vec3 &op);
        Vec3 operator -(const Vec3 &op) const;
        void operator -=(const Vec3 &op);
        Vec3 operator -() const;
        //Dot Product
        Float operator *(const Vec3 &op) const;	
        //Scalar Mult
        Vec3 operator *(const Float &scale) const;
        Vec3 operator *=(const Float &scale);
        //Scalar Div
        Vec3 operator /(const Float &scale) const;
        Vec3 operator /=(const Float &scale);
        //Cross Product
        Vec3 operator %(const Vec3 &op) const;
        Vec3 operator %=(const Vec3 &op);	
        //assignment
        Vec3 &operator =(const Vec3 &op);
        //value comparison
        bool operator ==(const Vec3 &op) const;
        bool operator !=(const Vec3 &op) const;
        //length comparisons
        bool operator >(const Vec3 &op) const;
        bool operator >=(const Vec3 &op) const;
        bool operator <(const Vec3 &op) const;
        bool operator <=(const Vec3 &op) const;
        Float LengthSquared() const;
        Float Length() const;
        void Normalize();
        Vec3 Normalized();
        Float AngleBetween(const Vec3 &op) const;
        void Set(Float x, Float y, Float z);    
        //alternative access method
        Float &operator [](unsigned char i) {
#ifdef _DEBUG
            assert((i == 0) || (i == 1) || (i == 2));
#endif
            switch (i)
            {
            case 0:
                return X;
                break;
            case 1:
                return Y;
                break;
            case 2:
                return Z;
                break;
            default:
                return Z;  //arbitrary, silences compiler warnings
                break;
            }
        }
        const Float &operator [](unsigned char i) const {
#ifdef _DEBUG
            assert((i == 0) || (i == 1) || (i == 2));
#endif
            switch (i)
            {
            case 0:
                return X;
                break;
            case 1:
                return Y;
                break;
            case 2:
                return Z;
                break;
            default:
                return Z;  //arbitrary, silences compiler warnings
                break;
            }
        }
    };
    
    class Mat3
    {
    public:
        
        Vec3 rows[3];
        
        // Constructors	
        Mat3();  // Constructed with an identity matrix
        Mat3(float a, float b, float c, float d, float e, float f, float g, float h, float i);
        Mat3(const Vec3 &row0, const Vec3 &row1, const Vec3 &row2);
        Mat3(const Mat3 &mat);
        Mat3(const Vec3 &vec); // Creates a matrix that is vec * vec transpose
        // Assignment operators	
        Mat3 &operator =(const Mat3 &mat);
        Mat3 operator +(const Mat3 &mat) const;
        Mat3 &operator +=(const Mat3& mat);
        Mat3 operator -(const Mat3 &mat) const;
        Mat3 &operator -=(const Mat3 &mat);
        Mat3 operator -() const;
        Mat3 operator *(const Mat3 &mat) const;
        Mat3 operator *(const float d) const;
        Mat3 &operator *=(const float d);
        Vec3 operator *(const Vec3 &v) const;
        Point3 operator *(const Point3 &point) const;
        Mat3 operator /(const float d) const;
        Mat3 &operator /=( const float d);
        bool operator ==(const Mat3 &mat) const;
        bool operator !=(const Mat3 &mat) const;
        Mat3 Transpose() const;
        void Set(const Vec3& row0, const Vec3& row1, const Vec3& row2);
        //return a row from the matrix
        Vec3 &operator [](unsigned char i);
        const Vec3 &operator [](unsigned char i) const;
    };
    
    // This class is for planes in 3-space
    class Plane3
    {
    public:
        // coefficients of the plane equation
        Float A, B, C, D;
        // constructors
        Plane3();
        Plane3(Float a, Float b, Float c, Float d);
        Plane3(const Plane3 &plane);
        // set the plane equation
        void Set(Float a, Float b, Float c, Float d);
        void SetNormal(const Vec3 &n);
        void SetPointOnPlane(const Point3 &p);
        void SetD(const Float &d);
        // evaluate a point wrt the plane
        Float operator *(const Point3 &p) const;
        // find intersection of three planes
        Point3 Intersect3Planes(const Plane3 &p1, const Plane3 &p2);
    };
    
    //this class is for HOMOGENEOUS vectors
    class Vec4
    {
    public:
        Float X, Y, Z, W;
        Vec4() : X(0.0), Y(0.0), Z(0.0), W(1.0) {}
        Vec4(Float x, Float y, Float z, Float w) : X(x), Y(y), Z(z), W(w) {}
        Vec4(Float x, Float y, Float z) : X(x), Y(y), Z(z), W(1.0) {}
        
        //create a Vec4 going from one point to another
        Vec4(const Point3 &src, const Point3 &dest): X(dest.X - src.X), Y(dest.Y - src.Y), Z(dest.Z - src.Z), W(1.0) {};
        Vec4(const Vec4 &vec) : X(vec.X), Y(vec.Y), Z(vec.Z), W(vec.W) {};
        Vec4(const Vec3 &vec) : X(vec.X), Y(vec.Y), Z(vec.Z), W(1.0) {};
        explicit Vec4(const Point3 &point) : X(point.X), Y(point.Y), Z(point.Z), W(1.0) {};
        //vector addition
        Vec4 operator +(const Vec4 &op) const {
            if (W == 0.0) {
                return *this;
            } else if (op.W == 0.0) {
                return op;
            } else
                return Vec4(X/W + op.X/op.W, Y/W + op.Y/op.W, Z/W + op.Z/op.W, 1.0);
        }
        
        Vec4 operator +(const Vec3 &op) const {
            if (W == 0.0)
                return *this;
            else
                return Vec4(X/W + op.X, Y/W + op.Y, Z/W + op.Z, 1.0);
        }
        void operator +=(const Vec4 &op) {
            if (W == 0.0) {
                return;
            } else if (op.W == 0.0) {
                *this = op;
                return;
            } else {
                X /= W, Y /= Y, Z /= W;
                X += op.X/op.W, Y += op.Y/op.W, Z += op.Z/op.W;
                X *= W, Y *= W; Z *= W;
            }
        }
        Vec4 operator -(const Vec4 &op) const {
            if (W == 0.0) {
                return *this;
            } else if (op.W == 0.0) {
                return -op;
            } else {
                return Vec4(X/W - op.X/op.W, Y/W - op.Y/op.W, Z/W - op.Z/op.W, 1.0);
            }
        }
        
        Vec4 operator -(const Vec3 &op) const {
            if (W == 0.0)
                return *this;
            else
                return Vec4(X/W - op.X, Y/W - op.Y, Z/W - op.Z, 1.0);
        }
        
        void operator -=(const Vec4 &op) {
            if (W == 0.0){
                return;
            } else if (op.W == 0.0) {
                *this = -op;
                return;
            } else {
                X /= W, Y /= Y, Z /= W;
                X -= op.X/op.W, Y -= op.Y/op.W, Z -= op.Z/op.W;
                X *= W, Y *= W; Z *= W;
            }
        }
        
        Vec4 operator -() const {
            return Vec4(-X, -Y, -Z, W);
        }
        
        //Dot Product
        Float operator *(const Vec4 &op) const {	return X/W * op.X/op.W + Y/W * op.Y/op.W + Z/W * op.Z/W; }
        
        //Scalar Mult
        Vec4 operator *(const Float &scale) const {	return Vec4(X * scale, Y * scale, Z * scale, W);};
        Vec4 operator *=(const Float &scale) {	
            X *= scale, Y *= scale, Z *= scale;
            return *this;
        }
        //Scalar Div
        Vec4 operator /(const Float &scale) const {	return Vec4(X / scale, Y / scale, Z / scale, W);}
        Vec4 operator /=(const Float &scale) {
            X /= scale, Y /= scale, Z /= scale;
            return *this;
        }
        
        //Cross Product
        Vec4 operator %(const Vec4 &op) const {
            return Vec4(Y/W * op.Z/op.W - Z/W * op.Y/op.W,
                Z/W * op.X/op.W - X/W * op.Z/op.W,
                X/W * op.Y/op.W - Y/W * op.X/op.W,
                1.0);		
        }
        Vec4 operator %=(const Vec4 &op) {
            Float newX = Y/W * op.Z/op.W - Z/W * op.Y/op.W;
            Float newY = Z/W * op.X/op.W - X/W * op.Z/op.W;
            Float newZ = X/W * op.Y/op.Y - Y/W * op.X/op.W;
            X = newX, Y = newY, Z = newZ;
            W = 1.0;
            return *this;
        }
        
        //assignment
        Vec4 &operator =(const Vec4 &op) {
            X = op.X;Y = op.Y;
            Z = op.Z;
            W = op.W;
            return *this;
        }
        Vec4 &operator =(const Vec3 &op) {
            X = op.X;
            Y = op.Y;
            Z = op.Z;
            W = 1.0;
            return *this;
        }
        Vec4 &operator =(const Point3 &op) {
            X = op.X;
            Y = op.Y;
            Z = op.Z;
            W = 1.0;
            return *this;
        }
        bool operator ==(const Vec4 &op) const {	return (X/W == op.X/op.W) && (Y/W == op.Y/op.W) && (Z/W == op.Z/op.W);}
        bool operator !=(const Vec4 &op) const {	return !(*this == op);}
        //length comparisons
        bool operator >(const Vec4 &op) const {	return LengthSquared() > op.LengthSquared();}
        bool operator >=(const Vec4 &op) const {	return LengthSquared() >= op.LengthSquared();}
        bool operator <(const Vec4 &op) const { return 	LengthSquared() < op.LengthSquared();}
        bool operator <=(const Vec4 &op) const { 	return LengthSquared() <= op.LengthSquared();}
        Float LengthSquared() const {	return (X/W)*(X/W) + (Y/W)*(Y/W) + (Z/W)*(Z/W);}
        Float Length() const {	return sqrt(LengthSquared());}
        void Normalize() {
            *this /= Length();
            Homogonize();
        }
        void Homogonize() {
            X /= W, Y /= W, Z /= W;
            W = 1.0;
        }
        Float AngleBetween(const Vec4 &op) const {
            return (*this / Length()) * (op / op.Length());
        }
        void Set(Float x, Float y, Float z, Float w) {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }
        //alternative access method
        Float &operator [](unsigned char i) {
#ifdef _DEBUG
            assert((i == 0) || (i == 1) || (i == 2) || (i == 3));
#endif
            switch (i) {
            case 0:
                return X;
                break;
            case 1:
                return Y;
                break;
            case 2:
                return Z;
                break;
            case 3:
                return W;
                break;
            default:
                return W;  //arbitrary, silences compiler warnings
                break;
            }
        }
        const Float &operator [](unsigned char i) const {
#ifdef _DEBUG
            assert((i == 0) || (i == 1) || (i == 2) || (i == 3));
#endif
            switch (i) {
            case 0:
                return X;
                break;
            case 1:
                return Y;
                break;
            case 2:
                return Z;
                break;
            case 3:
                return W;
                break;
            default:
                return W;  //arbitrary, silences compiler warnings
                break;
            }
        }
        };
        
        class Mat4 {
        public:	
            Float cells[4][4];
            // Constructors	
            Mat4() {     
                memset(cells, 0, sizeof(float) * 16);
                cells[0][3] = cells[1][3] = cells[2][3] = cells[3][3] = 1.0;
            }
            Mat4(const Vec4 &row0, const Vec4 &row1, 
                const Vec4 &row2, const Vec4 &row3) {
                cells[0][0] = row0.X;
                cells[0][1] = row0.Y;
                cells[0][2] = row0.Z;
                cells[0][3] = row0.W;
                
                cells[1][0] = row1.X;
                cells[1][1] = row1.Y;
                cells[1][2] = row1.Z;
                cells[1][3] = row1.W;
                
                cells[2][0] = row2.X;
                cells[2][1] = row2.Y;
                cells[2][2] = row2.Z;
                cells[2][3] = row2.W;
                
                cells[3][0] = row3.X;
                cells[3][1] = row3.Y;
                cells[3][2] = row3.Z;
                cells[3][3] = row3.W;
            }
            Mat4(const Mat4 &mat) {
                memcpy(this->cells, mat.cells, sizeof(Float) * 16);
            }
            Mat4(const float m[16]) { // column major
                cells[0][0] = m[0];
                cells[1][0] = m[1];
                cells[2][0] = m[2];
                cells[3][0] = m[3];
                
                cells[0][1] = m[4];
                cells[1][1] = m[5];
                cells[2][1] = m[6];
                cells[3][1] = m[7];
                
                cells[0][2] = m[8];
                cells[1][2] = m[9];
                cells[2][2] = m[10];
                cells[3][2] = m[11];
                
                cells[0][3] = m[12];
                cells[1][3] = m[13];
                cells[2][3] = m[14];
                cells[3][3] = m[15];
            }
            
            void Set(const float m[16]) {
                cells[0][0] = m[0];
                cells[1][0] = m[1];
                cells[2][0] = m[2];
                cells[3][0] = m[3];
                
                cells[0][1] = m[4];
                cells[1][1] = m[5];
                cells[2][1] = m[6];
                cells[3][1] = m[7];
                
                cells[0][2] = m[8];
                cells[1][2] = m[9];
                cells[2][2] = m[10];
                cells[3][2] = m[11];
                
                cells[0][3] = m[12];
                cells[1][3] = m[13];
                cells[2][3] = m[14];
                cells[3][3] = m[15];
            }
            
            // Assignment operator	
            Mat4 &operator =(const Mat4 &mat);
            Mat4 operator +(const Mat4 &mat) const;
            Mat4 &operator +=(const Mat4& mat);
            Mat4 operator -(const Mat4 &mat) const;
            Mat4 &operator -=(const Mat4 &mat);
            Mat4 operator -() const;
            Mat4 operator *(const Mat4 &mat) const;
            Mat4 operator *(const float d) const;
            Mat4 &operator *=(const float d);
            Vec4 operator *(const Vec4 &v) const {
                Vec4 Out;
                Out.X=(cells[0][0] * v.X + cells[0][1] * v.Y + cells[0][2] * v.Z + cells[0][3] * v.W);
                Out.Y=(cells[1][0] * v.X + cells[1][1] * v.Y + cells[1][2] * v.Z + cells[1][3] * v.W);
                Out.Z=(cells[2][0] * v.X + cells[2][1] * v.Y + cells[2][2] * v.Z + cells[2][3] * v.W);
                Out.W=(cells[3][0] * v.X + cells[3][1] * v.Y + cells[3][2] * v.Z + cells[3][3] * v.W);
                return Out;
            }
            Point3 operator *(const Point3 &point) const;
            Mat4 operator /(const float d) const;
            Mat4 &operator /=( const float d);
            bool operator ==(const Mat4 &mat) const;
            bool operator !=(const Mat4 &mat) const;
            Mat4 Transpose() const;
            Mat4 Inverse() const;
            void Set(const Vec4& row0, const Vec4& row1, const Vec4& row2, const Vec4& row3);
            void SetRow(int row, Vec4 to) {
                cells[row][0] = to.X;
                cells[row][1] = to.Y;
                cells[row][2] = to.Z;
                cells[row][3] = to.W;        
            }
    };
    
    class FloatColor
    {
    public:
        Float R, G, B;
        FloatColor(Float red, Float blue, Float green);
        FloatColor();
        FloatColor(const FloatColor &color);
        explicit FloatColor(const FloatColorA &color);
        explicit FloatColor(const ByteColor &color);
        explicit FloatColor(const ByteColorA &color);
        FloatColor &operator =(const FloatColor &color);
        bool operator ==(const FloatColor &color) const;
        bool operator !=(const FloatColor &color) const;
        //alternative access
        Float &operator [](unsigned int i);
        const Float &operator [](unsigned int i) const;
        void Clamp();    
    };
    
    class ByteColor
    {
    public:
        Byte R, G, B;
        ByteColor(Byte red, Byte blue, Byte green);
        ByteColor();
        ByteColor(const ByteColor &color);
        explicit ByteColor(const ByteColorA &color);
        explicit ByteColor(const FloatColor &color);
        explicit ByteColor(const FloatColorA &color);
        ByteColor &operator =(const ByteColor &color);
        bool operator ==(const ByteColor &color) const;
        bool operator !=(const ByteColor &color) const;
        //alternative access
        Byte &operator [](unsigned int i);
        const Byte &operator [](unsigned int i) const;
    };
    
    class FloatColorA
    {
    public:
        Float R, G, B, A;
        FloatColorA(Float red, Float blue, Float green, Float alpha);
        FloatColorA();
        FloatColorA(const FloatColorA &color);
        explicit FloatColorA(const FloatColor &color);
        explicit FloatColorA(const ByteColorA &color);
        explicit FloatColorA(const ByteColor &color);
        FloatColorA &operator =(const FloatColorA &color);
        bool operator ==(const FloatColorA &color) const;
        bool operator !=(const FloatColorA &color) const;
        //alternative access
        Float &operator [](unsigned int i);
        const Float &operator [](unsigned int i) const;
        void Clamp();
    };
    
    class ByteColorA
    {
    public:
        Byte R, G, B, A;
        ByteColorA(Byte red, Byte blue, Byte green, Byte alpha);
        ByteColorA();
        ByteColorA(const ByteColorA &color);
        explicit ByteColorA(const ByteColor &color);
        explicit ByteColorA(const FloatColorA &color);
        explicit ByteColorA(const FloatColor &color);
        ByteColorA &operator =(const ByteColorA &color);
        bool operator ==(const ByteColorA &color) const;
        bool operator !=(const ByteColorA &color) const;
        //alternative access
        Byte &operator [](unsigned int i);
        const Byte &operator [](unsigned int i) const;
    };
#pragma pack(pop)
    
    Point2 operator +(const Vec2 &vec, const Point2 &point);
    Point2 operator +(const Point2 &point, const Vec2 &vec);
    Point2 operator -(const Point2 &point, const Vec2 &vec);
    Vec2 operator -(const Point2 &point1, const Point2 &point2);
    
    Point3 operator +(const Vec3 &vec, const Point3 &point);
    Point3 operator +(const Point3 &point, const Vec3 &vec);
    Point3 operator -(const Point3 &point, const Vec3 &vec);
    Vec3 operator -(const Point3 &point1, const Point3 &point2);
    
    Point3 operator +(const Vec4 &vec, const Point3 &point);
    Point3 operator +(const Point3 &point, const Vec4 &vec);
    Point3 operator -(const Point3 &point, const Vec4 &vec);
    
    std::ostream &operator <<(std::ostream &out, const Point2 &point);
    std::ostream &operator <<(std::ostream &out, const Vec2 &vec);
    std::ostream &operator <<(std::ostream &out, const Point3 &point);
    std::ostream &operator <<(std::ostream &out, const Vec3 &vec);
    std::ostream& operator << (std::ostream &out, Mat3 &mat);
    std::ostream& operator << (std::ostream &out, Vec4 &vec);
    std::ostream& operator << (std::ostream &out, Mat4 &mat);
    std::ostream &operator <<(std::ostream &out, const FloatColor &color);
    std::ostream &operator <<(std::ostream &out, const ByteColor &color);
    std::ostream &operator <<(std::ostream &out, const FloatColorA &color);
    std::ostream &operator <<(std::ostream &out, const ByteColorA &color);
    
    std::istream &operator >>(std::istream &in, Point2 &point);
    std::istream &operator >>(std::istream &in, Vec2 &vec);
    std::istream &operator >>(std::istream &in, Point3 &point);
    std::istream &operator >>(std::istream &in, Vec3 &vec);
    std::istream& operator >> (std::istream &in, Mat3 &mat);
    std::istream &operator >>(std::istream &in, Vec4 &vec);
    std::istream& operator >> (std::istream &in, Mat4 &mat);
    std::istream &operator >>(std::istream &in, FloatColor &color);
    std::istream &operator >>(std::istream &in, ByteColor &color);
    std::istream &operator >>(std::istream &in, FloatColorA &color);
    std::istream &operator >>(std::istream &in, ByteColorA &color);
    Vec3 operator *(float scale, const Vec3 &vec);
    Mat3 operator *(const float d, const Mat3 &mat);
    Vec4 operator*(float scale, const Vec4 &vec);
    Mat4 operator *(const float d, const Mat4 &mat);
    
} //namespace VDS
#endif //PRIMTYPES_H

