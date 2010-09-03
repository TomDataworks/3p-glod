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
#ifdef _WIN32
//stop compiler from complaining about exception handling being turned off
#pragma warning(disable: 4530)
// disable warning C4786: symbol greater than 255 character, okay to ignore, STL generates many of these.
#pragma warning(disable: 4786)
//stop compiler from complaining about exception handling being turned off
#pragma warning(disable: 4530)

#include <windows.h>
#endif

#include <cassert>
#include <queue>
#include <vector>
#include <time.h>
#include <set>
#include <stdio.h>

#include "vds.h"
#include "forest.h"
#include "node.h"
#include "tri.h"
#include "vdsaux.h"

using namespace std;
using namespace VDS;

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif

const NodeIndex Forest::iNIL_NODE  = 0;
const NodeIndex Forest::iNIL_TRI   = 0;
const NodeIndex Forest::iROOT_NODE = 1;
const unsigned int Forest::VDS_FILE_FORMAT_MAJOR = 1;
const unsigned int Forest::VDS_FILE_FORMAT_MINOR = 4;
const unsigned int Forest::VIF_FILE_FORMAT_MAJOR = 2;
const unsigned int Forest::VIF_FILE_FORMAT_MINOR = 1;

// utility function prototypes
void sort_three(NodeIndex &rA, NodeIndex &rB, NodeIndex &rC);
#ifdef _WIN32
void WriteArray(void *pArray, unsigned int Length, HANDLE FileHandle);
#else
void WriteArray(void *pArray, unsigned int Length, int FileHandle);
#endif

Forest::Forest()
{
    mpNodes = NULL;
	mpNodeRenderData = NULL;
	mpTris = NULL;
	mNormalsPresent = false;
	mColorsPresent = false;
	mNumTextures = 0;
    mIsValid = false;
    mIsMMapped = false;
	mMMapFile = NULL;
    mNumNodes = 0;
	mNumNodePositions = 0;
	mNumPatches = 0;
	mNumTris = 0;
	DepthFirstArray = NULL;
	LocInArray = NULL;
	miHighlightedNode = iNIL_NODE;
	miHighlightedTri = iNIL_TRI;
	mNumErrorParams = 0;
	mErrorParamSize = 0;
	mpErrorParams = NULL;
}

Forest::~Forest()
{
    Reset();
}

void Forest::GiveContents(Forest &rForest)
{
    assert (mIsValid);
    rForest.Reset();
    rForest.mpNodes = mpNodes;
	rForest.mpTris = mpTris;
    rForest.mNumNodes = mNumNodes;
	rForest.mNumTris = mNumTris;
    mpNodes = NULL;
    mpTris = NULL;
    mIsValid = false;
    mNumNodes = 0;
	mNumTris = 0;
    if (mIsMMapped)
    {
        rForest.mIsMMapped = true;
        rForest.mMMapFile = mMMapFile;
#ifdef _WIN32
	UnmapViewOfFile(mMMapFile);
#endif
    }
    mIsMMapped = false;
}

