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
/*****************************************************************************\
  plyMT.C
  --
  Description : Read/write MT to a ply file
		
                Jonathan Cohen - November 1999
  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/mt/plyMT.C,v $
  $Revision: 1.4 $
  $Date: 2004/10/20 19:33:25 $
  $Author: gfx_friends $
  $Locker:  $
\*****************************************************************************/


/*----------------------------- Local Includes -----------------------------*/

#include <stdio.h>
#include <stddef.h>
#include <ply.h>
#include "mt.h"


/*------------------------------ Local Macros -------------------------------*/

#define OFFSET(structvar,field) \
   (((char *)(&(structvar.field))) - ((char *)&structvar))

/*------------------------------- Local Types -------------------------------*/

typedef struct plyMTInfo
{
    int root;
} plyMTInfo;

typedef struct plyVertex
{
    mtVec3  coord;          /* coordinates of vertex */
    mtVec3  normal;
    mtVec2  texcoord;
    mtColor color;
    mtReal  radius;  // used for mtPoint
} plyVertex;

typedef struct plyFace
{
    unsigned char nverts;    /* number of vertex indices in list */
    int *verts;              /* vertex index list */
#if 0
    int  patch_num;
#endif
} Face;

typedef struct plyNode
{
    int nchildren;
    int *children;
    int nparents;
    int *parents;
    float error;
} plyNode;

typedef struct plyArc
{
    int start;
    int end;
    int nfaces;
    int *faces;
    int npoints;
    int *points;
    int patchNumber;
    unsigned char borderFlag;
} plyArc;


typedef struct plyBVNode
{
} plyBVNode;

/*------------------------ Local Function Prototypes ------------------------*/


/*------------------------------ Local Globals ------------------------------*/

static
char *elem_names[] = { /* list of the kinds of elements in the user's object */
    "mtinfo", "vertex", "face", "node", "arc", "point", "bvnode"
};

static plyVertex dummyvert;

static
PlyProperty mtinfo_props[] = { /* list of property information for mtinfo */
  {"root", PLY_INT, PLY_INT, offsetof(plyMTInfo,root), 0, 0, 0, 0},
};

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
  {"radius", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,radius), 0, 0, 0, 0},
};

static
PlyProperty face_props[] = { /* list of property information for a face */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(plyFace,verts),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(plyFace,nverts)},
#if 0
  {"patch_num", PLY_INT, PLY_INT, offsetof(plyFace,patch_num), 0, 0, 0, 0},
#endif
};

static
PlyProperty node_props[] = { /* list of property information for a node */
  {"error", PLY_FLOAT, PLY_FLOAT, offsetof(plyNode,error), 0, 0, 0, 0},
  {"parent_arc_indices", PLY_INT, PLY_INT, offsetof(plyNode,parents),
     1, PLY_INT, PLY_INT, offsetof(plyNode,nparents)},
  {"child_arc_indices", PLY_INT, PLY_INT, offsetof(plyNode,children),
     1, PLY_INT, PLY_INT, offsetof(plyNode,nchildren)},
};

static
PlyProperty arc_props[] = { /* list of property information for an arc */
  {"start_node_index", PLY_INT, PLY_INT, offsetof(plyArc,start), 0, 0, 0, 0},
  {"end_node_index", PLY_INT, PLY_INT, offsetof(plyArc,end), 0, 0, 0, 0},
  {"point_indices", PLY_INT, PLY_INT, offsetof(plyArc,points),
     1, PLY_INT, PLY_INT, offsetof(plyArc,npoints)},
  {"face_indices", PLY_INT, PLY_INT, offsetof(plyArc,faces),
     1, PLY_UCHAR, PLY_INT, offsetof(plyArc,nfaces)},
  {"face_indices", PLY_INT, PLY_INT, offsetof(plyArc,faces),
     1, PLY_INT, PLY_INT, offsetof(plyArc,nfaces)},
  {"patch_number", PLY_INT, PLY_INT, offsetof(plyArc,patchNumber), 0, 0, 0, 0},
  {"border", PLY_UCHAR, PLY_UCHAR, offsetof(plyArc,borderFlag), 0, 0, 0, 0},
};

#if 0

static
PlyProperty bvnode_props[] = { /* list of property information for a bvnode */
};

static
PlyProperty *elem_props[6] = 
{
    mtinfo_props, vert_props, face_props, node_props, arc_props,
    bvnode_props
};

static int num_elements = 6;
#endif

/*---------------------------------Functions-------------------------------- */

/*****************************************************************************\
  $Log: plyMT.C,v $
  Revision 1.4  2004/10/20 19:33:25  gfx_friends
  Rewrote MTHierarchy driver, made the VDS hack in Operation.C vds-specific

  Revision 1.3  2003/07/26 01:17:27  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.2  2003/01/15 20:53:36  gfx_friends
  Fixed compilation warnings.

  Revision 1.1.1.1  2003/01/14 23:31:46  gfx_friends
  Argh!

  Revision 1.1  2003/01/14 23:07:10  gfx_friends
  Import.

  Revision 1.11  2001/01/03 17:43:26  cohen
  added supported for passing PLY "other elements" through

  Revision 1.10  2001/01/03 00:29:46  cohen
  fixed bug in setting up point properties

  Revision 1.9  2000/09/05 00:11:41  cohen
  made libMT.a compile under SGI CC. This involved setting a compiler
  option to correctly scope variables defined in a for declaration and
  also a few work arounds for other idiosyncrasies. plyfile.C still does
  not compile because strdup() does not exist on SGIs when compiling in
  ANSI mode, but it should be okay to link with gcc-compiled libply because
  it is C only (no C++).

  Revision 1.8  2000/08/24 19:52:38  cohen
  Allow loading of ply file without nodes and arcs

  Revision 1.7  2000/08/11 22:16:17  cohen
  Lots of changes. I tried to clean up some of the code. Added sub-classes
  to the vertex class. Added some support for Point primitives. Moved the
  yucky globals into the library from viewMT.C (at least the ones that
  the library seems to require).

  Revision 1.6  2000/03/13 23:25:55  cohen
  Fixed problem with having only an unsigned char representing the number
  of triangles in an arc. Now it can be stored as int or unsigned char.

  Revision 1.5  2000/01/06 05:09:39  cohen
  Added ability to read and write using stdin/stdout

  Revision 1.4  1999/12/29 10:32:53  cohen
  commented out some experimental stuff I was considering using to remove
  more of the cases from the ply reading/writing (trying to convert more
  and more to general loops)

  Revision 1.3  1999/12/10 07:15:08  cohen
  optimized ply mt file loading slightly by allocating the correct number
  of vertices, triangle, nodes, and arcs from the start and by skipping
  the merging of arcs (because they are typically merged before writing
  the file)

  Revision 1.2  1999/12/10 06:30:56  cohen
  Renamed graph "edges" to "arcs".
  Added Bounding Volume Nodes (BVNodes), but no actual bounding volumes yet.
  Added reading/writing of MT from/to specialized ply file format.
  Fixed but in mergeArcs() (I was missing the last arc, so the graph got
  slightly disconnected).

  Revision 1.1  1999/12/05 21:29:05  cohen
  Initial revision

\*****************************************************************************/
