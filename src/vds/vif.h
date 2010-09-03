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
#ifndef VIF_H
#define VIF_H

#include "vds.h"
#include "primtypes.h"

// This class implements Vif version 2.3

struct VifMerge
{
	unsigned int NumNodesInMerge;	// number of child vertices
	unsigned int *NodesBeingMerged;	// array of child vertices' indices
	unsigned int ParentNode;		// index of parent vertex into which child vertices are merged
	unsigned int ErrorParamIndex;	// index of error parameter memory in 

	// This represents a collapse of the vertices whose indices (into the Vif's Vertices
	// array) are in NodesBeingMerged into the vertex whose index (into the Vif's Vertices
	// array) is ParentNode.
	// NOTE: None of the child vertices in NodesBeingMerged should be clones of each other
	// (i.e. both reference the same VertexPosition).
	// NOTE: if the ParentNode is a clone of one of the NodesBeingMerged, this results
	// in what used to be called a "cluster", where instead of being merged into a new parent
	// vertex, the nodes are collapsed into an existing vertex.
};

struct VifVertex
{
	unsigned int VertexPosition;	// index into Vif's VertexPositions array
	VDS::PatchIndex PatchID;	// patch of which this vertex is a part (must be
								// between 1 and NumPatches, inclusive)
	bool CoincidentVertexFlag;	// if true, this vertex is coincident with another vertex(ices)
	unsigned int CoincidentVertex;	// the next vertex in the looped list of vertices with which
									// this vertex is coincident
									// (ignored if CoincidentVertexFlag is false)
};

struct VifTri
{
	unsigned int Corners[3];	// indices into Vertices array
	VDS::PatchIndex PatchID;	// patch of which this triangle is a part
								// note that PatchID must be identical to the PatchID
								// of each of the vertices indexed by Corners
};

class Vif
{
public:
	Vif();
	~Vif();

	unsigned int NumVerts;	// number of vertices in the hierarchy (counting coincident
							// vertices separately and including those created by merges,
							// if any)
	unsigned int NumVertexPositions;	// number of vertex positions in the hierarchy - 
										// multiple vertices can point to the same vertex
										// position; thus, NumVertexPositions should be
										// less than or equal to NumVerts
	unsigned int NumTris;	// number of triangles in the hierarchy
	VDS::PatchIndex NumPatches;	// number of patches in the hierarchy

	// vertex information present
	bool ColorsPresent;		// if these are false, then the corresponding information
	bool NormalsPresent;	// (colors/normals) in VertexPositions is just ignored
	unsigned int NumTextures;

	VDS::VertexRenderDatum *VertexPositions;	// array of geometric positions
	VifVertex *Vertices;			// array of vertices
	VifTri *Triangles;				// array of triangles

	VDS::Point2 **TextureCoords;		// array of texture coordinates, if present
	// TextureCoords[i][j] are the j'th set of texture coordinates for vertex i
	// (if only one set of texture coords are defined, j will always be 0)

	unsigned int NumMerges;		// number of merges in hierarchy
	VifMerge *Merges;			// array of merges

	unsigned int NumErrorParams;			// number of elements in the error array
	int ErrorParamSize;			// size of each entry in the error parameters array
	float *ErrorParams;			// array of error parameters


	// Writes data to an ascii VIF2.2 format file
	// Returns true if write was successful, false if error occurred
	bool WriteVif2_2(const char *Filename);

	// Writes data to an ascii VIF2.3 format file
	// Returns true if write was successful, false if error occurred
	bool WriteVif2_3(const char *Filename);

	// Reads Vif from a VIF2.2 format file
	// return true if read was successful, false if error occurred
	bool ReadVif2_2(const char *Filename);

	// Reads Vif from a VIF2.3 format file
	// return true if read was successful, false if error occurred
	bool ReadVif2_3(const char *Filename);

	
	unsigned int maxVertexPositions;
	unsigned int maxVerts;
	unsigned int maxTris;
	unsigned int maxMerges;

	unsigned int addVertPos(VDS::Point3& coord, VDS::ByteColorA& color,
			     VDS::Vec3& normal, VDS::Point2*& texcoord);
	unsigned int addVert(unsigned int vpos, VDS::PatchIndex patchid, 
				 bool coincidentvertflag, unsigned int coincidentvert);
	unsigned int addTri(unsigned int v0, unsigned int v1, unsigned int v2,
				 VDS::PatchIndex patchid);
	unsigned int addMerge(VifMerge& merge);
};

#endif
