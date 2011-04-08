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
  plycollapses.c
  --
  Description : Read in a ply file with edge collapses and construct mt
		
                Jonathan Cohen - November 1999
  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/mt/plycollapses.C,v $
  $Revision: 1.7 $
  $Date: 2004/07/08 16:16:34 $
  $Author: gfx_friends $
  $Locker:  $
\*****************************************************************************/


/*----------------------------- Local Includes -----------------------------*/

#include <stdio.h>
#include <math.h>
#if defined(_WIN32) || defined(__APPLE__)
#include <string.h>
#include <float.h>
#else
#include <strings.h>
#include <values.h>
#endif
#include <ply.h>
#include "mt.h"

/*----------------------------- Local Constants -----------------------------*/

#define X 0
#define Y 1
#define Z 2

/*------------------------------ Local Macros -------------------------------*/

#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define MIN(x,y) ((x)<(y) ? (x) : (y))

#define OFFSET(structvar,field) \
   (((char *)(&(structvar.field))) - ((char *)&structvar))


/*------------------------------- Local Types -------------------------------*/


typedef struct Point{
    float x;
    float y;
    float z;
} Point;


#if 0

/* user's vertex and face definitions for a polygonal object */

typedef struct BorderInfo
{
    int            vertex_index;
    unsigned char  npatch_ids, nu, nv;
    float         *u, *v;
    int           *patch_ids;
} BorderInfo;

typedef struct Vertex
{
    Point  coord;            /* coordinates of vertex */
    float  u, v;
    unsigned char nfaces;    /* number of face indices in list */
    int *faces;              /* face index list */
    int  mapped_id;
    BorderInfo *borderinfo;
    void *other_props;       /* other properties */
    vdsNode *node;
} Vertex;

typedef struct BorderCollapse
{
    int vert1, vert2;
    Point newvert;
    unsigned char npatch_ids, nu, nv;
    int *patch_ids;
    float *u, *v;
    float cost;
    int index;
} BorderCollapse;

#endif

typedef struct Face
{
    unsigned char nverts;    /* number of vertex indices in list */
    int *verts;              /* vertex index list */
    int mttri;
    int mtarc;
#if 0
    int  patch_num;
    void *other_props;       /* other properties */
#endif
} Face;


typedef struct EdgeCollapse 
{
    int vert1, vert2;    /* don't use edge id, because I may want to
			    allow the edge info to be discarded later */
    Point  newvert;
    float u, v;
    float cost;
} EdgeCollapse;

typedef struct vertexInfo
{
    int newID;
    int nfaces;
    int *faces;
} vertexInfo;

/*------------------------ Local Function Prototypes ------------------------*/


/*------------------------------ Local Globals ------------------------------*/

static mtVertex dummyvert;

static
PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,coord.data[0]), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,coord.data[1]), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,coord.data[2]), 0, 0, 0, 0},
#if 0
  {"u", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,texcoord.data[0]), 0, 0, 0, 0},
  {"v", PLY_FLOAT, PLY_FLOAT, OFFSET(dummyvert,texcoord.data[1]), 0, 0, 0, 0},
#endif
};

static
PlyProperty face_props[] = { /* list of property information for a face */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
#if 0
  {"patch_num", PLY_INT, PLY_INT, offsetof(Face,patch_num), 0, 0, 0, 0},
#endif
};

#if 0
static
PlyProperty borderinfo_props[]  = { /* list of property information for a vertex */
  {"vertex_index", PLY_INT, PLY_INT, offsetof(BorderInfo,vertex_index),
   0, 0, 0, 0},
  {"u", PLY_FLOAT, PLY_FLOAT, offsetof(BorderInfo,u),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(BorderInfo,nu)},
  {"v", PLY_FLOAT, PLY_FLOAT, offsetof(BorderInfo,v),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(BorderInfo,nv)},
  {"patch_ids", PLY_INT, PLY_INT, offsetof(BorderInfo,patch_ids),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(BorderInfo,npatch_ids)},
};
#endif

