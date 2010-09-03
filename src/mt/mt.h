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
/* Protection from multiple includes. */
#ifndef INCLUDED_MT_H
#define INCLUDED_MT_H

#ifndef _WIN32
#pragma interface
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ply.h>

#if !defined(_WIN32) && !defined(__APPLE__)
#include <values.h>
#else
#include <float.h>
#define MAXINT INT_MAX
#define MAXFLOAT FLT_MAX
#endif

#ifdef _WIN32
#ifndef for
#define for if(0); else for
#endif
#endif

typedef float mtReal;

enum mtDimension {MT_X=0, MT_Y=1, MT_Z=2};

#define MAX_POINT_SIZE  20

class mtVec3
{
  public:
    mtVec3(mtReal x=0.0, mtReal y=0.0, mtReal z=0.0)
    {
	data[0] = x;
	data[1] = y;
	data[2] = z;
    };

    mtVec3(const mtVec3 &vec)
    {
	data[0] = vec.data[0];
	data[1] = vec.data[1];
	data[2] = vec.data[2];
    };
    
    mtReal &operator[] (mtDimension index)
    {
	return data[index];
    };
    
    mtReal &operator[] (int index)
    {
	return data[index];
    };
    
    mtVec3 &operator = (const mtVec3 &vec)
    {
	data[0] = vec.data[0];
	data[1] = vec.data[1];
	data[2] = vec.data[2];
	return *this;
    };
    
    mtVec3 &operator += (const mtVec3 &vec)
    {
	data[0] += vec.data[0];
	data[1] += vec.data[1];
	data[2] += vec.data[2];
	return *this;
    };

    mtVec3 &operator -= (const mtVec3 &vec)
    {
	data[0] -= vec.data[0];
	data[1] -= vec.data[1];
	data[2] -= vec.data[2];
	return *this;
    };
    
    mtVec3 &operator *= (const mtVec3 &vec)
    {
	data[0] *= vec.data[0];
	data[1] *= vec.data[1];
	data[2] *= vec.data[2];
	return *this;
    };
    
    mtVec3 &operator *= (mtReal scale)
    {
	data[0] *= scale;
	data[1] *= scale;
	data[2] *= scale;
	return *this;
    };
 
    mtVec3 operator + (const mtVec3 &vec) const
    {
	mtVec3 result = *this;
	result += vec;
	return result;
    };
    
    mtVec3 operator - (const mtVec3 &vec) const
    {
	mtVec3 result = *this;
	result -= vec;
	return result;
    };
 
    mtVec3 operator * (const mtVec3 &vec) const
    {
	mtVec3 result = *this;
	result *= vec;
	return result;
    };
    
    mtVec3 operator * (mtReal scale) const
    {
	mtVec3 result = *this;
	result *= scale;
	return result;
    };
    
    mtVec3 operator ^ (const mtVec3 &vec) const
    {
	mtVec3 result;
	result[0] = data[1]*vec.data[2] - data[2]*vec.data[1];
	result[1] = data[2]*vec.data[0] - data[0]*vec.data[2];
	result[2] = data[0]*vec.data[1] - data[1]*vec.data[0];
	return result;
    };
    
    void set(mtReal x, mtReal y, mtReal z)
    {
	data[0] = x;
	data[1] = y;
	data[2] = z;
    };
 
    void set(mtReal *d)
    {
	data[0] = d[0];
	data[1] = d[1];
	data[2] = d[2];
    };
 
    void print()
    {
	fprintf(stderr, "(%f, %f, %f)", 
		data[0], data[1], data[2]);
	return;
    };
 
    mtReal dot(const mtVec3 &vec) const
    {
	return
	    data[0]*vec.data[0] + data[1]*vec.data[1] +
	    data[2]*vec.data[2];
    };
    
    mtReal length()
    {
 	return (mtReal) sqrt(this->dot(*this));
    };

    mtReal SquaredLength()
    {
	return (mtReal) this->dot(*this);
    };

    mtVec3 &normalize()
    {
	mtReal len;
	
	len = this->length();
	
	if (len == 0.0)
	    fprintf(stderr, "Warning: Cannot normalize 0 vector!\n");
	else
	    *this *= (mtReal) 1.0/len;
	
	return *this;
    };
 
