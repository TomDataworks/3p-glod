/******************************************************************************
 * Copyright 2003 Jonathan Cohen, Nat Duca, David Luebke, Brenden Schubert    *
 *                Johns Hopkins University and University of Virginia         *
 ******************************************************************************
 * This file is distributed as part of the GLOD library, and as such, falls   *
 * under the terms of the GLOD public license. GLOD is distributed without    *
 * any warranty, implied or otherwise. See the GLOD license for more details. *
 *                                                                            *
 * You should have recieved a copy of the GLOD Open-Source License with this  *
 * copy of GLOD; if not, please visit the GLOD web page,                      *
 * http://www.cs.jhu.edu/~graphics/GLOD/license for more information          *
 ******************************************************************************/
/* 3D-ish Math library
 * Taken from U of VA CVS
 ****************************************************************************
 *
 *    NOTE ALL MATRICES ARE IN NON-GL ORDER!!!
 *    M[ROW][COLUMN]
 *
 ****************************************************************************/

#ifndef NAT_MATH_H
#define NAT_MATH_H

#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
#define inline
#pragma warning( disable : 4244 )
#endif
#ifndef M_PI
#define M_PI 3.14159265359
#endif

typedef float  NPoint[3];
typedef float  Vec2[2];
typedef float  Vec3[3];
typedef float  Vec4[4];
typedef float  Mat3[3][3];
typedef float  Mat4[4][4];
typedef double Quadric[10];
typedef struct Quaternion {
    float w;
    Vec3  v;
} Quaternion;
typedef struct AABB {
    NPoint min;
    NPoint max;
} AABB;


#ifndef FALSE
#define FALSE           0
#endif
#ifndef TRUE
#define TRUE            1
#endif

#define X               0
#define Y               1
#define Z               2

/* Floating point manipulation */
#define F_EQ(a,b,thresh)     ( fabs((a)-(b)) <= (thresh) )
#define F_CLAMP(a,lo,hi)     { a = ((a) < (lo) ? (lo) : ((a) > (hi) ? (hi) : (a))); }

/* Vec2 manipulation */
#define VEC2_ZERO(a)         { a[0]=a[1]=0; }
#define VEC2_COPY(a,b)       { memcpy(a,b,sizeof(Vec2)); }
#define VEC2_SET(a,r,s)      { a[0]=r; a[1]=s; }
#define VEC2_NEG(a,b)        { a[0] = -b[0]; a[1] = -b[1]; }
#define VEC2_SCALE(a,s)      { a[0]*= s; a[1] *= s; }
#define VEC2_OP(a,b,op,c)    { a[0] = b[0] op c[0];                      \
                               a[1] = b[1] op c[1]; }
#define VEC2_ASN_OP(a,op,b)  { a[0] op b[0]; a[1] op b[1]; }
#define VEC2_DOT(a,b)        ( a[0]*b[0] + a[1]*b[1] )
#define VEC2_LEN(a)          ( sqrt(VEC2_DOT(a,a)) )
#define VEC2(v)              v[X], v[Y]

/* Vec3 manipulation */
#define VEC3_ZERO(a)         { a[0]=a[1]=a[2]=0; }
#define VEC3_COPY(a,b)       { memcpy(a,b,sizeof(Vec3)); }
#define VEC3_SET(a,r,s,t)    { a[0]=r; a[1]=s; a[2]=t; }
#define VEC3_NEG(a,b)        { a[0] = -b[0]; a[1] = -b[1]; a[2] = -b[2]; }
#define VEC3_SCALE(a,s)      { a[0]*= s; a[1] *= s; a[2] *= s; }
#define VEC3_OP(a,b,op,c)    { a[0] = b[0] op c[0];                      \
                               a[1] = b[1] op c[1];                      \
                               a[2] = b[2] op c[2]; }
