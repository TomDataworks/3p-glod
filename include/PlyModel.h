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
#ifndef PLYMODEL_H
#define PLYMODEL_H

#include <assert.h>
#include "ply.h"

#define DEFAULT_TEXTURE_NAME "../data/Checkerboard.ppm"
#define DEFAULT_NORMALMAP_NAME "base"      // with "-%d.ppm" for complete name


typedef struct Point{
    float x;
    float y;
    float z;
    float &operator[](int index){
        switch (index){
        case 0:
            return x;
            break;
        case 1:
            return y;
            break;
        case 2:
            return z;
            break;
        }
        assert(false);
        return x;
    };
}Point;
typedef Point Vector;
typedef struct Point2D{
    float u;
    float v;
    float &operator[](int index){
        switch (index){
        case 0:
            return u;
            break;
        case 1:
            return v;
            break;
        }
        assert(false);
        return u;
    };
}Point2D;

typedef struct Color{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
    unsigned char &operator[](int index){
        switch (index){
        case 0:
            return r;
            break;
        case 1:
            return g;
            break;
        case 2:
            return b;
            break;
        case 3:
            return a;
            break;
        }
        assert(false);
        return r;
    };
}Color;

typedef float FloatColor[3];  // glMaterial has no ubyte call...

typedef struct Vertex {
    Point   coord;
    Vector  normal;
    Point2D texcoord;
    void *other_props;       /* other properties */
    Color color;
    float radius;  
} Vertex;

// optimize vertex memory (multiple of 4 bytes and no unnecessary
// memory).  
typedef struct VertexArray {
    float coord[3]; //, coord_Y, coord_Z;
    float normal[3]; //, normal_Y, normal_Z;
    float texcoord[2]; //, texcoord_V; // we have one extra byte here to fill 128 bytes
    float color[3]; //_R, color_G, color_B;
} VertexArray;

#define VERTEX_ARRAY_ELEMENTS 1
#define VERTEX_ARRAY_ARRAYS 2

#ifndef MAXFLOAT
#define MAXFLOAT              FLT_MAX
#endif

#define FALSE 0
#define TRUE  1

#define X 0
#define Y 1
#define Z 2

#define U 0
#define V 1

// texture modes
#define TEXTURE_OFF       0
#define TEXTURE_ON        1

#define NUM_TEXTURE_MODES 2

#define SPHERE_DLIST_ID   1
#define SPHERE_RESOLUTION 6

typedef struct Face {
    unsigned char nverts;    /* number of vertex indices in list */
    int *verts;              /* vertex index list */
    void *other_props;       /* other properties */
    Vector normal;
    int patch_num;
    Color color;
} Face;

static PlyProperty vert_props[] = { /* list of property information for a vertex */
    {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord.x), 0, 0, 0, 0},
    {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord.y), 0, 0, 0, 0},
    {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord.z), 0, 0, 0, 0},
    {"nx", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,normal.x), 0, 0, 0, 0},
    {"ny", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,normal.y), 0, 0, 0, 0},
    {"nz", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,normal.z), 0, 0, 0, 0},      
    {"u", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,texcoord.u), 0, 0, 0, 0},      
    {"v", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,texcoord.v), 0, 0, 0, 0},      
#if 1
    {"confidence", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, color.r), 0, 0, 0, 0},
    {"red", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex, color.r), 0, 0, 0, 0},
    {"green", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex, color.g), 0, 0, 0, 0},
    {"blue", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex, color.b), 0, 0, 0, 0},
#endif
};

static PlyProperty face_props[] = { /* list of property information for a vertex */
    {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
        1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
    {"patch_num", PLY_INT, PLY_INT, offsetof(Face,patch_num), 0, 0, 0, 0},
#if 0
    {"red", PLY_UCHAR, PLY_UCHAR, offsetof(Face,red), 0, 0, 0, 0},
    {"green", PLY_UCHAR, PLY_UCHAR, offsetof(Face,green), 0, 0, 0, 0},
    {"blue", PLY_UCHAR, PLY_UCHAR, offsetof(Face,blue), 0, 0, 0, 0},
#endif
};

#define MAX_FACE_PROPS (sizeof(face_props) / sizeof(PlyProperty))
#define MAX_VERT_PROPS (sizeof(vert_props) / sizeof(PlyProperty))

typedef struct Patch
{
    int texture_id;
    Color color;
    
    unsigned int *indices;            // vertex indices in each patch to be sent to vertex array
    int nindices;
    VertexArray *varray; // used when doing VERTEX_ARRAY_ARRAYS
} Patch;

typedef struct PlyModel 
{
    float     max[3];
    float     min[3];
    Vertex   *vlist;
    int       nverts;
    Face     *flist;
    int       nfaces;
    Patch    *plist;
    int       npatches;
    
    PlyOtherElems *other_elements;
    PlyOtherProp  *vert_other,*face_other;
    int       nelems;
    char    **elist;
    int       num_comments;
    char    **comments;
    int       num_obj_info;
    char    **obj_info;
    
    Color     solid_color;
    int       file_type;
    
    int       has_vertex;
    int       vert_has[MAX_VERT_PROPS];
    int       has_face;
    int       face_has[MAX_FACE_PROPS];
    
    int       has_vertex_colors;
    int       has_vertex_normals;
    int       has_texcoords;
    int       has_texture;
    int       has_patches;
    int       has_normalmap;
    //int       has_diffuse;
    int       texture_id;
    char     *texture_name;
    
    VertexArray  *varray;
    int           vamode;
} PlyModel;

void MakePatches(PlyModel *model);
void SetupNormalMap(PlyModel *model);
void SetupTexture(PlyModel *model,char* filename);

void ComputeFaceNormals(PlyModel *model);
void ComputeVertexNormals(PlyModel *model, int inv);
void ComputeVertexRadii(PlyModel *model);
float CenterOnOrigin(PlyModel* model);
void InvertVertexNormals(PlyModel* model);

void SetupVertexArray(PlyModel *model, int mode);
void BindVertexArray(PlyModel* model, int patch);
void DrawModelVA(PlyModel *model, int patch); // does a drawelements or drawarrays

void DrawModelImmediate(PlyModel *model);
void DrawSphere(Point center, double radius);

void read_plyfile(char *filename, PlyModel *model);

void DeleteModel(PlyModel* model);

#endif