    mtVec3 &clamp(mtReal min=0.0, mtReal max=1.0)
    {
	data[0] = (data[0] < min) ? min : ((data[0] > max) ? max : data[0]);
	data[1] = (data[1] < min) ? min : ((data[1] > max) ? max : data[1]);
	data[2] = (data[2] < min) ? min : ((data[2] > max) ? max : data[2]);
	return *this;
    };
    
    mtReal data[3];
};


class mtVec2
{
  public:
    mtVec2(mtReal x=0.0, mtReal y=0.0)
    {
	data[0] = x;
	data[1] = y;
    };

    mtVec2(const mtVec2 &vec)
    {
	data[0] = vec.data[0];
	data[1] = vec.data[1];
    };
    
    mtReal &operator[] (mtDimension index)
    {
	return data[index];
    };
    
    mtReal &operator[] (int index)
    {
	return data[index];
    };
    
    mtVec2 &operator = (const mtVec2 &vec)
    {
	data[0] = vec.data[0];
	data[1] = vec.data[1];
	return *this;
    };
    
    mtVec2 &operator += (const mtVec2 &vec)
    {
	data[0] += vec.data[0];
	data[1] += vec.data[1];
	return *this;
    };

    mtVec2 &operator -= (const mtVec2 &vec)
    {
	data[0] -= vec.data[0];
	data[1] -= vec.data[1];
	return *this;
    };
    
    mtVec2 &operator *= (const mtVec2 &vec)
    {
	data[0] *= vec.data[0];
	data[1] *= vec.data[1];
	return *this;
    };
    
    mtVec2 &operator *= (mtReal scale)
    {
	data[0] *= scale;
	data[1] *= scale;
	return *this;
    };
 
    mtVec2 operator + (const mtVec2 &vec) const
    {
	mtVec2 result = *this;
	result += vec;
	return result;
    };
    
    mtVec2 operator - (const mtVec2 &vec) const
    {
	mtVec2 result = *this;
	result -= vec;
	return result;
    };
 
    mtVec2 operator * (const mtVec2 &vec) const
    {
	mtVec2 result = *this;
	result *= vec;
	return result;
    };
    
    mtVec2 operator * (mtReal scale) const
    {
	mtVec2 result = *this;
	result *= scale;
	return result;
    };
    
    mtReal operator ^ (const mtVec2 &vec) const
    {
	mtReal result = data[0]*vec.data[1] - data[1]*vec.data[0];
	return result;
    };
    
    void set(mtReal x, mtReal y)
    {
	data[0] = x;
	data[1] = y;
    };
 
    void print()
    {
	fprintf(stderr, "(%f, %f)", 
		data[0], data[1]);
	return;
    };
 
    mtReal dot(const mtVec2 &vec) const
    {
	return data[0]*vec.data[0] + data[1]*vec.data[1];
    };
    
    mtReal length()
    {
	return (mtReal) sqrt(this->dot(*this));
    };
    
    mtVec2 &normalize()
    {
	mtReal len;
	
	len = this->length();
	
	if (len == 0.0)
	    fprintf(stderr, "Warning: Cannot normalize 0 vector!\n");
	else
	    *this *= (mtReal) 1.0/len;
	
	return *this;
    };
 
    mtVec2 &clamp(mtReal min=0.0, mtReal max=1.0)
    {
	data[0] = (data[0] < min) ? min : ((data[0] > max) ? max : data[0]);
	data[1] = (data[1] < min) ? min : ((data[1] > max) ? max : data[1]);
	return *this;
    };
    
    mtReal data[2];
};

class mtColor
{
  public:
    mtColor(unsigned char r=0, unsigned char g=0, unsigned char b=0)
    {
	data[0] = r;
	data[1] = g;
	data[2] = b;
    };

    mtColor(const mtColor &col)
    {
	data[0] = col.data[0];
	data[1] = col.data[1];
	data[2] = col.data[2];
    };
    
    unsigned char &operator[] (int index)
    {
	return data[index];
    };
    
    mtColor &operator = (const mtColor &col)
    {
	data[0] = col.data[0];
	data[1] = col.data[1];
	data[2] = col.data[2];
	return *this;
    };
    