#define VEC3_ASN_OP(a,op,b)  { a[0] op b[0]; a[1] op b[1]; a[2] op b[2]; }
#define VEC3_DOT(a,b)        ( a[0]*b[0] + a[1]*b[1] + a[2]*b[2] )
#define VEC3_CROSS(a,b,c)    { a[0]=b[1]*c[2]-b[2]*c[1];                 \
                               a[1]=b[2]*c[0]-b[0]*c[2];                 \
                               a[2]=b[0]*c[1]-b[1]*c[0]; }
#define VEC3_NORM(a)         { float LAmag = 1.0f/VEC3_LEN(a);           \
                               VEC3_SCALE(a,LAmag); }
#define VEC3_LEN(a)          ( sqrt(VEC3_DOT(a,a)) )

#define VEC3(v)              v[X], v[Y], v[Z]
#define VEC3_PADD(a,b)       a[X]+b[X], a[Y]+b[Y], a[Z]+b[Z]
#define VEC3_PSCALE(v,s)     v[X]*s, v[Y]*s, v[Z]*s

#define VEC3_MAJDIM(v)       (fabs(v[X])>fabs(v[Y]))                     \
                                 ? ((fabs(v[X])>fabs(v[Z])) ? X : Z)     \
                                 : ((fabs(v[Y])>fabs(v[Z])) ? Y : Z)

#define VEC3_PRINT(v)	     printf("(%.3f, %.3f, %.3f)", (v)[0],(v)[1],(v)[2]);
#define VEC3_PRINTLN(v)	     printf("(%.3f, %.3f, %.3f)\n", (v)[0],(v)[1],(v)[2]);

/* Vec4 manipulation */
#define VEC4(v)              v[0], v[1], v[2], v[3]
#define VEC4_SET(v, x, y, z, w) { (v)[0] = x; (v)[1] = y; (v)[2] = z; (v)[3] = w; }
#define VEC4_COPY(a,b)       { memcpy(a,b,sizeof(Vec4)); }

/* Screenspace manipulation */
static inline void POINT_TO_VEC3(int x, int y, int width, int height, float *v)
        {
            float dist;
            /* project x, y onto a hemi-sphere centered within width, height.*/
            v[X] = (width - 2.0 * x) / width;
            v[Y] = (2.0 * y - height) / height;
            dist = sqrt(v[X]*v[X] + v[Y]*v[Y]);
            v[Z] = cos((M_PI / 4.0) * ((dist < 1.0) ? dist : 1.0));
            VEC3_NORM(v);
        }


/* Matrix manipulation  (matrix is of form mat[row][column]) */
#define VEC3_TO_TRANSLATE_MAT4(v,m)                                      \
        {                                                                \
            memset(m,0,sizeof(Mat4));                                    \
            m[0][0] = 1; m[1][1] = 1; m[2][2] = 1; m[3][3] = 1;          \
            m[0][3] = v[X];                                              \
            m[1][3] = v[Y];                                              \
            m[2][3] = v[Z];                                              \
        }

#define FLOAT_TO_SCALE_MAT4(s,m)                                         \
        {                                                                \
            memset(m,0,sizeof(Mat4));                                    \
            m[0][0] = s; m[1][1] = s; m[2][2] = s; m[3][3] = 1;          \
        }

#define MAT3_VEC3_MULT(m,p,r)                                            \
        {                                                                \
            r[X] = m[0][0]*p[X] + m[0][1]*p[Y] + m[0][2]*p[Z];           \
            r[Y] = m[1][0]*p[X] + m[1][1]*p[Y] + m[1][2]*p[Z];           \
            r[Z] = m[2][0]*p[X] + m[2][1]*p[Y] + m[2][2]*p[Z];           \
        }
#define MAT4_VEC4_MULT(m,p,r)                                                   \
        {                                                                       \
            r[0] = m[0][0]*p[0] + m[0][1]*p[1] + m[0][2]*p[2] + m[0][3]*p[3];   \
            r[1] = m[1][0]*p[0] + m[1][1]*p[1] + m[1][2]*p[2] + m[1][3]*p[3];   \
            r[2] = m[2][0]*p[0] + m[2][1]*p[1] + m[2][2]*p[2] + m[2][3]*p[3];   \
            r[3] = m[3][0]*p[0] + m[3][1]*p[1] + m[3][2]*p[2] + m[3][3]*p[3];   \
        }
