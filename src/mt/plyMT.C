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

void
mtVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
}

void
mtCVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[8]);
    ply_describe_property(ply, elem_name, &vert_props[9]);
    ply_describe_property(ply, elem_name, &vert_props[10]);
}

void
mtNVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[3]);
    ply_describe_property(ply, elem_name, &vert_props[4]);
    ply_describe_property(ply, elem_name, &vert_props[5]);
}

void
mtTVertex::describeProperties(PlyFile *ply, char *elem_name)
{
    ply_describe_property(ply, elem_name, &vert_props[0]);
    ply_describe_property(ply, elem_name, &vert_props[1]);
    ply_describe_property(ply, elem_name, &vert_props[2]);
    ply_describe_property(ply, elem_name, &vert_props[6]);
    ply_describe_property(ply, elem_name, &vert_props[7]);
}

void
mtCNVertex::describeProperties(PlyFile *ply, char *elem_name)
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
mtCTVertex::describeProperties(PlyFile *ply, char *elem_name)
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
mtNTVertex::describeProperties(PlyFile *ply, char *elem_name)
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
mtCNTVertex::describeProperties(PlyFile *ply, char *elem_name)
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

void
mtVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
}

void
mtNVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->normal = normal;
}

void
mtTVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->texcoord = texcoord;
}

void
mtCVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->color = color;
}

void
mtCNVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->color = color;
    pvert->normal = normal;
}

void
mtCTVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->color = color;
    pvert->texcoord = texcoord;
}

void
mtNTVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->normal = normal;
    pvert->texcoord = texcoord;
}

void
mtCNTVertex::fillPlyVertex(PlyFile *ply, plyVertex *pvert)
{
    pvert->coord = coord;
    pvert->color = color;
    pvert->normal = normal;
    pvert->texcoord = texcoord;
}