    void set(unsigned char r, unsigned char g, unsigned char b)
    {
	data[0] = r;
	data[1] = g;
	data[2] = b;
    };
 
    void set(unsigned char *c)
    {
	data[0] = c[0];
	data[1] = c[1];
	data[2] = c[2];
    };
 
    void print()
    {
	fprintf(stderr, "(%d, %d, %d)", 
		(int)(data[0]), (int)(data[1]), (int)(data[2]));
	return;
    };
 
    mtColor &clamp(unsigned char min=0, unsigned char max=255)
    {
	data[0] = (data[0] < min) ? min : ((data[0] > max) ? max : data[0]);
	data[1] = (data[1] < min) ? min : ((data[1] > max) ? max : data[1]);
	data[2] = (data[2] < min) ? min : ((data[2] > max) ? max : data[2]);
	return *this;
    };
    
    unsigned char data[3];
};

class MT;
class mtCut;

struct plyVertex;

class mtVertex
{
  public:
    mtVec3 coord;

    mtVertex() {};
    mtVertex(const mtVertex &vert) {set(vert.coord);};
    mtVertex(mtVec3 crd)
    {
	coord = crd;
    };
    mtVertex &operator = (const mtVertex &v)
    {
	coord = v.coord;
	return *this;
    };
    
    void set(mtVec3 crd)
    {
	coord = crd;
    };
    virtual void print()
    {
        coord.print();
    };
    virtual void draw();
    virtual int size() const { return sizeof(mtVertex); };
    virtual mtVertex *makeNew() const { return new mtVertex; };
    virtual mtVertex *makeNew(int num) const { return new mtVertex[num]; };
    virtual void copySame(mtVertex *destVert) const 
    { 
	*((mtVertex *)(destVert)) = *this;
    };
    virtual void describeProperties(PlyFile *ply, char *elem_name);
    virtual void fillPlyVertex(PlyFile *ply, plyVertex *pvert);
};

class mtNVertex : public mtVertex 
{
  public:
    mtVec3 normal;

    mtNVertex() {coord.set(0,0,0); normal.set(1,0,0);};
    mtNVertex(const mtNVertex &vert) {set(vert.coord, vert.normal);};
    mtNVertex(mtVec3 crd, mtVec3 nrm)
    {
	coord = crd;
 	normal = nrm;
    };
    mtNVertex &operator = (const mtNVertex &v)
    {
	coord = v.coord;
	normal = v.normal;
	return *this;
    }
#if 1
    mtNVertex &operator = (const mtVertex &v)
    {
	coord = v.coord;
	//normal.set(1,0,0);
	return *this;
    }
#endif
    void set(mtVec3 crd, mtVec3 nrm)
    {
	coord = crd;
        normal = nrm; 
    };
    void print()
    {    
        coord.print();
	normal.print();
    };
    virtual void draw();
    virtual int size() const { return sizeof(mtNVertex); };
    virtual mtVertex *makeNew() const { return new mtNVertex; };
    virtual mtVertex *makeNew(int num) const { return new mtNVertex[num]; };
    virtual void copySame(mtVertex *destVert) const 
    { 
	*((mtNVertex *)(destVert)) = *this;
    };
    virtual void describeProperties(PlyFile *ply, char *elem_name);
    virtual void fillPlyVertex(PlyFile *ply, plyVertex *pvert);
};

class mtTVertex : public mtVertex
{
  public:
    mtVec2 texcoord;

    mtTVertex() {};
    mtTVertex(const mtTVertex &vert) {set(vert.coord, vert.texcoord);};
    mtTVertex(mtVec3 crd, mtVec2 txcrd)
    {
	coord = crd;
	texcoord = txcrd;
    };
    mtTVertex &operator = (const mtTVertex &v)
    {
	coord = v.coord;
	texcoord = v.texcoord;
	return *this;
    };
    void set(mtVec3 crd, mtVec2 txcrd)
    {
	coord = crd;
	texcoord = txcrd;
    };
    void print()
    {
        coord.print();
	texcoord.print();
    };
    virtual void draw();
    virtual int size() const { return sizeof(mtTVertex); };
    virtual mtVertex *makeNew() const { return new mtTVertex; };
    virtual mtVertex *makeNew(int num) const { return new mtTVertex[num]; };
    virtual void copySame(mtVertex *destVert) const 
    { 
	*((mtTVertex *)(destVert)) = *this;
    };
    virtual void describeProperties(PlyFile *ply, char *elem_name);
    virtual void fillPlyVertex(PlyFile *ply, plyVertex *pvert);
};