#define mrc3(i,j,a,b,r) r[i][j] = a[i][0]*b[0][j]+a[i][1]*b[1][j]+       \
                                  a[i][2]*b[2][j]
#define MAT3_MAT3_MULT(a,b,r)                                            \
        {                                                                \
            mrc3(0,0,a,b,r);mrc3(0,1,a,b,r);mrc3(0,2,a,b,r);             \
            mrc3(1,0,a,b,r);mrc3(1,1,a,b,r);mrc3(1,2,a,b,r);             \
            mrc3(2,0,a,b,r);mrc3(2,1,a,b,r);mrc3(2,2,a,b,r);             \
        }
#define mrc4(i,j,a,b,r) r[i][j] = a[i][0]*b[0][j]+a[i][1]*b[1][j]+       \
                                  a[i][2]*b[2][j]+a[i][3]*b[3][j]
#define MAT4_MAT4_MULT(a,b,r)                                            \
        {                                                                \
            mrc4(0,0,a,b,r);mrc4(0,1,a,b,r);mrc4(0,2,a,b,r);mrc4(0,3,a,b,r);\
            mrc4(1,0,a,b,r);mrc4(1,1,a,b,r);mrc4(1,2,a,b,r);mrc4(1,3,a,b,r);\
            mrc4(2,0,a,b,r);mrc4(2,1,a,b,r);mrc4(2,2,a,b,r);mrc4(2,3,a,b,r);\
            mrc4(3,0,a,b,r);mrc4(3,1,a,b,r);mrc4(3,2,a,b,r);mrc4(3,3,a,b,r);\
        }

#define TRANSFORM(m,p,r)                                                 \
        {                                                                \
            float tr_w,tr_sc;                                            \
            r[X] = m[0][0]*p[X] + m[0][1]*p[Y] + m[0][2]*p[Z] + m[0][3]; \
            r[Y] = m[1][0]*p[X] + m[1][1]*p[Y] + m[1][2]*p[Z] + m[1][3]; \
            r[Z] = m[2][0]*p[X] + m[2][1]*p[Y] + m[2][2]*p[Z] + m[2][3]; \
            tr_w = m[3][0]*p[X] + m[3][1]*p[Y] + m[3][2]*p[Z] + m[3][3]; \
            tr_sc = 1/tr_w;  VEC3_SCALE(r,tr_sc);                        \
        }

static inline int MAT3_INVERT(Mat3 mat, Mat3 b)
{
    float a[3][3];
    float temp[3];
    float atemp;
    int i,j,k,i1;

    memset(b, 0, sizeof(Mat3));
    for (j=0; j<3; j++)
        b[j][j] = 1.0;

    memcpy(a, mat, sizeof(Mat3));

    for (j=0; j<3; j++) {
        i1 = j;
        for (i=j+1; i<3; i++) {
            if (fabs(a[i][j]) > fabs(a[i1][j])) {
                i1 = i;
            }
        }
        /* swap rows i1 and j in a and b to put pivot on diagonal */
        memcpy(temp,  a[i1], sizeof(Vec3));
        memcpy(a[i1], a[j],  sizeof(Vec3));
        memcpy(a[j],  temp,  sizeof(Vec3));
        memcpy(temp,  b[i1], sizeof(Vec3));
        memcpy(b[i1], b[j],  sizeof(Vec3));
        memcpy(b[j],  temp,  sizeof(Vec3));

        /* scale row j to have a unit diagonal */
        if (fabs(a[j][j]) <= 0.000001) {
            return FALSE;
        }
        atemp = a[j][j];
        for (k=0; k<3; k++) {
            b[j][k] /= atemp;
            a[j][k] /= atemp;
        }

        /* eliminate off-diagonal elements in col j of a and b */
        for (i=0; i<3; i++) {
            if (i != j) {
                atemp = a[i][j];
                for (k=0; k<3; k++) {
                    b[i][k] -= b[j][k] * atemp;
                    a[i][k] -= a[j][k] * atemp;
                }
            }
        }
    }
    return TRUE;
}