bool Forest::GetDataFromVif(const Vif &v)
{
	unsigned int i,j;
//    Point2 *tex_coords;
	Float mAvgEdgeLength;
	NodeIndex root;
	TriIndex tri;
    NodeIndex a, b, c;
    NodeIndex ab_first_ancestor, bc_first_ancestor;

	Reset();

	mColorsPresent = v.ColorsPresent;
	mNormalsPresent = v.NormalsPresent;
	mNumTextures = v.NumTextures;
	mNumNodePositions = v.NumVertexPositions;
	mNumNodes = v.NumVerts;
	mNumTris = v.NumTris;
	mNumPatches = v.NumPatches;
	mNumErrorParams = v.NumErrorParams;
	if (mNumErrorParams > 0)
	{
		mErrorParamSize = v.ErrorParamSize;
		mpErrorParams = new float[mNumErrorParams * mErrorParamSize];
	}
	else
	{
		mErrorParamSize = 0;
		mpErrorParams = NULL;
	}

	mAvgEdgeLength = 0.0;

	mpNodeRenderData = new VertexRenderDatum[mNumNodePositions];
	for (i = 0; i < mNumNodePositions; ++i)
	{
		memcpy(&mpNodeRenderData[i], &v.VertexPositions[i], sizeof(VertexRenderDatum));
		for (j = 0; j < mNumTextures; ++j)
		{
			// vds currently only supports one set of texture coords,
			// so only copy the first set from the vif
			if (j == 0)
				mpNodeRenderData[i].TexCoords = v.TextureCoords[i][j];
		}
	}

	mpNodes = new Node[mNumNodes + 1];
	TriIndex *FirstLiveTris = new TriIndex[mNumNodes + 1];
	for (i = 0; i <= mNumNodes; ++i)
	{
		mpNodes[i].miParent = iNIL_NODE;
		mpNodes[i].miLeftSibling = iNIL_NODE;
		mpNodes[i].miRightSibling = iNIL_NODE;
		mpNodes[i].miFirstChild = iNIL_NODE;
		mpNodes[i].miFirstSubTri = iNIL_TRI;
//		mpNodes[i].mRadius = 0.0f;
		mpNodes[i].mXBBoxOffset = 0;
		mpNodes[i].mYBBoxOffset = 0;
		mpNodes[i].mZBBoxOffset = 0;
		mpNodes[i].mBBoxCenter.X = 0;
		mpNodes[i].mBBoxCenter.Y = 0;
		mpNodes[i].mBBoxCenter.Z = 0;
		mpNodes[i].mCoincidentVertex = iNIL_NODE;
		FirstLiveTris[i] = iNIL_TRI;
	}

	for (i = 1; i <= mNumNodes; ++i)
	{
		mpNodes[i].mpRenderData = &(mpNodeRenderData[v.Vertices[i-1].VertexPosition]);
		mpNodes[i].mPatchID = v.Vertices[i-1].PatchID - 1;
		if (v.Vertices[i-1].CoincidentVertexFlag)
		{
			mpNodes[i].mCoincidentVertex = v.Vertices[i-1].CoincidentVertex + 1;
		}
		else
		{
			mpNodes[i].mCoincidentVertex = iNIL_NODE;
		}

		if (mpNodes[i].mPatchID >= mNumPatches)
		{
			cerr << "Error - node " << i << " has PatchID out of range." << endl;
			return false;
		}

	}

	unsigned int node;
    for (node = 0; node < v.NumVerts; node++)
    {
		if (v.Vertices[node].CoincidentVertexFlag)
		{
			i = node;
			if (v.Vertices[i].CoincidentVertex == i)
			{
				cerr << "Error - Coincident vertex points to self." << endl;
				return false;
			}
			while (v.Vertices[i].CoincidentVertex != node)
			{
				if (!v.Vertices[i].CoincidentVertexFlag)
				{
					cerr << "Error - Coincident vertex doesn't have coincident vertex flag set." << endl;
					return false;
				}
				i = v.Vertices[i].CoincidentVertex;
			}
		}
	}

	mpTris = new Tri[mNumTris + 1];
	TriIndex **NextLiveTris = new TriIndex*[mNumTris + 1];
	for (i = 1; i <= mNumTris; ++i)
	{
		mpTris[i].miNextSubTri = iNIL_TRI;
		mpTris[i].miCorners[0] = iNIL_TRI;
		mpTris[i].miCorners[1] = iNIL_TRI;
		mpTris[i].miCorners[2] = iNIL_TRI;

		NextLiveTris[i] = new TriIndex[3];
		NextLiveTris[i][0] = iNIL_TRI;
		NextLiveTris[i][1] = iNIL_TRI;
		NextLiveTris[i][2] = iNIL_TRI;
	}

	for (i = 1; i <= mNumTris; ++i)
	{
		mpTris[i].miCorners[0] = v.Triangles[i-1].Corners[0] + 1;
		mpTris[i].miCorners[1] = v.Triangles[i-1].Corners[1] + 1;
		mpTris[i].miCorners[2] = v.Triangles[i-1].Corners[2] + 1;
		mpTris[i].mPatchID = v.Triangles[i-1].PatchID - 1;
		mpTris[i].AddToLiveTriListUsingCorners(i, 0, *this, FirstLiveTris, NextLiveTris);
		mpTris[i].AddToLiveTriListUsingCorners(i, 1, *this, FirstLiveTris, NextLiveTris);
		mpTris[i].AddToLiveTriListUsingCorners(i, 2, *this, FirstLiveTris, NextLiveTris);

		for (j = 0; j < 3; j++)
		{
			mAvgEdgeLength += mpNodes[mpTris[i].miCorners[j]].mpRenderData->Position.DistanceTo(mpNodes[mpTris[i].miCorners[(j + 1) % 3]].mpRenderData->Position);
		}

		if ((mpTris[i].mPatchID != mpNodes[mpTris[i].miCorners[0]].mPatchID)
			|| (mpTris[i].mPatchID != mpNodes[mpTris[i].miCorners[1]].mPatchID)
			|| (mpTris[i].mPatchID != mpNodes[mpTris[i].miCorners[2]].mPatchID))
		{
			cerr << "Error - triangle " << i << " has different PatchID than one of its corners." << endl;
			return false;
		}

		if (mpTris[i].mPatchID >= mNumPatches)
		{
			cerr << "Error - triangle " << i << " has PatchID out of range." << endl;
			return false;
		}
	}

	mAvgEdgeLength /= ((Float) mNumTris * 3.0);
	for (i = 0; i < v.NumMerges; ++i)
	{
		mpNodes[v.Merges[i].ParentNode+1].miErrorParamIndex = v.Merges[i].ErrorParamIndex;
		if (v.Merges[i].NumNodesInMerge <= 0)
		{
			cerr << "Error - Merge " << i << " has NumNodesInMerge = " << v.Merges[i].NumNodesInMerge << endl;
			return false;
		}
		mpNodes[v.Merges[i].ParentNode+1].miFirstChild = v.Merges[i].NodesBeingMerged[0]+1;
		for (j = 0; j < v.Merges[i].NumNodesInMerge; ++j)
		{
			mpNodes[v.Merges[i].NodesBeingMerged[j]+1].miParent = v.Merges[i].ParentNode+1;
			if (j != (v.Merges[i].NumNodesInMerge - 1))
				mpNodes[v.Merges[i].NodesBeingMerged[j]+1].miRightSibling = v.Merges[i].NodesBeingMerged[j+1]+1;
			if (j != 0)
				mpNodes[v.Merges[i].NodesBeingMerged[j]+1].miLeftSibling = v.Merges[i].NodesBeingMerged[j-1]+1;
		}
		mpNodes[v.Merges[i].NodesBeingMerged[v.Merges[i].NumNodesInMerge-1]+1].miRightSibling = iNIL_NODE;
		mpNodes[v.Merges[i].NodesBeingMerged[0]+1].miLeftSibling = iNIL_NODE;
	}
	if (v.NumMerges == 0)
	{
		// TODO: need to put nodes in node vector and call cluster_octtree
		fprintf(stderr, "Error - Hierarchy not found or unusable; Forest::GetDataFromVif() hierarchy generation not implemented yet.\n");
		return false;
	}

	root = 1;
	while (mpNodes[root].miParent != iNIL_NODE)
	{
		root = mpNodes[root].miParent;
	}
	SwapNodes(iROOT_NODE, root, FirstLiveTris);

	ReorderNodesDepthFirst(FirstLiveTris, NextLiveTris);

	for (tri = 1; tri <= mNumTris; tri++)
    {
        std::vector<NodeIndex> vec;
        a = mpTris[tri].miCorners[0];
        b = mpTris[tri].miCorners[1];
        c = mpTris[tri].miCorners[2];
        sort_three(a, b, c);        
        //to find the ancestor climb Forest from greater ID node
        //until a node with ID > lesser ID node is reached.
        bc_first_ancestor = mpNodes[c].miParent;
        while (bc_first_ancestor > b)
        {        
            bc_first_ancestor = mpNodes[bc_first_ancestor].miParent;
        }
        
        if (bc_first_ancestor >= a) //found node tri is subtri of
        {
            mpTris[tri].AddToSubTriList(tri, bc_first_ancestor, *this);
        }
        else //the node the tri is a subtri of is ancestor of a,b
        {       
            ab_first_ancestor = mpNodes[b].miParent;
            while (ab_first_ancestor > a)
            {        
                ab_first_ancestor = mpNodes[ab_first_ancestor].miParent;
			}
            mpTris[tri].AddToSubTriList(tri, ab_first_ancestor, *this);
        }
    }	

	if (mNumErrorParams > 0)
	{
		for (i = iROOT_NODE; i <= mNumNodes; ++i)
		{
			if (mpNodes[i].miFirstChild == iNIL_NODE)
				mpNodes[i].miErrorParamIndex = 0;
		}
		if (mpErrorParams != NULL)
			delete[] mpErrorParams;
		mpErrorParams = new float[mNumErrorParams * mErrorParamSize];
		memcpy(mpErrorParams, v.ErrorParams, mNumErrorParams * mErrorParamSize * sizeof(float));
	}
	else
	{
		int numinteriornodes = 0;
		for (i = iROOT_NODE; i <= mNumNodes; ++i)
		{
			if (mpNodes[i].miFirstChild != iNIL_NODE)
				++numinteriornodes;
		}

		if (mpErrorParams != NULL)
			delete[] mpErrorParams;
		mpErrorParams = new float[numinteriornodes + 1];
		mErrorParamSize = 1;

		ForestComputeBBoxes(iROOT_NODE, FirstLiveTris, NextLiveTris);
		float bboxdiag2 = 0;
		mpErrorParams[0] = 0.0f;
		int errorindexassigned = 1;
		for (i = iROOT_NODE; i <= mNumNodes; ++i)
		{
			if (mpNodes[i].miFirstChild != iNIL_NODE)
			{
				bboxdiag2 = (mpNodes[i].mXBBoxOffset * mpNodes[i].mXBBoxOffset) +
					(mpNodes[i].mYBBoxOffset * mpNodes[i].mYBBoxOffset) +
					(mpNodes[i].mZBBoxOffset * mpNodes[i].mZBBoxOffset);
				mpNodes[i].miErrorParamIndex = errorindexassigned;
				mpErrorParams[errorindexassigned] = sqrt(bboxdiag2);
				++errorindexassigned;
			}
			else
				mpNodes[i].miErrorParamIndex = 0;
		}
		mNumErrorParams = errorindexassigned;
	}
	SetValid();

	delete[] FirstLiveTris;
	FirstLiveTris = NULL;
	for (i = 1; i <= mNumTris; ++i)
	{
		delete[] NextLiveTris[i];
	}
	delete[] NextLiveTris;
	NextLiveTris = NULL;

	return true;
}

