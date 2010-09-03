/* GLOD: Vertex sharer and interface to XBS
***************************************************************************
* $Id: Model.C,v 1.36 2004/12/13 22:21:58 gfx_friends Exp $
**************************************************************************/
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


/*----------------------------- Local Includes -----------------------------*/

#include <stdio.h>
#include <stddef.h>
#include <ply.h>

#ifndef _WIN32
#pragma implementation
#endif

#include <Model.h>
#include <xbs.h>
#include <Discrete.h>
#include <DiscretePatch.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifdef MLBPQ
#include <MLBPriorityQueue.h>
#else
#include <Heap.h>
#endif

#include "PermissionGrid.h"

#if 0
#define VERBOSE
#endif

/*----------------------------- Local Constants -----------------------------*/

#if defined(_WIN32) || defined(__APPLE__)
typedef unsigned int Boolean;
#undef min
#undef max
#else
enum Boolean {FALSE=0, TRUE=1};
#endif

/*------------------------------ Local Macros -------------------------------*/

#define for if(0); else for
#define OFFSET(structvar,field)                                 \
    (((char *)(&(structvar.field))) - ((char *)&structvar))

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/*------------------------------- Local Types -------------------------------*/

typedef struct plyVertex
{
        xbsVec3  coord;          /* coordinates of vertex */
        xbsVec3  normal;
        xbsVec2  texcoord;
        xbsColor color;
} plyVertex;

typedef struct plyFace
{
        unsigned char nverts;    /* number of vertex indices in list */
        int *verts;              /* vertex index list */
        int  patch_num;
} Face;

/*------------------------------ Local Globals ------------------------------*/

// PLY reading and writing stuff...
static
char *elem_names[] = { /* list of the kinds of elements in the user's object */
    "vertex", "face" 
};

static plyVertex dummyvert;

static
PlyProperty vert_props[] = { /* list of property information for a vertex */
    {"x", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,coord.data[0]), 0, 0, 0, 0},
    {"y", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,coord.data[1]), 0, 0, 0, 0},
    {"z", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,coord.data[2]), 0, 0, 0, 0},
    {"nx", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,normal.data[0]), 0, 0, 0, 0},
    {"ny", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,normal.data[1]), 0, 0, 0, 0},
    {"nz", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,normal.data[2]), 0, 0, 0, 0},
    {"u", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,texcoord.data[0]), 0, 0, 0, 0},
    {"v", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,texcoord.data[1]), 0, 0, 0, 0},
    {"red", PLY_UCHAR, PLY_UCHAR, OFFSET(dummyvert,color.data[0]), 0, 0, 0, 0},
    {"green", PLY_UCHAR, PLY_UCHAR, OFFSET(dummyvert,color.data[1]), 0, 0, 0, 0},
    {"blue", PLY_UCHAR, PLY_UCHAR, OFFSET(dummyvert,color.data[2]), 0, 0, 0, 0},  
};

static
PlyProperty face_props[] = { /* list of property information for a face */
    {"vertex_indices", PLY_INT, PLY_INT, offsetof(plyFace,verts),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(plyFace,nverts)},
    {"patch_num", PLY_INT, PLY_INT, offsetof(plyFace,patch_num), 0, 0, 0, 0},
};

/*------------------------ Local Function Prototypes ------------------------*/


/*---------------------------------Functions-------------------------------- */


// hack
int foo()
{
#ifdef MLBPQ
    MLBPriorityQueue *heap = NULL;
#else
    Heap *heap = NULL;
#endif
    heap->min();
    return 0;
}

/*****************************************************************************\
 @ compare_pointers
 -----------------------------------------------------------------------------
 description : A simple comparison function for use with qsort.
 input       : Addresses of two pointers.
 output      : Positive number if a>b, negative number if a<b, and 0
               if they are equal
 notes       :
\*****************************************************************************/
static int
compare_pointers (const void *a, const void *b)
{
    const void **ia = (const void **) a;
    const void **ib = (const void **) b;
    
    return (*ia > *ib) - (*ia < *ib);
} /** End of compare_pointers() **/


xbsVertex::~xbsVertex()
{
    if (tris != NULL)
    {
        delete tris;
        tris = NULL;
        numTris = 0;
    }
    if (ops != NULL)
    {
        // This seems a bit dangerous, because the vertex does
        // not actually allocate the operations, but go ahead
        // and delete any operations remaining on the
        // vertices. The caller always has the option to set
        // numOps to 0 before deleting...
        for (int i=0; i<numOps; i++)
        {
            delete ops[i];
            ops[i] = NULL;
        }
        delete ops;
        ops = NULL;
        numOps = 0;
    }
    if (errorData != NULL){
        delete errorData;
        errorData = NULL;
    }
};

/*****************************************************************************\
 @ xbsVertex::onBorder
 -----------------------------------------------------------------------------
 description : Decide if this xbsVertex is on a geometric border.
 input       : 
 output      : 
 notes       : A border vertex would normally mean a vertex which is
               homeomorphic to a half-disc. A vertex which is
               homeomorphic to a disc is manifold, and has a single,
               topologically connected ring of triangles around it. A
               border would be a ring with a single piece comprising
               one or more edge-adjacent triangles. It's not clear
               what we should call a border in the presence of totally
               non-manifold vertices. For the moment, we have a
               heuristic that says if the number of edges around a
               vertex is equal to the number of triangles, it is a
               non-border, otherwise it is a border. There are surely
               some cases where this will not give us what we
               want. This call is currently used for "border lock"
               mode, which prevents removal/modification of any vertex
               which says it is on the border.
\*****************************************************************************/
int xbsVertex::onBorder()
{
    // This is really just a heuristic. It's not clear exactly what we want
    // to call a border in the presence of all sorts of non-manifold vertex
    // neighborhoods.
    
    // if the number of triangles around a vertex is less than the number
    // of edges around a vertex, then it is on a geometric border

    //
    // count number of neighboring triangles
    //
    
    int numNeighborTris = coincidentNumTris();
    
    //
    // count number of neighboring edges
    //

    xbsVertex *min = minCoincident();

    xbsVertex **neighborVerts = new xbsVertex*[numNeighborTris*3];
    int numNeighborVerts = 0;
    xbsVertex *current = this;
    do
    {
        for (int i=0; i<current->numTris; i++)
            for (int j=0; j<3; j++)
            {
                xbsVertex *minJ = current->tris[i]->verts[j]->minCoincident();
                if (minJ != min)
                    neighborVerts[numNeighborVerts++] = minJ;
            }
        current = current->nextCoincident;
    } while (current != this);
    
    qsort(neighborVerts, numNeighborVerts, sizeof(xbsVertex *), compare_pointers);

    // compact neighbor list (removing redundancies)
    if (numNeighborVerts > 0)
    {
        int current=0;
        for (int i=1; i<numNeighborVerts; i++)
            if (neighborVerts[i] != neighborVerts[current])
                neighborVerts[++current] = neighborVerts[i];
        numNeighborVerts = current+1;
    }

    delete [] neighborVerts;

    if (numNeighborVerts > numNeighborTris)
        return 1;

    else if (numNeighborVerts == numNeighborTris)
        return 0;

    else
    {
        //fprintf(stderr, "Fewer neighbor edges than triangles?!!\n");
        return 1;
    }
}

// Draw functions for immediate mode rendering of xbsVertex and its
// subclasses. Used to be used by the MT hierarchy. Still probably a
// handy enough thing to keep around, given that it's pretty simple
// stuff.
void xbsVertex::draw()
{
    glVertex3fv(this->coord.data);
}

void xbsCVertex::draw()
{
    glColor3ubv(this->color.data);
    glVertex3fv(this->coord.data);
}