/* list of property information for an edge collapse */
static
PlyProperty edge_collapse_props[] = { 
  {"vert1", PLY_INT, PLY_INT, offsetof(EdgeCollapse,vert1), 0, 0, 0, 0},
  {"vert2", PLY_INT, PLY_INT, offsetof(EdgeCollapse,vert2), 0, 0, 0, 0},
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(EdgeCollapse,newvert.x),
   0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(EdgeCollapse,newvert.y),
   0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(EdgeCollapse,newvert.z),
   0, 0, 0, 0},
  {"cost", PLY_FLOAT, PLY_FLOAT, offsetof(EdgeCollapse,cost), 0, 0, 0, 0},
  {"u", PLY_FLOAT, PLY_FLOAT, offsetof(EdgeCollapse,u),
   0, 0, 0, 0},
  {"v", PLY_FLOAT, PLY_FLOAT, offsetof(EdgeCollapse,v),
   0, 0, 0, 0},
};

#if 0
static
PlyProperty border_collapse_props[] = { 
  {"vert1", PLY_INT, PLY_INT, offsetof(BorderCollapse,vert1), 0, 0, 0, 0},
  {"vert2", PLY_INT, PLY_INT, offsetof(BorderCollapse,vert2), 0, 0, 0, 0},
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(BorderCollapse,newvert.x),
   0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(BorderCollapse,newvert.y),
   0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(BorderCollapse,newvert.z),
   0, 0, 0, 0},
  {"cost", PLY_FLOAT, PLY_FLOAT, offsetof(BorderCollapse,cost), 0, 0, 0, 0},
  {"u", PLY_FLOAT, PLY_FLOAT, offsetof(BorderCollapse,u),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(BorderCollapse,nu)},
  {"v", PLY_FLOAT, PLY_FLOAT, offsetof(BorderCollapse,v),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(BorderCollapse,nv)},
  {"patch_ids", PLY_INT, PLY_INT, offsetof(BorderCollapse,patch_ids),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(BorderCollapse,npatch_ids)},
  {"collapse_index", PLY_INT, PLY_INT, offsetof(BorderCollapse,index), 0, 0, 0, 0},
};
#endif

/*** the PLY object ***/

#if 0
static Vertex        **vlist;
static PlyOtherProp   *vert_other;
static BorderInfo    **blist;
static int             nborderinfos;
static PlyOtherProp   *face_other;
static BorderCollapse **bclist;
static int              nbordercollapses = 0;
static PlyOtherElems  *other_elements = NULL;
static int             num_comments;
static char 	     **comments;
static int  	       num_obj_info;
static char 	     **obj_info;
static int  	       file_type;
vdsNode *vertexTree;
#endif

static Face           *flist; // used until MT faces can be created
static int    	       nfaces;

static EdgeCollapse   *clist;
static int    	       ncollapses=0;
static int             has_texcoord;
static int             has_borderinfo;
static int    	       nverts;
static vertexInfo     *vinfo;

/*---------------------------------Functions-------------------------------- */


/*****************************************************************************
*****************************************************************************/
inline void updatebox(float *min, float *max, float *data)
{
   if(min[0] > data[0]) min[0] = data[0];
   if(min[1] > data[1]) min[1] = data[1];
   if(min[2] > data[2]) min[2] = data[2];
   if(max[0] < data[0]) max[0] = data[0];
   if(max[1] < data[1]) max[1] = data[1];
   if(max[2] < data[2]) max[2] = data[2];
}


/*****************************************************************************
*****************************************************************************/
void findMTextent(MT *mt, float *min, float *max)
{
   static int mtSink = 0;
   min[0] = min[1] = min[2] = +MAXFLOAT;
   max[0] = max[1] = max[2] = -MAXFLOAT;

   mtNode *mtSnode = mt->getNode(mtSink);

   for(int i=0; i<mtSnode->getNumParents(); i++)
   {
      mtArc *larc = mt->getArc(mtSnode->getParent(i));
      for(int j=0; j<larc->getNumTris(); j++)
      {
         updatebox(min, max,
	       mt->getVert(mt->getTri(larc->getTri(j))->verts[0])->coord.data);
         updatebox(min, max,
	       mt->getVert(mt->getTri(larc->getTri(j))->verts[1])->coord.data);
         updatebox(min, max,
	       mt->getVert(mt->getTri(larc->getTri(j))->verts[2])->coord.data);
      }
   }
}