bool Forest::GiveDataToVif(Vif &v)
{
    // VIF vertex indices start at 0 NOT iFIRST_VALID_NODE!!

	unsigned int i, j;
	
	v.ColorsPresent = mColorsPresent;
	v.NormalsPresent = mNormalsPresent;
	v.NumTextures = mNumTextures;
	v.NumVertexPositions = mNumNodePositions;
	v.NumVerts = mNumNodes;
	v.NumTris = mNumTris;
	v.NumPatches = mNumPatches;
	v.NumErrorParams = mNumErrorParams;
	v.ErrorParamSize = mErrorParamSize;

	v.NumVertexPositions = mNumNodePositions;
	v.VertexPositions = new VertexRenderDatum[v.NumVertexPositions];
	for (i = 0; i < mNumNodePositions; ++i)
	{
		memcpy(&v.VertexPositions[i], &mpNodeRenderData[i], sizeof(VertexRenderDatum));
	}

	v.Vertices = new VifVertex[v.NumVerts];
	for (i = 0; i < mNumNodes; ++i)
	{
		v.Vertices[i].VertexPosition = mpNodes[i+1].mpRenderData - mpNodeRenderData;
		v.Vertices[i].PatchID = mpNodes[i+1].mPatchID + 1;
		if (mpNodes[i+1].mCoincidentVertex == iNIL_NODE)
		{
			v.Vertices[i].CoincidentVertexFlag = false;
			v.Vertices[i].CoincidentVertex = 666666;
		}
		else
		{
			v.Vertices[i].CoincidentVertexFlag = true;
			v.Vertices[i].CoincidentVertex = mpNodes[i+1].mCoincidentVertex - 1;
		}
	}

	unsigned int node;
    for (node = 0; node < v.NumVerts; node++)
    {
		if (v.Vertices[node].CoincidentVertexFlag)
		{
			i = node;
			if (v.Vertices[i].CoincidentVertex == i)
			{
				cerr << "Error - Coincident vertex points to self." << endl;
				return false;
			}
			while (v.Vertices[i].CoincidentVertex != node)
			{
				if (!v.Vertices[i].CoincidentVertexFlag)
				{
					cerr << "Error - Coincident vertex doesn't have coincident vertex flag set." << endl;
					return false;
				}
				i = v.Vertices[node].CoincidentVertex;
			}
		}
	}

	v.Triangles = new VifTri[v.NumTris];
	for (i = 0; i < mNumTris; ++i)
	{
		v.Triangles[i].Corners[0] = mpTris[i+1].miCorners[0] - 1;
		v.Triangles[i].Corners[1] = mpTris[i+1].miCorners[1] - 1;
		v.Triangles[i].Corners[2] = mpTris[i+1].miCorners[2] - 1;
		v.Triangles[i].PatchID = mpTris[i+1].mPatchID + 1;
	}

	if (v.ErrorParams != NULL)
		delete[] v.ErrorParams;
	v.ErrorParams = new float[mNumErrorParams * mErrorParamSize];
	memcpy(v.ErrorParams, mpErrorParams, mNumErrorParams * mErrorParamSize * sizeof(float));

	v.NumMerges = 0;
	for (i = 1; i <= mNumNodes; ++i)
	{
		if (mpNodes[i].miFirstChild != iNIL_NODE)
			++v.NumMerges;
	}
	v.Merges = new VifMerge[v.NumMerges];
	unsigned int merge = 0;
	NodeIndex child = 0;
	unsigned int numnodesinmerge = 0;
	for (i = 1; i <= mNumNodes; ++i)
	{
		if (mpNodes[i].miFirstChild != iNIL_NODE)
		{
			numnodesinmerge = 0;
			child = mpNodes[i].miFirstChild;
			while (child != iNIL_NODE)
			{
				++numnodesinmerge;
				child = mpNodes[child].miRightSibling;
			}
			v.Merges[merge].NumNodesInMerge = numnodesinmerge;
			v.Merges[merge].ParentNode = i - 1;
			v.Merges[merge].ErrorParamIndex = mpNodes[i].miErrorParamIndex;
			v.Merges[merge].NodesBeingMerged = new unsigned int[numnodesinmerge];
			child = mpNodes[i].miFirstChild;
			for (j = 0; j < numnodesinmerge; ++j)
			{
				v.Merges[merge].NodesBeingMerged[j] = child - 1;
				child = mpNodes[child].miRightSibling;
			}
			++merge;
		}
	}
	return true;
}

int Forest::ReadBinaryVDSfromBuffer(char* buffer)
{
  char* cur_buf = buffer;
  DWORD num_bytes_read = 0;
  unsigned int major, minor;

  Reset();
  mIsMMapped = false;
	
#define READ(a,b,c,d,e) memcpy((void*)b, cur_buf, c); cur_buf += c; *d += c;
  READ(hFile, &major, sizeof(VDS_FILE_FORMAT_MAJOR), &num_bytes_read, NULL);
  READ(hFile, &minor, sizeof(VDS_FILE_FORMAT_MINOR), &num_bytes_read, NULL);
  if (major != VDS_FILE_FORMAT_MAJOR || minor != VDS_FILE_FORMAT_MINOR)
  {
    cerr << "Incompatible VDS file version." << endl;
    return 0;
  }    
  
  READ(hFile, &mColorsPresent, sizeof(mColorsPresent), &num_bytes_read, NULL);
  READ(hFile, &mNormalsPresent, sizeof(mNormalsPresent), &num_bytes_read, NULL);
  READ(hFile, &mNumTextures, sizeof(mNumTextures), &num_bytes_read, NULL);
  READ(hFile, &mNumNodes, sizeof(NodeIndex), &num_bytes_read, NULL);
  READ(hFile, &mNumNodePositions, sizeof(NodeIndex), &num_bytes_read, NULL);
  READ(hFile, &mNumTris, sizeof(NodeIndex), &num_bytes_read, NULL);
  READ(hFile, &mNumPatches, sizeof(PatchIndex), &num_bytes_read, NULL);
  READ(hFile, &mNumErrorParams, sizeof(NodeIndex), &num_bytes_read, NULL);
  READ(hFile, &mErrorParamSize, sizeof(int), &num_bytes_read, NULL);
  
  mpErrorParams = new float[mNumErrorParams * mErrorParamSize];
  READ(hFile, mpErrorParams, mErrorParamSize * mNumErrorParams * sizeof(float), &num_bytes_read, NULL);
  
  mpNodes = new Node[mNumNodes + 1];
  READ(hFile, mpNodes, sizeof(Node) * (mNumNodes + 1), &num_bytes_read, NULL);

  mpNodeRenderData = new VertexRenderDatum[mNumNodePositions];
  READ(hFile, mpNodeRenderData, sizeof(VertexRenderDatum) * (mNumNodePositions), &num_bytes_read, NULL);
  
  mpTris = new Tri[mNumTris + 1];
  READ(hFile, mpTris, sizeof(Tri) * (mNumTris + 1), &num_bytes_read,NULL);
    
  VertexRenderDataIndicesToPointers();

  SetValid();
  return 1;
}

bool Forest::ReadBinaryVDS(const char *Filename)
{
#ifdef _WIN32
    HANDLE hFile; 
#else
    int hFile;
#endif
    DWORD num_bytes_read;
    unsigned int major, minor;

    Reset();
    mIsMMapped = false;
#ifdef _WIN32	
    hFile = CreateFile(Filename,       // create Filename
		GENERIC_READ,                // open for reading 
		0,                            // do not share
		NULL,                         // no security 
		OPEN_EXISTING,                // open existing file only 
		FILE_ATTRIBUTE_NORMAL,        // normal file
		NULL);                        // no attr. template 

    if (hFile == INVALID_HANDLE_VALUE) 
    {
        return false;
    } 
	
    ReadFile(hFile, &major, sizeof(VDS_FILE_FORMAT_MAJOR), &num_bytes_read, NULL);
    ReadFile(hFile, &minor, sizeof(VDS_FILE_FORMAT_MINOR), &num_bytes_read, NULL);
    if (major != VDS_FILE_FORMAT_MAJOR || minor != VDS_FILE_FORMAT_MINOR)
    {
        cerr << "Incompatible VDS file version." << endl;
        CloseHandle(hFile);
        return false;
    }    
	
    ReadFile(hFile, &mColorsPresent, sizeof(mColorsPresent), &num_bytes_read, NULL);
    ReadFile(hFile, &mNormalsPresent, sizeof(mNormalsPresent), &num_bytes_read, NULL);
    ReadFile(hFile, &mNumTextures, sizeof(mNumTextures), &num_bytes_read, NULL);
    ReadFile(hFile, &mNumNodes, sizeof(NodeIndex), &num_bytes_read, NULL);
	ReadFile(hFile, &mNumNodePositions, sizeof(NodeIndex), &num_bytes_read, NULL);
    ReadFile(hFile, &mNumTris, sizeof(NodeIndex), &num_bytes_read, NULL);
	ReadFile(hFile, &mNumPatches, sizeof(PatchIndex), &num_bytes_read, NULL);
	ReadFile(hFile, &mNumErrorParams, sizeof(NodeIndex), &num_bytes_read, NULL);
	ReadFile(hFile, &mErrorParamSize, sizeof(int), &num_bytes_read, NULL);
    
	mpErrorParams = new float[mNumErrorParams * mErrorParamSize];
    ReadFile(hFile, mpErrorParams, mNumErrorParams * mErrorParamSize * sizeof(float), &num_bytes_read, NULL);

	mpNodes = new Node[mNumNodes + 1];
    ReadFile(hFile, mpNodes, sizeof(Node) * (mNumNodes + 1), &num_bytes_read, NULL);

	mpNodeRenderData = new VertexRenderDatum[mNumNodePositions];
	ReadFile(hFile, mpNodeRenderData, sizeof(VertexRenderDatum) * (mNumNodePositions), &num_bytes_read, NULL);

    mpTris = new Tri[mNumTris + 1];
    ReadFile(hFile, mpTris, sizeof(Tri) * (mNumTris + 1), &num_bytes_read,NULL);
    
    CloseHandle(hFile);
	
	VertexRenderDataIndicesToPointers();

    SetValid();
    return true;
#else
    hFile = 0;
    num_bytes_read = major = minor = 0;
    fprintf(stderr, "file i/o not implemented for non-win32");
    return false;
#endif
}

