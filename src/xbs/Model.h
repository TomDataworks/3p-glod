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
#ifndef INCLUDED_MODEL_H
#define INCLUDED_MODEL_H

#define XBSVERTEX // this tells AttribSet that xbs vertices are present and to
                  // provide a getAt and setFrom function

#ifndef _WIN32
#pragma interface
#endif

#include <stdlib.h>
#if !defined(_WIN32) && !defined(__APPLE__)
#include <values.h>
#else
#include <float.h>
#endif
#include <stdio.h>
#include <math.h>
#include <ply.h>
#include <mt.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <glod_core.h>
#include <primtypes.h>
//note: permissiongrid is included later in this file.
using namespace VDS;

typedef float xbsReal;

enum xbsDimension {XBS_X=0, XBS_Y=1, XBS_Z=2};

class DiscreteLevel;

class xbsVec3
{
    public:
        xbsVec3(xbsReal x=0.0, xbsReal y=0.0, xbsReal z=0.0)
        {
            data[0] = x;
            data[1] = y;
            data[2] = z;
        };

        xbsVec3(const xbsVec3 &vec)
        {
            data[0] = vec.data[0];
            data[1] = vec.data[1];
            data[2] = vec.data[2];
        };
    
        xbsReal &operator[] (xbsDimension index)
        {
            return data[index];
        };
    
        xbsReal &operator[] (int index)
        {
            return data[index];
        };
    
        xbsVec3 &operator = (const xbsVec3 &vec)
        {
            data[0] = vec.data[0];
            data[1] = vec.data[1];
            data[2] = vec.data[2];
            return *this;
        };
    
        xbsVec3 &operator += (const xbsVec3 &vec)
        {
            data[0] += vec.data[0];
            data[1] += vec.data[1];
            data[2] += vec.data[2];
            return *this;
        };

        xbsVec3 &operator -= (const xbsVec3 &vec)
        {
            data[0] -= vec.data[0];
            data[1] -= vec.data[1];
            data[2] -= vec.data[2];
            return *this;
        };
    
        xbsVec3 &operator *= (const xbsVec3 &vec)
        {
            data[0] *= vec.data[0];
            data[1] *= vec.data[1];
            data[2] *= vec.data[2];
            return *this;
        };
    
        xbsVec3 &operator *= (xbsReal scale)
        {
            data[0] *= scale;
            data[1] *= scale;
            data[2] *= scale;
            return *this;
        };
 
        xbsVec3 &operator /= (const xbsVec3 &vec)
        {
            data[0] /= vec.data[0];
            data[1] /= vec.data[1];
            data[2] /= vec.data[2];
            return *this;
        };
    
        xbsVec3 &operator /= (xbsReal scale)
        {
            xbsReal invscale = (xbsReal)1.0 / scale;
            data[0] *= invscale;
            data[1] *= invscale;
            data[2] *= invscale;
            return *this;
        };
 
        xbsVec3 operator + (const xbsVec3 &vec) const
        {
            xbsVec3 result = *this;
            result += vec;
            return result;
        };
    
        xbsVec3 operator - (const xbsVec3 &vec) const
        {
            xbsVec3 result = *this;
            result -= vec;
            return result;
        };
 
        xbsVec3 operator * (const xbsVec3 &vec) const
        {
            xbsVec3 result = *this;
            result *= vec;
            return result;
        };
    
        xbsVec3 operator * (xbsReal scale) const
        {
            xbsVec3 result = *this;
            result *= scale;
            return result;
        };

        xbsVec3 operator / (const xbsVec3 &vec) const
        {
            xbsVec3 result = *this;
            result /= vec;
            return result;
        };
    
        xbsVec3 operator / (xbsReal scale) const
        {
            xbsVec3 result = *this;
            result /= scale;
            return result;
        };
    
        xbsVec3 operator ^ (const xbsVec3 &vec) const
        {
            xbsVec3 result;
            result[0] = data[1]*vec.data[2] - data[2]*vec.data[1];
            result[1] = data[2]*vec.data[0] - data[0]*vec.data[2];
            result[2] = data[0]*vec.data[1] - data[1]*vec.data[0];
            return result;
        };
    
        void set(xbsReal x, xbsReal y, xbsReal z)
        {
            data[0] = x;
            data[1] = y;
            data[2] = z;
        };
 
        void set(xbsReal *d)
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
 
        xbsReal dot(const xbsVec3 &vec) const
        {
            return
                data[0]*vec.data[0] + data[1]*vec.data[1] +
                data[2]*vec.data[2];
        };

        xbsVec3 cross(const xbsVec3 &b)
        {
            xbsVec3 result;
            result[0] = data[1]*b.data[2] - data[2]*b.data[1];
            result[1] = data[2]*b.data[0] - data[0]*b.data[2];
            result[2] = data[0]*b.data[1] - data[1]*b.data[0];
            return result;
        };

        xbsReal length()
        {
            return sqrt(this->dot(*this));
        };

        xbsReal SquaredLength()
        {
            return this->dot(*this);
        };

        xbsVec3 &normalize()
        {
            xbsReal len;
        
            len = this->length();
        
            if (len == 0.0)
                fprintf(stderr, "Warning: Cannot normalize 0 vector!\n");
            else
                *this *= (xbsReal)1.0/len;
        
            return *this;
        };
 
        xbsVec3 &clamp(xbsReal min=0.0, xbsReal max=1.0)
        {
            data[0] = (data[0] < min) ? min : ((data[0] > max) ? max : data[0]);
            data[1] = (data[1] < min) ? min : ((data[1] > max) ? max : data[1]);
            data[2] = (data[2] < min) ? min : ((data[2] > max) ? max : data[2]);
            return *this;
        };
    