class mtCVertex : public mtVertex
{
  public:
    mtColor color;

    mtCVertex() {};
    mtCVertex(const mtCVertex &vert) {set(vert.coord, vert.color);};
    mtCVertex(mtVec3 crd, mtColor clr)
    {
	coord = crd;
	color = clr;
    };
    mtCVertex &operator = (const mtCVertex &v)
    {
	coord = v.coord;
	color = v.color;
	return *this;
    };
    void set(mtVec3 crd, mtColor clr)
    {
	coord = crd;
	color = clr;
    };
    void print()
    {    
        coord.print();
	color.print();
    };
    virtual void draw();
    virtual int size() const { return sizeof(mtCVertex); };
    virtual mtVertex *makeNew() const { return new mtCVertex; };
    virtual mtVertex *makeNew(int num) const { return new mtCVertex[num]; };
    virtual void copySame(mtVertex *destVert) const 
    { 
	*((mtCVertex *)(destVert)) = *this;
    };
    virtual void describeProperties(PlyFile *ply, char *elem_name);
    virtual void fillPlyVertex(PlyFile *ply, plyVertex *pvert);
};


class mtCNVertex : public mtVertex
{
  public:
    mtColor color;
    mtVec3  normal;
    
    mtCNVertex() {};
    mtCNVertex(const mtCNVertex &vert) {set(vert.coord, vert.color,
					    vert.normal);};
    mtCNVertex(mtVec3 crd, mtColor clr, mtVec3 nrm)
    {
	coord = crd;
	color = clr;
	normal = nrm;
    };
    mtCNVertex &operator = (const mtCNVertex &v)
    {
	coord = v.coord;
	color = v.color;
	normal = v.normal;
	return *this;
    };
    void set(mtVec3 crd, mtColor clr, mtVec3 nrm)
    {
	coord = crd;
	color = clr;
	normal = nrm;
    };
    void print()
    {    
        coord.print();
	color.print();
	normal.print();
    };
    virtual void draw();
    virtual int size() const { return sizeof(mtCNVertex); };
    virtual mtVertex *makeNew() const { return new mtCNVertex; };
    virtual mtVertex *makeNew(int num) const { return new mtCNVertex[num]; };
    virtual void copySame(mtVertex *destVert) const 
    { 
	*((mtCNVertex *)(destVert)) = *this;
    };
    virtual void describeProperties(PlyFile *ply, char *elem_name);
    virtual void fillPlyVertex(PlyFile *ply, plyVertex *pvert);
};


class mtCTVertex : public mtVertex
{
  public:
    mtColor color;
    mtVec2  texcoord;
    
    mtCTVertex() {};
    mtCTVertex(const mtCTVertex &vert) {set(vert.coord, vert.color,
					    vert.texcoord);};
    mtCTVertex(mtVec3 crd, mtColor clr, mtVec2 txcrd)
    {
	coord = crd;
	color = clr;
	texcoord = txcrd;
    };
    mtCTVertex &operator = (const mtCTVertex &v)
    {
	coord = v.coord;
	color = v.color;
	texcoord = v.texcoord;
	return *this;
    };
    void set(mtVec3 crd, mtColor clr, mtVec2 txcrd)
    {
	coord = crd;
	color = clr;
	texcoord = txcrd;
    };
    void print()
    {    
        coord.print();
	color.print();
	texcoord.print();
    };
    virtual void draw();
    virtual int size() const { return sizeof(mtCTVertex); };
    virtual mtVertex *makeNew() const { return new mtCTVertex; };
    virtual mtVertex *makeNew(int num) const { return new mtCTVertex[num]; };
    virtual void copySame(mtVertex *destVert) const 
    { 
	*((mtCTVertex *)(destVert)) = *this;
    };
    virtual void describeProperties(PlyFile *ply, char *elem_name);
    virtual void fillPlyVertex(PlyFile *ply, plyVertex *pvert);
};


class mtNTVertex : public mtVertex
{
  public:
    mtVec3  normal;
    mtVec2  texcoord;
    