bool Forest::MemoryMapVDS(const char *Filename)
{
#ifdef _WIN32
    HANDLE hFile;
    HANDLE hFileMapping;
    unsigned int major, minor;
    unsigned int offset;
//    unsigned int i;
    Reset();
    
    hFile = CreateFile(Filename,       // create Filename 
		GENERIC_READ | GENERIC_WRITE,                // open for reading 
		0,                            // do not share 
		NULL,                         // no security 
		OPEN_ALWAYS,                // open existing file only 
		FILE_ATTRIBUTE_NORMAL,        // normal file
		NULL);                        // no attr. template 
	
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        return false;
    } 
    
    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_WRITECOPY, 0, 0, NULL);
    if (hFileMapping == INVALID_HANDLE_VALUE || hFileMapping == NULL)
    {      
        CloseHandle(hFile);        
        return false;
    }
    mMMapFile = (PBYTE) MapViewOfFile(hFileMapping, FILE_MAP_COPY, 0, 0, 0);
    offset = 0;
    major = *((unsigned int *)mMMapFile);
    offset += sizeof(VDS_FILE_FORMAT_MAJOR + offset);
    minor = *((unsigned int *)(mMMapFile + offset));
    offset += sizeof(VDS_FILE_FORMAT_MINOR);
	
    if (major != VDS_FILE_FORMAT_MAJOR || minor != VDS_FILE_FORMAT_MINOR)
    {
        cerr << "Incompatible VDS file version." << endl;
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        UnmapViewOfFile(mMMapFile);
        return false;
    }
    mIsMMapped = true;
    mColorsPresent = *((bool *) (mMMapFile + offset));
    offset += sizeof(mColorsPresent);
    mNormalsPresent = *((bool *) (mMMapFile + offset));
    offset += sizeof(mNormalsPresent);
    mNumTextures = *((unsigned int *) (mMMapFile + offset));
    offset += sizeof(mNumTextures);
    mNumNodes = *((NodeIndex *) (mMMapFile + offset));
    offset += sizeof(mNumNodes);
	mNumNodePositions = *((NodeIndex *) (mMMapFile + offset));
	offset += sizeof(mNumNodePositions);
    mNumTris = *((TriIndex *) (mMMapFile + offset));
    offset += sizeof(mNumTris);
	mNumPatches = *((PatchIndex *) (mMMapFile + offset));
	offset += sizeof(mNumPatches);
	mNumErrorParams = *((NodeIndex *) (mMMapFile + offset));
	offset += sizeof(mNumErrorParams);
	mErrorParamSize = *((int *) (mMMapFile + offset));
	offset += sizeof(mErrorParamSize);
	
	// TODO: set up memory mapping to use interleaved render data instead of parallel arrays
	
	mpErrorParams = (float *) (mMMapFile + offset);
	offset += (mNumErrorParams * mErrorParamSize * sizeof(float));
    mpNodes = (Node *) (mMMapFile + offset);
    offset += (mNumNodes + 1) * sizeof(Node);
    mpNodeRenderData = (VertexRenderDatum *) (mMMapFile + offset);
    offset += (mNumNodePositions) * sizeof(VertexRenderDatum);
    mpTris = (Tri *) (mMMapFile + offset);
    offset += (mNumTris + 1) * sizeof(Tri);
	
	VertexRenderDataIndicesToPointers();

    CloseHandle(hFileMapping);
    CloseHandle(hFile);
    SetValid();
    return true;
#else
    fprintf(stderr, "memory mapping not implemented for non-win32");
    return false;
#endif
}

int Forest::GetBinaryVDSSize() // added by Nat for GLOD compatibility on 8/29/03
                               // GLOD must know in-advance the size of the VDS file that will be created.
{
    DWORD num_bytes_written = 0;
//    unsigned int i;
	
    assert(mIsValid);
#define COUNT_BYTE(a,b,c,d,e) num_bytes_written += c;
#define COUNT_ARRAY(a,b,c) num_bytes_written += b;
    
    COUNT_BYTE(hFile, &VDS_FILE_FORMAT_MAJOR, sizeof(VDS_FILE_FORMAT_MAJOR), &num_bytes_written,NULL);
    COUNT_BYTE(hFile, &VDS_FILE_FORMAT_MINOR, sizeof(VDS_FILE_FORMAT_MINOR), &num_bytes_written,NULL);
    
    COUNT_BYTE(hFile, &mColorsPresent, sizeof(mColorsPresent), &num_bytes_written, NULL);
    COUNT_BYTE(hFile, &mNormalsPresent, sizeof(mNormalsPresent), &num_bytes_written, NULL);
    COUNT_BYTE(hFile, &mNumTextures, sizeof(mNumTextures), &num_bytes_written, NULL);
    COUNT_BYTE(hFile, &mNumNodes, sizeof(NodeIndex), &num_bytes_written, NULL);
    COUNT_BYTE(hFile, &mNumNodePositions, sizeof(NodeIndex), &num_bytes_written, NULL);
    COUNT_BYTE(hFile, &mNumTris, sizeof(NodeIndex), &num_bytes_written, NULL);
    COUNT_BYTE(hFile, &mNumPatches, sizeof(PatchIndex), &num_bytes_written, NULL);
	COUNT_BYTE(hFile, &mNumErrorParams, sizeof(NodeIndex), &num_bytes_written, NULL);
	COUNT_BYTE(hFile, &mErrorParamSize, sizeof(int), &num_bytes_written, NULL);
    
	COUNT_ARRAY(mpErrorParams, mNumErrorParams * mErrorParamSize * sizeof(float), hFile);

    VertexRenderDataPointersToIndices();
    COUNT_ARRAY(mpNodes, sizeof(Node) * (mNumNodes + 1), hFile);
    VertexRenderDataIndicesToPointers();

    COUNT_ARRAY(mpNodeRenderData, sizeof(VertexRenderDatum) * (mNumNodePositions), hFile);

    COUNT_ARRAY(mpTris, sizeof(Tri) * (mNumTris + 1), hFile);

    return num_bytes_written;
}

int Forest::WriteBinaryVDStoBuffer(char* buffer) { /* Added by Nat for GLOD compat on 8/29/03 */
  char* cur_buf = buffer;
  DWORD num_bytes_written=0;
//  unsigned int i;
	
  assert(mIsValid);

#define WRITE(a,b,c,d,e) memcpy(cur_buf, b, c); cur_buf += c; *d += c;
#define WRITE_A(a,b,c) memcpy(cur_buf, a, b); cur_buf += b;

  WRITE(hFile, &VDS_FILE_FORMAT_MAJOR, sizeof(VDS_FILE_FORMAT_MAJOR), &num_bytes_written,NULL);
  WRITE(hFile, &VDS_FILE_FORMAT_MINOR, sizeof(VDS_FILE_FORMAT_MINOR), &num_bytes_written,NULL);
  
  WRITE(hFile, &mColorsPresent, sizeof(mColorsPresent), &num_bytes_written, NULL);
  WRITE(hFile, &mNormalsPresent, sizeof(mNormalsPresent), &num_bytes_written, NULL);
  WRITE(hFile, &mNumTextures, sizeof(mNumTextures), &num_bytes_written, NULL);
  WRITE(hFile, &mNumNodes, sizeof(NodeIndex), &num_bytes_written, NULL);
  WRITE(hFile, &mNumNodePositions, sizeof(NodeIndex), &num_bytes_written, NULL);
  WRITE(hFile, &mNumTris, sizeof(NodeIndex), &num_bytes_written, NULL);
  WRITE(hFile, &mNumPatches, sizeof(PatchIndex), &num_bytes_written, NULL);
  WRITE(hFile, &mNumErrorParams, sizeof(NodeIndex), &num_bytes_written, NULL);
  WRITE(hFile, &mErrorParamSize, sizeof(int), &num_bytes_written, NULL);

  WRITE_A(mpErrorParams, mNumErrorParams * mErrorParamSize * sizeof(float), hFile);
  
  VertexRenderDataPointersToIndices();
  WRITE_A(mpNodes, sizeof(Node) * (mNumNodes + 1), hFile);
  VertexRenderDataIndicesToPointers();
  
  WRITE_A(mpNodeRenderData, sizeof(VertexRenderDatum) * (mNumNodePositions), hFile);
  
  WRITE_A(mpTris, sizeof(Tri) * (mNumTris + 1), hFile);
  
  return true;
}