#if 0
void readPPM(char *filename, Image *image)
{
    FILE  *fp;
    char   input_line[256];
    int    size;
    char   s1[16];
    int    i1, i2;
    
    size = sizeof(input_line);
    
    fp = fopen(filename, "r");
    if (!fp)
    {
	fprintf(stderr, "Couldn't open texture file, %s.\n",
		filename);
	exit(1);
    }

    if (fgets(input_line, size, fp) == NULL)
    {
	fprintf(stderr, "Couldn't read magic number line.\n");
	exit(1);
    }
    
    if (sscanf(input_line, "%s", &s1) != 1) 
    {
	fprintf(stderr, "Error reading magic number");
	exit(1);
    }
    if (strcmp(s1, "P6"))
    {
	fprintf(stderr, "Bad magic number: not a binary ppm file.\n");
	exit(1);
    }

    if (fgets(input_line, size, fp) == NULL)
    {
	fprintf(stderr, "Couldn't read resolution line.\n");
	exit(1);
    }
    if (sscanf(input_line, "%d %d", &i1, &i2) != 2)
    {
	fprintf(stderr, "Error reading resolution.\n");
	exit(1);
    }
    if ((i1 <= 0) || (i2 <= 0))
    {
	fprintf(stderr, "Invalid resolution: %d x %d.\n", i1, i2);
	exit(1);
    }

    image->xres = i1;
    image->yres = i2;

    fgets(input_line, size, fp);

    image->data = (unsigned char *)calloc(sizeof(unsigned char),
					 image->xres*image->yres*3);
    if (fread((void *)(image->data), sizeof(unsigned char),
	      image->xres * image->yres * 3, fp) != image->xres*image->yres*3)
    {
	fprintf(stderr, "Error reading image data.\n");
	exit(1);
    }

    fclose(fp);
}
#endif