static inline int MAT4_INVERT(Mat4 mat, Mat4 b)
{
    float a[4][4];
    float temp[4];
    float atemp;
    int i,j,k,i1;

    memset(b, 0, sizeof(Mat4));
    for (j=0; j<4; j++)
        b[j][j] = 1.0;

    memcpy(a, mat, sizeof(Mat4));

    for (j=0; j<4; j++) {
        i1 = j;
        for (i=j+1; i<4; i++) {
            if (fabs(a[i][j]) > fabs(a[i1][j])) {
                i1 = i;
            }
        }
        /* swap rows i1 and j in a and b to put pivot on diagonal */
        memcpy(temp,  a[i1], sizeof(Vec4));
        memcpy(a[i1], a[j],  sizeof(Vec4));
        memcpy(a[j],  temp,  sizeof(Vec4));
        memcpy(temp,  b[i1], sizeof(Vec4));
        memcpy(b[i1], b[j],  sizeof(Vec4));
        memcpy(b[j],  temp,  sizeof(Vec4));

        /* scale row j to have a unit diagonal */
        if (fabs(a[j][j]) <= 0.000001) {
            return FALSE;
        }
        atemp = a[j][j];
        for (k=0; k<4; k++) {
            b[j][k] /= atemp;
            a[j][k] /= atemp;
        }

        /* eliminate off-diagonal elements in col j of a and b */
        for (i=0; i<4; i++) {
            if (i != j) {
                atemp = a[i][j];
                for (k=0; k<4; k++) {
                    b[i][k] -= b[j][k] * atemp;
                    a[i][k] -= a[j][k] * atemp;
                }
            }
        }
    }
    return TRUE;
}

static inline void MAT_TRANSPOSE(int size, float *mat, float *transp)
{
    int i, j; float* tmat = NULL;
    if(mat == transp) {
      tmat = (float*) malloc(size*size*sizeof(float));
      memcpy(tmat, mat, sizeof(float)*size*size);
      mat = tmat;
    }

    for (i=0; i<size; i++) {
        for (j=0; j<size; j++) {
            transp[j*size+i] = mat[i*size+j];
        }
    }

    if(tmat != NULL) free(tmat);
}


/* Quadric manipulation */
#define QUAD_ZERO(q)         { memset(q,0,sizeof(Quadric)); }
#define QUAD_COPY(a,b)       { memcpy(a,b,sizeof(Quadric)); }
#define QUAD_INIT(q,plane)                                               \
        {                                                                \
            float a=plane[0], b=plane[1], c=plane[2], d=plane[3];        \
                                                                         \
            q[0] = (a*a); q[1] = (a*b); q[2] = (a*c); q[3] = (a*d);      \
                          q[4] = (b*b); q[5] = (b*c); q[6] = (b*d);      \
                                        q[7] = (c*c); q[8] = (c*d);      \
                                                      q[9] = (d*d);      \
        }
#define QUAD_ASN_OP(a,op,b)                                              \
        {                                                                \
            a[0] op b[0]; a[1] op b[1]; a[2] op b[2]; a[3] op b[3];      \
                          a[4] op b[4]; a[5] op b[5]; a[6] op b[6];      \
                                        a[7] op b[7]; a[8] op b[8];      \
                                                      a[9] op b[9];      \
        }
#define QUAD_OP(a,op,b)                                                  \
        {                                                                \
            a[0] op b;    a[1] op b;    a[2] op b;    a[3] op b;         \
                          a[4] op b;    a[5] op b;    a[6] op b;         \
                                        a[7] op b;    a[8] op b;         \
                                                      a[9] op b;         \
        }
#define QUAD_TENSOR(q)                                                   \
        {                                                                \
          { q[0], q[1], q[2] },                                          \
          { q[1], q[4], q[5] },                                          \
          { q[2], q[5], q[7] }                                           \
        }
#define QUAD_VECTOR(q)                                                   \
        {                                                                \
          q[3], q[6], q[8]                                               \
        }