void xbsNVertex::draw()
{
    glNormal3fv(this->normal.data);
    glVertex3fv(this->coord.data);
}

void xbsTVertex::draw()
{
    glTexCoord2fv(this->texcoord.data);
    glVertex3fv(this->coord.data);
}

void xbsCNVertex::draw()
{
    glColor3ubv(this->color.data);
    glNormal3fv(this->normal.data);
}

void xbsCTVertex::draw()
{
    glColor3ubv(this->color.data);
    glTexCoord2fv(this->texcoord.data);
}

void xbsNTVertex::draw()
{
    glNormal3fv(this->normal.data);
    glTexCoord2fv(this->texcoord.data);
}

void xbsCNTVertex::draw()
{
    glColor3ubv(this->color.data);
    glNormal3fv(this->normal.data);
    glTexCoord2fv(this->texcoord.data);
}

// Routines for setting up writing a bunch of vertices of some
// particular subclass of xbsVertex to a PLY file
void
xbsVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
}

void
xbsCVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[8]);
    ply_describe_property(ply, elem_name, &vert_props[9]);
    ply_describe_property(ply, elem_name, &vert_props[10]);
}

void
xbsNVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[3]);
    ply_describe_property(ply, elem_name, &vert_props[4]);
    ply_describe_property(ply, elem_name, &vert_props[5]);
}

void
xbsTVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[6]);
    ply_describe_property(ply, elem_name, &vert_props[7]);
}

void
xbsCNVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[8]);
    ply_describe_property(ply, elem_name, &vert_props[9]);
    ply_describe_property(ply, elem_name, &vert_props[10]);
    ply_describe_property(ply, elem_name, &vert_props[3]);
    ply_describe_property(ply, elem_name, &vert_props[4]);
    ply_describe_property(ply, elem_name, &vert_props[5]);
}

void
xbsCTVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[8]);
    ply_describe_property(ply, elem_name, &vert_props[9]);
    ply_describe_property(ply, elem_name, &vert_props[10]);
    ply_describe_property(ply, elem_name, &vert_props[6]);
    ply_describe_property(ply, elem_name, &vert_props[7]);
}

void
xbsNTVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[3]);
    ply_describe_property(ply, elem_name, &vert_props[4]);
    ply_describe_property(ply, elem_name, &vert_props[5]);
    ply_describe_property(ply, elem_name, &vert_props[6]);
    ply_describe_property(ply, elem_name, &vert_props[7]);
}

void
xbsCNTVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[8]);
    ply_describe_property(ply, elem_name, &vert_props[9]);
    ply_describe_property(ply, elem_name, &vert_props[10]);
    ply_describe_property(ply, elem_name, &vert_props[3]);
    ply_describe_property(ply, elem_name, &vert_props[4]);
    ply_describe_property(ply, elem_name, &vert_props[5]);
    ply_describe_property(ply, elem_name, &vert_props[6]);
    ply_describe_property(ply, elem_name, &vert_props[7]);
}

// Routines for filling in a plyVertex structure, which are used to
// actually write out the vertex data to a PLY file
void
xbsVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
}

void
xbsNVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->normal = normal;
}

void
xbsTVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->texcoord = texcoord;
}

void
xbsCVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->color = color;
}

void
xbsCNVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->color = color;
    pvert->normal = normal;
}

void
xbsCTVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->color = color;
    pvert->texcoord = texcoord;
}

void
xbsNTVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->normal = normal;
    pvert->texcoord = texcoord;
}

void
xbsCNTVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->color = color;
    pvert->normal = normal;
    pvert->texcoord = texcoord;
}

// Routines to convert from xbsVertex to mtVertex
mtVertex *
xbsVertex::makeMTVertex()
{
    mtVec3 crd(coord[0], coord[1], coord[2]);
    
    mtVertex *vert = new mtVertex(crd);
    return vert;
}

mtVertex *
xbsNVertex::makeMTVertex()
{
    mtVec3 crd(coord[0], coord[1], coord[2]);
    mtVec3 nrm(normal[0], normal[1], normal[2]);

    mtVertex *vert = new mtNVertex(crd, nrm);
    return vert;
}

mtVertex *
xbsTVertex::makeMTVertex()
{
    mtVec3 crd(coord[0], coord[1], coord[2]);
    mtVec2 tex(texcoord[0], texcoord[1]);

    mtVertex *vert = new mtTVertex(crd, tex);
    return vert;
}

mtVertex *
xbsCVertex::makeMTVertex()
{
    mtVec3 crd(coord[0], coord[1], coord[2]);
    mtColor col(color[0], color[1], color[2]);

    mtVertex *vert = new mtCVertex(crd, col);
    return vert;
}

mtVertex *
xbsCNVertex::makeMTVertex()
{
    mtVec3 crd(coord[0], coord[1], coord[2]);
    mtColor col(color[0], color[1], color[2]);
    mtVec3 nrm(normal[0], normal[1], normal[2]);

    mtVertex *vert = new mtCNVertex(crd, col, nrm);
    return vert;
}

mtVertex *
xbsCTVertex::makeMTVertex()
{
    mtVec3 crd(coord[0], coord[1], coord[2]);
    mtColor col(color[0], color[1], color[2]);
    mtVec2 tex(texcoord[0], texcoord[1]);

    mtVertex *vert = new mtCTVertex(crd, col, tex);
    return vert;
}

mtVertex *
xbsNTVertex::makeMTVertex()
{
    mtVec3 crd(coord[0], coord[1], coord[2]);
    mtVec3 nrm(normal[0], normal[1], normal[2]);
    mtVec2 tex(texcoord[0], texcoord[1]);

    mtVertex *vert = new mtNTVertex(crd, nrm, tex);
    return vert;
}

mtVertex *
xbsCNTVertex::makeMTVertex()
{
    mtVec3 crd(coord[0], coord[1], coord[2]);
    mtColor col(color[0], color[1], color[2]);
    mtVec3 nrm(normal[0], normal[1], normal[2]);
    mtVec2 tex(texcoord[0], texcoord[1]);

    mtVertex *vert = new mtCNTVertex(crd, col, nrm, tex);
    return vert;
}