void doCollapses(MT *mt)
{
    int           i, j;
    int           keepvert_id, tossvert_id;
    vertexInfo   *keepvert, *tossvert;
    Face         *face;
    int           count, current;
    vertexInfo   *othervert;
    int           faces_removed;
    int           collapsenum;
    EdgeCollapse *collapse;
    
    for (collapsenum=0; collapsenum<ncollapses;
	 collapsenum++)
    {
	collapse = &(clist[collapsenum]);
	if ((collapse->vert1 < 0) || (collapse->vert2 < 0))
	    break;

	// create new MT node for this collapse
	int nodeID = mt->addNode();
	mtNode *node = mt->getNode(nodeID);

	// This really needs to be multiplied by bbox_diagonal / 100.0.
	// Plycollapses files were written out with
	// cost = deviation / bbox_diagonal * 100
	// we now do this after the whole MT is build
	node->setError(collapse->cost);
	
	keepvert_id = MIN(collapse->vert1, collapse->vert2);
	keepvert    = &(vinfo[keepvert_id]);
	tossvert_id = MAX(collapse->vert1, collapse->vert2);
	tossvert    = &(vinfo[tossvert_id]);

	// create new MT vertex and store new ID
	keepvert->newID =
	    mt->addVertex(mtVertex(mtVec3(collapse->newvert.x,
					  collapse->newvert.y,
					  collapse->newvert.z)/*,
								 mtVec2(collapse->u, collapse->v)*/));
	

	/* append face list of tossvert to keepvert */

	// count new faces from tossvert
	int newFaces=0;
	for (i=0; i<tossvert->nfaces; i++)
	{
	    face = &(flist[tossvert->faces[i]]);
	    // exclude faces already connected to keepvert
	    if ((face->verts[0] != keepvert_id) &&
		(face->verts[1] != keepvert_id) &&
		(face->verts[2] != keepvert_id))
		newFaces++;
	}
	    
	REALLOCN(keepvert->faces, int, keepvert->nfaces,
		 keepvert->nfaces + newFaces);

	for (i=0; i<tossvert->nfaces; i++)
	{
	    face = &(flist[tossvert->faces[i]]);
	    // exclude faces already connected to keepvert
	    if ((face->verts[0] != keepvert_id) &&
		(face->verts[1] != keepvert_id) &&
		(face->verts[2] != keepvert_id))
		keepvert->faces[keepvert->nfaces++] = tossvert->faces[i];
	}

	// add all faces as children of new mtNode
	for (i=0; i<keepvert->nfaces; i++)
	{
	    // node->addChild(mt, flist[keepvert->faces[i]].mtarc);
	    mt->getArc(flist[keepvert->faces[i]].mtarc)->
		setStart(nodeID);
	}
	
	/* destroy tossvert */
	FREE(tossvert->faces);
	tossvert->nfaces = 0;
	tossvert->newID = -1;

	/* change all faces' vertex indices from tossvert to keepvert */
	for (i=0; i<keepvert->nfaces; i++)
	{
	    face = &(flist[keepvert->faces[i]]);
	    if (face == NULL)
		fprintf(stderr, "DANGER!\n");
	    if (face->nverts != 3)
	    {
		fprintf(stderr, "Non-triangular faces not allowed!\n");
		exit(1);
	    }
	    for (j=0; j<3; j++)
		if (face->verts[j] == tossvert_id)
		    face->verts[j] = keepvert_id;
	}
	
	/* delete all degenerate faces */
	faces_removed = 0;
	for (i=0; i<keepvert->nfaces; i++)
	{
	    face = &(flist[keepvert->faces[i]]);
	    if (face->nverts == 0)
	    {
		keepvert->faces[i] = -1;
		continue;
	    }

	    othervert = NULL;
	    for (j=0, count=0; j<3; j++)
	    {
		if (face->verts[j] == keepvert_id)
		    count++;
		else
		    othervert = &(vinfo[face->verts[j]]);
	    }
	    
	    if (count > 1)
	    {
		/* remove this face from the list of the other vertex */
		for (j=0, current=0; j<othervert->nfaces; j++)
		    if (othervert->faces[j] != keepvert->faces[i])
			othervert->faces[current++] = othervert->faces[j];
		REALLOCN(othervert->faces, int, othervert->nfaces, current);
		othervert->nfaces = current;
		
		FREE(face->verts);
		face->verts = NULL;
		face->nverts = 0;
		face->mttri = -1;
		face->mtarc = -1;
		
		keepvert->faces[i] = -1;
		
		faces_removed++;
	    }
	}
	
	/* pack face list */
	for (i=0, current=0; i<keepvert->nfaces; i++)
	    if (keepvert->faces[i] != -1)
		keepvert->faces[current++] = keepvert->faces[i];
	REALLOCN(keepvert->faces, int, keepvert->nfaces, current);
	keepvert->nfaces = current;

	// create new mtTriangles and mtArcs, and add as parents to the
	//   collapse Node
	for (i=0; i<keepvert->nfaces; i++)
	{
	    face = &(flist[keepvert->faces[i]]);
	    face->mttri =
		mt->addTriangle(vinfo[face->verts[0]].newID,
				vinfo[face->verts[1]].newID,
				vinfo[face->verts[2]].newID);
	    face->mtarc = mt->addArc(face->mttri);
	    mt->getArc(face->mtarc)->setEnd(nodeID);
	}
    }
    

    return;
}

void makeRoot(MT *mt)
{
    int rootID = mt->addNode();
    mtNode *root = mt->getNode(rootID);
    
    for (int i=0; i<nfaces; i++)
    {
	Face *face = &(flist[i]);
	if (face->mtarc != -1)
        {
	    mt->getArc(face->mtarc)->setStart(rootID);
	}
    }

    root->setError(MAXFLOAT);
    mt->setRoot(rootID);
}