/***************** NOTE: KEEP the above two functions in SYNC
 *****************       with this function!                   */
bool Forest::WriteBinaryVDS(const char *Filename)
{
#ifdef _WIN32
    HANDLE hFile; 
    DWORD num_bytes_written;
//    unsigned int i;
	
    assert(mIsValid);
    hFile = CreateFile(Filename,       // create
		GENERIC_WRITE,                // open for writing 
		0,                            // do not share 
		NULL,                         // no security 
		CREATE_ALWAYS,                // overwrite existing 
		FILE_ATTRIBUTE_NORMAL,        // normal file
		NULL);                        // no attr. template 
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    
    WriteFile(hFile, &VDS_FILE_FORMAT_MAJOR, sizeof(VDS_FILE_FORMAT_MAJOR), &num_bytes_written,NULL);
    WriteFile(hFile, &VDS_FILE_FORMAT_MINOR, sizeof(VDS_FILE_FORMAT_MINOR), &num_bytes_written,NULL);
    
    WriteFile(hFile, &mColorsPresent, sizeof(mColorsPresent), &num_bytes_written, NULL);
    WriteFile(hFile, &mNormalsPresent, sizeof(mNormalsPresent), &num_bytes_written, NULL);
    WriteFile(hFile, &mNumTextures, sizeof(mNumTextures), &num_bytes_written, NULL);
    WriteFile(hFile, &mNumNodes, sizeof(NodeIndex), &num_bytes_written, NULL);
    WriteFile(hFile, &mNumNodePositions, sizeof(NodeIndex), &num_bytes_written, NULL);
    WriteFile(hFile, &mNumTris, sizeof(NodeIndex), &num_bytes_written, NULL);
    WriteFile(hFile, &mNumPatches, sizeof(PatchIndex), &num_bytes_written, NULL);
	WriteFile(hFile, &mNumErrorParams, sizeof(NodeIndex), &num_bytes_written, NULL);
	WriteFile(hFile, &mErrorParamSize, sizeof(int), &num_bytes_written, NULL);

	WriteArray(mpErrorParams, mNumErrorParams * mErrorParamSize * sizeof(float), hFile);
    
    VertexRenderDataPointersToIndices();
    WriteArray(mpNodes, sizeof(Node) * (mNumNodes + 1), hFile);
    VertexRenderDataIndicesToPointers();

    WriteArray(mpNodeRenderData, sizeof(VertexRenderDatum) * (mNumNodePositions), hFile);

    WriteArray(mpTris, sizeof(Tri) * (mNumTris + 1), hFile);
    CloseHandle(hFile);

    return true;
#else
    fprintf(stderr, "vds binary writing not implemented for non-win32");
    return false;
#endif
}

void Forest::VertexRenderDataIndicesToPointers()
{
	unsigned int i;
	for (i = 1; i <= mNumNodes; ++i)
	{
		mpNodes[i].mpRenderData = &(mpNodeRenderData[((unsigned int)mpNodes[i].mpRenderData)]);
	}
}

void Forest::VertexRenderDataPointersToIndices()
{
	unsigned int i;
	for (i = 1; i <= mNumNodes; ++i)
	{
		mpNodes[i].mpRenderData = (VertexRenderDatum*)(mpNodes[i].mpRenderData - mpNodeRenderData);
	}
}

void Forest::Reset()
{
    if (mIsMMapped)
	{
#ifdef _WIN32
		VertexRenderDataPointersToIndices();
        UnmapViewOfFile(mMMapFile);
#else
		(void) 1;
#endif
	}
    else
    {
        if (mpNodes != NULL)
		{
            delete[] mpNodes;
		}
        if (mpNodeRenderData != NULL)
		{
            delete[] mpNodeRenderData;
		}
        if (mpTris != NULL)
		{
            delete[] mpTris;
		}
		if (mpErrorParams != NULL)
		{
			delete[] mpErrorParams;
		}
    }
    mpNodes = NULL;
	mpNodeRenderData = NULL;
	mpTris = NULL;
	mpErrorParams = NULL;
	mColorsPresent = false;
	mNormalsPresent = false;
	mNumTextures = 0;
    mIsValid = false;
    mNumNodes = 0;
	mNumTris = 0;
	mNumErrorParams = 0;
	mErrorParamSize = 0;
    mIsMMapped = false;
	mMMapFile = NULL;
	miHighlightedNode = iNIL_NODE;
	miHighlightedTri = iNIL_TRI;
}

void Forest::GetBoundingBox(float &minx, float &maxx, float &miny, float &maxy, float &minz, float &maxz)
{
	unsigned int i;
	minx = miny = minz = 1e10;
	maxx = maxy = maxz = -1e10;
	for (i = 1; i <= mNumNodes; ++i)
	{
		if (mpNodes[i].mpRenderData->Position.X < minx)
			minx = mpNodes[i].mpRenderData->Position.X;
		if (mpNodes[i].mpRenderData->Position.X > maxx)
			maxx = mpNodes[i].mpRenderData->Position.X;
		if (mpNodes[i].mpRenderData->Position.Y < miny)
			miny = mpNodes[i].mpRenderData->Position.Y;
		if (mpNodes[i].mpRenderData->Position.Y > maxy)
			maxy = mpNodes[i].mpRenderData->Position.Y;
		if (mpNodes[i].mpRenderData->Position.Z < minz)
			minz = mpNodes[i].mpRenderData->Position.Z;
		if (mpNodes[i].mpRenderData->Position.Z > maxz)
			maxz = mpNodes[i].mpRenderData->Position.Z;
	}
	if (minx == 1e10)
		minx = 0;
	if (maxx == -1e10)
		maxx = 0;
	if (miny == 1e10)
		miny = 0;
	if (maxy == -1e10)
		maxy = 0;
	if (minz == 1e10)
		minz = 0;
	if (maxz == -1e10)
		maxz = 0;
}

bool Forest::NodesAreCoincidentOrEqual(NodeIndex iNode1, NodeIndex iNode2)
{
	if (iNode1 == iNode2)
		return true;

	// if indices are outside of the range of valid nodes, they obviously cannot be coincident
	if ((iNode1 > mNumNodes) || (iNode2 > mNumNodes))
		return false;

	// if either of the indices is the null node, they obviously cannot be coincident
	if ((iNode1 == iNIL_NODE) || (iNode2 == iNIL_NODE))
		return false;

	// if iNode1 has no coincident nodes, they obviously cannot be coincident
	if (mpNodes[iNode1].mCoincidentVertex == iNIL_NODE)
		return false;

	NodeIndex i = iNode1;
	while (mpNodes[i].mCoincidentVertex != iNode1)
	{
		i = mpNodes[i].mCoincidentVertex;
		if (i == iNode2)
			return true;
	}
	return false;
}

void Forest::SetValid()
{
    if (!(mNumNodes == 0 || mNumTris == 0))
    {
        mIsValid = true;
    }
    else
    {
        mIsValid = false;
    }
}