/*****************************************************************************\
 @ Model::Model(GLOD_RawObject *obj)
 -----------------------------------------------------------------------------
 description : Construct a Model from a GLOD_RawObject.
 input       : 
 output      : 
 notes       : No vertex sharing is applied at this point, vertices'
               triangle lists have not been created, etc.
\*****************************************************************************/
Model::Model(GLOD_RawObject *obj)
{
    init();

    unsigned int flags = obj->patches[0]->data_flags;
    
    // verify that all patches have the same attributes
    for (unsigned int pnum=1; pnum<obj->num_patches; pnum++)
        if (obj->patches[pnum]->data_flags != obj->patches[0]->data_flags)
        {
            fprintf(stderr, "Patches must all have the same attributes.\n");
            return;
        }

    if (flags & GLOD_HAS_VERTEX_COLORS_4)
        fprintf(stderr, "VERTEX_COLORS_4 not supported yet. Ignoring colors.\n");

    //
    // load the vertices into the model
    //
    
    // count vertices and allocate vertex list
    maxVerts = 0;
    for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
        maxVerts += obj->patches[pnum]->num_vertices;
    verts = new xbsVertex *[maxVerts];
    numVerts = 0;

    // add vertices
    xbsVec3  coord;
    xbsColor color;
    xbsVec3  normal;
    xbsVec2  texcoord;
    if (flags & GLOD_HAS_VERTEX_COLORS_3)
    {
        if (flags & GLOD_HAS_VERTEX_NORMALS)
        {
            if (flags & GLOD_HAS_TEXTURE_COORDS_2)
            {
                for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
                {
                    GLOD_RawPatch *patch = obj->patches[pnum];
                    for (unsigned int vnum=0; vnum<patch->num_vertices; vnum++)
                    {
                        coord.set(patch->vertices[vnum*3+0],
                                  patch->vertices[vnum*3+1],
                                  patch->vertices[vnum*3+2]);
                        color.set((unsigned char)
                                  (patch->vertex_colors[vnum*3+0]*255.0),
                                  (unsigned char)
                                  (patch->vertex_colors[vnum*3+1]*255.0),
                                  (unsigned char)
                                  (patch->vertex_colors[vnum*3+2]*255.0));
                        normal.set(patch->vertex_normals[vnum*3+0],
                                   patch->vertex_normals[vnum*3+1],
                                   patch->vertex_normals[vnum*3+2]);
                        texcoord.set(patch->vertex_texture_coords[vnum*2+0],
                                     patch->vertex_texture_coords[vnum*2+1]);
                        addVert(new xbsCNTVertex(coord, color, normal, texcoord));
                    }
                }
            }
            else
            {
                for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
                {
                    GLOD_RawPatch *patch = obj->patches[pnum];
                    for (unsigned int vnum=0; vnum<patch->num_vertices; vnum++)
                    {
                        coord.set(patch->vertices[vnum*3+0],
                                  patch->vertices[vnum*3+1],
                                  patch->vertices[vnum*3+2]);
                        color.set((unsigned char)
                                  (patch->vertex_colors[vnum*3+0]*255.0),
                                  (unsigned char)
                                  (patch->vertex_colors[vnum*3+1]*255.0),
                                  (unsigned char)
                                  (patch->vertex_colors[vnum*3+2]*255.0));
                        normal.set(patch->vertex_normals[vnum*3+0],
                                   patch->vertex_normals[vnum*3+1],
                                   patch->vertex_normals[vnum*3+2]);
                        addVert(new xbsCNVertex(coord, color, normal));
                    }
                }
            }
        }
        else
        {
            if (flags & GLOD_HAS_TEXTURE_COORDS_2)
            {
                for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
                {
                    GLOD_RawPatch *patch = obj->patches[pnum];
                    for (unsigned int vnum=0; vnum<patch->num_vertices; vnum++)
                    {
                        coord.set(patch->vertices[vnum*3+0],
                                  patch->vertices[vnum*3+1],
                                  patch->vertices[vnum*3+2]);
                        color.set((unsigned char)
                                  (patch->vertex_colors[vnum*3+0]*255.0),
                                  (unsigned char)
                                  (patch->vertex_colors[vnum*3+1]*255.0),
                                  (unsigned char)
                                  (patch->vertex_colors[vnum*3+2]*255.0));
                        texcoord.set(patch->vertex_texture_coords[vnum*2+0],
                                     patch->vertex_texture_coords[vnum*2+1]);
                        addVert(new xbsCTVertex(coord, color, texcoord));
                    }
                }
            }
            else
            {
                for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
                {
                    GLOD_RawPatch *patch = obj->patches[pnum];
                    for (unsigned int vnum=0; vnum<patch->num_vertices; vnum++)
                    {
                        coord.set(patch->vertices[vnum*3+0],
                                  patch->vertices[vnum*3+1],
                                  patch->vertices[vnum*3+2]);
                        color.set((unsigned char)
                                  (patch->vertex_colors[vnum*3+0]*255.0),
                                  (unsigned char)
                                  (patch->vertex_colors[vnum*3+1]*255.0),
                                  (unsigned char)
                                  (patch->vertex_colors[vnum*3+2]*255.0));
                        addVert(new xbsCVertex(coord, color));
                    }
                }
            }
        }
    }
    else
    {
        if (flags & GLOD_HAS_VERTEX_NORMALS)
        {
            if (flags & GLOD_HAS_TEXTURE_COORDS_2)
            {
                for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
                {
                    GLOD_RawPatch *patch = obj->patches[pnum];
                    for (unsigned int vnum=0; vnum<patch->num_vertices; vnum++)
                    {
                        coord.set(patch->vertices[vnum*3+0],
                                  patch->vertices[vnum*3+1],
                                  patch->vertices[vnum*3+2]);
                        normal.set(patch->vertex_normals[vnum*3+0],
                                   patch->vertex_normals[vnum*3+1],
                                   patch->vertex_normals[vnum*3+2]);
                        texcoord.set(patch->vertex_texture_coords[vnum*2+0],
                                     patch->vertex_texture_coords[vnum*2+1]);
                        addVert(new xbsNTVertex(coord, normal, texcoord));
                    }
                }
            }
            else
            {
                for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
                {
                    GLOD_RawPatch *patch = obj->patches[pnum];
                    for (unsigned int vnum=0; vnum<patch->num_vertices; vnum++)
                    {
                        coord.set(patch->vertices[vnum*3+0],
                                  patch->vertices[vnum*3+1],
                                  patch->vertices[vnum*3+2]);
                        normal.set(patch->vertex_normals[vnum*3+0],
                                   patch->vertex_normals[vnum*3+1],
                                   patch->vertex_normals[vnum*3+2]);
                        addVert(new xbsNVertex(coord, normal));
                    }
                }
            }
        }
        else
        {
            if (flags & GLOD_HAS_TEXTURE_COORDS_2)
            {
                for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
                {
                    GLOD_RawPatch *patch = obj->patches[pnum];
                    for (unsigned int vnum=0; vnum<patch->num_vertices; vnum++)
                    {
                        coord.set(patch->vertices[vnum*3+0],
                                  patch->vertices[vnum*3+1],
                                  patch->vertices[vnum*3+2]);
                        texcoord.set(patch->vertex_texture_coords[vnum*2+0],
                                     patch->vertex_texture_coords[vnum*2+1]);
                        addVert(new xbsTVertex(coord, texcoord));
                    }
                }
            }
            else
            {
                for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
                {
                    GLOD_RawPatch *patch = obj->patches[pnum];
                    for (unsigned int vnum=0; vnum<patch->num_vertices; vnum++)
                    {
                        coord.set(patch->vertices[vnum*3+0],
                                  patch->vertices[vnum*3+1],
                                  patch->vertices[vnum*3+2]);
                        addVert(new xbsVertex(coord));
                    }
                }
            }
        }
    }

    // count triangles and allocate triangle list
    maxTris = 0;
    for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
        maxTris += obj->patches[pnum]->num_triangles;
    tris = new xbsTriangle *[maxTris];
    numTris = 0;

    // add triangles
    int vertex_bias = 0;
    for (unsigned int pnum=0; pnum<obj->num_patches; pnum++)
    {
        GLOD_RawPatch *patch = obj->patches[pnum];
        for (unsigned int tnum=0; tnum<patch->num_triangles; tnum++)
        {
            addTri(
                new xbsTriangle(verts[patch->triangles[tnum*3+0]+vertex_bias],
                                verts[patch->triangles[tnum*3+1]+vertex_bias],
                                verts[patch->triangles[tnum*3+2]+vertex_bias],
                                (int)pnum)
                );
        }
        vertex_bias += patch->num_vertices;
    }

    numPatches = (int)obj->num_patches;
}