        xbsReal data[3];
};


class xbsVec2
{
    public:
        xbsVec2(xbsReal x=0.0, xbsReal y=0.0)
        {
            data[0] = x;
            data[1] = y;
        };

        xbsVec2(const xbsVec2 &vec)
        {
            data[0] = vec.data[0];
            data[1] = vec.data[1];
        };
    
        xbsReal &operator[] (xbsDimension index)
        {
            return data[index];
        };
    
        xbsReal &operator[] (int index)
        {
            return data[index];
        };
    
        xbsVec2 &operator = (const xbsVec2 &vec)
        {
            data[0] = vec.data[0];
            data[1] = vec.data[1];
            return *this;
        };
    
        xbsVec2 &operator += (const xbsVec2 &vec)
        {
            data[0] += vec.data[0];
            data[1] += vec.data[1];
            return *this;
        };

        xbsVec2 &operator -= (const xbsVec2 &vec)
        {
            data[0] -= vec.data[0];
            data[1] -= vec.data[1];
            return *this;
        };
    
        xbsVec2 &operator *= (const xbsVec2 &vec)
        {
            data[0] *= vec.data[0];
            data[1] *= vec.data[1];
            return *this;
        };
    
        xbsVec2 &operator *= (xbsReal scale)
        {
            data[0] *= scale;
            data[1] *= scale;
            return *this;
        };
 
        xbsVec2 operator + (const xbsVec2 &vec) const
        {
            xbsVec2 result = *this;
            result += vec;
            return result;
        };
    
        xbsVec2 operator - (const xbsVec2 &vec) const
        {
            xbsVec2 result = *this;
            result -= vec;
            return result;
        };
 
        xbsVec2 operator * (const xbsVec2 &vec) const
        {
            xbsVec2 result = *this;
            result *= vec;
            return result;
        };
    
        xbsVec2 operator * (xbsReal scale) const
        {
            xbsVec2 result = *this;
            result *= scale;
            return result;
        };
    
        xbsReal operator ^ (const xbsVec2 &vec) const
        {
            xbsReal result = data[0]*vec.data[1] - data[1]*vec.data[0];
            return result;
        };
    
        void set(xbsReal x, xbsReal y)
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
 
        xbsReal dot(const xbsVec2 &vec) const
        {
            return data[0]*vec.data[0] + data[1]*vec.data[1];
        };
    
        xbsReal length()
        {
            return sqrt(this->dot(*this));
        };
    
        xbsVec2 &normalize()
        {
            xbsReal len;
        
            len = this->length();
        
            if (len == 0.0)
                fprintf(stderr, "Warning: Cannot normalize 0 vector!\n");
            else
                *this *= (xbsReal)1.0/len;
        
            return *this;
        };
 
        xbsVec2 &clamp(xbsReal min=0.0, xbsReal max=1.0)
        {
            data[0] = (data[0] < min) ? min : ((data[0] > max) ? max : data[0]);
            data[1] = (data[1] < min) ? min : ((data[1] > max) ? max : data[1]);
            return *this;
        };
    
        xbsReal data[2];
};

class xbsColor
{
    public:
        xbsColor(unsigned char r=0, unsigned char g=0, unsigned char b=0)
        {
            data[0] = r;
            data[1] = g;
            data[2] = b;
        };

        xbsColor(const xbsColor &col)
        {
            data[0] = col.data[0];
            data[1] = col.data[1];
            data[2] = col.data[2];
        };
    
        unsigned char &operator[] (int index)
        {
            return data[index];
        };
    
        xbsColor &operator = (const xbsColor &col)
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
 
        xbsColor &operator *= (xbsReal scale)
        {
            data[0] = (unsigned char)(data[0] * scale);
            data[1] = (unsigned char)(data[1] * scale);
            data[2] = (unsigned char)(data[2] * scale);
            clamp();
            return *this;
        };

        xbsColor operator * (xbsReal scale) const
        {
            xbsColor result = *this;
            result *= scale;
            return result;
        };
    
        xbsColor &operator += (const xbsColor &col)
        {
            data[0] += col.data[0];
            data[1] += col.data[1];
            data[2] += col.data[2];
            return *this;
        };

        xbsColor operator + (const xbsColor &col) const
        {
            xbsColor result = *this;
            result += col;
            return result;
        };
    
        void print()
        {
            fprintf(stderr, "(%d, %d, %d)", 
                    (int)(data[0]), (int)(data[1]), (int)(data[2]));
            return;
        };
 
        xbsColor &clamp(unsigned char min=0, unsigned char max=255)
        {
            data[0] = (data[0] < min) ? min : ((data[0] > max) ? max : data[0]);
            data[1] = (data[1] < min) ? min : ((data[1] > max) ? max : data[1]);
            data[2] = (data[2] < min) ? min : ((data[2] > max) ? max : data[2]);
            return *this;
        };
    
        unsigned char data[3];
};


struct plyVertex;

class xbsTriangle;
class Operation;


class xbsVertex;

class xbsTriangle
{
    public:
        xbsVertex *verts[3];
        int patchNum;

        int index; // index within some Model -- this field is
                   // manipulated by the model class itself, and may
                   // be changed when other triangles are added to or
                   // removed from the model
    