void initMT(MT *mt)
{
    // initial vertices have already been loaded

    // create the sink node

    int sinkID = mt->addNode();
    
    // create MT faces (and arcs)
    for (int i=0; i<nfaces; i++)
    {
	Face *face = &(flist[i]);

	// this makes use of the fact that we know that the order of
	// vertices in the MT is the same as the order of vertices in the
	// PLY file
	face->mttri =
	    mt->addTriangle(face->verts[0], face->verts[1],
			    face->verts[2]);
	face->mtarc = mt->addArc(face->mttri);

	// add arc to sink node
	mt->getArc(face->mtarc)->setEnd(sinkID);
    }
}

void buildFaceLists(MT *mt)
{
    int     i, j;
    Face *face;
    vertexInfo *v;

    nverts = mt->getNumVerts();
    ALLOCN(vinfo, vertexInfo, nverts);
    
    /* count faces for each vertex and allocate face lists */
    for (i=0; i<nverts; i++)
    {
	vinfo[i].newID = i;
	vinfo[i].nfaces = 0;
    }
    for (i=0; i<nfaces; i++)
    {
	face = &(flist[i]);
	for (j=0; j<3; j++)
	    vinfo[face->verts[j]].nfaces++;
    }
    for (i=0; i<nverts; i++)
    {
	ALLOCN(vinfo[i].faces, int, vinfo[i].nfaces);
	vinfo[i].nfaces = 0;
    }

    /* add faces to face lists of all their vertices */
    for (i=0; i<nfaces; i++)
    {
	face = &(flist[i]);
	for (j=0; j<3; j++)
	{
	    v = &(vinfo[face->verts[j]]);
	    v->faces[v->nfaces++] = i;
	}
    }

    return;
}