/*****************************************************************************\
 @ Model::Model(DiscreteLevel *obj)
 -----------------------------------------------------------------------------
 description : Convert a DiscreteLevel to a Model.
 input       : 
 output      : 
 notes       : Currently used primarily for writing a DiscreteLevel
               out to a PLY file.
\*****************************************************************************/
Model::Model(DiscreteLevel *obj)
{
    init();

    
    //
    // load the vertices into the model
    //
    
    // count vertices and allocate vertex list
    maxVerts = 0;
    for (int pnum=0; pnum<obj->numPatches; pnum++)
    {
        DiscretePatch *patch = &(obj->patches[pnum]);
        maxVerts += patch->getVerts().getSize();
    }
    verts = new xbsVertex *[maxVerts];
    numVerts = 0;

    // add vertices
    xbsVec3 coord;  xbsColor color;
    xbsVec3 normal; xbsVec2 texcoord;
    if (obj->hasColor())
    {
        if (obj->hasNormal())
        {
            if (obj->hasTexcoord())
            {
                for (int pnum=0; pnum<obj->numPatches; pnum++)
                {
                    DiscretePatch *patch = &(obj->patches[pnum]);
                    for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
                    {
                        patch->getVerts().getAt(vnum,coord,color,normal,texcoord);
                        addVert(new xbsCNTVertex(coord,
                                                 color,
                                                 normal,
                                                 texcoord));
                    }
                }
            }
            else
            {
                for (int pnum=0; pnum<obj->numPatches; pnum++)
                {
                    DiscretePatch *patch = &(obj->patches[pnum]);
                    for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
                    {
                        patch->getVerts().getAt(vnum,coord,color,normal,texcoord);
                        addVert(new xbsCNVertex(coord,
                                                color,
                                                normal));
                    }
                }
            }
        }
        else
        {
            if (obj->hasTexcoord())
            {
                for (int pnum=0; pnum<obj->numPatches; pnum++)
                {
                    DiscretePatch *patch = &(obj->patches[pnum]);
                    for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
                    {
                        patch->getVerts().getAt(vnum,coord,color,normal,texcoord);
                        addVert(new xbsCTVertex(coord,
                                                color,
                                                texcoord));
                    }
                }
            }
            else
            {
                for (int pnum=0; pnum<obj->numPatches; pnum++)
                {
                    DiscretePatch *patch = &(obj->patches[pnum]);
                    for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
                    {
                        patch->getVerts().getAt(vnum,coord,color,normal,texcoord);
                        addVert(new xbsCVertex(coord,
                                               color));
                    }
                }
            }
        }
    }
    else
    {
        if (obj->hasNormal())
        {
            if (obj->hasTexcoord())
            {
                for (int pnum=0; pnum<obj->numPatches; pnum++)
                {
                    DiscretePatch *patch = &(obj->patches[pnum]);
                    for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
                    {
                        patch->getVerts().getAt(vnum,coord,color,normal,texcoord);
                        addVert(new xbsNTVertex(coord,
                                                normal,
                                                texcoord));
                    }
                }
            }
            else
            {
                for (int pnum=0; pnum<obj->numPatches; pnum++)
                {
                    DiscretePatch *patch = &(obj->patches[pnum]);
                    for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
                    {
                        patch->getVerts().getAt(vnum,coord,color,normal,texcoord);
                        addVert(new xbsNVertex(coord,
                                               normal));
                    }
                }
            }
        }
        else
        {
            if (obj->hasTexcoord())
            {
                for (int pnum=0; pnum<obj->numPatches; pnum++)
                {
                    DiscretePatch *patch = &(obj->patches[pnum]);
                    for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
                    {
                        patch->getVerts().getAt(vnum,coord,color,normal,texcoord);
                        addVert(new xbsTVertex(coord,
                                               texcoord));
                    }
                }
            }
            else
            {
                for (int pnum=0; pnum<obj->numPatches; pnum++)
                {
                    DiscretePatch *patch = &(obj->patches[pnum]);
                    for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
                    {
                        patch->getVerts().getAt(vnum,coord,color,normal,texcoord);
                        addVert(new xbsVertex(coord));
                    }
                }
            }
        }
    }

    // count triangles and allocate triangle list
    maxTris = obj->numTris;
    tris = new xbsTriangle *[maxTris];
    numTris = 0;

    // add triangles
    int indexBase = 0;
    for (int pnum=0; pnum<obj->numPatches; pnum++)
    {
        DiscretePatch *patch = &(obj->patches[pnum]);
        for (unsigned int index=0; index<patch->numIndices; index+=3)
        {
            addTri(
                new xbsTriangle(verts[patch->indices[index]+indexBase],
                                verts[patch->indices[index+1]+indexBase],
                                verts[patch->indices[index+2]+indexBase],
                                pnum)
                );
        }
        indexBase += patch->getNumVerts();
    }

    numPatches = (int)obj->numPatches;

    // For Half_Edge_Collapse discrete hierarchy, we add ALL the vertices
    // for every level, so many vertices on the simplified levels are unused
    removeEmptyVerts();
}

/*****************************************************************************\
 @ ~Model::Model
 -----------------------------------------------------------------------------
 description : Initializes the permission grid
 input       : assumes verts and tris are set correctly
 output      : Nothing.  Permission grid is created
 notes       :
\*****************************************************************************/
Model::~Model()
{
    for (int vnum=0; vnum<numVerts; vnum++)
        delete verts[vnum];
    delete verts;
    verts = NULL;
    for (int tnum=0; tnum<numTris; tnum++)
        delete tris[tnum];

    delete tris;
    tris = NULL;
    if (other_elems != NULL)
        free(other_elems);
            
    delete snapshotTriSpecs;
    snapshotTriSpecs = NULL;

    delete snapshotErrorSpecs;
    snapshotErrorSpecs = NULL;
    if (permissionGrid)
        delete permissionGrid;
};