        // Hierarchy data - these fields are manipulated by the output
        // hierarchy
        int mtIndex;
        int endNodeIndex;

    
        xbsTriangle()
        {
            verts[0] = verts[1] = verts[2] = NULL;
            patchNum = 0;
            index = -1;
            mtIndex = endNodeIndex = -1;
        };
        xbsTriangle(xbsVertex *v0, xbsVertex *v1, xbsVertex *v2, int patch=0)
        {
            xbsTriangle();
            verts[0] = v0;
            verts[1] = v1;
            verts[2] = v2;
            patchNum = patch;
        };
        void set(xbsVertex *v0, xbsVertex *v1, xbsVertex *v2, int patch=0)
        {
            verts[0] = v0;
            verts[1] = v1;
            verts[2] = v2;
            patchNum = patch;
        };
        void print();
        void draw();
};



// The xbsVertex describes a mesh vertex that is part of a Model. It
// must have 3D geometric coordinates, and may have other attributes
// as well. After a Model has been "shared", vertices with the same
// geometric coordinates have been identified. If all attribute values
// are the same, these vertices are stored as the same xbsVertex (even
// if their triangles lie on different "patches"). If the geometric
// coordinates are the same, but the attribute values are different,
// they are stored as multiple xbsVertices, but linked together in a
// circularly linked list, formed using the nextCoincident
// field. There are some important policy issues regarding this
// coincident vertex ring. One of the vertices of the coincident ring
// known as the minCoincident, is often used as a designated
// representative for the entire ring. Currently, simplification
// operations always refer to the minCoincident vertex and are stored
// in the operation list of the minCoincident vertex. Error values are
// currently stored only on the minCoincident vertex as well. If the
// minCoincident for a coincident vertex ring changes, we must
// transfer all these responsibilities from the old minCoincident to
// the new minCoincident. Triangle pointers are stored on their
// correct vertices, not with the minCoincident.  Normally, we would
// expect every vertex in the model to have one or more triangles
// attached to it, but VDS currently forces us to keep some of these
// empty vertices around until they may be properly merged and
// deleted.
class xbsVertex
{
    public:
        xbsVec3 coord;

        // indexing data
        xbsVertex *nextCoincident;
        xbsTriangle **tris;
        int numTris;

        // index of xbsVertex in a Model, which is free to manipulate
        // the index as vertices are added or removed from the Model.
        int index; 
    
        // SimpQueue data
        Operation **ops;
        int numOps;

        // error data
        //float error;
        GLOD_ErrorData *errorData;

        // hierarchy data to be manipulated by the output hierarchy
        int mtIndex;
    
        void init()
        {
            coord.set(0,0,0);
            nextCoincident = this;
            tris = NULL;
            numTris = 0;
            ops = NULL;
            numOps = 0;
            errorData = NULL;
            mtIndex = -1;
            index = -1;
        };

        xbsVertex() { init(); };
        xbsVertex(const xbsVertex &vert) {init(); set(vert.coord);};
        xbsVertex(xbsVec3 crd) {init(); coord=crd;};

        virtual ~xbsVertex();
    
        xbsVertex &operator = (const xbsVertex &v)
        {
            coord = v.coord;
            return *this;
        };
    
        void set(xbsVec3 crd) {coord=crd;};

        // routines for handling list of adjacent tris
        void reallocTris(int count)
        {
            if (count < numTris)
            {
                fprintf(stderr, "Can't downsize vdata tris.\n");
                exit(1);
            }
            xbsTriangle **newTris = new xbsTriangle *[count];
            for (int i=0; i<numTris; i++)
                newTris[i] = tris[i];
            delete tris;
            tris = newTris;
        }
        void freeTris()
        {
            delete [] tris;
            tris = NULL;
            numTris = 0;
        }
        void addTri(xbsTriangle *tri)
        {
            // assumes space already allocated
            tris[numTris++] = tri;
            return;
        }
        int removeTri(xbsTriangle *tri)
        {
            int current = 0;
            for (int i=0; i<numTris; i++)
                if (tris[i] != tri)
                    tris[current++] = tris[i];
            int numRemoved = numTris - current;
            numTris = current;
            return numRemoved;
        }

    
        // routines for handling ring of coincident vertices (circularly linked
        // list)

        // Find the representative vertex by picking the one with the
        // smallest address
        inline xbsVertex *minCoincident()
        {
            xbsVertex *min = this;
            for (xbsVertex *current=nextCoincident; current != this;
                 current = current->nextCoincident)
                if (current < min)
                    min = current;
            return min;
        }

        // Find the vertex that would become the new representative if
        // this one were removed
        inline xbsVertex *minCoincidentExceptThis()
        {
            xbsVertex *min = NULL;
            for (xbsVertex *current=nextCoincident; current != this;
                 current = current->nextCoincident)
                if ((min == NULL) || (current < min))
                    min = current;
            return min;
        }
    
        // Count number of vertices that still have triangles attached
        inline xbsVertex *minNonemptyCoincident()
        {
            xbsVertex *min = NULL;
            xbsVertex *current = this;
            do
            {
                if (current->numTris > 0)
                {
                    if ((min == NULL) || (current < min))
                        min = current;
                }
                current = current->nextCoincident;
            } while (current != this);
            return min;
        }

        // The index of a coincident vertex is the number of links
        // that must be followed to reach this vertex when starting
        // from the representative
        inline int coincidentIndex()
        {
            int index = 0;
            for (xbsVertex *current = minCoincident();
                 current != this; index++, current=current->nextCoincident);
            return index;
        }