    mtNTVertex() {};
    mtNTVertex(const mtNTVertex &vert) {set(vert.coord, vert.normal,
					    vert.texcoord);};
    mtNTVertex(mtVec3 crd, mtVec3 nrm, mtVec2 txcrd)
    {
	coord = crd;
	normal = nrm;
	texcoord = txcrd;
    };
    mtNTVertex &operator = (const mtNTVertex &v)
    {
	coord = v.coord;
	normal = v.normal;
	texcoord = v.texcoord;
	return *this;
    };
    void set(mtVec3 crd, mtVec3 nrm, mtVec2 txcrd)
    {
	coord = crd;
	normal = nrm;
	texcoord = txcrd;
    };
    void print()
    {    
        coord.print();
	normal.print();
	texcoord.print();
    };
    virtual void draw();
    virtual int size() const { return sizeof(mtNTVertex); };
    virtual mtVertex *makeNew() const { return new mtNTVertex; };
    virtual mtVertex *makeNew(int num) const { return new mtNTVertex[num]; };
    virtual void copySame(mtVertex *destVert) const 
    { 
	*((mtNTVertex *)(destVert)) = *this;
    };
    virtual void describeProperties(PlyFile *ply, char *elem_name);
    virtual void fillPlyVertex(PlyFile *ply, plyVertex *pvert);
};


class mtCNTVertex : public mtVertex
{
  public:
    mtColor color;
    mtVec3  normal;
    mtVec2  texcoord;
    
    mtCNTVertex() {};
    mtCNTVertex(const mtCNTVertex &vert) {set(vert.coord, vert.color,
					      vert.normal, vert.texcoord);};
    mtCNTVertex(mtVec3 crd, mtColor clr, mtVec3 nrm, mtVec2 txcrd)
    {
	coord = crd;
	color = clr;
	normal = nrm;
	texcoord = txcrd;
    };
    mtCNTVertex &operator = (const mtCNTVertex &v)
    {
	coord = v.coord;
	color = v.color;
	normal = v.normal;
	texcoord = v.texcoord;
	return *this;
    };
    void set(mtVec3 crd, mtColor clr, mtVec3 nrm, mtVec2 txcrd)
    {
	coord = crd;
	color = clr;
	normal = nrm;
	texcoord = txcrd;
    };
    void print()
    {    
        coord.print();
	color.print();
	normal.print();
	texcoord.print();
    };
    virtual void draw();
    virtual int size() const { return sizeof(mtCNTVertex); };
    virtual mtVertex *makeNew() const { return new mtCNTVertex; };
    virtual mtVertex *makeNew(int num) const { return new mtCNTVertex[num]; };
    virtual void copySame(mtVertex *destVert) const 
    { 
	*((mtCNTVertex *)(destVert)) = *this;
    };
    virtual void describeProperties(PlyFile *ply, char *elem_name);
    virtual void fillPlyVertex(PlyFile *ply, plyVertex *pvert);
};


class mtView;

class mtPoint
{
  public:
    mtVertex *sample;
    mtReal    radius;

    mtPoint() { sample = NULL; radius = 0.0; };
    
    void draw(mtView *view = NULL);
};

#if 0
class mtPrimitive
{
};

class mtTriSet : mtPrimitive
{
    int  numTris;
    int *tris;    // indices into tris of an MT
};

class mtTriStripSet : mtPrimitive
{
};

class mtPointSet : mtPrimitive
{
};

#endif

class mtStrip
{
  public:
    int numVerts;
    mtNVertex *verts;

    mtStrip() { numVerts = 0; };
    mtStrip(int n)
    {
	verts = new mtNVertex[n];
	numVerts = 0;
    }; // We should really keep MAX around.
    mtStrip(int n, mtNVertex *v)
    {
	verts = new mtNVertex[n];
	numVerts = n;
	for(int i=0; i<n; i++)
	    verts[i]=v[i];
    };
    void addNVert(mtNVertex *v) { verts[numVerts++] = *v; };
    void addVert(mtVertex *v) { verts[numVerts++] = *v; };
    void addNormal2(mtVec3 *n) { verts[numVerts].normal = *n; };
    void draw();
};

class mtTriangle
{
  public:
    int verts[3];
    mtVec3 normal;