void
MT::readPlyMT(char *filename)
{
    bool has_mtinfo, has_verts, has_points, has_faces, has_nodes,
	    has_arcs, has_bvnodes;
    bool mtinfo_has[1], vert_has[11], point_has[12], face_has[1];
    bool node_has[1], arc_has[7];//, bvnode_has[1];
    bool vert_has_coord, vert_has_normal, vert_has_texcoord,
	    vert_has_color;
    bool point_has_coord, point_has_normal, point_has_texcoord,
	    point_has_color, point_has_radius;
    
    FILE *fp;
    
    if (filename == NULL)
	fp = stdin;
    else
    {
	fp = fopen(filename, "r");
	if (!fp)
	{
	    fprintf(stderr, "MT::readPlyMT(): "
		    "Couldn't open file, %s, for reading\n", filename);
	    exit(1);
	}
    }
    
    int nelems;
    char **elist;
    
    PlyFile *ply = ply_read(fp, &nelems, &elist);

    has_mtinfo = has_verts = has_points = has_faces = has_nodes = has_arcs
	= has_bvnodes = false;
    
    for (int i=0; i<nelems; i++)
    {
	int num_elems, nprops;
	char *elem_name = elist[i];
	PlyProperty **plist =
	    ply_get_element_description(ply, elem_name, &num_elems,
					&nprops);

	if (equal_strings("mtinfo", elem_name))
	{
	    has_mtinfo = true;
	    
	    for (int k=0; k<1; k++)
		mtinfo_has[k] = false;
	    
	    for (int j=0; j<nprops; j++)
	    {
		for (int k=0; k<1; k++)
		{
		    if (equal_strings(mtinfo_props[k].name,
				      plist[j]->name))
		    {
			ply_get_property(ply, elem_name, &mtinfo_props[k]);
			mtinfo_has[k] = true;
		    }
		}
	    }
	    
	    /* test for necessary properties */
	    for (int k=0; k<1; k++)
	    {
		if (mtinfo_has[k] == false)
		{
		    fprintf(stderr, "MTInfos must have %s\n",
			    mtinfo_props[k].name);
		    exit(1);
		}
	    }

	    /* grab the mtinfos */
	    plyMTInfo pMTInfo;
	    if (num_elems != 1)
	    {
		fprintf(stderr, "Num mtinfo elems != 1 (%d)\n",
			num_elems);
		exit(1);
	    }
	    
	    for (int j=0; j<num_elems; j++)
	    {
		ply_get_element(ply, (void *)&pMTInfo);
		setRoot(pMTInfo.root);
	    }	    
	}
	else if (equal_strings("vertex", elem_name))
	{
	    has_verts = true;
	    
	    for (int k=0; k<11; k++)
		vert_has[k] = false;
	    vert_has_coord = vert_has_normal = vert_has_texcoord =
		vert_has_color = false;
	    
	    for (int j=0; j<nprops; j++)
	    {
		for (int k=0; k<11; k++)
		{
		    if (equal_strings(vert_props[k].name,
				      plist[j]->name))
		    {
			ply_get_property(ply, elem_name, &vert_props[k]);
			vert_has[k] = true;
		    }
		}
	    }

	    if ((vert_has[0] == true) &&
		(vert_has[1] == true) &&
		(vert_has[2] == true))
		vert_has_coord = true;
	    if ((vert_has[3] == true) &&
		(vert_has[4] == true) &&
		(vert_has[5] == true))
		vert_has_normal = true;
	    if ((vert_has[6] == true) &&
		(vert_has[7] == true))
		vert_has_texcoord = true;
	    if ((vert_has[8] == true) &&
		(vert_has[9] == true) &&
		(vert_has[10] == true))
		vert_has_color = true;
	    
	    if (vert_has_coord == false)
	    {
		fprintf(stderr, "Vertices must have x, y, and z\n");
		exit(1);
	    }
	    int attrib_count = 0;
	    if (vert_has_normal)
		attrib_count++;
	    if (vert_has_texcoord)
		attrib_count++;
	    if (vert_has_color)
		attrib_count++;

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
			    mtCNTVertex mtCNTVert;
			    mtCNTVert.set(pVert.coord, pVert.color,
					  pVert.normal, pVert.texcoord);
			    
			    if (j==0)
				allocateVerts(num_elems, mtCNTVert);
			    
			    addVertex(mtCNTVert);
			}
			else
			{
			    mtCNVertex mtCNVert;
			    mtCNVert.set(pVert.coord, pVert.color,
					 pVert.normal);
			    
			    if (j==0)
				allocateVerts(num_elems, mtCNVert);
			    
			    addVertex(mtCNVert);
			}
		    }
		    else if (vert_has_texcoord)
		    {
			mtCTVertex mtCTVert;
			mtCTVert.set(pVert.coord, pVert.color,
				     pVert.texcoord);
			
			if (j==0)
			    allocateVerts(num_elems, mtCTVert);
			
			addVertex(mtCTVert);
		    }
		    else
		    {
			mtCVertex mtCVert;
			mtCVert.set(pVert.coord, pVert.color);
			
			if (j==0)
			    allocateVerts(num_elems, mtCVert);
			
			addVertex(mtCVert);
		    }
		}
		else if (vert_has_normal)
		{
		    if (vert_has_texcoord)
		    {
			mtNTVertex mtNTVert;
			mtNTVert.set(pVert.coord, pVert.normal,
				     pVert.texcoord);
			
			if (j==0)
			    allocateVerts(num_elems, mtNTVert);
			
			addVertex(mtNTVert);
		    }
		    else
		    {
			mtNVertex mtNVert;
			mtNVert.set(pVert.coord, pVert.normal);
			
			if (j==0)
			    allocateVerts(num_elems, mtNVert);
			
			addVertex(mtNVert);
		    }
		}
		else if (vert_has_texcoord)
		{
		    mtTVertex mtTVert;
		    mtTVert.set(pVert.coord, pVert.texcoord);
		    
		    if (j==0)
			allocateVerts(num_elems, mtTVert);
		    
		    addVertex(mtTVert);
		}
		else
		{
		    mtVertex mtVert;
		    mtVert.set(pVert.coord);
		    
		    if (j==0)
			allocateVerts(num_elems, mtVert);
		    
		    addVertex(mtVert);
		}
	    }
	}
	else if (equal_strings("point", elem_name))
	{
	    has_points = true;
	    
	    for (int k=0; k<12; k++)
		point_has[k] = false;
	    point_has_coord = point_has_normal = point_has_texcoord =
		point_has_color = false;
	    
	    for (int j=0; j<nprops; j++)
	    {
		for (int k=0; k<12; k++)
		{
		    if (equal_strings(vert_props[k].name,
				      plist[j]->name))
		    {
			ply_get_property(ply, elem_name, &vert_props[k]);
			point_has[k] = true;
		    }
		}
	    }

	    if ((point_has[0] == true) &&
		(point_has[1] == true) &&
		(point_has[2] == true))
		point_has_coord = true;
	    if ((point_has[3] == true) &&
		(point_has[4] == true) &&
		(point_has[5] == true))
		point_has_normal = true;
	    if ((point_has[6] == true) &&
		(point_has[7] == true))
		point_has_texcoord = true;
	    if ((point_has[8] == true) &&
		(point_has[9] == true) &&
		(point_has[10] == true))
		point_has_color = true;
	    if (point_has[11] == true)
		point_has_radius = true;
	    
	    if (point_has_coord == false)
	    {
		fprintf(stderr, "Points must have x, y, and z\n");
		exit(1);
	    }
	    if (point_has_radius == false)
	    {
		fprintf(stderr, "Points must have radius.\n");
		exit(1);
	    }
	    
	    int attrib_count = 0;
	    if (point_has_normal)
		attrib_count++;
	    if (point_has_texcoord)
		attrib_count++;
	    if (point_has_color)
		attrib_count++;

	    /* grab the points */
	    mtPoint point;
	    allocatePoints(num_elems);
	    for (int j=0; j<num_elems; j++)
	    {
		plyVertex pVert;
		ply_get_element(ply, (void *)&pVert);

		point.radius = pVert.radius;

		if (point_has_normal)
		    point.sample =
			new mtNVertex(pVert.coord, pVert.normal);
		else if (point_has_texcoord)
		    point.sample =
			new mtTVertex(pVert.coord, pVert.texcoord);
		else if (point_has_color)
		    point.sample =
			new mtCVertex(pVert.coord, pVert.color);
		else
		    point.sample =
			new mtVertex(pVert.coord);

		addPoint(point);
	    }
	    point.sample = NULL;
	}

	else if (equal_strings("face", elem_name))
	{
	    has_faces = true;
	    
	    for (int k=0; k<1; k++)
		face_has[k] = false;
	    
	    for (int j=0; j<nprops; j++)
	    {
		for (int k=0; k<1; k++)
		{
		    if (equal_strings(face_props[k].name,
				      plist[j]->name))
		    {
			ply_get_property(ply, elem_name, &face_props[k]);
			face_has[k] = true;
		    }
		}
	    }
	    
	    /* test for necessary properties */
	    for (int k=0; k<1; k++)
	    {
		if (face_has[k] == false)
		{
		    fprintf(stderr, "Faces must have %s\n",
			    face_props[k].name);
		    exit(1);
		}
	    }

	    /* grab the faces */
	    plyFace pFace;
	    allocateTris(num_elems);
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
		
		addTriangle(pFace.verts[0], pFace.verts[1],
				pFace.verts[2]);
		free(pFace.verts);
		pFace.verts = NULL;
		pFace.nverts = 0;
	    }
	}
	else if (equal_strings("node", elem_name))
	{
	    has_nodes = true;
	    
	    for (int k=0; k<1; k++)
		node_has[k] = false;

	    for (int j=0; j<nprops; j++)
	    {
		for (int k=0; k<1; k++)
		{
		    if (equal_strings(node_props[k].name,
				      plist[j]->name))
		    {
			ply_get_property(ply, elem_name, &node_props[k]);
			node_has[k] = true;
		    }
		}
	    }
	    
	    /* test for necessary properties */
	    for (int k=0; k<1; k++)
	    {
		if (node_has[k] == false)
		{
		    fprintf(stderr, "Nodes must have %s\n",
			    node_props[k].name);
		    exit(1);
		}
	    }

	    /* grab the nodes */
	    plyNode pNode;
	    allocateNodes(num_elems);
	    for (int j=0; j<num_elems; j++)
	    {
		ply_get_element(ply, (void *)&pNode);
		int nodeID = addNode();
		getNode(nodeID)->setError(pNode.error);
	    }
	}
	else if (equal_strings("arc", elem_name))
	{
	    has_arcs = true;
	    
	    for (int k=0; k<7; k++)
		arc_has[k] = false;
	    
	    for (int j=0; j<nprops; j++)
	    {
		for (int k=0; k<7; k++)
		{
		    if (k==4) k++;
		    if (equal_strings(arc_props[k].name,
				      plist[j]->name))
		    {
			/* this call does not affect the type of
			   count_external, so it works whether the triangle
			   count is stored as int or uchar */
			ply_get_property(ply, elem_name, &arc_props[k]);
			arc_has[k] = true;
		    }
		}
	    }
	    
	    /* test for necessary properties */
	    for (int k=0; k<4; k++)
	    {
		if (k==2) k++;
		if (arc_has[k] == false)
		{
		    fprintf(stderr, "Arcs must have %s\n",
			    arc_props[k].name);
		    exit(1);
		}
	    }

	    /* grab the arcs */
	    plyArc pArc;
	    allocateArcs(num_elems);
	    for (int j=0; j<num_elems; j++)
	    {
		ply_get_element(ply, (void *)&pArc);
		int arcID = addArc(pArc.faces, pArc.nfaces);
		getArc(arcID)->setStart(pArc.start);
		getArc(arcID)->setEnd(pArc.end);
		free(pArc.faces);
		pArc.faces = NULL;
		pArc.nfaces = 0;
		if (arc_has[2] == true)
		{
		    getArc(arcID)->addPoints(pArc.points, pArc.npoints);
		    free(pArc.points);
		    pArc.points = NULL;
		    pArc.npoints = 0;
		}
		if (arc_has[5] == true)
		{
		    getArc(arcID)->setPatchNumber(pArc.patchNumber);
		    if ((pArc.patchNumber+1) > numPatches)
			numPatches = pArc.patchNumber+1;
		}
		
		if (arc_has[6] == true)
		{
		    if (pArc.borderFlag == 0)
			getArc(arcID)->clearBorder();
		    else if (pArc.borderFlag == 1)
			getArc(arcID)->setBorder();
		    else
		    {
			fprintf(stderr, "Invalid border flag.\n");
			exit(1);
		    }
		}
	    }
	}