        // Return a vertex given its index around the coincident ring
        // (this could be dangerously slow if it encourages code to
        // write a loop over index number and fetch each vertex using
        // its index)
        inline xbsVertex *coincidentVert(int index)
        {
            xbsVertex *vert = minCoincident();
            for (int i=0; i<index; i++)
                vert = vert->nextCoincident;
            return vert;
        }
        // Count the number of vertices on the coincident ring.
        inline int numCoincident()
        {
            int num=1;
            for (xbsVertex *current = this->nextCoincident;
                 current != this;
                 num++, current=current->nextCoincident);
            return num;
        }
        // Count the number of vertices on the coincident ring,
        // excluding vertices with no triangles        
        inline int numNonEmptyCoincident()
        {
            int num=0;
            xbsVertex *current = this;
            do
            {
                if (current->numTris > 0)
                    num++;
                current = current->nextCoincident;
            } while (current != this);

#if 0
            // This shouldn't happen for discrete hierarchy output mode
            if (num != numCoincident())
                fprintf(stderr, "Empty vert found.\n");
#endif
        
            return num;
        }
        // Count number of triangles attached to all vertices of a
        // coincident ring
        inline int coincidentNumTris()
        {
            int ntris = 0;
            xbsVertex *current = this;
            do 
            {
                ntris += current->numTris;
                current = current->nextCoincident;
            } while (current != this);

            return ntris;
        }

        // Check to see if two vertices are adjacent (i.e. are
        // connected by some triangle edge). This test actually checks
        // to see if any coincident vertex of one vertex is adjacent
        // to any coincident vertex of the other.
        inline int coincidentIsAdjacent(xbsVertex *v2)
        {
            xbsVertex *current = this;
            xbsVertex *v2min = v2->minCoincident();
            do
            {
                for (int i=0; i<current->numTris; i++)
                {
                    xbsTriangle *tri = current->tris[i];
                    if ((tri->verts[0]->minCoincident() == v2min) ||
                        (tri->verts[1]->minCoincident() == v2min) ||
                        (tri->verts[2]->minCoincident() == v2min))
                        return 1;
                }
                current = current->nextCoincident;
            } while (current != this);

            return 0;
        }
    

        bool equals(xbsVertex* other, int* others_offset) {
            int offset = 0;
            xbsVertex* cur = this;
            do {
                if(cur == other) {
                    *others_offset = offset;
                    return true;
                }
                offset ++; cur = cur->nextCoincident;
            } while(cur != this);
            return false;
        }

        int onBorder(); // is this vertex on a geometric border?
    
        virtual void print()
        {
            coord.print();
        };
        virtual void draw();

        // size and allocation functions
        virtual int size() const { return sizeof(xbsVertex); };
        virtual xbsVertex *makeNew() const { return new xbsVertex; };
        virtual xbsVertex *makeNew(int num) const { return new xbsVertex[num]; };
        // notice this is just a byte copy
        virtual void copySame(xbsVertex *destVert) const 
        { 
            *((xbsVertex *)(destVert)) = *this;
        };

        // attribute manipulation functions
        virtual void hasAttributes(char& hasColor, char& hasNormal,
                                   char& hasTexcoord)
        {
            hasColor = 0;
            hasNormal = 0;
            hasTexcoord = 0;
            return;
        }
        virtual int attribsEqual(xbsVertex *vert)
        {
            vert = vert;    
            return 1;
        }    
        virtual void interp(xbsVertex *vert1, xbsVertex *vert2, float t)
        {
            xbsVertex *v1 = (xbsVertex *)vert1;
            xbsVertex *v2 = (xbsVertex *)vert2;

            coord    = v1->coord    * (1.0-t)   +   v2->coord    * t;
        }

        // conversion functions to other data types
        virtual mtVertex *makeMTVertex();
        virtual void fillData(xbsVec3& crd, xbsColor& clr,
                              xbsVec3& nrm, xbsVec2& tcrd)
        {
            crd = coord;
            return;
        };
        virtual void fillVDSData(Point3& crd, ByteColorA& clr,
                                 Vec3& nrm, Point2& tcrd)
        {
            crd.Set(coord[0], coord[1], coord[2]);
            return;
        }    
};

// The 7 child classes of xbsVertex provide all combinations of color,
// normal, and texture coordinates. This class hierarchy should go
// away when we convert to using the AttribSet mechanism.

class xbsNVertex : public xbsVertex 
{
    public:
        xbsVec3 normal;