// this function now only for use in preprocessing stage
// currently only used to swap root node to index 1
void Forest::SwapNodes(NodeIndex iNode1, NodeIndex iNode2, TriIndex *FirstLiveTris)
{
    NodeIndex child;
    if (iNode1 != iNode2)
    {

		NodeIndex i;
		if (mpNodes[iNode1].mCoincidentVertex)
		{
			i = mpNodes[iNode1].mCoincidentVertex;
			while (mpNodes[i].mCoincidentVertex != iNode1)
			{
				i = mpNodes[i].mCoincidentVertex;
			}
			mpNodes[i].mCoincidentVertex = iNode2;
		}
		if (mpNodes[iNode2].mCoincidentVertex)
		{
			i = mpNodes[iNode2].mCoincidentVertex;
			while (mpNodes[i].mCoincidentVertex != iNode2)
			{
				i = mpNodes[i].mCoincidentVertex;
			}
			mpNodes[i].mCoincidentVertex = iNode1;
		}

        //with double linking the only problem sibling case is when the nodes being swapped
        //are immediate siblings
        if (mpNodes[iNode1].miRightSibling == iNode2)
        {
            //1 is directly to left of 2 in the Forest
            if (mpNodes[iNode1].miLeftSibling == iNIL_NODE)
            {
                //i is the first child of i,j's common parent
                mpNodes[mpNodes[iNode1].miParent].miFirstChild = iNode2;
            }
            else
            {
                mpNodes[mpNodes[iNode1].miLeftSibling].miRightSibling = iNode2;
            }
            mpNodes[iNode1].miRightSibling = iNode1;
            mpNodes[iNode2].miLeftSibling = iNode2;
            mpNodes[mpNodes[iNode2].miRightSibling].miLeftSibling = iNode1;
			//update their children's parent pointers
            child = mpNodes[iNode1].miFirstChild;
            while (child != iNIL_NODE)
            {
                mpNodes[child].miParent = iNode2;
                child = mpNodes[child].miRightSibling;
            }
            child = mpNodes[iNode2].miFirstChild;
            while (child != iNIL_NODE)
            {
                mpNodes[child].miParent = iNode1;
                child = mpNodes[child].miRightSibling;
            }
        }
        else if (mpNodes[iNode2].miRightSibling == iNode1)
		{
			//same as above with 1, 2 switched
			if (mpNodes[iNode2].miLeftSibling == iNIL_NODE)
			{        
				mpNodes[mpNodes[iNode2].miParent].miFirstChild = iNode1;
			}
			else
			{
				mpNodes[mpNodes[iNode2].miLeftSibling].miRightSibling = iNode1;
			}
			mpNodes[iNode2].miRightSibling = iNode2;
			mpNodes[iNode1].miLeftSibling = iNode1;
			mpNodes[mpNodes[iNode1].miRightSibling].miLeftSibling = iNode2;
			
			child = mpNodes[iNode1].miFirstChild;
			while (child != iNIL_NODE)
			{
				mpNodes[child].miParent = iNode2;
				child = mpNodes[child].miRightSibling;
			}
			child = mpNodes[iNode2].miFirstChild;
			while (child != iNIL_NODE)
			{
				mpNodes[child].miParent = iNode1;
				child = mpNodes[child].miRightSibling;
			}
		}
		else if (mpNodes[iNode1].miParent == iNode2)
		{           
			child = mpNodes[iNode2].miFirstChild;
			if (child == iNode1)
			{
				//go through 2's children to be consistent with else part
				while (child != iNIL_NODE)
				{
					mpNodes[child].miParent = iNode1;
					child = mpNodes[child].miRightSibling;
				}
				mpNodes[iNode2].miFirstChild = iNode2;
			}
			else
			{
				//must go through 2's chidren before changing 1's left child's right pointer
				while (child != iNIL_NODE)
				{
					mpNodes[child].miParent = iNode1;
					child = mpNodes[child].miRightSibling;
				}
				mpNodes[mpNodes[iNode1].miLeftSibling].miRightSibling = iNode2;
			}
			if (mpNodes[iNode2].miLeftSibling == iNIL_NODE)
			{
				mpNodes[mpNodes[iNode2].miParent].miFirstChild = iNode1;
			}
			else
			{
				mpNodes[mpNodes[iNode2].miLeftSibling].miRightSibling = iNode1;
			}
			child = mpNodes[iNode1].miFirstChild;
			while (child != iNIL_NODE)
			{
				mpNodes[child].miParent = iNode2;
				child = mpNodes[child].miRightSibling;
			}            
			mpNodes[mpNodes[iNode1].miRightSibling].miLeftSibling = iNode2;
			mpNodes[mpNodes[iNode2].miRightSibling].miLeftSibling = iNode1;
		}
		else if (mpNodes[iNode2].miParent == iNode1)
		{
			child = mpNodes[iNode1].miFirstChild;
			if (child == iNode2)
			{
				//go through 2's children to be consistent with else part
				while (child != iNIL_NODE)
				{
					mpNodes[child].miParent = iNode2;
					child = mpNodes[child].miRightSibling;
				}
				mpNodes[iNode1].miFirstChild = iNode1;
			}
			else
			{
				//must go through 2's chidren before changing 1's left child's right pointer
				while (child != iNIL_NODE)
				{
					mpNodes[child].miParent = iNode2;
					child = mpNodes[child].miRightSibling;
				}
				mpNodes[mpNodes[iNode2].miLeftSibling].miRightSibling = iNode1;
			}
			if (mpNodes[iNode1].miLeftSibling == iNIL_NODE)
			{
				mpNodes[mpNodes[iNode1].miParent].miFirstChild = iNode2;
			}
			else
			{
				mpNodes[mpNodes[iNode1].miLeftSibling].miRightSibling = iNode2;
			}
			child = mpNodes[iNode2].miFirstChild;
			while (child != iNIL_NODE)
			{
				mpNodes[child].miParent = iNode1;
				child = mpNodes[child].miRightSibling;
			}            
			mpNodes[mpNodes[iNode1].miRightSibling].miLeftSibling = iNode2;
			mpNodes[mpNodes[iNode2].miRightSibling].miLeftSibling = iNode1;
		}
		else
		{
			//They are not immediate siblings or parent/child
			if (mpNodes[iNode1].miLeftSibling == iNIL_NODE)
			{
				mpNodes[mpNodes[iNode1].miParent].miFirstChild = iNode2;
			}
			else
			{
				mpNodes[mpNodes[iNode1].miLeftSibling].miRightSibling = iNode2;
			}
			
			if (mpNodes[iNode2].miLeftSibling == iNIL_NODE)
			{
				mpNodes[mpNodes[iNode2].miParent].miFirstChild = iNode1;
			}
			else
			{
				mpNodes[mpNodes[iNode2].miLeftSibling].miRightSibling = iNode1;
			}
			mpNodes[mpNodes[iNode1].miRightSibling].miLeftSibling = iNode2;
			mpNodes[mpNodes[iNode2].miRightSibling].miLeftSibling = iNode1;
			child = mpNodes[iNode1].miFirstChild;
			while (child != iNIL_NODE)
			{
				mpNodes[child].miParent = iNode2;
				child = mpNodes[child].miRightSibling;
			}
			child = mpNodes[iNode2].miFirstChild;
			while (child != iNIL_NODE)
			{
				mpNodes[child].miParent = iNode1;
				child = mpNodes[child].miRightSibling;
			}
		}
		//swap the memory contents of the nodes
		SwapNodeMemory(iNode1, iNode2);

		int k;
		TriIndex tri;
		for (tri = 1; tri <= mNumTris; ++tri)
		{
			for (k = 0; k < 3; ++k)
			{
				if (mpTris[tri].miCorners[k] == iNode1)
					mpTris[tri].miCorners[k] = iNode2;
				else if (mpTris[tri].miCorners[k] == iNode2)
					mpTris[tri].miCorners[k] = iNode1;
			}
		}

		TriIndex tmptri = FirstLiveTris[iNode1];
		FirstLiveTris[iNode1] = FirstLiveTris[iNode2];
		FirstLiveTris[iNode2] = tmptri;
	}
}