#if 0
	else if (equal_strings("bvnode", elem_name))
	{
	    has_bvnodes = true;
	    
	    for (int k=0; k<0; k++)
		bvnode_has[k] = false;
	    
	    for (int j=0; j<nprops; j++)
	    {
		for (int k=0; k<0; k++)
		{
		    if (equal_strings(bvnode_props[k].name,
				      plist[j]->name))
		    {
			ply_get_property(ply, elem_name, &bvnode_props[k]);
			bvnode_has[k] = true;
		    }
		}
	    }
	    
	    /* test for necessary properties */
	    for (int k=0; k<0; k++)
	    {
		if (bvnode_has[k] == false)
		{
		    fprintf(stderr, "BVNodes must have %s\n",
			    bvnode_props[k].name);
		    exit(1);
		}
	    }

#if 0
            /* grab the bvnodes */
	    plyBVNode pBVNode;
	    for (int j=0; j<num_elems; j++)
	    {
		ply_get_element(ply, (void *)&pBVNode);
	    }
#endif
	}
#endif
	else
	    other_elems = ply_get_other_element(ply, elem_name, num_elems);
    }
    ply_close(ply);

    if (has_nodes == true)
    {
	connectArcs();
	buildBVH();
    }
    
    return;
}

void
MT::writePlyMT(char *filename)
{
    FILE *fp;

    if (filename == NULL)
	fp = stdout;
    else
    {
	fp = fopen(filename, "w");
	if (!fp)
	{
	    fprintf(stderr, "MT::writePlyMT(): "
		    "Couldn't open file, %s, for writing\n", filename);
	    exit(1);
	}
    }

    int num_elems = 5;
    
    if (numPoints > 0)
	num_elems = 6;

    PlyFile *ply =
        //       ply_write(fp, num_elems, elem_names, PLY_BINARY_NATIVE);
        ply_write(fp, num_elems, elem_names, PLY_ASCII);

    // each element could be described and written by its respective class,
    // but do it all here for now

    
    /* describe element properties */

    ply_element_count(ply, "mtinfo", 1);
    ply_describe_property(ply, "mtinfo", &mtinfo_props[0]);
    
    ply_element_count(ply, "vertex", numVerts);
    getVert(0)->describeProperties(ply, "vertex");

    ply_element_count(ply, "face", numTris);
    ply_describe_property(ply, "face", &face_props[0]);
#if 0
    ply_describe_property(ply, "face", &face_props[1]);
#endif

    ply_element_count(ply, "node", numNodes);
    ply_describe_property(ply, "node", &node_props[0]);
#if 0
    // don't write out parents and children
    // recreate them on loading instead
    // this avoids dealing with the large number of parents of the sink
    // node ( >> 255 )
    ply_describe_property(ply, "node", &node_props[1]);
    ply_describe_property(ply, "node", &node_props[2]);
#endif
    
    ply_element_count(ply, "arc", numArcs);
    ply_describe_property(ply, "arc", &arc_props[0]);
    ply_describe_property(ply, "arc", &arc_props[1]);

    // only include point property if there are some point elements
    if (numPoints > 0)
	ply_describe_property(ply, "arc", &arc_props[2]);
	
    // count max tris per arc to choose int or uchar for external counter
    int maxTrisPerArc = 0;
    char writeTexID = 0;
    char writeBorder = 0;
    for (int i=0; i<numArcs; i++)
    {
	mtArc *arc = getArc(i);
	int tris = arc->getNumTris();
	maxTrisPerArc = (tris > maxTrisPerArc) ? tris : maxTrisPerArc;
	if (arc->getPatchNumber() != 0)
	    writeTexID = 1;
	if (arc->isBorder() != 0)
	    writeBorder = 1;
    }
    if (maxTrisPerArc <= 255)
	ply_describe_property(ply, "arc", &arc_props[3]);
    else
	ply_describe_property(ply, "arc", &arc_props[4]);
    if (writeTexID == 1)
	ply_describe_property(ply, "arc", &arc_props[5]);
    if (writeBorder == 1)
	ply_describe_property(ply, "arc", &arc_props[6]);
    


    
    if (numPoints > 0)
    {
	ply_element_count(ply, "point", numPoints);
	getPoint(0)->sample->describeProperties(ply, "point");
	ply_describe_property(ply, "point", &vert_props[11]);
    }
    

#if 0
    ply_element_count(ply, "bvnode", numNodes);
#endif

    ply_describe_other_elements(ply, other_elems);
    
    ply_header_complete(ply);

    
    /* set up and write the mtinfo element */
    ply_put_element_setup(ply, "mtinfo");
    plyMTInfo pmtinfo;
    pmtinfo.root = root;
    ply_put_element(ply, (void *) &pmtinfo);

    
    /* set up and write the vertex elements */
    ply_put_element_setup (ply, "vertex");
    plyVertex pvert;
    for (int i=0; i<numVerts; i++)
    {
	getVert(i)->fillPlyVertex(ply, &pvert);
	ply_put_element(ply, (void *) &pvert);
    }
    
    
    /* set up and write the face elements */
    ply_put_element_setup(ply, "face");
    plyFace pface;
    pface.nverts = 3;
    for (int i=0; i<numTris; i++)
    {
	pface.verts = tris[i].verts;
	ply_put_element(ply, (void *) &pface);
    }

    
    /* set up and write the node elements */
    ply_put_element_setup(ply, "node");
    plyNode pnode;
    for (int i=0; i<numNodes; i++)
    {
	mtNode *node = &nodes[i];
	
	pnode.parents = new int[node->getNumParents()];
	pnode.nparents = 0;
	for (int j=0; j<node->getNumParents(); j++)
	    pnode.parents[pnode.nparents++] = node->getParent(j);
	
	pnode.children = new int[node->getNumChildren()];
	pnode.nchildren = 0;
	for (int j=0; j<node->getNumChildren(); j++)
	    pnode.children[pnode.nchildren++] = node->getChild(j);

	pnode.error = node->getError();

	ply_put_element(ply, (void *) &pnode);

	delete pnode.parents;
	delete pnode.children;
    }

    /* set up and write the arc elements */
    ply_put_element_setup (ply, "arc");
    plyArc parc;
    for (int i=0; i<numArcs; i++)
    {
	mtArc *arc = &arcs[i];

	parc.start = arc->getStart();
	parc.end = arc->getEnd();
	parc.faces = new int[arc->getNumTris()];
	parc.nfaces = 0;
	
	for (int j=0; j<arc->getNumTris(); j++)
	    parc.faces[parc.nfaces++] = arc->getTri(j);

	if (arc->getNumPoints() > 0)
	    parc.points = new int[arc->getNumPoints()];
	else
	    parc.points = NULL;
	parc.npoints = 0;

	for (int j=0; j<arc->getNumPoints(); j++)
	    parc.points[parc.npoints++] = arc->getPoint(j);

	parc.patchNumber = arc->getPatchNumber();
	parc.borderFlag = arc->isBorder();
	ply_put_element(ply, (void *) &parc);
	delete parc.faces;
	if (arc->getNumPoints() > 0)
	    delete parc.points;
    }

    if (numPoints > 0)
    {
	/* set up and write the point elements */
	ply_put_element_setup (ply, "point");
	plyVertex pvert;
	for (int i=0; i<numPoints; i++)
	{	    
	    getPoint(i)->sample->fillPlyVertex(ply, &pvert);
	    pvert.radius = getPoint(i)->radius;
	    ply_put_element(ply, (void *) &pvert);
	}
    }
    
#if 0
    /* set up and write the bvnode elements */
    ply_put_element_setup(ply, "bvnode");
    plyBVNode pbvnode;
    for (int i=0; i<numNodes; i++)
    {
	ply_put_element(ply, (void *) &pbvnode);
    }
#endif

    ply_put_other_elements (ply);

    /* close the PLY file */
    ply_close(ply);
}




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