        xbsNVertex() : xbsVertex() {coord.set(0,0,0); normal.set(1,0,0);};
        xbsNVertex(const xbsNVertex &vert) : xbsVertex() {set(vert.coord, vert.normal);};
        xbsNVertex(xbsVec3 crd, xbsVec3 nrm) : xbsVertex()
        {
            coord = crd;
            normal = nrm;
        };
        xbsNVertex &operator = (const xbsNVertex &v)
        {
            coord = v.coord;
            normal = v.normal;
            return *this;
        }
#if 1
        xbsNVertex &operator = (const xbsVertex &v)
        {
            coord = v.coord;
            //normal.set(1,0,0);
            return *this;
        }
#endif
        void set(xbsVec3 crd, xbsVec3 nrm)
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
        virtual int size() const { return sizeof(xbsNVertex); };
        virtual xbsVertex *makeNew() const { return new xbsNVertex; };
        virtual xbsVertex *makeNew(int num) const { return new xbsNVertex[num]; };
        virtual void copySame(xbsVertex *destVert) const 
        { 
            *((xbsNVertex *)(destVert)) = *this;
        };
        virtual mtVertex *makeMTVertex();
        virtual int attribsEqual(xbsVertex *vert)
        {
            xbsNVertex *v = (xbsNVertex *)vert;
            if ((normal[0] != v->normal[0]) ||
                (normal[1] != v->normal[1]) ||
                (normal[2] != v->normal[2]))
                return 0;
            return 1;
        }    
        virtual void interp(xbsVertex *vert1, xbsVertex *vert2, float t)
        {
            xbsNVertex *v1 = (xbsNVertex *)vert1;
            xbsNVertex *v2 = (xbsNVertex *)vert2;

            coord    = v1->coord    * (1.0-t)   +   v2->coord    * t;
            normal   = v1->normal   * (1.0-t)   +   v2->normal   * t;
            normal.normalize();
        }
        virtual void hasAttributes(char& hasColor, char& hasNormal,
                                   char& hasTexcoord)
        {
            hasColor = 0;
            hasNormal = 1;
            hasTexcoord = 0;
            return;
        }
        virtual void fillData(xbsVec3& crd, xbsColor& clr,
                              xbsVec3& nrm, xbsVec2& tcrd)
        {
            crd = coord;
            nrm = normal;
            return;
        };
        virtual void fillVDSData(Point3& crd, ByteColorA& clr,
                                 Vec3& nrm, Point2& tcrd)
        {
            crd.Set(coord[0], coord[1], coord[2]);
            nrm.Set(normal[0], normal[1], normal[2]);
            return;
        }    
};

class xbsTVertex : public xbsVertex
{
    public:
        xbsVec2 texcoord;

        xbsTVertex() : xbsVertex() {};
        xbsTVertex(const xbsTVertex &vert) : xbsVertex() {set(vert.coord, vert.texcoord);};
        xbsTVertex(xbsVec3 crd, xbsVec2 txcrd) : xbsVertex()
        {
            coord = crd;
            texcoord = txcrd;
        };
        xbsTVertex &operator = (const xbsTVertex &v)
        {
            coord = v.coord;
            texcoord = v.texcoord;
            return *this;
        };
        void set(xbsVec3 crd, xbsVec2 txcrd)
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
        virtual int size() const { return sizeof(xbsTVertex); };
        virtual xbsVertex *makeNew() const { return new xbsTVertex; };
        virtual xbsVertex *makeNew(int num) const { return new xbsTVertex[num]; };
        virtual void copySame(xbsVertex *destVert) const 
        { 
            *((xbsTVertex *)(destVert)) = *this;
        };
        virtual mtVertex *makeMTVertex();
        virtual int attribsEqual(xbsVertex *vert)
        {
            xbsTVertex *v = (xbsTVertex *)vert;
            if ((texcoord[0] != v->texcoord[0]) ||
                (texcoord[1] != v->texcoord[1]))
                return 0;
            return 1;
        }    
        virtual void interp(xbsVertex *vert1, xbsVertex *vert2, float t)
        {
            xbsTVertex *v1 = (xbsTVertex *)vert1;
            xbsTVertex *v2 = (xbsTVertex *)vert2;

            coord    = v1->coord    * (1.0-t)   +   v2->coord    * t;
            texcoord = v1->texcoord * (1.0-t)   +   v2->texcoord * t;
        }
        virtual void hasAttributes(char& hasColor, char& hasNormal,
                                   char& hasTexcoord)
        {
            hasColor = 0;
            hasNormal = 0;
            hasTexcoord = 1;
            return;
        }
        virtual void fillData(xbsVec3& crd, xbsColor& clr,
                              xbsVec3& nrm, xbsVec2& tcrd)
        {
            crd = coord;
            tcrd = texcoord;
            return;
        };
        virtual void fillVDSData(Point3& crd, ByteColorA& clr,
                                 Vec3& nrm, Point2& tcrd)
        {
            crd.Set(coord[0], coord[1], coord[2]);
            tcrd.Set(texcoord[0], texcoord[1]);
            return;
        }    
};

class xbsCVertex : public xbsVertex
{
    public:
        xbsColor color;

        xbsCVertex() : xbsVertex() {};
        xbsCVertex(const xbsCVertex &vert) : xbsVertex() {set(vert.coord, vert.color);};
        xbsCVertex(xbsVec3 crd, xbsColor clr) : xbsVertex()
        {
            coord = crd;
            color = clr;
        };
        xbsCVertex &operator = (const xbsCVertex &v)
        {
            coord = v.coord;
            color = v.color;
            return *this;
        };
        void set(xbsVec3 crd, xbsColor clr)
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
        virtual int size() const { return sizeof(xbsCVertex); };
        virtual xbsVertex *makeNew() const { return new xbsCVertex; };
        virtual xbsVertex *makeNew(int num) const { return new xbsCVertex[num]; };
        virtual void copySame(xbsVertex *destVert) const 
        { 
            *((xbsCVertex *)(destVert)) = *this;
        };
        virtual mtVertex *makeMTVertex();
        virtual int attribsEqual(xbsVertex *vert)
        {
            xbsCVertex *v = (xbsCVertex *)vert;
            if ((color[0] != v->color[0]) ||
                (color[1] != v->color[1]) ||
                (color[2] != v->color[2]))
                return 0;
            return 1;
        }    
        virtual void interp(xbsVertex *vert1, xbsVertex *vert2, float t)
        {
            xbsCVertex *v1 = (xbsCVertex *)vert1;
            xbsCVertex *v2 = (xbsCVertex *)vert2;

            coord    = v1->coord    * (1.0-t)   +   v2->coord    * t;
            color    = v1->color    * (1.0-t)   +   v2->color    * t;
        }
        virtual void hasAttributes(char& hasColor, char& hasNormal,
                                   char& hasTexcoord)
        {
            hasColor = 1;
            hasNormal = 0;
            hasTexcoord = 0;
            return;
        }
        virtual void fillData(xbsVec3& crd, xbsColor& clr,
                              xbsVec3& nrm, xbsVec2& tcrd)
        {
            crd = coord;
            clr = color;
            return;
        };
        virtual void fillVDSData(Point3& crd, ByteColorA& clr,
                                 Vec3& nrm, Point2& tcrd)
        {
            crd.Set(coord[0], coord[1], coord[2]);
            clr.R = color[0];
            clr.G = color[1];
            clr.B = color[2];
            clr.A = 255;
            return;
        }    
};