    mtTriangle() {};
    mtTriangle(int v0, int v1, int v2)
    {
	verts[0] = v0;
	verts[1] = v1;
	verts[2] = v2;
    };
    void set(int v0, int v1, int v2)
    {
	verts[0] = v0;
	verts[1] = v1;
	verts[2] = v2;
    };
    void setNormal(mtVec3 n) { normal = n; }
    void setNormal(mtReal nx, mtReal ny, mtReal nz)
        { normal.set(nx, ny, nz); }
    void print(MT *mt);
    void draw(MT *mt);
};

class mtArc
{
  private:
    int  start;
    int  end;
    int  numTris;
    int *tris;
    int  numPoints;
    int *points;
    int  keepcolor;
    int  frameno;
#ifdef VERTEXARRAY
    int  numStrips;
    mtStrip *strip;
    int     *stripLen;
#else
    int  numStrips;
    mtStrip **strips; // Should try *strips[MAXSTRIPS]
#endif
    mtReal radius;
    mtVec3 center;
    int  patchNumber;
    char borderFlag;
    
  public:
    mtArc()
    {
	start = end = -1;
	tris = NULL;
	numTris = 0;
	points = NULL;
	numPoints = 0;
	patchNumber = 0;
	borderFlag = 0;
    };
    mtArc(int tri)
    {
	start = end = -1;
	tris = new int;
	tris[0] = tri;
	numTris = 1;
	points = NULL;
	numPoints = 0;
	patchNumber = 0;
	borderFlag = 0;
    }
    
    void setStart(int s) { start = s; };
    void setEnd(int e)   { end = e; };
    int  getStart() const { return start; };
    int  getEnd()   const { return end;   };
    unsigned int  getdlistID(MT *mt) const;
    void setTri(int triNum, int tri) { tris[triNum] = tri; };
    void addTri(int tri);
    void addTris(int *newTris, int num);
    int  getNumTris() const { return numTris; };
    int  getTri(int tnum) const { return tris[tnum]; };
    void deleteTris()
    {
	delete tris;
	numTris = 0;
    };
    void addPoints(int *newPoints, int num);
    int  getNumPoints() const { return numPoints; };
    int  getPoint(int pnum) const { return points[pnum]; };
    mtReal getError(MT *mt, mtCut *cut, float *ret_d=NULL);
    void computeSPH(MT *mt);
    void makeStrips(MT *mt);
    void draw(MT *mt, mtCut *cut, float d=0);
    void drawReal(MT *mt, mtView *view = NULL);
    int  getFrame() { return frameno; }
    void setPatchNumber(int patchNum) { patchNumber=patchNum; };
    int  getPatchNumber() const { return patchNumber; };
    void setBorder() { borderFlag = 1; };
    void clearBorder() { borderFlag = 0; };
    char isBorder() const { return borderFlag; };
};

class mtNode
{
  private:
    int numParents;
    int *parents;

    int numChildren;
    int *children;

    mtReal error;

  public:
    void addParent(MT *mt, int arc);
    void addChild(MT *mt, int arc);

    void allocateParents(int numPar);
    void addParentNoAllocate(MT *mt, int arc);
    void clearParents()
    {
        if (parents)
	  delete parents;
	parents = NULL;
	numParents = 0;
    }

    void allocateChildren(int numChil);
    void addChildNoAllocate(MT *mt, int arc);
    void clearChildren()
    {
        if (children)
	  delete children;
	children=NULL;
	numChildren=0;
    }
    
    mtNode()
    {
	numParents = 0;
	numChildren = 0;
	parents = NULL;
	children = NULL;
	error = 0.0;
    }
    
    void setError(mtReal err) { error = err; };
    mtReal getError() { return error; };
    int getNumParents() { return numParents; };
    int getNumChildren() { return numChildren; };
    int getParent(int i) { return parents[i]; };
    int getChild(int i)  { return children[i]; };
};

class mtView
{
  public:
     mtVec3 eye;
     mtVec3 viewd;
     int    frameno;
     mtReal zp;
     void updateView(mtReal *e, mtReal *v, mtReal z)
     {
         eye.set(e);
         viewd.set(v);
         zp = z;
     }
     void nextFrame() { frameno ++; };
     void setFrame(int fno) { frameno = fno; };
     int getFrame() { return frameno; };
};