#define QUAD_SCALAR(q)       q[9]

static inline float QUAD_EVAL(Quadric q, Vec3 v)
        {
            float x=v[0],y=v[1],z=v[2];

            return ( x*x*q[0] + 2*x*y*q[1] + 2*x*z*q[2] + 2*x*q[3]
                              +   y*y*q[4] + 2*y*z*q[5] + 2*y*q[6]
                                           +   z*z*q[7] + 2*z*q[8]
                                                        +     q[9] );
        }
static inline int QUAD_OPTIMAL(Quadric q, Vec3 vnew)
        {
            Mat3 tensor = QUAD_TENSOR(q);
            Vec3 vector = QUAD_VECTOR(q);
            Mat3 inverse;

            if (!MAT3_INVERT(tensor,inverse)) {
                 return FALSE;
            }
            MAT3_VEC3_MULT(inverse,vector,vnew);
            VEC3_NEG(vnew,vnew);
            return TRUE;
        }

/* Quaternion manipulation */
#define QUAT_COPY(a,b)       { memcpy(a,b,sizeof(Quaternion)); }
#define QUAT_INIT(q,v)       { VEC3_COPY((q)->v,v);  (q)->w=0.0; }
#define QUAT_IDENTITY(q)    { VEC3_ZERO((q)->v); (q)->w=1.0; }
static inline void QUAT_FROM_AXIS(Quaternion* q, float theta, float x, float y, float z) {
                    Vec3 axis; VEC3_SET(axis, x,y,z);
		    VEC3_NORM(axis);
    		    float sin_a = sin( theta / 2 );
    		    float cos_a = cos( theta / 2 );

    		    q->v[0] = axis[0] * sin_a;
    		    q->v[1] = axis[1] * sin_a;
    		    q->v[2] = axis[2] * sin_a;
    		    q->w    = cos_a;
        }

static inline void QUAT_CONJ(Quaternion *q) {
  VEC3_NEG(q->v,q->v);
}

static inline void QUAT_FROM_ROTATION_MATRIX(Mat4 m, Quaternion* q) {
                    float t,s,x,y,z,w;
		    t = 1 + m[0][0] + m[1][1] + m[2][2];
		    if(t > .000001) {
		      s = .5 / sqrt(t);
		      x = ( m[2][1] - m[1][2] ) * s;
		      y = ( m[0][2] - m[2][0] ) * s;
		      z = ( m[1][0] - m[0][1] ) * s;
		      w = .25 / s;
		    } else {  // Which value along the diagnal has the greatest value?
		      if (m[0][0] > m[1][1])  {
			if (m[0][0] > m[2][2]) { // If [0][0] is the greatest
			  s = sqrt(1.0 + m[0][0] - m[1][1]
				                 - m[2][2]) * 2.0;
			  x = 0.5 / s;
			  y = (m[0][1] + m[1][0]) / s;
			  z = (m[0][2] + m[2][0]) / s;
			  w = (m[1][2] + m[2][1]) / s;
			} else { // matrix[2][2]
			  s = sqrt(1.0 + m[2][2] - m[0][0]
				                 - m[1][1]) * 2.0;
			  x = (m[0][2] + m[2][0]) / s;
			  y = (m[0][2] + m[2][0]) / s;
			  z = 0.5 / s;
			  w = (m[0][1] + m[1][0]) / s;
			}
		      } else if (m[1][1] > m[2][2]) {	// Matrix[1][1]
			s = sqrt(1.0 + m[1][1] - m[0][0]
				               - m[2][2]) * 2.0;
			x = (m[0][1] + m[1][0]) / s;
			y = 0.5 / s;
			z = (m[1][2] + m[2][1]) / s;
			w = (m[0][2] + m[2][0]) / s;
		      } else { //  Matrix[2][2]
			
			s = sqrt(1.0 + m[2][2] - m[0][0]
				               - m[1][1]) * 2.0;
			x = (m[0][2] + m[2][0]) / s;
			y = (m[0][2] + m[2][0]) / s;
			z = 0.5 / s;
			w = (m[0][1] + m[1][0]) / s;
		      }
		    }
		    
		    VEC3_SET(q->v, x,y,z);
		    q->w = w;
                 }

