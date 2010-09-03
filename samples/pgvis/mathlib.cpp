#include <stdio.h>
#include "mathlib.h"

Point operator* (Matrix &m, Point &p)
{
	// must assert m is either 3x4 or 4x4 matrix
	float *mm = m.mb_element;
	return Point(mm[0]*p.x + mm[1]*p.y + mm[2]*p.z + mm[3],
				 mm[4]*p.x + mm[5]*p.y + mm[6]*p.z + mm[7],
				 mm[8]*p.x + mm[9]*p.y + mm[10]*p.z + mm[11]);
}

Vector operator* (Matrix &m, Vector &v)
{
	float *mm = m.mb_element;
	return Vector(mm[0]*v.x + mm[1]*v.y + mm[2]*v.z + mm[3],
				  mm[4]*v.x + mm[5]*v.y + mm[6]*v.z + mm[7],
				  mm[8]*v.x + mm[9]*v.y + mm[10]*v.z + mm[11]);
}

BBox Union(const BBox &b, const Point &p) {
    BBox ret = b;
    ret.pMin.x = min(b.pMin.x, p.x);
    ret.pMin.y = min(b.pMin.y, p.y);
    ret.pMin.z = min(b.pMin.z, p.z);
    ret.pMax.x = max(b.pMax.x, p.x);
    ret.pMax.y = max(b.pMax.y, p.y);
    ret.pMax.z = max(b.pMax.z, p.z);
    return ret;
}

BBox Union(const BBox &b, const BBox &b2) {
    BBox ret;
    ret.pMin.x = min(b.pMin.x, b2.pMin.x);
    ret.pMin.y = min(b.pMin.y, b2.pMin.y);
    ret.pMin.z = min(b.pMin.z, b2.pMin.z);
    ret.pMax.x = max(b.pMax.x, b2.pMax.x);
    ret.pMax.y = max(b.pMax.y, b2.pMax.y);
    ret.pMax.z = max(b.pMax.z, b2.pMax.z);
    return ret;
}

BBox Intersection(const BBox &b1, const BBox &b2) {
    BBox ret;
    ret.pMin.x = max(b1.pMin.x, b2.pMin.x);
    ret.pMin.y = max(b1.pMin.y, b2.pMin.y);
    ret.pMin.z = max(b1.pMin.z, b2.pMin.z);
    ret.pMax.x = min(b1.pMax.x, b2.pMax.x);
    ret.pMax.y = min(b1.pMax.y, b2.pMax.y);
    ret.pMax.z = min(b1.pMax.z, b2.pMax.z);
    return ret;
}

bool BBox::IntersectP(const Ray &ray, float *hitt0,
        float *hitt1) const {
    // Initialize parametric interval
    float t0 = ray.mint, t1 = ray.maxt;
    // Check X slab
    float invRayDir = 1.f / ray.D.x;
    float tNear = (pMin.x - ray.O.x) * invRayDir;
    float tFar  = (pMax.x - ray.O.x) * invRayDir;
    // Update parametric interval
    if (tNear > tFar) swap(tNear, tFar);
    t0 = max(tNear, t0);
    t1 = min(tFar,  t1);
    if (t0 > t1) return false;
    // Check Y slab
    invRayDir = 1.f / ray.D.y;
    tNear = (pMin.y - ray.O.y) * invRayDir;
    tFar  = (pMax.y - ray.O.y) * invRayDir;
    // Update parametric interval
    if (tNear > tFar) swap(tNear, tFar);
    t0 = max(tNear, t0);
    t1 = min(tFar,  t1);
    if (t0 > t1) return false;
    // Check Z slab
    invRayDir = 1.f / ray.D.z;
    tNear = (pMin.z - ray.O.z) * invRayDir;
    tFar  = (pMax.z - ray.O.z) * invRayDir;
    // Update parametric interval
    if (tNear > tFar) swap(tNear, tFar);
    t0 = max(tNear, t0);
    t1 = min(tFar,  t1);
    if (t0 > t1) return false;
    if (hitt0) *hitt0 = t0;
    if (hitt1) *hitt1 = t1;
    return true;
}

bool read_mat4(char *fname, Matrix &m)
{
	FILE *fp;
	fp = fopen(fname, "rb");
	m.resize(4, 4);
	if(!fp)
		return false;
	for(int i=0;i<4;i++) {
		if(fscanf(fp, "%f%f%f%f\n", m[i]+0, m[i]+1, m[i]+2, m[i]+3)!=4)
			return false;
	}
	fclose(fp);
	return true;
}