// Performs the actual swapping of the memory contents of nodes.
// this function now only for use in preprocessing stage
// currently only used to swap root node to index 1
void Forest::SwapNodeMemory(NodeIndex iNode1, NodeIndex iNode2)
{
	VertexRenderDatum *node1renderdata = mpNodes[iNode1].mpRenderData;
	VertexRenderDatum *node2renderdata = mpNodes[iNode2].mpRenderData;

    unsigned int *p;
    unsigned int *q;
    unsigned int i;
//    unsigned int j;
    unsigned int num_words;
	
    p = (unsigned int *) &mpNodes[iNode1];
    q = (unsigned int *) &mpNodes[iNode2];
    num_words = sizeof(Node) / sizeof(p);
    for (i = 0; i < num_words; i++, p++, q++)
    {
        *p ^= *q;
        *q ^= *p;
        *p ^= *q;
    }

	mpNodes[iNode1].mpRenderData = node2renderdata;
	mpNodes[iNode2].mpRenderData = node1renderdata;
}

void Forest::ReorderNodesDepthFirst(TriIndex *FirstLiveTris, TriIndex **NextLiveTris)
{
////	TriIndex iLiveTri, iNextLiveTri;
	unsigned int i;
	NodeIndex child;

	DepthFirstArray = new NodeIndex[mNumNodes+1];
	LocInArray = new NodeIndex[mNumNodes+1];
	Node* new_node_array = new Node[mNumNodes+1];
	TriIndex *NewFirstLiveTris = new TriIndex[mNumNodes+1];
	if (!DepthFirstArray || !LocInArray || !new_node_array || !NewFirstLiveTris)
	{
		cerr << "Error: Unable to allocate enough memory to reorder nodes depth-first." << endl;
		return;
	}

	for (i = 0; i <= mNumNodes; ++i)
	{
		new_node_array[i].miParent = iNIL_NODE;
		new_node_array[i].miLeftSibling = iNIL_NODE;
		new_node_array[i].miRightSibling = iNIL_NODE;
		new_node_array[i].miFirstChild = iNIL_NODE;
		NewFirstLiveTris[i] = iNIL_TRI;
	}

	Tri* new_tri_array = new Tri[mNumTris+1];
	TriIndex **NewNextLiveTris = new TriIndex*[mNumTris+1];
	// we actually don't change NextLiveTris right now, because we don't sort
	// tris.  if we start sorting tris, then we will need to use this array.

	for (i = 1; i <= mNumTris; ++i)
	{
		new_tri_array[i].miNextSubTri = mpTris[i].miNextSubTri;
		new_tri_array[i].mPatchID = mpTris[i].mPatchID;
		new_tri_array[i].miCorners[0] = iNIL_TRI;
		new_tri_array[i].miCorners[1] = iNIL_TRI;
		new_tri_array[i].miCorners[2] = iNIL_TRI;
		NewNextLiveTris[i] = new TriIndex[3];
		NewNextLiveTris[i][0] = iNIL_TRI;
		NewNextLiveTris[i][1] = iNIL_TRI;
		NewNextLiveTris[i][2] = iNIL_TRI;
	}

	DepthFirstArray[0] = 0;
	LocInArray[0] = 0;
	DFSindex = 1;
	DFSvisit(iROOT_NODE);

	for (i = 1; i <= mNumNodes; ++i)
	{
		// copy everything about the node NOT dealing with tree connectivity
//		new_node_array[i].mRadius = mpNodes[DepthFirstArray[i]].mRadius;
		new_node_array[i].mXBBoxOffset = mpNodes[DepthFirstArray[i]].mXBBoxOffset;
		new_node_array[i].mYBBoxOffset = mpNodes[DepthFirstArray[i]].mYBBoxOffset;
		new_node_array[i].mZBBoxOffset = mpNodes[DepthFirstArray[i]].mZBBoxOffset;
		new_node_array[i].mBBoxCenter.X = mpNodes[DepthFirstArray[i]].mBBoxCenter.X;
		new_node_array[i].mBBoxCenter.Y = mpNodes[DepthFirstArray[i]].mBBoxCenter.Y;
		new_node_array[i].mBBoxCenter.Z = mpNodes[DepthFirstArray[i]].mBBoxCenter.Z;
		new_node_array[i].miErrorParamIndex = mpNodes[DepthFirstArray[i]].miErrorParamIndex;
		new_node_array[i].miFirstSubTri = mpNodes[DepthFirstArray[i]].miFirstSubTri;
		new_node_array[i].mpRenderData = mpNodes[DepthFirstArray[i]].mpRenderData;
		new_node_array[i].mPatchID = mpNodes[DepthFirstArray[i]].mPatchID;
		new_node_array[i].mCoincidentVertex = LocInArray[mpNodes[DepthFirstArray[i]].mCoincidentVertex];

		NewFirstLiveTris[i] = FirstLiveTris[DepthFirstArray[i]];

		// update parent's firstchild index if node is the first child (and not the root node)
		if (mpNodes[DepthFirstArray[i]].miLeftSibling == iNIL_NODE)
		{
			if (DepthFirstArray[i] != iROOT_NODE)
				new_node_array[LocInArray[mpNodes[DepthFirstArray[i]].miParent]].miFirstChild = i;
		}
		else		// else update left sibling's rightsibling index
		{
			new_node_array[LocInArray[mpNodes[DepthFirstArray[i]].miLeftSibling]].miRightSibling = i;
		}		

		// if not rightmost sibling, update right sibling's leftsibling index
		if (mpNodes[DepthFirstArray[i]].miRightSibling != iNIL_NODE)
		{
			new_node_array[LocInArray[mpNodes[DepthFirstArray[i]].miRightSibling]].miLeftSibling = i;
		}

		// update all children's parent pointers
		child = mpNodes[DepthFirstArray[i]].miFirstChild;
		while (child != iNIL_NODE)
		{
			new_node_array[LocInArray[child]].miParent = i;
			child = mpNodes[child].miRightSibling;
		}
	}

	// supported tris are currently in livetrilist -
	// go through them and change tris' corner to new index
	for (i = 1; i <= mNumTris; ++i)
	{
		new_tri_array[i].miCorners[0] = LocInArray[mpTris[i].miCorners[0]];
		new_tri_array[i].miCorners[1] = LocInArray[mpTris[i].miCorners[1]];
		new_tri_array[i].miCorners[2] = LocInArray[mpTris[i].miCorners[2]];
	}

	memcpy(&new_node_array[0], &mpNodes[0], sizeof(Node));
	delete[] mpNodes;
	mpNodes = new_node_array;
	memcpy(&new_tri_array[0], &mpTris[0], sizeof(Tri));
	delete[] mpTris;
	mpTris = new_tri_array;

	memcpy(FirstLiveTris, NewFirstLiveTris, (mNumNodes + 1) * sizeof(TriIndex));
	delete[] NewFirstLiveTris;
	NewFirstLiveTris = NULL;
	for (i = 1; i <= mNumTris; ++i)
	{
		delete[] NewNextLiveTris[i];
	}
	delete[] NewNextLiveTris;
	NewNextLiveTris = NULL;
	delete[] DepthFirstArray;
	DepthFirstArray = NULL;
	delete[] LocInArray;
	LocInArray = NULL;
}

// utility function used only inside ReorderNodesDepthFirst()
void Forest::DFSvisit(NodeIndex i)
{
	DepthFirstArray[DFSindex] = i;
	LocInArray[i] = DFSindex;
	++DFSindex;

	NodeIndex child = mpNodes[i].miFirstChild;
	while (child != iNIL_NODE)
	{
		DFSvisit(child);
		child = mpNodes[child].miRightSibling;
	}
}