class xbsCNVertex : public xbsVertex
{
    public:
        xbsColor color;
        xbsVec3  normal;
    
        xbsCNVertex() : xbsVertex() {};
        xbsCNVertex(const xbsCNVertex &vert) : xbsVertex() {set(vert.coord, vert.color,
                                                                vert.normal);};
        xbsCNVertex(xbsVec3 crd, xbsColor clr, xbsVec3 nrm) : xbsVertex()
        {
            coord = crd;
            color = clr;
            normal = nrm;
        };
        xbsCNVertex &operator = (const xbsCNVertex &v)
        {
            coord = v.coord;
            color = v.color;
            normal = v.normal;
            return *this;
        };
        void set(xbsVec3 crd, xbsColor clr, xbsVec3 nrm)
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
        virtual int size() const { return sizeof(xbsCNVertex); };
        virtual xbsVertex *makeNew() const { return new xbsCNVertex; };
        virtual xbsVertex *makeNew(int num) const { return new xbsCNVertex[num]; };
        virtual void copySame(xbsVertex *destVert) const 
        { 
            *((xbsCNVertex *)(destVert)) = *this;
        };
        virtual mtVertex *makeMTVertex();
        virtual int attribsEqual(xbsVertex *vert)
        {
            xbsCNVertex *v = (xbsCNVertex *)vert;
            if ((color[0] != v->color[0]) ||
                (color[1] != v->color[1]) ||
                (color[2] != v->color[2]))
                return 0;
            if ((normal[0] != v->normal[0]) ||
                (normal[1] != v->normal[1]) ||
                (normal[2] != v->normal[2]))
                return 0;
            return 1;
        }    
        virtual void interp(xbsVertex *vert1, xbsVertex *vert2, float t)
        {
            xbsCNVertex *v1 = (xbsCNVertex *)vert1;
            xbsCNVertex *v2 = (xbsCNVertex *)vert2;

            coord    = v1->coord    * (1.0-t)   +   v2->coord    * t;
            color    = v1->color    * (1.0-t)   +   v2->color    * t;
            normal   = v1->normal   * (1.0-t)   +   v2->normal   * t;
            normal.normalize();
        }
        virtual void hasAttributes(char& hasColor, char& hasNormal,
                                   char& hasTexcoord)
        {
            hasColor = 1;
            hasNormal = 1;
            hasTexcoord = 0;
            return;
        }
        virtual void fillData(xbsVec3& crd, xbsColor& clr,
                              xbsVec3& nrm, xbsVec2& tcrd)
        {
            crd = coord;
            clr = color;
            nrm = normal;
            return;
        };
        virtual void fillVDSData(Point3& crd, ByteColorA& clr,
                                 Vec3& nrm, Point2& tcrd)
        {
            crd.Set(coord[0], coord[1], coord[2]);
            clr.R = color[0];
            clr.G = color[1];
            clr.B = color[2];
            clr.A = 255;
            nrm.Set(normal[0], normal[1], normal[2]);
            return;
        }    
};


class xbsCTVertex : public xbsVertex
{
    public:
        xbsColor color;
        xbsVec2  texcoord;
    
        xbsCTVertex() : xbsVertex() {};
        xbsCTVertex(const xbsCTVertex &vert) : xbsVertex() {set(vert.coord, vert.color,
                                                                vert.texcoord);};
        xbsCTVertex(xbsVec3 crd, xbsColor clr, xbsVec2 txcrd) : xbsVertex()
        {
            coord = crd;
            color = clr;
            texcoord = txcrd;
        };
        xbsCTVertex &operator = (const xbsCTVertex &v)
        {
            coord = v.coord;
            color = v.color;
            texcoord = v.texcoord;
            return *this;
        };
        void set(xbsVec3 crd, xbsColor clr, xbsVec2 txcrd)
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
        virtual int size() const { return sizeof(xbsCTVertex); };
        virtual xbsVertex *makeNew() const { return new xbsCTVertex; };
        virtual xbsVertex *makeNew(int num) const { return new xbsCTVertex[num]; };
        virtual void copySame(xbsVertex *destVert) const 
        { 
            *((xbsCTVertex *)(destVert)) = *this;
        };
        virtual mtVertex *makeMTVertex();
        virtual int attribsEqual(xbsVertex *vert)
        {
            xbsCTVertex *v = (xbsCTVertex *)vert;
            if ((color[0] != v->color[0]) ||
                (color[1] != v->color[1]) ||
                (color[2] != v->color[2]))
                return 0;
            if ((texcoord[0] != v->texcoord[0]) ||
                (texcoord[1] != v->texcoord[1]))
                return 0;
            return 1;
        }    
        virtual void interp(xbsVertex *vert1, xbsVertex *vert2, float t)
        {
            xbsCTVertex *v1 = (xbsCTVertex *)vert1;
            xbsCTVertex *v2 = (xbsCTVertex *)vert2;

            coord    = v1->coord    * (1.0-t)   +   v2->coord    * t;
            color    = v1->color    * (1.0-t)   +   v2->color    * t;
            texcoord = v1->texcoord * (1.0-t)   +   v2->texcoord * t;
        }
        virtual void hasAttributes(char& hasColor, char& hasNormal,
                                   char& hasTexcoord)
        {
            hasColor = 1;
            hasNormal = 0;
            hasTexcoord = 1;
            return;
        }
        virtual void fillData(xbsVec3& crd, xbsColor& clr,
                              xbsVec3& nrm, xbsVec2& tcrd)
        {
            crd = coord;
            clr = color;
            tcrd = texcoord;
            return;
        };
        virtual void fillVDSData(Point3& crd, ByteColorA& clr,
                                 Vec3& nrm, Point2& tcrd)
        {
            crd.Set(coord[0], coord[1], coord[2]);
            clr.R = color[0];
            clr.G = color[1];
            clr.B = color[2];
            clr.A = 255;
            tcrd.Set(texcoord[0], texcoord[1]);
            return;
        }    
};