/*****************************************************************************\
 @ initPermissionGrid
 -----------------------------------------------------------------------------
 description : Initializes the permission grid
 input       : assumes verts and tris are set correctly
 output      : Nothing.  Permission grid is created
 notes       :
\*****************************************************************************/
void Model::initPermissionGrid()
{
    int vnum, tnum;
    if (DEBUG_PERMISSION_GRID) 
        fprintf (stdout, "\n\tInitializing Permission Grid:\n\tDetermining min/max...");
    xbsVec3 minVertex ( MAXFLOAT,  MAXFLOAT,  MAXFLOAT);
    xbsVec3 maxVertex (-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
    for (vnum=0; vnum<numVerts; ++vnum)
    {
        if (verts[vnum]->coord[0] < minVertex[0])  
            minVertex[0] = verts[vnum]->coord[0];
        if (verts[vnum]->coord[1] < minVertex[1])  
            minVertex[1] = verts[vnum]->coord[1];
        if (verts[vnum]->coord[2] < minVertex[2])  
            minVertex[2] = verts[vnum]->coord[2];

        if (verts[vnum]->coord[0] > maxVertex[0])  
            maxVertex[0] = verts[vnum]->coord[0];
        if (verts[vnum]->coord[1] > maxVertex[1])  
            maxVertex[1] = verts[vnum]->coord[1];
        if (verts[vnum]->coord[2] > maxVertex[2])  
            maxVertex[2] = verts[vnum]->coord[2];
    }
    if (DEBUG_PERMISSION_GRID) 
        fprintf (stdout, "done.\n");
    permissionGrid = new PermissionGrid(minVertex, maxVertex);
    permissionGrid->createGrid((xbsReal)0.05, pgPrecision); // XXX how do i set this to the current error?

    if (DEBUG_PERMISSION_GRID) 
        fprintf (stdout, "\n\tInserting original triangles...");
    for (tnum=0; tnum<numTris; ++tnum)
        permissionGrid->insertTriangle(tris[tnum]);
    permissionGrid->dumpToOutfile("pg.dat");
    if (DEBUG_PERMISSION_GRID) 
        fprintf (stdout, "done.\n");
};

/*****************************************************************************\
 @ compare_ints
 -----------------------------------------------------------------------------
 description : A simple comparison function for use with qsort.
 input       : Addresses of two ints.
 output      : Positive number if a>b, negative number if a<b, and 0
               if they are equal
 notes       :
\*****************************************************************************/
static int
compare_ints (const void *a, const void *b)
{
    const int *ia = (const int *) a;
    const int *ib = (const int *) b;
    
    return (*ia > *ib) - (*ia < *ib);
} /** End of compare_ints() **/


void
Model::splitPatchVerts()
{
    for (int i=0; i<numVerts; i++)
    {
        xbsVertex *vert = verts[i];
        if (vert->numTris == 0)
            continue;
        int patchNum = vert->tris[0]->patchNum;
        int multiplePatches = 0;
        for (int j=0; j<vert->numTris; j++)
        {
            if (vert->tris[j]->patchNum != patchNum)
            {
                multiplePatches = 1;
                break;
            }
        }
        if (multiplePatches == 0)
            continue;

        //fprintf(stderr, "splitting multipatch vertex\n");
        
        // make list of unique patch ids around vertex
        int *patchNums = new int[vert->numTris];
        for (int j=0; j<vert->numTris; j++)
            patchNums[j] = vert->tris[j]->patchNum;
        qsort(patchNums, vert->numTris, sizeof(int), compare_ints);
        int current=0;
        for (int i=1; i<vert->numTris; i++)
            if (patchNums[i] != patchNums[current])
                patchNums[++current] = patchNums[i];
        int numPatches = current+1;

        // now make new vertices all patches after first one, and reassign
        // the appropriate triangles to them
        for (int patchIndex = 1; patchIndex < numPatches; patchIndex++)
        {
            xbsVertex *newvert = vert->makeNew();
            addVert(newvert);
            newvert->reallocTris(vert->numTris);
            vert->copySame(newvert);
            for (int tnum=0; tnum<vert->numTris; tnum++)
            {
                xbsTriangle *tri = vert->tris[tnum];
                if (tri->patchNum != patchNums[patchIndex])
                    continue;

                // remove triangle from around vert and add to newvert
                vert->tris[tnum] = vert->tris[vert->numTris-1];
                vert->numTris--;
                tnum--;
                newvert->tris[newvert->numTris++] = tri;

                // change triangle's vertex pointers to use newvert
                for (int tvnum=0; tvnum<3; tvnum++)
                    if (tri->verts[tvnum] == vert)
                        tri->verts[tvnum] = newvert;
            }

            
            // link this new vertex as to the coincident ring
            newvert->nextCoincident = vert->nextCoincident;
            vert->nextCoincident = newvert;
        }
        
        delete [] patchNums;
        patchNums = NULL;
    }
    
#ifdef VERBOSE
    fprintf(stderr, "Verts after splitting: %d\n", numVerts);
#endif
    
}

/*****************************************************************************\
 @ Model::readPly
 -----------------------------------------------------------------------------
 description : Read a PLY file, adding its vertices and faces to this Model.
 input       : Name of PLY file to read. NULL reads from stdin.
 output      : 
 notes       : PLY file vertices must have x,y,z and may have any of
               the standard combinations of colors (r,g,b), normals
               (nx,ny,nz) and texture coordinates (u,v).
\*****************************************************************************/
void
Model::readPly(char *filename)
{
    Boolean has_verts, has_faces;
    Boolean vert_has[11], face_has[2];
    Boolean vert_has_coord, vert_has_normal, vert_has_texcoord,
        vert_has_color;
    
    int nelems;
    char **elist;
    int file_type;
    float version;
    
    PlyFile *ply = ply_open_for_reading(filename, &nelems, &elist,
                                        &file_type, &version);

    has_verts = has_faces = FALSE;

    vert_has_coord = vert_has_normal = vert_has_texcoord =
        vert_has_color = FALSE;
    
    // first allocate vertices so face pointers may be set (in case faces
    // appear before vertices in file
    for (int i=0; i<nelems; i++)
    {
        int num_elems, nprops;
        char *elem_name = elist[i];
        PlyProperty **plist =
            ply_get_element_description(ply, elem_name, &num_elems,
                                        &nprops);

        if (equal_strings("vertex", elem_name))
        {
            has_verts = TRUE;
            
            for (int k=0; k<11; k++)
                vert_has[k] = FALSE;
            
            for (int j=0; j<nprops; j++)
            {
                for (int k=0; k<11; k++)
                {
                    if (equal_strings(vert_props[k].name,
                                      plist[j]->name))
                    {
                        ply_get_property(ply, elem_name, &vert_props[k]);
                        vert_has[k] = TRUE;
                    }
                }
            }

            if ((vert_has[0] == TRUE) &&
                (vert_has[1] == TRUE) &&
                (vert_has[2] == TRUE))
                vert_has_coord = TRUE;
            if ((vert_has[3] == TRUE) &&
                (vert_has[4] == TRUE) &&
                (vert_has[5] == TRUE))
                vert_has_normal = TRUE;
            if ((vert_has[6] == TRUE) &&
                (vert_has[7] == TRUE))
                vert_has_texcoord = TRUE;
            if ((vert_has[8] == TRUE) &&
                (vert_has[9] == TRUE) &&
                (vert_has[10] == TRUE))
                vert_has_color = TRUE;
            
            if (vert_has_coord == FALSE)
            {
                fprintf(stderr, "Vertices must have x, y, and z\n");
                exit(1);
            }

            /* allocate the vertices */
            verts = new xbsVertex *[num_elems];
            maxVerts = num_elems;
            numVerts = 0;
            
            if (vert_has_color)
            {
                if (vert_has_normal)
                {
                    if (vert_has_texcoord)
                    {
                        for (int j=0; j<num_elems; j++)
                            addVert(new xbsCNTVertex());
                    }
                    else
                    {
                        for (int j=0; j<num_elems; j++)
                            addVert(new xbsCNVertex());
                    }
                }
                else if (vert_has_texcoord)
                {
                    for (int j=0; j<num_elems; j++)
                        addVert(new xbsCTVertex());
                }
                else
                {
                    for (int j=0; j<num_elems; j++)
                        addVert(new xbsCVertex());
                }
            }
            else if (vert_has_normal)
            {
                if (vert_has_texcoord)
                {
                    for (int j=0; j<num_elems; j++)
                        addVert(new xbsNTVertex());
                }
                else
                {
                    for (int j=0; j<num_elems; j++)
                        addVert(new xbsNVertex());
                }
            }
            else if (vert_has_texcoord)
            {
                for (int j=0; j<num_elems; j++)
                    addVert(new xbsTVertex());
            }
            else
            {
                for (int j=0; j<num_elems; j++)
                    addVert(new xbsVertex());
            }
        }
    }

    
    for (int i=0; i<nelems; i++)
    {
        int num_elems, nprops;
        char *elem_name = elist[i];
        PlyProperty **plist =
            ply_get_element_description(ply, elem_name, &num_elems,
                                        &nprops);

        if (equal_strings("vertex", elem_name))
        {
            /* grab the vertices */
            for (int j=0; j<num_elems; j++)
            {
                plyVertex pVert;
                ply_get_element(ply, (void *)&pVert);

                if (vert_has_color)
                {
                    if (vert_has_normal)
                    {
                        if (vert_has_texcoord)
                        {
                            ((xbsCNTVertex *)(verts[j]))
                                ->set(pVert.coord, pVert.color,
                                      pVert.normal, pVert.texcoord);
                        }
                        else
                        {
                            ((xbsCNVertex *)(verts[j]))
                                ->set(pVert.coord, pVert.color,
                                      pVert.normal);
                        }
                    }
                    else if (vert_has_texcoord)
                    {
                        ((xbsCTVertex *)(verts[j]))
                            ->set(pVert.coord, pVert.color,
                                  pVert.texcoord);
                    }
                    else
                    {
                        ((xbsCVertex *)(verts[j]))
                            ->set(pVert.coord, pVert.color);
                    }
                }
                else if (vert_has_normal)
                {
                    if (vert_has_texcoord)
                    {
                        ((xbsNTVertex *)(verts[j]))
                            ->set(pVert.coord, pVert.normal,
                                  pVert.texcoord);
                    }
                    else
                    {
                        ((xbsNVertex *)(verts[j]))
                            ->set(pVert.coord, pVert.normal);
                    }
                }
                else if (vert_has_texcoord)
                {
                    ((xbsTVertex *)(verts[j]))
                        ->set(pVert.coord, pVert.texcoord);
                }
                else
                {
                    ((xbsVertex *)(verts[j]))
                        ->set(pVert.coord);
                }
            }
        }
        else if (equal_strings("face", elem_name))
        {
            has_faces = TRUE;
            
            for (int k=0; k<2; k++)
                face_has[k] = FALSE;
            
            for (int j=0; j<nprops; j++)
            {
                for (int k=0; k<2; k++)
                {
                    if (equal_strings(face_props[k].name,
                                      plist[j]->name))
                    {
                        ply_get_property(ply, elem_name, &face_props[k]);
                        face_has[k] = TRUE;
                    }
                }
            }
            
            /* test for necessary properties */
            if (face_has[0] != TRUE)
            {
                fprintf(stderr, "Faces must have vertex_indices.\n");
                exit(1);
            }
            
            /* grab the faces */
            plyFace pFace;
            pFace.patch_num = 0;
            tris = new xbsTriangle *[num_elems];
            maxTris = num_elems;
            numTris = 0;
            for (int j=0; j<num_elems; j++)
            {
                ply_get_element(ply, (void *)&pFace);
                if (pFace.nverts != 3)
                {
                    fprintf(stderr, "Faces must have 3 verts "
                            "(#%d has %d)\n",
                            j, pFace.nverts);
                    exit(1);
                }
                
                addTri(
                    new xbsTriangle(verts[pFace.verts[0]],
                                    verts[pFace.verts[1]],
                                    verts[pFace.verts[2]],
                                    pFace.patch_num)
                    );
                
                numPatches = MAX(pFace.patch_num + 1, numPatches);
                free(pFace.verts);
                pFace.verts = NULL;
                pFace.nverts = 0;
            }
        }

        else
            other_elems = ply_get_other_element(ply, elem_name, num_elems);
    }
    ply_close(ply);
    
    return;
}



/*****************************************************************************\
 @ Model::writePly
 -----------------------------------------------------------------------------
 description : Write out this Model to a PLY file.
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
Model::writePly(char *filename)
{
    if (indexed == 0)
        indexVertTris();

    int num_elems = 2;
    float version;
    
    PlyFile *ply =
        ply_open_for_writing(filename, num_elems, elem_names,
                             PLY_BINARY_NATIVE, &version);


    // each element could be described and written by its respective class,
    // but do it all here for now

    
    /* describe element properties */

    ply_element_count(ply, "vertex", numVerts);
    getVert(0)->describeProperties(ply, "vertex");

    ply_element_count(ply, "face", numTris);
    ply_describe_property(ply, "face", &face_props[0]);
    if (numPatches > 1)
        ply_describe_property(ply, "face", &face_props[1]);

    ply_describe_other_elements(ply, other_elems);
    
    ply_header_complete(ply);

    
    /* set up and write the vertex elements */
    ply_put_element_setup (ply, "vertex");
    plyVertex pvert;
    for (int i=0; i<numVerts; i++)
    {
        getVert(i)->fillPlyVertex(ply, &pvert);
        ply_put_element(ply, (void *) &pvert);
    }
    

    // temporarily store vertex indices in place of pointers
    for (int vnum=0; vnum<numVerts; vnum++)
    {
        xbsVertex *vert = verts[vnum];
        for (int tnum=0; tnum<vert->numTris; tnum++)
        {
            xbsTriangle *tri = vert->tris[tnum];
            for (int tvnum=0; tvnum<3; tvnum++)
            {
                if (tri->verts[tvnum] == vert)
                    tri->verts[tvnum] = (xbsVertex *)vnum;
            }
        }
    }
    
    /* set up and write the face elements */
    ply_put_element_setup(ply, "face");
    plyFace pface;
    pface.verts = new int[3];
    pface.nverts = 3;
    for (int i=0; i<numTris; i++)
    {
        for (int j=0; j<3; j++)
            pface.verts[j] = (int)(tris[i]->verts[j]);
        pface.patch_num = tris[i]->patchNum;
        ply_put_element(ply, (void *) &pface);
    }
    delete pface.verts;
    
    // replace vertex indices with pointers again
    for (int vnum=0; vnum<numVerts; vnum++)
    {
        xbsVertex *vert = verts[vnum];
        for (int tnum=0; tnum<vert->numTris; tnum++)
        {
            xbsTriangle *tri = vert->tris[tnum];
            for (int tvnum=0; tvnum<3; tvnum++)
            {
                if ((int)(tri->verts[tvnum]) == vnum)
                    tri->verts[tvnum] = vert;
            }
        }
    }
    
    
    ply_put_other_elements (ply);

    /* close the PLY file */
    ply_close(ply);
}