#define OBJERROR 0
#define SCRERROR 1

#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

/*****************************************************************************

 Time is in milliseconds

*****************************************************************************/
class mtTime
{
   public:
#if !(defined (__APPLE__) || defined (_WIN32))
      struct timeval mtclock;

      mtTime()       { mtclock.tv_sec = 0; mtclock.tv_usec = 0; }
      void zero()    { mtclock.tv_sec = 0; mtclock.tv_usec = 0;}
      void getTime() { gettimeofday(&mtclock, NULL); };
      int  operator- (const mtTime &t1)
      {
         double a = double(mtclock.tv_sec)+double(mtclock.tv_usec)/1.0E6;
         double b = double(t1.mtclock.tv_sec)+double(t1.mtclock.tv_usec)/1.0E6;
         return (int)((a-b)*1000.0);
      };
      int since()
      { 
	      mtTime now;
         now.getTime();
         return now - *this;
      };
#else
      mtTime()       { }
      void zero()    { }
      void getTime() { };
      int  operator- (const mtTime &t1) { return 0; };
      int since()    { return 0; };
#endif

};

typedef struct _gtStat {
   mtTime  start;
   int     gTimeUpdate;
   int     gTimeDrawTris;
   int     gTimeDrawPts;
   int     gStripStat;
   int     gVertStat;
   int     gNewArcStat;
   int     gArcStat;
   int     gNodeStat;
   int     gNumPoints;
   int     gNumTris;
} gStat;

class mtCut
{
  private:
    int     numArcs;
    int     maxArcs;
    int    *arcs;
    float  *depths;
    char   *nodeAboveCut; // a bit vector would be enough...
    mtView  view;
    
    void cleanArcList(MT *mt);
    void raiseNode(MT *mt, int nodeID, mtReal error);
    void lowerNode(MT *mt, int nodeID, mtReal error);
    void addArc(MT *mt, int arcID, float d=0.0);
    void raiseErrorCut(MT *mt, mtReal error);
    void lowerErrorCut(MT *mt, mtReal error);

  public:
    char    dumpMode;
    char    errorMode;
    char    renderMode;
    gStat   stat;
    
    mtCut()
    {
	numArcs = 0;
	maxArcs = 0;
	arcs = NULL;
	depths = NULL;
	nodeAboveCut = NULL;
	dumpMode = 0;
	errorMode = OBJERROR;
	renderMode = 0;
    };
    void newErrorCut(MT *mt, mtReal error);
    void adaptErrorCut(MT *mt, mtReal error);
    int getNumArcs() const {return numArcs;};
    int getNumTris(MT *mt) const;
    int *getArc(int arcno) { return &arcs[arcno]; }
    float *getDepth(int arcno) { return &depths[arcno]; }
    mtView *getView() { return &view; }
    void updateView(mtReal *eye, mtReal *viewd, mtReal zp)
         { view.updateView(eye, viewd, zp); };
    void draw(MT *mt);
};

#if 0
inline mtArc *
MT::getArc(int i);

inline mtNode *
MT::getNode(int i);

class mtBVNode;

inline int MT::bvnodeIndex(mtBVNode &bvnode);
#endif

typedef int mtBV;

class mtBVNode
{
  public:
    mtBV bv;

    mtBVNode() {};
    void constructBV() {};
    mtBV *getBV() { return &bv; };

    // eventually, these may all be virtual functions that depend on the
    // type of SBVH we have built.

    // this implementation is for mtBVNode associated with each mtNode
    inline int getNumTris(MT *mt);
    inline int getTri(MT *mt, int index);
    inline int getNumChildren(MT *mt);
    inline int getChild(MT *mt, int index);
};


class MT
{
  private:
    int numVerts;
    int maxVerts;
    mtVertex *verts; // Don't even access this array directly for most
                     // internal MT functions. It is dangerous because
                     // mtVertex now has derived classes, so we must be
                     // careful when indexing this array!!! Always access
                     // verts through the getVert() function, which does
                     // the addressing correctly.
    int numTris;
    int maxTris;
    mtTriangle *tris;
    int numArcs;
    int maxArcs;
    mtArc *arcs;
    int numNodes;
    int maxNodes;
    mtNode *nodes;
    int numPoints;
    int maxPoints;
    mtPoint *points;
    mtBVNode *bvnodes;
    unsigned int dlistBase;
    PlyOtherElems *other_elems;
    int numPatches;
    mtPoint **pointCache[MAX_POINT_SIZE];
    int       numPointsInCache[MAX_POINT_SIZE];
    int       maxPointsInCache[MAX_POINT_SIZE];
    