class xbsNTVertex : public xbsVertex
{
    public:
        xbsVec3  normal;
        xbsVec2  texcoord;
    
        xbsNTVertex()  : xbsVertex(){};
        xbsNTVertex(const xbsNTVertex &vert)  : xbsVertex(){set(vert.coord, vert.normal,
                                                                vert.texcoord);};
        xbsNTVertex(xbsVec3 crd, xbsVec3 nrm, xbsVec2 txcrd) : xbsVertex()
        {
            coord = crd;
            normal = nrm;
            texcoord = txcrd;
        };
        xbsNTVertex &operator = (const xbsNTVertex &v)
        {
            coord = v.coord;
            normal = v.normal;
            texcoord = v.texcoord;
            return *this;
        };
        void set(xbsVec3 crd, xbsVec3 nrm, xbsVec2 txcrd)
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
        virtual int size() const { return sizeof(xbsNTVertex); };
        virtual xbsVertex *makeNew() const { return new xbsNTVertex; };
        virtual xbsVertex *makeNew(int num) const { return new xbsNTVertex[num]; };
        virtual void copySame(xbsVertex *destVert) const 
        { 
            *((xbsNTVertex *)(destVert)) = *this;
        };
        virtual mtVertex *makeMTVertex();
        virtual int attribsEqual(xbsVertex *vert)
        {
            xbsNTVertex *v = (xbsNTVertex *)vert;
            if ((normal[0] != v->normal[0]) ||
                (normal[1] != v->normal[1]) ||
                (normal[2] != v->normal[2]))
                return 0;
            if ((texcoord[0] != v->texcoord[0]) ||
                (texcoord[1] != v->texcoord[1]))
                return 0;
            return 1;
        }    
        virtual void interp(xbsVertex *vert1, xbsVertex *vert2, float t)
        {
            xbsNTVertex *v1 = (xbsNTVertex *)vert1;
            xbsNTVertex *v2 = (xbsNTVertex *)vert2;

            coord    = v1->coord    * (1.0-t)   +   v2->coord    * t;
            normal   = v1->normal   * (1.0-t)   +   v2->normal   * t;
            texcoord = v1->texcoord * (1.0-t)   +   v2->texcoord * t;
            normal.normalize();
        }
        virtual void hasAttributes(char& hasColor, char& hasNormal,
                                   char& hasTexcoord)
        {
            hasColor = 0;
            hasNormal = 1;
            hasTexcoord = 1;
            return;
        }
        virtual void fillData(xbsVec3& crd, xbsColor& clr,
                              xbsVec3& nrm, xbsVec2& tcrd)
        {
            crd = coord;
            nrm = normal;
            tcrd = texcoord;
            return;
        };
        virtual void fillVDSData(Point3& crd, ByteColorA& clr,
                                 Vec3& nrm, Point2& tcrd)
        {
            crd.Set(coord[0], coord[1], coord[2]);
            nrm.Set(normal[0], normal[1], normal[2]);
            tcrd.Set(texcoord[0], texcoord[1]);
            return;
        }    
};


class xbsCNTVertex : public xbsVertex
{
    public:
        xbsColor color;
        xbsVec3  normal;
        xbsVec2  texcoord;
    
