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
#ifndef FOREST_H
#define FOREST_H

#include "vds.h"
#include "renderer.h"
#include "vif.h"

namespace VDS
{

class Forest
{
public: // PUBLIC FUNCTIONS
	Forest();
	virtual ~Forest();
	
	// This function gives the contents (nodes, tris) to another
	// Forest class. THIS Forest WILL BE RESET.  This is used to facilitate
	// things like using one Forest derived class to build a Forest and another 
	// to simplify/render. The receiving Forest will start off in the initial
	// state (as in after a SetValid call)
	void GiveContents(Forest &rForest);

	// Reads a Vif class into VDS data structures
	// Returns true if successful, false if error occurred
	bool GetDataFromVif(const Vif &v);

	// Dumps contents of Forest into given Vif
	// Returns true if successful, false if error occurred
	bool GiveDataToVif(Vif &v);

	// Reads All VDSdata structure data from a binary VDS file
	bool ReadBinaryVDS(const char *Filename);

	// Memory maps a binary VDS file instead of reading it into memory
	bool MemoryMapVDS(const char *Filename);

	// GLOD Readback Functions --- these are hacked copies of the Read/WriteBinaryVDS functions
	int GetBinaryVDSSize();
        int WriteBinaryVDStoBuffer(char* buffer);
        int ReadBinaryVDSfromBuffer(char* buffer);

	// Writes All VDSdata structure data to a binary VDS file
	bool WriteBinaryVDS(const char *Filename);

	// converts vertex renderdata member of nodes from indices to pointers
	void VertexRenderDataIndicesToPointers();

	// converts vertex renderdata member of nodes from pointers to indices
	void VertexRenderDataPointersToIndices();

	// Resets Forests member variables and frees all memory allocated by Forest.
	// User callbacks are retained.
	virtual void Reset();

	void GetBoundingBox(float &minx, float &maxx, float &miny, float &maxy, float &minz, float &maxz);

	// returns true if iNode1 and iNode2 are coincident or if iNode1 == iNode2
	bool NodesAreCoincidentOrEqual(NodeIndex iNode1, NodeIndex iNode2);

protected: // PRIVATE FUNCTIONS
		
	//Initializes Refs if needed and checks for existence of nodes and tris
	void SetValid();

	//Swaps any two nodes.  Only fixes node-node relationships and subtri lists.
	//Everything else must be fixed by the caller.
	void SwapNodes(NodeIndex iNode1, NodeIndex iNode2, TriIndex *FirstLiveTris);

	void SwapNodeMemory(NodeIndex iNode1, NodeIndex iNode2);

	//Reorders nodes in data structure to depth first
	void ReorderNodesDepthFirst(TriIndex *FirstLiveTris, TriIndex **NextLiveTris);
	void DFSvisit(NodeIndex i);
	void ForestComputeBBoxes(NodeIndex iNode, TriIndex *FirstLiveTris, TriIndex **NextLiveTris);

	NodeIndex first_ancestor_of(NodeIndex a, NodeIndex b);

public: // DEBUG FUNCTIONS
	void PrintForestInfo(Cut *pCut);
	void PrintForestStructure();
	void PrintNodeInfo(NodeIndex i, Cut *pCut, int tabs);
	void PrintTriInfo(Cut *pCut);

	bool CheckIfProxiesMatchProxyBackRefs(Renderer *pRenderer);
	void CheckForDuplicateNodeRefs();
	void CheckLiveTriListsC(TriIndex *FirstLiveTris, TriIndex **NextLiveTris);

public:	// PUBLIC DATA
	Node *mpNodes;
	VertexRenderDatum *mpNodeRenderData;
	Tri *mpTris;
	float *mpErrorParams;

	bool mNormalsPresent;
	bool mColorsPresent;
	unsigned int mNumTextures;	// (CURRENTLY ONLY SINGLE TEXTURE SUPPORTED)
	bool mIsValid;
	bool mIsMMapped;
#ifdef _WIN32
	BYTE *mMMapFile;
#else
	char *mMMapFile;
#endif
	NodeIndex mNumNodes;
	NodeIndex mNumNodePositions;
	TriIndex mNumTris;
	PatchIndex mNumPatches;
	NodeIndex mNumErrorParams;
	int mErrorParamSize;

protected: // PRIVATE DATA
	NodeIndex DFSindex;
	NodeIndex *DepthFirstArray;
	NodeIndex *LocInArray;
        
public: // DEBUG DATA
	NodeIndex miHighlightedNode;
	TriIndex miHighlightedTri;

public: //STATIC DATA
	static const NodeIndex iROOT_NODE;
	static const NodeIndex iNIL_NODE;
	static const NodeIndex iNIL_TRI;  
	static const unsigned int VDS_FILE_FORMAT_MAJOR;
	static const unsigned int VDS_FILE_FORMAT_MINOR;
	static const unsigned int VIF_FILE_FORMAT_MAJOR;
	static const unsigned int VIF_FILE_FORMAT_MINOR;
		
	//Friends
	friend class Node;
	friend class Tri;
};

void StdViewIndependentError(NodeIndex iNode, const Forest &Forest);

} // namespace VDS

#endif // #ifndef FOREST_H