void Matrix::create(int _m, int _n, float* element)
{
	m = _m;
	n = _n;
	if(m < 1 || n < 1)
		throw MatrixException("Incorrect matrix size.");
	if(mb_element)
		delete [] mb_element;
	mb_element = new float[m*n];
	if(!mb_element)
		throw MatrixException("Not enough memory : allocating matrix memory.");
	if(element)
	{
		for(int i=0; i<m*n; i++)
			mb_element[i] = element[i];
	} else {
		memset(mb_element, 0, sizeof(float) * m * n);
	}
}

float Matrix::det()
{
	Matrix tmp;
	float det_value = 1.0;
	tmp = uppertriangle();

    for (int i=0; i<n; i++)
		det_value = det_value * tmp[i][i];

	return det_value*idf;
}

Matrix Matrix:: uppertriangle()
{
	float f1, temp;
	int i,j,v=1;
	idf = 1.0;

	if(!checkSquared())
		throw MatrixException("Attempt to operate on a non-square matrix.");
	Matrix tmp(this);
	for(i=0; i<m-1; i++)
		for(j=i+1; j<m; j++)
		{
			v = 1;
			while(tmp[i][i] == 0)
			{
				if(i+v >= m)
				{
					idf = 0;
					break;
				}
				else
				{
					for(int c=0; c<m; c++)
					{
						temp = tmp[i][c];
						tmp[i][c] = tmp[i+v][c];
						tmp[i+v][c] = temp;
					}
					v++;
					idf = idf*(-1);
				}
			}
			if(tmp[i][i] != 0)
			{
				f1 = -tmp[j][i]/tmp[i][i];
				for(int s=i; s<m; s++)
					tmp[j][s] = f1*tmp[i][s] + tmp[j][s];
			}
		}
    return tmp;
}

Matrix Matrix::adj()
{
	Matrix tmp(m,n);

	int t = m;
	int i,j,ii,jj,iii,jjj;
	float det_value;

	if(!checkSquared())
		throw MatrixException("Attempt to operate on a non-square matrix.");
	
	for(i=0; i<t; i++)
		for(j=0; j<t; j++)
		{
			Matrix aux(t-1,t-1);
			for(ii=0,iii=0; ii<t; iii=(ii==i?iii:iii+1),ii++)
				for(jj=0,jjj=0; jj<t; jjj=(jj==j?jjj:jjj+1),jj++)
					if((ii!=i)&&(jj!=j))
						aux[iii][jjj]=mb_element[ii*n+jj];

			det_value = aux.det();
			tmp[i][j] = det_value*((i+j)%2?-1:1);
		}
	tmp = tmp.transpose();
	return tmp;
}

Matrix Matrix::inv()
{
	if(!checkSquared())
		throw MatrixException("Attempt to operate on a non-square matrix.");
	Matrix tmp(this);
	Matrix adj_mat = this->adj();

	float det_value = this->det();
	float dd;

	if(det_value == 0.0)
		throw MatrixException("Attempt to invert a singular matrix.");
	dd = 1.0f / det_value;
	
	int i,j;
	for(i=0; i<m; i++)
		for(j=0; j<m; j++)
			tmp[i][j] = dd*adj_mat[i][j];

	return tmp;
}

Matrix Matrix::transpose()
{
	Matrix tmp(n,m);

	int i,j;

	for(i=0; i<m; i++)
		for(j=0; j<n; j++)
			tmp[j][i] = (*this)[i][j];
	return tmp;
}

float Matrix::pythag(float a, float b)
{
	float absa,absb,absc;
	absa=fabsf(a);
	absb=fabsf(b);
	if (absa > absb)
	{
		absc = absb/absa;
		return absa*sqrtf(1.0f+absc*absc);
	}
	else
	{
		absc = absa/absb;
		return (absb == 0 ? 0 : absb*sqrtf(1.0f+absc*absc));
	}
}


#define SIGN(a,b) ((b) >= 0 ? fabsf(a) : -fabsf(a))
#define MAX(a,b) ((a)>(b)?(a):(b))