static inline void QUAT_TO_MAT4(Quaternion *quat, Mat4 m)
        {
            float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

            x2 = quat->v[X] * 2;   y2 = quat->v[Y] * 2;   z2 = quat->v[Z] * 2;
            xx = quat->v[X] * x2;  xy = quat->v[X] * y2;  xz = quat->v[X] * z2;
            yy = quat->v[Y] * y2;  yz = quat->v[Y] * z2;  zz = quat->v[Z] * z2;
            wx = quat->w    * x2;  wy = quat->w    * y2;  wz = quat->w    * z2;

            m[0][0] = 1.0 - (yy + zz);
            m[0][1] = xy - wz;
            m[0][2] = xz + wy;
            m[0][3] = 0.0;
 
            m[1][0] = xy + wz;
            m[1][1] = 1.0 - (xx + zz);
            m[1][2] = yz - wx;
            m[1][3] = 0.0;

            m[2][0] = xz - wy;
            m[2][1] = yz + wx;
            m[2][2] = 1.0 - (xx + yy);
            m[2][3] = 0.0;

            m[3][0] = 0;
            m[3][1] = 0;
            m[3][2] = 0;
            m[3][3] = 1;
        }


static inline void QUAT_TO_ORIENTATION(Quaternion *quat, Vec3 sideways, Vec3 up, Vec3 forward)
        {
            float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

            x2 = quat->v[X] * 2;   y2 = quat->v[Y] * 2;   z2 = quat->v[Z] * 2;
            xx = quat->v[X] * x2;  xy = quat->v[X] * y2;  xz = quat->v[X] * z2;
            yy = quat->v[Y] * y2;  yz = quat->v[Y] * z2;  zz = quat->v[Z] * z2;
            wx = quat->w    * x2;  wy = quat->w    * y2;  wz = quat->w    * z2;

	    if(sideways != NULL) {
	      sideways[0] = 1.0 - (yy + zz);
	      sideways[1] = xy - wz;
	      sideways[2] = xz + wy;
	    }
	    
	    if(up != NULL) {
	      up[0] = xy + wz;
	      up[1] = 1.0 - (xx + zz);
	      up[2] = yz - wx;
	    }

	    if(forward != NULL) {
	      forward[0] = - ( xz - wy );
	      forward[1] = - ( yz + wx );
	      forward[2] = - ( 1.0 - (xx + yy) );
	    }
        }

static inline void QUAT_DEBUG(Quaternion *quat) {
	Vec3 s; Vec3 up; Vec3 fw;
	QUAT_TO_ORIENTATION(quat,s,up,fw);
	printf("Quaternion: s=");
	VEC3_PRINT(s);
	printf(", up=");
	VEC3_PRINT(up);
	printf(", fw=");
	VEC3_PRINT(fw);
	printf("\n");
}

static inline void MAT4_TO_ORIENTATION(Mat4 m, Vec3 sideways, Vec3 up, Vec3 forward)
        {
	  if(sideways != NULL) {
	    sideways[0] = m[0][0];
            sideways[1] = m[0][1];
            sideways[2] = m[0][2];
	  }
 
	  if(up != NULL) {
	    up[0] = m[1][0];
            up[1] = m[1][1];
            up[2] = m[1][2];
	  }

	  if(forward != NULL) {
            forward[0] = -m[2][0];
            forward[1] = -m[2][1];
            forward[2] = -m[2][2];
	  }
        }

/* [row][column] is non-gl, which is what we speak
   [column][row] is GL */