/*****************************************************************************\
 @ Model::indexVertTris
 -----------------------------------------------------------------------------
 description : For each vertex in the model, create a list of pointers
               to the triangles which use that vertex.
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
Model::indexVertTris()
{
    // count triangles around each vertex
    for (int vnum=0; vnum<numVerts; vnum++)
        verts[vnum]->numTris = 0;

    for (int tnum=0; tnum<numTris; tnum++)
        for (int vnum=0; vnum<3; vnum++)
            tris[tnum]->verts[vnum]->numTris++;

    // allocate lists
    for (int vnum=0; vnum<numVerts; vnum++)
    {
        xbsVertex *vert = verts[vnum];
        vert->tris = new xbsTriangle *[vert->numTris];
        vert->numTris = 0;
    }

    // fill in triangle lists
    for (int tnum=0; tnum<numTris; tnum++)
    {
        xbsTriangle *tri = tris[tnum];
        for (int vnum=0; vnum<3; vnum++)
        {
            xbsVertex *vert = tri->verts[vnum];
            vert->tris[vert->numTris++] = tri;
        }
    }

    indexed = 1;
}

/*****************************************************************************\
 @ Model::removeEmptyVerts
 -----------------------------------------------------------------------------
 description : Remove from the model all vertices which have no
               triangles attached (i.e. vertices that are not used by
               any triangles)
 input       : 
 output      : 
 notes       : This doesn't account for operations attached to
               vertices (it is generally called before any operations
               are attached to the vertices)
\*****************************************************************************/
void
Model::removeEmptyVerts()
{
    if (indexed == 0)
        indexVertTris();
    
    for (int vnum=0; vnum<numVerts; vnum++)
    {
        xbsVertex *vert = verts[vnum];
        if (vert->numTris > 0)
            continue;

        // remove vertex from coincident ring
        xbsVertex *prev;
        for (prev = vert; prev->nextCoincident != vert;
             prev = prev->nextCoincident);
        prev->nextCoincident = vert->nextCoincident;

        // remove vertex from model
        removeVert(vert);

        vnum--;
    }
}

/*****************************************************************************\
 @ Model::addVert
 -----------------------------------------------------------------------------
 description : Add a vertex to this model.
 input       : Vertex to be added.
 output      : Current index of the vertex in the model.
 notes       : A vertex may only be placed into a single model,
               because it has a field which stores its index in the
               model. The vertex index field is maintained by the
               model class, and may be modified whenever other
               vertices are removed from a model.
\*****************************************************************************/
int
Model::addVert(xbsVertex *vert)
{
    if (vert->index != -1)
    {
        fprintf(stderr, "Vertex is already in a Model!\n");
        exit(1);
    }
    
    if (numVerts+1 > maxVerts)
    {
        if (maxVerts == 0)
        {
            verts = new xbsVertex *[1];
            maxVerts = 1;
        }
        while (numVerts+1 > maxVerts)
        {
            xbsVertex **newverts = new xbsVertex *[maxVerts*2];
            for (int i=0; i<numVerts; i++)
                newverts[i] = verts[i];
            delete verts;
            verts = newverts;
            maxVerts *= 2;
        }
    }

    verts[numVerts++] = vert;
    vert->index = numVerts-1;
    return vert->index;
}