        xbsCNTVertex()  : xbsVertex(){};
        xbsCNTVertex(const xbsCNTVertex &vert) : xbsVertex() {set(vert.coord, vert.color,
                                                                  vert.normal, vert.texcoord);};
        xbsCNTVertex(xbsVec3 crd, xbsColor clr, xbsVec3 nrm, xbsVec2 txcrd) : xbsVertex()
        {
            coord = crd;
            color = clr;
            normal = nrm;
            texcoord = txcrd;
        };
        xbsCNTVertex &operator = (const xbsCNTVertex &v)
        {
            coord = v.coord;
            color = v.color;
            normal = v.normal;
            texcoord = v.texcoord;
            return *this;
        };
        void set(xbsVec3 crd, xbsColor clr, xbsVec3 nrm, xbsVec2 txcrd)
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
        virtual int size() const { return sizeof(xbsCNTVertex); };
        virtual xbsVertex *makeNew() const { return new xbsCNTVertex; };
        virtual xbsVertex *makeNew(int num) const { return new xbsCNTVertex[num]; };
        virtual void copySame(xbsVertex *destVert) const 
        { 
            *((xbsCNTVertex *)(destVert)) = *this;
        };
        virtual mtVertex *makeMTVertex();
        virtual int attribsEqual(xbsVertex *vert)
        {
            xbsCNTVertex *v = (xbsCNTVertex *)vert;
            if ((color[0] != v->color[0]) ||
                (color[1] != v->color[1]) ||
                (color[2] != v->color[2]))
                return 0;
            if ((normal[0] != v->normal[0]) ||
                (normal[1] != v->normal[1]) ||
                (normal[2] != v->normal[2]))
                return 0;
            if ((texcoord[0] != v->texcoord[0]) ||
                (texcoord[1] != v->texcoord[1]))
                return 0;
            return 1;
        }
        virtual void interp(xbsVertex *vert1, xbsVertex *vert2, float t)
        {
            xbsCNTVertex *v1 = (xbsCNTVertex *)vert1;
            xbsCNTVertex *v2 = (xbsCNTVertex *)vert2;

            coord    = v1->coord    * (1.0-t)   +   v2->coord    * t;
            color    = v1->color    * (1.0-t)   +   v2->color    * t;
            normal   = v1->normal   * (1.0-t)   +   v2->normal   * t;
            texcoord = v1->texcoord * (1.0-t)   +   v2->texcoord * t;
            normal.normalize();
        }
        virtual void hasAttributes(char& hasColor, char& hasNormal,
                                   char& hasTexcoord)
        {
            hasColor = 1;
            hasNormal = 1;
            hasTexcoord = 1;
            return;
        }
        virtual void fillData(xbsVec3& crd, xbsColor& clr,
                              xbsVec3& nrm, xbsVec2& tcrd)
        {
            crd = coord;
            clr = color;
            nrm = normal;
            tcrd = texcoord;
            return;
        };
        virtual void fillVDSData(Point3& crd, ByteColorA& clr,
                                 Vec3& nrm, Point2& tcrd)
        {
            crd.Set(coord[0], coord[1], coord[2]);
            clr.R = color[0];
            clr.G = color[1];
            clr.B = color[2];
            clr.A = 255;
            nrm.Set(normal[0], normal[1], normal[2]);
            tcrd.Set(texcoord[0], texcoord[1]);
            return;
        }    
};


class PermissionGrid;

// The Model class stores a triangle mesh, mostly for the purpose of
// building a simplification hierarchy. It has arrays of xbsVertex and
// xbsTriangles, and these elements may be added and removed using
// special functions. The class provides vertex sharing, creation of
// triangle pointers on the vertices ("indexing"), ply
// reading/writing, and creation from a RawObject and a
// DiscreteLevel.
class Model
{
    private:
        xbsVertex **verts;
        int numVerts;
        int maxVerts;
        xbsTriangle **tris;
        int numTris;
        int maxTris;
        int numPatches;    
        PlyOtherElems *other_elems;
        char indexed;

    
        // private methods related to vertex sharing
        void share_vertices();
        void matchAttributes();
    
        void init()
        {
            numTris = numVerts = 0;
            maxTris = maxVerts = 0;
            tris = NULL;
            verts = NULL;
            numPatches = 1;
            indexed = 0;
            other_elems = NULL;
            borderLock = 0;
            snapMode = PercentReduction;
            reductionPercent = 0.5;
            numSnapshotSpecs = 0;
            snapshotTriSpecs = NULL;
            numSnapshotErrorSpecs = 0;
            snapshotErrorSpecs = NULL;
            errorMetric = GLOD_METRIC_SPHERES;
            permissionGrid = NULL;
            pgPrecision = 2.0;
        };

    public:

        // just a convenient place to store some parameters for
        // performing simplification on this Model.
        int borderLock;
        SnapshotMode snapMode;
        float reductionPercent;
        int numSnapshotSpecs;
        unsigned int *snapshotTriSpecs;
        int numSnapshotErrorSpecs;
        GLfloat *snapshotErrorSpecs;
        int errorMetric;
        PermissionGrid * permissionGrid;
        float pgPrecision;

        Model() { init(); };
        Model(DiscreteLevel *obj);
        Model(GLOD_RawObject* obj);
        ~Model(); /* moved to Model.C */

        void indexVertTris();
        void removeEmptyVerts();
        void setOtherElements(PlyOtherElems *other_elements)
        { other_elems = other_elements; };
        int addVert(xbsVertex *vert);
        void removeVert(xbsVertex *vert);
        int addTri(xbsTriangle *tri);
        void removeTri(xbsTriangle *tri);
        int getNumVerts() const {return numVerts;};
        int getNumTris() const {return numTris;};
        int getNumPatches() const { return numPatches; };
        void setNumPatches(int p) { numPatches = p; }
        xbsTriangle *getTri(int tri) { return tris[tri]; };
        xbsVertex *getVert(int vert) { return verts[vert]; };
        void printStats();
        void share(float coord_tolerance = 0.0);
        void splitPatchVerts();
        void hasAttributes(char& hasColor, char& hasNormal,
                           char& hasTexcoord)
        {
            if (numVerts > 0)
                verts[0]->hasAttributes(hasColor, hasNormal, hasTexcoord);
            else
                hasColor = hasNormal = hasTexcoord = 0;
            return;
        }
        void verify();
        void testVertOps();
        void initPermissionGrid();
};


/* Protection from multiple includes. */
#endif // INCLUDED_MODEL_H