static inline void QUAT_LOOKAT(Quaternion* q,
			       float eyeX,    float eyeY,    float eyeZ,
			       float centerX, float centerY, float centerZ,
			       float upX,     float upY,     float upZ) {
  // Make vectors
  Vec3 f; VEC3_SET(f, centerX-eyeX, centerY-eyeY, centerZ-eyeZ);
  Vec3 up; VEC3_SET(up, upX, upY, upZ);

  VEC3_NORM(f);

  Vec3 s; Vec3 u;
  VEC3_CROSS(s,f,up); 
  VEC3_CROSS(u,s,f);
  VEC3_NORM(s); VEC3_NORM(u);

  // Make matrix
  Mat4 m;
  m[0][0] = s[0];    m[0][1] = s[1];   m[0][2] = s[2];   m[0][3] = 0;
  m[1][0] = u[0];    m[1][1] = u[1];   m[1][2] = u[2];   m[1][3] = 0;
  m[2][0] = -f[0];   m[2][1] = -f[1];  m[2][2] = -f[2];  m[2][3] = 0;
  m[3][0] = 0;       m[3][1] = 0;      m[3][2] = 0;      m[3][3] = 1;

  // Make Quaternion
  QUAT_FROM_ROTATION_MATRIX(m, q);
}

static inline void QUAT_NORM(Quaternion *q)
        {
            float *v = q->v;
            float sq = VEC3_DOT(v,v) + q->w*q->w;
            float l = (sq > 0.0) ? 1.0 / sqrt(sq) : 1.0;

            VEC3_SCALE(v,l);
            q->w *= l;
        }

static inline void QUAT_MULT(Quaternion *q1, Quaternion *q2, Quaternion *res)
        {
            /* allows result to be in the same memory location as q1 or q2 */
            float q1w=q1->w, q1x=q1->v[X], q1y=q1->v[Y], q1z=q1->v[Z];
            float q2w=q2->w, q2x=q2->v[X], q2y=q2->v[Y], q2z=q2->v[Z];

            res->v[X] = q1w*q2x + q1x*q2w + q1y*q2z - q1z*q2y;
            res->v[Y] = q1w*q2y + q1y*q2w + q1z*q2x - q1x*q2z;
            res->v[Z] = q1w*q2z + q1z*q2w + q1x*q2y - q1y*q2x;
            res->w    = q1w*q2w - q1x*q2x - q1y*q2y - q1z*q2z;

            /* make sure the resulting quaternion is a unit quaternion */
            QUAT_NORM(res);
        }

static inline void QUAT_FROM_VEC3(Vec3 vfrom, Vec3 vto, Quaternion* res)
	{
	    Vec3 from; Vec3 to; VEC3_COPY(from, vfrom); VEC3_COPY(to,vto);
	    VEC3_NORM(from);
	    VEC3_NORM(to);

	    res->v[X] = from[Y]*to[Z] - from[Z]*to[Y];
      	    res->v[Y] = from[Z]*to[X] - from[X]*to[Z];
      	    res->v[Z] = from[X]*to[Y] - from[Y]*to[X];
      	    res->w    = from[X]*to[X] + from[Y]*to[Y] + from[Z]*to[Z];
	    QUAT_NORM(res);
	}

static inline void QUAT_TRANSFORM(Quaternion* q, Vec3 v, Vec3 out) {
  const double q00 = 2.0 * q->v[0] * q->v[0];
  const double q11 = 2.0 * q->v[1] * q->v[1];
  const double q22 = 2.0 * q->v[2] * q->v[2];

  const double q01 = 2.0 * q->v[0] * q->v[1];
  const double q02 = 2.0 * q->v[0] * q->v[2];
  const double q03 = 2.0 * q->v[0] * q->w;

  const double q12 = 2.0 * q->v[1] * q->v[2];
  const double q13 = 2.0 * q->v[1] * q->w;

  const double q23 = 2.0 * q->v[2] * q->w;
  
  VEC3_SET(out,
           (1.0 - q11 - q22)*v[0] + (      q01 - q23)*v[1] + (      q02 + q13) *v[2],
           (      q01 + q23)*v[0] + (1.0 - q22 - q00)*v[1] + (      q12 - q03) *v[2],
           (      q02 - q13)*v[0] + (      q12 + q03)*v[1] + (1.0 - q11 - q00) *v[2] );
}