float* Matrix::svd(Matrix &u,Matrix &v)
{
	int flag,i,its,j,jj,k,l=0,nm=0;
	float c,f,h,s,x,y,z;
	float anorm=0.0,g=0.0,scale=0.0;
	float *rv1,*w;
	int m = this->m;
	int n = this->n;
	Matrix a(this);
	v.resize(n,n);

	if (m < n) 
		throw MatrixException("SVDCMP: You must augment matrix with extra zero rows");
	rv1 = new float[n];
	w = new float[n];

	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i-1]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += fabsf(a[k-1][i-1]);
			if (scale) {
				for (k=i;k<=m;k++) {
					a[k-1][i-1] /= scale;
					s += a[k-1][i-1]*a[k-1][i-1];
				}
				f=a[i-1][i-1];
				g = -SIGN(sqrtf(s),f);
				h=f*g-s;
				a[i-1][i-1]=f-g;
				if (i != n) {
					for (j=l;j<=n;j++) {
						for (s=0.0,k=i;k<=m;k++) s += a[k-1][i-1]*a[k-1][j-1];
						f=s/h;
						for (k=i;k<=m;k++) a[k-1][j-1] += f*a[k-1][i-1];
					}
				}
				for (k=i;k<=m;k++) a[k-1][i-1] *= scale;
			}
		}
		w[i-1]=scale*g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += fabsf(a[i-1][k-1]);
			if (scale) {
				for (k=l;k<=n;k++) {
					a[i-1][k-1] /= scale;
					s += a[i-1][k-1]*a[i-1][k-1];
				}
				f=a[i-1][l-1];
				g = -SIGN(sqrtf(s),f);
				h=f*g-s;
				a[i-1][l-1]=f-g;
				for (k=l;k<=n;k++) rv1[k-1]=a[i-1][k-1]/h;
				if (i != m) {
					for (j=l;j<=m;j++) {
						for (s=0.0,k=l;k<=n;k++) s += a[j-1][k-1]*a[i-1][k-1];
						for (k=l;k<=n;k++) a[j-1][k-1] += s*rv1[k-1];
					}
				}
				for (k=l;k<=n;k++) a[i-1][k-1] *= scale;
			}
		}
		anorm=MAX(anorm,(fabsf(w[i-1])+fabsf(rv1[i-1])));
	}
	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++)
					v[j-1][i-1]=(a[i-1][j-1]/a[i-1][l-1])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[i-1][k-1]*v[k-1][j-1];
					for (k=l;k<=n;k++) v[k-1][j-1] += s*v[k-1][i-1];
				}
			}
			for (j=l;j<=n;j++) v[i-1][j-1]=v[j-1][i-1]=0.0;
		}
		v[i-1][i-1]=1.0;
		g=rv1[i-1];
		l=i;
	}
	for (i=n;i>=1;i--) {
		l=i+1;
		g=w[i-1];
		if (i < n)
			for (j=l;j<=n;j++) a[i-1][j-1]=0.0;
		if (g) {
			g=1.0f/g;
			if (i != n) {
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=m;k++) s += a[k-1][i-1]*a[k-1][j-1];
					f=(s/a[i-1][i-1])*g;
					for (k=i;k<=m;k++) a[k-1][j-1] += f*a[k-1][i-1];
				}
			}
			for (j=i;j<=m;j++) a[j-1][i-1] *= g;
		} else {
			for (j=i;j<=m;j++) a[j-1][i-1]=0.0;
		}
		++a[i-1][i-1];
	}
	for (k=n;k>=1;k--) {
		for (its=1;its<=1000;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if (fabs(rv1[l-1])+anorm == anorm) {
					flag=0;
					break;
				}
				if (fabs(w[nm-1])+anorm == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i-1];
					if (fabs(f)+anorm != anorm) {
						g=w[i-1];
						h=pythag(f,g);
						w[i-1]=h;
						h=1.0f/h;
						c=g*h;
						s=(-f*h);
						for (j=1;j<=m;j++) {
							y=a[j-1][nm-1];
							z=a[j-1][i-1];
							a[j-1][nm-1]=y*c+z*s;
							a[j-1][i-1]=z*c-y*s;
						}
					}
				}
			}
			z=w[k-1];
			if (l == k) {
				if (z < 0.0) {
					w[k-1] = -z;
					for (j=1;j<=n;j++) v[j-1][k-1]=(-v[j-1][k-1]);
				}
				break;
			}
			if (its == 1000) 
				printf("warning: no convergence in 1000 SVDCMP iterations");
			x=w[l-1];
			nm=k-1;
			y=w[nm-1];
			g=rv1[nm-1];
			h=rv1[k-1];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0f*h*y);
			g=pythag(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i-1];
				y=w[i-1];
				h=s*g;
				g=c*g;
				z=pythag(f,h);
				rv1[j-1]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g=g*c-x*s;
				h=y*s;
				y=y*c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj-1][j-1];
					z=v[jj-1][i-1];
					v[jj-1][j-1]=x*c+z*s;
					v[jj-1][i-1]=z*c-x*s;
				}
				z=pythag(f,h);
				w[j-1]=z;
				if (z) {
					z=1.0f/z;
					c=f*z;
					s=h*z;
				}
				f=(c*g)+(s*y);
				x=(c*y)-(s*g);
				for (jj=1;jj<=m;jj++) {
					y=a[jj-1][j-1];
					z=a[jj-1][i-1];
					a[jj-1][j-1]=y*c+z*s;
					a[jj-1][i-1]=z*c-y*s;

				}
			}
			rv1[l-1]=0.0;
			rv1[k-1]=f;
			w[k-1]=x;
		}
	}
	delete [] rv1;
	u=a;
	return w;
}