void Forest::ForestComputeBBoxes(NodeIndex iNode, TriIndex *FirstLiveTris, TriIndex **NextLiveTris)
{
    NodeIndex child;
	int k;
	TriIndex livetri;
    std::vector<Point3> point_vector;
    std::vector<Point3>::iterator pt_iter;    
//    Float radius_squared;
//    Float distance_squared;
    Point3 node_position(mpNodes[iNode].mpRenderData->Position[0], mpNodes[iNode].mpRenderData->Position[1], mpNodes[iNode].mpRenderData->Position[2]);
//    Float child_distance;
//	mpNodes[iNode].mRadius = 0.0;
	mpNodes[iNode].mXBBoxOffset = 0.0f;
	mpNodes[iNode].mYBBoxOffset = 0.0f;
	mpNodes[iNode].mZBBoxOffset = 0.0f;
	mpNodes[iNode].mBBoxCenter.X = 0.0f;
	mpNodes[iNode].mBBoxCenter.Y = 0.0f;
	mpNodes[iNode].mBBoxCenter.Z = 0.0f;
	child = mpNodes[iNode].miFirstChild;
    while(iNIL_NODE != child)
    {
        ForestComputeBBoxes(child, FirstLiveTris, NextLiveTris);

		// push points corresponding to opposite corners of the child's bbox
		Point3 bboxc1(mpNodes[child].mBBoxCenter.X - mpNodes[child].mXBBoxOffset,
			mpNodes[child].mBBoxCenter.Y - mpNodes[child].mYBBoxOffset,
			mpNodes[child].mBBoxCenter.Z - mpNodes[child].mZBBoxOffset);
		point_vector.push_back(bboxc1);

		Point3 bboxc2(mpNodes[child].mBBoxCenter.X + mpNodes[child].mXBBoxOffset,
			mpNodes[child].mBBoxCenter.Y + mpNodes[child].mYBBoxOffset,
			mpNodes[child].mBBoxCenter.Z + mpNodes[child].mZBBoxOffset);
		point_vector.push_back(bboxc2);

        child = mpNodes[child].miRightSibling;
	}
    if (point_vector.empty())
	{
		// node is a leaf node - bound all vertices of all triangles it supports
		livetri = FirstLiveTris[iNode];
		while (livetri != iNIL_NODE)
		{
			Point3 v0(mpNodes[mpTris[livetri].miCorners[0]].mpRenderData->Position[0],
				mpNodes[mpTris[livetri].miCorners[0]].mpRenderData->Position[1],
				mpNodes[mpTris[livetri].miCorners[0]].mpRenderData->Position[2]);
			point_vector.push_back(v0);
			Point3 v1(mpNodes[mpTris[livetri].miCorners[1]].mpRenderData->Position[0],
				mpNodes[mpTris[livetri].miCorners[1]].mpRenderData->Position[1],
				mpNodes[mpTris[livetri].miCorners[1]].mpRenderData->Position[2]);
			point_vector.push_back(v1);
			Point3 v2(mpNodes[mpTris[livetri].miCorners[2]].mpRenderData->Position[0],
				mpNodes[mpTris[livetri].miCorners[2]].mpRenderData->Position[1],
				mpNodes[mpTris[livetri].miCorners[2]].mpRenderData->Position[2]);
			point_vector.push_back(v2);
			k = mpTris[livetri].GetNodeIndexC(livetri, iNode, *this);
			livetri = NextLiveTris[livetri][k];
		}
	}

	if (point_vector.empty())
	{
		cerr << "Warning: in node bounding box calculation; leaf node " << iNode << " supports no triangles" << endl;
		mpNodes[iNode].mBBoxCenter = node_position;
		mpNodes[iNode].mXBBoxOffset = 0.0f;
		mpNodes[iNode].mYBBoxOffset = 0.0f;
		mpNodes[iNode].mZBBoxOffset = 0.0f;
		return;
	}

	// iterate through points, updating iNode's bounding box
	Float xmin = FLT_MAX;
	Float xmax = -FLT_MAX;
	Float ymin = FLT_MAX;
	Float ymax = -FLT_MAX;
	Float zmin = FLT_MAX;
	Float zmax = -FLT_MAX;

    for (pt_iter = point_vector.begin(); pt_iter != point_vector.end(); pt_iter++)
	{
		if (pt_iter->X < xmin)
			xmin = pt_iter->X;
		if (pt_iter->X > xmax)
			xmax = pt_iter->X;
		if (pt_iter->Y < ymin)
			ymin = pt_iter->Y;
		if (pt_iter->Y > ymax)
			ymax = pt_iter->Y;
		if (pt_iter->Z < zmin)
			zmin = pt_iter->Z;
		if (pt_iter->Z > zmax)
			zmax = pt_iter->Z;
    }

	mpNodes[iNode].mBBoxCenter.X = (xmax + xmin) / 2.0f;
	mpNodes[iNode].mBBoxCenter.Y = (ymax + ymin) / 2.0f;
	mpNodes[iNode].mBBoxCenter.Z = (zmax + zmin) / 2.0f;
	mpNodes[iNode].mXBBoxOffset = (xmax - xmin) / 2.0f;
	mpNodes[iNode].mYBBoxOffset = (ymax - ymin) / 2.0f;
	mpNodes[iNode].mZBBoxOffset = (zmax - zmin) / 2.0f;
}

#ifdef _WIN32
void WriteArray(void *pArray, unsigned int Length, HANDLE FileHandle)
#else
void WriteArray(void *pArray, unsigned int Length, int FileHandle)
#endif
{
#ifdef _WIN32 
    volatile static const unsigned int max_chars_to_write = 50000000 / sizeof(char);
    unsigned char *p;
    DWORD num_bytes_written;
	
    p = (unsigned char *) pArray;
    while (p < (unsigned char *) pArray + Length)
    {
        if (p + max_chars_to_write < (unsigned char *) pArray + Length)
        {
            WriteFile(FileHandle, p, sizeof(unsigned char) * max_chars_to_write, &num_bytes_written, NULL);
        }
        else
        {
            WriteFile(FileHandle, p, sizeof(unsigned char) * (Length % max_chars_to_write), &num_bytes_written, NULL);
        }
        p += max_chars_to_write;
    }
#else
    fprintf(stderr, "array writing not implemented for non-win32");
#endif
}

//function for sorting corners in BuildSubTriLists
void sort_three(NodeIndex &rA, NodeIndex &rB, NodeIndex &rC)
{
    int temp;    
    if ((rA<= rB) && (rB <= rC))
    {
        return;
    }
    else if ((rA <= rC) && (rC <= rB))
    {
        temp = rB;
        rB = rC;
        rC = temp;
        return;
    }
    else if ((rB <= rA) && (rA <= rC))
    {
        temp = rA;
        rA = rB;
        rB = temp;
        return;
    }
    else if ((rB <= rC) && (rC <= rA))
    {
        temp = rB;
        rB = rC;        
        rC = rA;
        rA = temp;
        return;
    }
    else if ((rC <= rA) && (rA <= rB))
    {
        temp = rA;
        rA = rC;
        rC = rB;
        rB = temp;
    }
    else if ((rC <= rB) && (rB <= rA))
    {
        temp = rA;
        rA = rC;
        rC = temp;        
    }
}

NodeIndex Forest::first_ancestor_of(NodeIndex a, NodeIndex b)
{
	NodeIndex temp;
	if (a > b)
	{
		temp = a;
		a = b;
		b = temp;
	}
	NodeIndex ancestor = mpNodes[b].miParent;
	if (mpNodes[ancestor].mCoincidentVertex == iNIL_NODE)
	{
		if (ancestor <= a)
			return ancestor;
		else
			return first_ancestor_of(a, ancestor);
	}
	else
	{
		temp = ancestor;
		while (mpNodes[temp].mCoincidentVertex != ancestor)
		{
			temp = mpNodes[temp].mCoincidentVertex;
			if (temp <= a)
				return temp;
		}
		return first_ancestor_of(a, ancestor);
	}
}


void VDS::StdViewIndependentError(NodeIndex iNode, const Forest &Forest)
{
	Node *pNode = &(Forest.mpNodes[iNode]);
	float bboxdiag2 = (pNode->mXBBoxOffset * pNode->mXBBoxOffset) +
		(pNode->mYBBoxOffset * pNode->mYBBoxOffset) +
		(pNode->mZBBoxOffset * pNode->mZBBoxOffset);
	Forest.mpErrorParams[Forest.mpNodes[iNode].miErrorParamIndex] = sqrt(bboxdiag2);
}

#include "forest_debug_functions.cpp"