/*****************************************************************************\
 @ read_plycollapses()
 -----------------------------------------------------------------------------
 description : Read in the PLY file from standard in.
 input       : PLY file from stdin
 output      : Surface mesh
 notes       :
\*****************************************************************************/
void read_plycollapses(char *filename, MT *mt)
{
    PlyFile 	 *ply;
    int     	  i, j;
    int     	  nprops;
    int           nelems;
    char        **element_list;
    int     	  num_elems;
    PlyProperty **plist;
    char         *elem_name;
    float 	  version;
    bool   	  has_x, has_y, has_z, has_u, has_v;
    bool   	  has_fverts, has_fpatchnum;
    bool   	  has_vert1, has_vert2, has_cx, has_cy, has_cz, has_cost;
    bool      has_cu, has_cv;
#if 0
    int           has_bvi, has_bu, has_bv, has_bpid;
#endif
    int           file_type;
    FILE         *fp;
    mtVertex      vert;
    
    /*** Read in the original PLY object ***/

    if (filename == NULL)
	fp = stdin;
    else
    {
	fp = fopen(filename, "r");
	if (!fp)
	{
	    fprintf(stderr, "Couldn't open ply file, %s, for reading.\n",
		    filename);
	    exit(1);
	}
    }
    
    ply  = ply_read (fp, &nelems, &element_list);
    ply_get_info (ply, &version, &file_type);

#if 0
    has_borderinfo = false;
#endif

    has_x = has_y = has_z = has_u = has_v = false;
    has_vert1 = has_vert2 = has_cx = has_cy = has_cz = has_cost = false;
    has_cu = has_cv = false;
    
    for (i = 0; i < nelems; i++)
    {
	/* get the description of the first element */
	elem_name = element_list[i];
	plist =
	    ply_get_element_description (ply, elem_name, &num_elems, &nprops);
	
	if (equal_strings ("vertex", elem_name))
	{
	    /* set up for getting vertex elements */
	    /* verify which properties these vertices have */
	    
	    for (j=0; j<nprops; j++)
	    {
		if (equal_strings("x", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &vert_props[0]);  /* x */
		    has_x = true;
		}
		else if (equal_strings("y", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &vert_props[1]);  /* y */
		    has_y = true;
		}
		else if (equal_strings("z", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &vert_props[2]);  /* z */
		    has_z = true;
		}
#if 1
		else if (equal_strings("u", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &vert_props[3]);  /* u */
		    has_u = true;
		}
		else if (equal_strings("v", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &vert_props[4]);  /* v */
		    has_v = true;
		}
#endif
	    }
#if 0
	    vert_other =
		ply_get_other_properties(ply, elem_name,
					 offsetof(Vertex,other_props));
#endif
	    
	    /* test for necessary properties */
	    if ((!has_x) || (!has_y) || (!has_z))
	    {
		fprintf(stderr, "Vertices must have x, y, and z coordinates\n");
		exit(-1);
	    }
	    
	    /* grab all the vertex elements */
	    for (j = 0; j < num_elems; j++)
	    {
		ply_get_element (ply, (void *)&vert);
		mt->addVertex(vert);
	    }
	}
	else if (equal_strings ("face", elem_name))
	{
	    /* create a list to hold all the face elements */
	    ALLOCN(flist, Face, num_elems);
	    nfaces = num_elems;

	    /* set up for getting face elements */
	    /* verify which properties these vertices have */
	    has_fverts = has_fpatchnum = false;
	    
	    for (j=0; j<nprops; j++)
	    {
		if (equal_strings("vertex_indices", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &face_props[0]);
		    has_fverts = true;
		}
#if 0
		if (equal_strings("patch_num", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &face_props[1]);
		    has_fpatchnum = true;
		}
#endif
	    }
#if 0
	    face_other =
		ply_get_other_properties(ply, elem_name,
					 offsetof(Face,other_props));
#endif
	    
	    /* test for necessary properties */
	    if (!has_fverts)
	    {
		fprintf(stderr, "Faces must have vertex indices\n");
		exit(-1);
	    }
	    
	    /* grab all the face elements */
	    for (j = 0; j < num_elems; j++)
	    {
		ply_get_element (ply, (void *)&(flist[j]));
		if (flist[j].nverts != 3)
		{
		    fprintf(stderr, "Faces must be triangulated\n");
		    fprintf(stderr, "  face #%d: nverts = %d\n",
			    j, flist[j].nverts);
		    exit(-1);
		}

#if 0
		if (!has_fpatchnum)
		   flist[j]->patch_num = 0;
#endif

	    }
	}
	else if (equal_strings ("borderinfo", elem_name))
	{
	    has_borderinfo = true;
#if 0
	    /* create a list to hold all the borderinfo elements */
	    ALLOCN(blist, BorderInfo *, num_elems);
	    nborderinfos = num_elems;
	    
	    /* set up for getting borderinfo elements */
	    /* verify which properties these borderinfos have */
	    has_bvi = has_bu = has_bv = has_bpid = false;
	    
	    for (j=0; j<nprops; j++)
	    {
		if (equal_strings("vertex_index", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &borderinfo_props[0]);
		    has_bvi = true;
		}
		if (equal_strings("u", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &borderinfo_props[1]);
		    has_bu = true;
		}
		if (equal_strings("v", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &borderinfo_props[2]);
		    has_bv = true;
		}
		if (equal_strings("patch_ids", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &borderinfo_props[3]);
		    has_bpid = true;
		}
	    }
	    
	    /* test for necessary properties */
	    if ((!has_bvi) || (!has_bu) || (!has_bv) || (!has_bpid))
	    {
		fprintf(stderr, "BorderInfo doesn't have all properties.\n");
		exit(-1);
	    }
	    
	    /* grab all the borderinfo elements */
	    for (j = 0; j < num_elems; j++)
	    {
		ALLOCN(blist[j], BorderInfo, 1);
		ply_get_element (ply, (void *)(blist[j]));
	    }
#endif
	}
	else if (equal_strings ("edge_collapse", elem_name))
	{
	    /* create a list to hold all the edge_collapse  elements */
	    ALLOCN(clist, EdgeCollapse, num_elems);
	    ncollapses = num_elems;
	    
	    /* set up for getting edge_collapse elements */
	    /* verify which properties these vertices have */
	    
	    for (j=0; j<nprops; j++)
	    {
		if (equal_strings("vert1", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &edge_collapse_props[0]);
		    has_vert1 = true;
		}
		if (equal_strings("vert2", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &edge_collapse_props[1]);
		    has_vert2 = true;
		}
		if (equal_strings("x", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &edge_collapse_props[2]);
		    has_cx = true;
		}
		if (equal_strings("y", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &edge_collapse_props[3]);
		    has_cy = true;
		}
		if (equal_strings("z", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &edge_collapse_props[4]);
		    has_cz = true;
		}
		if (equal_strings("cost", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &edge_collapse_props[5]);
		    has_cost = true;
		}
		if (equal_strings("u", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &edge_collapse_props[6]);
		    has_cu = true;
		}
		if (equal_strings("v", plist[j]->name))
		{
		    ply_get_property (ply, elem_name, &edge_collapse_props[7]);
		    has_cv = true;
		}
	    }
	    
	    /* test for necessary properties */
	    if ((!has_vert1) || (!has_vert2))
	    {
		fprintf(stderr, "Edge collapses must have vert1 and vert2\n");
		exit(1);
	    }
	    if ((!has_cx) || (!has_cy) || (!has_cz))
	    {
		fprintf(stderr, "Edge collapses must have x, y, and z\n");
		exit(1);
	    }
	    if (!has_cost)
	    {
		fprintf(stderr, "Edge collapses must have cost\n");
		exit(1);
	    }
	    
	    /* grab all the edge_collapse elements */
	    for (j = 0; j < num_elems; j++)
	    {
		ply_get_element (ply, (void *)&(clist[j]));
	    }
	}
	else if (equal_strings ("border_collapse", elem_name))
	{
#if 0
            /* create a list to hold all the borderinfo elements */
	    ALLOCN(bclist, BorderCollapse *, num_elems);
	    nbordercollapses = num_elems;
	    
	    ply_get_property (ply, elem_name, &border_collapse_props[0]);
	    ply_get_property (ply, elem_name, &border_collapse_props[1]);
	    ply_get_property (ply, elem_name, &border_collapse_props[2]);
	    ply_get_property (ply, elem_name, &border_collapse_props[3]);
	    ply_get_property (ply, elem_name, &border_collapse_props[4]);
	    ply_get_property (ply, elem_name, &border_collapse_props[5]);
	    ply_get_property (ply, elem_name, &border_collapse_props[6]);
	    ply_get_property (ply, elem_name, &border_collapse_props[7]);
	    ply_get_property (ply, elem_name, &border_collapse_props[8]);
	    ply_get_property (ply, elem_name, &border_collapse_props[9]);
	    
	    /* grab all the borderinfo elements */
	    for (j = 0; j < num_elems; j++)
	    {
		ALLOCN(bclist[j], BorderCollapse, 1);
		ply_get_element (ply, (void *)(bclist[j]));
	    }
#endif
	}
#if 1
	else
	    /* other_elements = */
	    ply_get_other_element (ply, elem_name, num_elems);
#endif
    }
    
#if 0
    comments = ply_get_comments (ply, &num_comments);
    obj_info = ply_get_obj_info (ply, &num_obj_info);
#endif
    
    ply_close (ply);

    if ((has_u) && (has_v) && (has_cu) && (has_cv))
	has_texcoord = true;
    else
	has_texcoord = false;

    
#if 0
    if (has_borderinfo)
	for (i=0; i<nborderinfos; i++)
	   vlist[blist[i]->vertex_index]->borderinfo = blist[i];
#endif
    
    if (has_borderinfo)
    {
	fprintf(stderr, "border collapses not supported yet.\n");
	exit(1);
    }

#if 0
    if ((!strcmp(filename, "textured.ply")) && (has_texcoord == true))
    {
	readPPM("texture.ppm", &obj->texture);
	obj->hasTexture = 1;
    }
#endif
} /** End of read_plycollapses() **/