/*****************************************************************************\
 @ Model::addTri
 -----------------------------------------------------------------------------
 description : Add a triangle to this model.
 input       : Triangle to be added.
 output      : Current index of the triangle in the model.
 notes       : A triangle may only be placed into a single model,
               because it has a field which stores its index in the
               model. The triangle index field is maintained by the
               model class, and may be modified whenever other
               triangles are removed from a model.
\*****************************************************************************/
int
Model::addTri(xbsTriangle *tri)
{
    if (numTris+1 > maxTris)
    {
        if (maxTris == 0)
        {
            tris = new xbsTriangle *[1];
            maxTris = 1;
        }
        while (numTris+1 > maxTris)
        {
            xbsTriangle **newtris = new xbsTriangle *[maxTris*2];
            for (int i=0; i<numTris; i++)
                newtris[i] = tris[i];
            delete tris;
            tris = newtris;
            maxTris *= 2;
        }
    }

    tris[numTris++] = tri;
    tri->index = numTris-1;
    return tri->index;
}

/*****************************************************************************\
 @ Model::removeVert
 -----------------------------------------------------------------------------
 description : Remove a vertex from a model.
 input       : Vertex to be removed (which must be in the current model).
 output      : 
 notes       : This call can change the indices of other vertices in
               the model, so other functions should not depend on the
               stability of the index field.
\*****************************************************************************/
void
Model::removeVert(xbsVertex *vert)
{
    int index = vert->index;
    
    if ((index < 0) || (index > numVerts-1))
    {
        fprintf(stderr, "removeVert(): invalid index\n");
        exit(1);
    }

    if (verts[index] != vert)
    {
        fprintf(stderr, "removeVert(): vertex not found in model.\n");
        exit(1);
    }
    
    verts[index] = verts[numVerts-1];
    verts[index]->index = index;

    vert->index = -1;    
    numVerts--;

    if (numVerts < maxVerts / 2)
    {
        xbsVertex **newverts = new xbsVertex *[maxVerts/2];
        for (int i=0; i<numVerts; i++)
            newverts[i] = verts[i];
        delete verts;
        verts = newverts;
        maxVerts /= 2;
    }
}


/*****************************************************************************\
 @ Model::removeTri
 -----------------------------------------------------------------------------
 description : Remove a triangle from a model.
 input       : Triangle to be removed (which must be in the current model).
 output      : 
 notes       : This call can change the indices of other triangles in
               the model, so other functions should not depend on the
               stability of the index field.
\*****************************************************************************/
void
Model::removeTri(xbsTriangle *tri)
{
    int index = tri->index;
    
    if ((index < 0) || (index > numTris-1))
    {
        fprintf(stderr, "removeTri(): invalid index\n");
        exit(1);
    }

    if (tris[index] != tri)
    {
        fprintf(stderr, "removeTri(): tri not found in model.\n");
        exit(1);
    }
    
    tris[index] = tris[numTris-1];
    tris[index]->index = index;
    
    tri->index = -1;
    numTris--;

    if (numTris < maxTris / 2)
    {
        xbsTriangle **newtris = new xbsTriangle *[maxTris/2];
        for (int i=0; i<numTris; i++)
            newtris[i] = tris[i];
        delete tris;
        tris = newtris;
        maxTris /= 2;
    }
}

/*****************************************************************************\
 @ Model::verify
 -----------------------------------------------------------------------------
 description : Try to validate the correctness of the data in this model.
 input       : 
 output      : 
 notes       : This is obviously a slow call -- it's generally used
               for debugging purposes.
\*****************************************************************************/
void Model::verify()
{
    // Make sure we can find all model vertices by traversing coincident
    // rings of minCoincident vertices
    int vertCount = 0;
    for (int i=0; i<numVerts; i++)
    {
        xbsVertex *vert = getVert(i);
        if (vert->minCoincident() != vert)
            continue;
        vertCount += vert->numCoincident();
    }
    if (vertCount != numVerts)
    {
        fprintf(stderr, "Coincident ring problem: vertCount != numVerts!\n");
        exit(1);
    }

    // Make sure all coincident vertices have the same geometric
    // coordinates
    for (int i=0; i<numVerts; i++)
    {
        xbsVertex *vert = getVert(i);
        xbsVertex *min = vert->minCoincident();
        if (min == vert)
            continue;
        if ((min->coord - vert->coord).SquaredLength() != 0.0)
        {
            fprintf(stderr, "Coincident vertices have different coords!\n");
        }
    }


    // Make sure triangles appear on all three vertices
    for (int i=0; i<numTris; i++)
    {
        xbsTriangle *tri = tris[i];
        for (int j=0; j<3; j++)
        {
            xbsVertex *vert = tri->verts[j];
            int found = 0;
            for (int k=0; k<vert->numTris; k++)
            {
                if (vert->tris[k] == tri)
                {
                    found = 1;
                    break;
                }
            }
            if (found == 0)
            {
                fprintf(stderr, "Triangle not found on vertex.\n");
                exit(1);
            }
        }
    }

    // Make sure vertex appears on each of its triangles
    for (int i=0; i<numVerts; i++)
    {
        xbsVertex *vert = verts[i];
        for (int j=0; j<vert->numTris; j++)
        {
            xbsTriangle *tri = vert->tris[j];
            if ((tri->verts[0] != vert) && (tri->verts[1] != vert) &&
                (tri->verts[2] != vert))
            {
                fprintf(stderr, "Vertex not found on triangle.\n");
                exit(1);
            }
        }
    }
    
    // Make sure non of the triangles are degenerate
    for (int i=0; i<numTris; i++)
    {
        xbsTriangle *tri = tris[i];
        xbsVertex *min0 = tri->verts[0]->minCoincident();
        xbsVertex *min1 = tri->verts[1]->minCoincident();
        xbsVertex *min2 = tri->verts[2]->minCoincident();
        if ((min0 == min1) || (min1 == min2) || (min2 == min0))
        {
            fprintf(stderr, "Degenerate triangle in model!\n");
        }
    }
    
}

/*****************************************************************************\
 @ Model::testVertOps
 -----------------------------------------------------------------------------
 description : A debugging routine to verify that none of the
               operations on any of the vertices have a NULL source or
               a source equal to destination.
 input       : 
 output      : 
 notes       : Obviously a slow routine, when it is not "#if 0"ed out.
\*****************************************************************************/
void Model::testVertOps()
{
#if 0
    for (int i=0; i<numVerts; i++)
    {
        xbsVertex *vert = getVert(i);
        for (int j=0; j<vert->numOps; j++)
        {
            xbsReal x = vert->ops[j]->getSource()->coord.data[0];
            if (vert->ops[j]->getSource() == vert->ops[j]->getDestination())
                fprintf(stderr, "Problem!\n");
        }
    }
#endif
}