static inline void QUAT_TRANSFORM_INVERSE(Quaternion* q, Vec3 v,Vec3 out)
{
  const double q00 = 2.0 * q->v[0] * q->v[0];
  const double q11 = 2.0 * q->v[1] * q->v[1];
  const double q22 = 2.0 * q->v[2] * q->v[2];

  const double q01 = 2.0 * q->v[0] * q->v[1];
  const double q02 = 2.0 * q->v[0] * q->v[2];
  const double q03 = 2.0 * q->v[0] * q->w;

  const double q12 = 2.0 * q->v[1] * q->v[2];
  const double q13 = 2.0 * q->v[1] * q->w;

  const double q23 = 2.0 * q->v[2] * q->w;

  VEC3_SET(out,
             (1.0 - q11 - q22)*v[0] + (      q01 + q23)*v[1] + (      q02 - q13)
*v[2],
             (      q01 - q23)*v[0] + (1.0 - q22 - q00)*v[1] + (      q12 + q03)
*v[2],
             (      q02 + q13)*v[0] + (      q12 - q03)*v[1] + (1.0 - q11 - q00)
*v[2] );
}



#define QUAT_PRINTLN(q) printf("(%.3f)(.%3f,%.3f,%.3f)\n",(q)->w,(q)->v[X],(q)->v[Y],(q)->v[Z]);

/* Axis-aligned bounding box manipulation */
#define AABB_INIT_VAL {{ 9999999.9f,  9999999.9f,  9999999.9f},          \
                       {-9999999.9f, -9999999.9f, -9999999.9f}}
#define AABB_INIT(aabb)                                                  \
        {                                                                \
            (aabb)->min[X] =  9999999.9f;                                \
            (aabb)->min[Y] =  9999999.9f;                                \
            (aabb)->min[Z] =  9999999.9f;                                \
            (aabb)->max[X] = -9999999.9f;                                \
            (aabb)->max[Y] = -9999999.9f;                                \
            (aabb)->max[Z] = -9999999.9f;                                \
        }
#define AABB_ADD_POINT(aabb,p)                                           \
        {                                                                \
            if (p[X] < (aabb)->min[X]) (aabb)->min[X] = p[X];            \
            if (p[Y] < (aabb)->min[Y]) (aabb)->min[Y] = p[Y];            \
            if (p[Z] < (aabb)->min[Z]) (aabb)->min[Z] = p[Z];            \
            if (p[X] > (aabb)->max[X]) (aabb)->max[X] = p[X];            \
            if (p[Y] > (aabb)->max[Y]) (aabb)->max[Y] = p[Y];            \
            if (p[Z] > (aabb)->max[Z]) (aabb)->max[Z] = p[Z];            \
        }
#define AABB_CENTER(aabb,c)                                              \
        {                                                                \
            VEC3_OP(c,(aabb)->max,+,(aabb)->min);                        \
            VEC3_SCALE(c,0.5f);                                          \
        }
#define AABB_SIZE(aabb,s)    { VEC3_OP(s,(aabb)->max,-,(aabb)->min); }


/* Triangle manipulation */
static inline void TRI_NORMAL(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n)
        {
            Vec3 u, v;

            VEC3_OP(u,v1,-,v0);
            VEC3_OP(v,v2,-,v0);
            VEC3_CROSS(n,u,v);
            VEC3_NORM(n);
        }

static inline void TRI_PLANE(Vec3 v0, Vec3 v1, Vec3 v2, Vec4 plane)
        {
            TRI_NORMAL(v0,v1,v2,plane);
            plane[3] = -VEC3_DOT(plane,v0);
        }

static inline float TRI_AREA(Vec3 v0, Vec3 v1, Vec3 v2)
        {
            Vec3 u, v, cp;
            VEC3_OP(u,v1,-,v0);
            VEC3_OP(v,v2,-,v0);
            VEC3_CROSS(cp,u,v);
            return (0.5f * VEC3_LEN(cp));
        }

#ifdef __cplusplus
}
#endif

#endif /* NAT_MATH_H */