void cleanUpPly(MT *mt)
{
    int i;

    for (i=0; i<nverts; i++)
    {
	if (vinfo[i].nfaces != 0)
	{
	    FREE(vinfo[i].faces);
	    vinfo[i].nfaces = 0;
	}
    }
    FREE(vinfo);
    FREE(clist);

    for (i=0; i<nfaces; i++)
    {
	if (flist[i].nverts != 0)
	{
	    FREE(flist[i].verts);
	    flist[i].nverts = 0;
	}
    }
    FREE(flist);
}

void scaleErrors(MT *mt)
{
    // plycollapses stores error as percent of bounding box diagonal,
    // whereas MT stores as absolute error. Now that we've loaded the whole
    // file, compute diagonal and scale errors to absolute errors
    float min[3], max[3];
    findMTextent(mt, min, max);
    float diag =  (max[0]-min[0])*(max[0]-min[0]);
    diag += (max[1]-min[1])*(max[1]-min[1]);
    diag += (max[2]-min[2])*(max[2]-min[2]);
    diag = sqrtf(diag);
    
    for (int i=0; i<mt->getNumNodes(); i++)
    {
	mtNode *node = mt->getNode(i);
	node->setError((float)(node->getError()*diag/100.0));
    }
}

void
MT::readPlyCollapses(char *filename)
{
    read_plycollapses(filename, this);
    buildFaceLists(this);
    initMT(this);
    doCollapses(this);
    makeRoot(this);
    cleanUpPly(this);
    connectArcs();
    scaleErrors(this);
}