/*****************************************************************************\
  $Log: Model.C,v $
  Revision 1.36  2004/12/13 22:21:58  gfx_friends
  Cleaned up some handling of Discrete when using half edge collapses.

  Revision 1.35  2004/12/08 15:21:13  jdt6a
  Fixed bugs in voxelizer.  Implemented easy triangle test method.  See glod email I'm about to send for more details.

  Revision 1.34  2004/09/14 20:14:02  jdt6a
  added voxelization against all 7 planes now.  no significant visual difference.

  still need to create the triangle to test correctly within the error metric class (which is completely wrong now, and may require some assistance from someone who knows more about glod than i) and rewrite the triangle validation method.

  -j

  Revision 1.33  2004/08/04 22:25:18  gfx_friends
  Switched the DiscretePatch hierarchy to use attribsets. The adaptation routines are still broken, but I am not sure why. --n

  Revision 1.32  2004/07/28 06:07:10  jdt6a
  more permission grid work.  most of voxelization code from dachille/kaufman paper in place, but only testing against plane of triangle right now (not the other 6 planes yet).

  run simple.exe with a "-pg" flag to get the permission grid version (which isn't fully working yet... for some reason the single plane testing which should be very conservative results in too strict of a grid, or i am not testing the grid correctly).  the point sampled version actually results in better, aka more simplified, models, so i think there is a bug somewhere in the voxelization or testing.

  after a run of simple, a file "pg.dat" will be dumped into the current directory.  the pgvis program lets you visualize this file, which is just the grid.

  Revision 1.31  2004/07/21 17:40:54  gfx_friends
  Added comments.

  Revision 1.30  2004/07/20 21:54:36  gfx_friends
  Major Discrete and AttribSet rewrite. Discrete now uses attribsets for its vertex data. -Nat

  Revision 1.29  2004/07/19 19:26:29  gfx_friends
  Fixed a memory leak. There's no global list of all allocated operations, so
  we now delete operations attached to a vertex when we delete the vertex.

  Revision 1.28  2004/07/14 14:59:51  gfx_friends
  Made handling of border heuristics more consistent. Cylinder now
  simplifies again, and the torus patch borders work pretty well, too.
  The case where borders are not preserved too well right now is using
  full edge collapses and error quadrics, because the location of the
  generated vertex does not in general lie along the collapsed edge
  (it is chosen by an optimization involving an inversion of the quadric
  matrix, yada yada yada). We may improve this by adding additional border
  edge planes into the quadric, as done in some papers by Garland and
  Lindstrom.

  Revision 1.27  2004/07/12 15:38:41  gfx_friends
  Converted the GLOD makefiles to the Rich-style makefiles: this means that concurrent Debug and release builds are possible. Also added a post-build step for Win32 to keep some external directory in sync (util/post_build.bat) with GLOD. Many other little tweaks and warning removals.

  Revision 1.26  2004/07/12 13:25:12  gfx_friends
  J.C.:

  Actually commented the ply header! Removed non-portable fopen calls from
  Model.C.

  Revision 1.25  2004/07/08 16:47:49  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.24  2004/06/25 18:58:42  gfx_friends
  New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default

  Revision 1.23  2004/06/16 20:30:35  gfx_friends
  values.h include change for osx

  Revision 1.22  2004/06/11 19:25:13  gfx_friends
  Made GLOD keep quiet during execution.

  Revision 1.21  2004/06/10 16:18:02  gfx_friends
  Added a Multi-level Bucket (MLB) Priority Queue to replace the
  standard binary heap as the main xbs priority queue. It makes use of
  the fact that the xbs priority queue is mostly monotonic, with newly
  inserted items almost always having larger keys than the key of the
  item at the top of the heap. It seems to be about 75% faster than the
  binary heap, so the priority queue is no longer a major bottleneck,
  even for the current fast error metric computation.

  Revision 1.20  2004/06/07 16:32:15  gfx_friends
  Windows compile problem change

  Revision 1.19  2004/06/03 19:04:08  gfx_friends
  Added a "border lock" mode to prevent xbs from moving/removing any
  vertices on a geometric border. The determination of whether or not
  something is on a geometric border is somewhat heuristic. It is not
  clear what we want to call a border in the presence of various sorts
  of non-manifold vertices (which may be created by xbs, even if the
  orinal model is manifold).

  To use border lock mode, set the object's GLOD_BUILD_BORDER_MODE to
  GLOD_BORDER_LOCK before building.

  Revision 1.18  2004/06/02 17:14:04  gfx_friends
  Changes to #includes so it works on a stock osx configuration

  Revision 1.17  2004/02/04 07:21:08  gfx_friends
  Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat

  Revision 1.16  2004/02/03 23:32:48  gfx_friends
  Changes to Discrete Manual to allow multi patch levels, all the other goodies.
  Still needs a change or two to straighten things out.

  Revision 1.15  2003/12/17 03:43:30  gfx_friends
  Added a new constructor for Model, which creates a model from a RawPatch, used for DISCRETE_MANUAL objects

  Revision 1.14  2003/07/26 01:17:43  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.13  2003/07/25 03:00:14  gfx_friends
  Jon Cohen -

  Cleaned up sharing a bit more -- now remove vertices which no longer
  have any triangles.

  Removed all #define VDS_CONTEXT nonsense. VDS no longer #includes
  anything from GLOD, so it is not necessary.

  Fixed some subtle problems with simplifying complicated
  multi-attribute models, like brain-gear.normals.ply (which has normals
  computed with sharp angle splitting). This model still trips some
  warning messages (oops4 and oops2) when I simplify to VDS_Hierarchy
  with Half_Edge_Collapse, but for now the program seems to recover well
  enough to generate a working VDS. (the warnings basically indicate
  that some vertices were not removed from the Model even after they
  have been merged in the VIF). But it's a sign that something still
  goes wrong in xbs occasionally.

  Shut up warning messages about things that are now considered okay.

  Revision 1.12  2003/07/23 19:55:34  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.11  2003/07/19 23:50:10  gfx_friends
  Jon Cohen:

  Fixed up output to VDS hierarchy from half edge collapse and edge
  collapse operators. Seems to work, but it's still subject to review by
  Brenden. Also fixed a bug in vertex sharing.

  Revision 1.10  2003/07/16 16:12:30  gfx_friends
  Added splitting of multi-patch vertices. Thus if a vertex touches
  triangles from multiple patches, it is split into a multi-attribute
  vertex (even if they have the same attributes). This is useful because
  each of these vertices can store information like its index in any
  hierarchy-related data structures.

  Revision 1.9  2003/07/16 03:12:29  gfx_friends
  Added xbs support for "multi-attribute vertices". These are
  geometrically coincident vertices that may have different
  attributes. Geometric coincidence is maintained throughout the
  simplification process and attributes are correctly propagated along.

  For the full edge collapse, the heuristics for preventing attribute
  seams along patch boundaries could still use a little work.

  Things seem to work for the DiscreteHierarchy output. VDS hierarchy
  has not been integrated yet.

  Revision 1.8  2003/07/01 22:41:53  gfx_friends
  Multiple patches are supported throughout the front end of GLOD. Only the readback demo functions however with multiple patches. -- nat

  Revision 1.7  2003/06/30 19:29:58  gfx_friends
  (1) InsertElements now works completely. --nat
  (2) Some XBS classes got moved from xbs.h into discrete.h and Hierarchy.h for
      cleanliness.

  Revision 1.6  2003/06/12 16:55:56  gfx_friends
  Fixed minor bug with xbs that delete memory that still had pointers to it floating around.

  Revision 1.5  2003/06/05 17:38:58  gfx_friends
  Patches to build on Win32.

  Revision 1.4  2003/01/20 04:14:40  gfx_friends
  Fixed texturing bugs.

  Revision 1.3  2003/01/19 01:11:25  gfx_friends
  *** empty log message ***

  Revision 1.2  2003/01/14 00:06:20  gfx_friends
  Added destructors.

  Revision 1.1  2003/01/13 20:30:15  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.3  2003/01/08 05:19:14  cohen
  Added first version of full edge collapse.

  Revision 1.2  2003/01/05 22:41:49  cohen
  Commented out a print statement

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/