    int root;
    mtCut currentCut;
    char    retainedMode;
    
    // what about root or roots of MT?
    
    void mergeArcs();
    void calcMaxHeight(int nodeID, int *nodeMax);
    int  calcMaxHeight();
    void buildDL();

  public:
    MT()
    {
	numNodes = numArcs = numTris = numVerts = numPoints = 0;
	maxNodes = maxArcs = maxTris = maxVerts = maxPoints = 0;
	nodes = NULL;
	arcs = NULL;
	tris = NULL;
	verts = NULL;
	points = NULL;
	root = -1;
	other_elems = NULL;
        for (int i=0; i<MAX_POINT_SIZE; i++) {
           pointCache[i] = new mtPoint *[1];
           maxPointsInCache[i] = 1;
           numPointsInCache[i] = 0;
        }
	retainedMode = 0;
	numPatches = 1;
    };
	        
    void readPlyCollapses(char *filename=NULL);
    void readPlyMT(char *filename=NULL);
    void writePlyMT(char *filename=NULL);
    void setOtherElements(PlyOtherElems *other_elements)
      { other_elems = other_elements; };
    int getNumVerts() {return numVerts;};
    int getNumTris()  {return numTris;};
    int getNumArcs() {return numArcs;};
    int getNumNodes() {return numNodes;};
    int getNumPoints() {return numPoints;};
    int getRoot()     {return root;};
    char getRetainedMode() {return retainedMode;};
    unsigned int getDlistBase() {return dlistBase;};
    int addVertex(const mtVertex &v);
    int addTriangle(int vert0, int vert1, int vert2);
    int addArc();
    int addArc(int tri);
    int addArc(int *tris, int numTris);
    int addNode();
    int addPoint(const mtPoint &p);
    void allocateVerts(int num, mtVertex &sampleVert);
    void allocateTris(int num);
    void allocateNodes(int num);
    void allocateArcs(int num);
    void allocatePoints(int num);
    void setRoot(int node) { root = node; };
    mtCut *newErrorCut(mtReal error);
    mtArc *getArc(int arc) { return &(arcs[arc]); };
    mtNode *getNode(int node) { return &(nodes[node]); };
    mtTriangle *getTri(int tri) { return &(tris[tri]); };
    mtVertex *getVert(int vert)
    {
	return ((mtVertex *)(((char *) (verts))+verts[0].size()*vert));
    };
    mtPoint *getPoint(int point) { return &(points[point]); };
    int vertexIndex(const mtVertex &vert)
    {
	return (((char *)(&vert)) - ((char *)(verts))) / verts[0].size();
    };
    int arcIndex(const mtArc &arc) { return &arc - arcs; };
    int triangleIndex(const mtTriangle &tri) { return &tri - tris; };
    int nodeIndex(const mtNode &node) { return &node - nodes; };
    int pointIndex(const mtPoint &point) { return &point - points; };
    int bvnodeIndex(const mtBVNode &bvnode) { return &bvnode-bvnodes; };
    void connectArcs();
    void removeUnusedTris();
    void removeUnusedVerts();
    void removeEmptyArcs();
    void printStats();
    void buildBVH();
    int  getBVHRoot() { return 0; };
    void computeSPH()
    {
	for(int i=0; i<numArcs; i++)
	    arcs[i].computeSPH(this);
    }
    void makeStrips()
    {
	for(int i=0; i<numArcs; i++)
	    arcs[i].makeStrips(this);
    }
    void computeNORM();
    void flushPointCaches();
    void cachePoint(mtPoint *pt, int pointSize);
    void drawPointCaches();
    void enableRetainedMode() { buildDL(); retainedMode = 1; };
    int getNumPatches() const { return numPatches; };
};

/* Protection from multiple includes. */
#endif // INCLUDED_MT_H