/*****************************************************************************\
  $Log: plycollapses.C,v $
  Revision 1.7  2004/07/08 16:16:34  gfx_friends
  many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)

  Revision 1.6  2004/06/16 20:30:34  gfx_friends
  values.h include change for osx

  Revision 1.5  2004/02/19 15:51:24  gfx_friends
  Made the system compile in Win32 and patched a bunch of warnings.

  Revision 1.4  2003/07/26 01:17:27  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.3  2003/06/05 17:40:11  gfx_friends
  Patches to build on Win32.

  Revision 1.2  2003/01/15 20:53:36  gfx_friends
  Fixed compilation warnings.

  Revision 1.1.1.1  2003/01/14 23:31:46  gfx_friends
  Argh!

  Revision 1.1  2003/01/14 23:07:10  gfx_friends
  Import.

  Revision 1.7  2000/09/05 00:11:41  cohen
  made libMT.a compile under SGI CC. This involved setting a compiler
  option to correctly scope variables defined in a for declaration and
  also a few work arounds for other idiosyncrasies. plyfile.C still does
  not compile because strdup() does not exist on SGIs when compiling in
  ANSI mode, but it should be okay to link with gcc-compiled libply because
  it is C only (no C++).

  Revision 1.6  2000/08/11 22:16:17  cohen
  Lots of changes. I tried to clean up some of the code. Added sub-classes
  to the vertex class. Added some support for Point primitives. Moved the
  yucky globals into the library from viewMT.C (at least the ones that
  the library seems to require).

  Revision 1.5  2000/01/06 05:10:05  cohen
  Added ability to use stdin/stdout for data files

  Revision 1.4  1999/12/29 10:38:22  cohen
  cleaned up a few compiler warnings

  Revision 1.3  1999/12/29 10:31:14  cohen
  Activated ply_get_other_element(), which MUST be called to remove
  the other elements from the input stream before moving on to the expected
  elements.

  Revision 1.2  1999/12/10 06:30:56  cohen
  Renamed graph "edges" to "arcs".
  Added Bounding Volume Nodes (BVNodes), but no actual bounding volumes yet.
  Added reading/writing of MT from/to specialized ply file format.
  Fixed but in mergeArcs() (I was missing the last arc, so the graph got
  slightly disconnected).

  Revision 1.1  1999/12/05 21:27:37  cohen
  Initial revision

\*****************************************************************************/
