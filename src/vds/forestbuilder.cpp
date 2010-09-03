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
//stop compiler from complaining about exception handling being turned off
#ifdef _WIN32
#pragma warning(disable: 4530)
#endif

#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>
#include "vds.h"
#include "forestbuilder.h"
#include <limits.h>
#include <math.h>
#if defined(_WIN32) || defined(__APPLE__)
#else
#include <values.h>
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef FLT_MAX
#define FLT_MAX 1e32
#endif

extern void VDS::StdViewIndependentError(NodeIndex iNode, const Forest &Forest);

using namespace std;

namespace VDS {

ForestBuilder::ForestBuilder()
{
    mForestStarted = false;
    mForestEnded = false;
    mGeometryStarted = false;
    mGeometryEnded = false;
    mNumAllocatedTris = 0;
    mNumAllocatedNodes = 0;
    mfMergeColorCreation = NULL;
    mfMergePositionCreation = NULL;
    mfMergeNormalCreation = NULL;
    mfMergeTexCoordsCreation = NULL;
	mFirstLiveTris = NULL;
	mNextLiveTris = NULL;
}

ForestBuilder::~ForestBuilder()
{
    Reset();
}

void ForestBuilder::Reset()
{
	unsigned int i;
	if (mFirstLiveTris != NULL)
	{
		delete[] mFirstLiveTris;
		mFirstLiveTris = NULL;
	}
	if (mNextLiveTris != NULL)
	{
		for (i = 1; i <= mNumTris; ++i)
		{
			delete[] mNextLiveTris[i];
		}
		delete[] mNextLiveTris;
		mNextLiveTris = NULL;
	}

    //:DEPENDENCY:
    //this call to Forest's version is redundant
    //when called by ~ForestBuilder() but necessary
    //when Reset called directly.
    //This means that Forest's version must work
    //properly if called multiple times.
    Forest::Reset();
    //all the arrays will be deleted
    mNumAllocatedNodes = mNumAllocatedTris = 0;
    mForestStarted = mForestEnded = mGeometryStarted = mGeometryEnded = false;
}

Point3 ForestBuilder::GetNodePosition(NodeIndex n) const
{
	return mpNodes[n].mpRenderData->Position;
}

void ForestBuilder::SetMergePositionCreationFunc(MergePositionCreationFunc fMergePositionCreation)
{
    mfMergePositionCreation = fMergePositionCreation;
}

void ForestBuilder::SetMergeNormalCreationFunc(MergeNormalCreationFunc fMergeNormalCreation)
{
    mfMergeNormalCreation = fMergeNormalCreation;
}

void ForestBuilder::SetMergeColorCreationFunc(MergeColorCreationFunc fMergeColorCreation)
{
    mfMergeColorCreation = fMergeColorCreation;
}

void ForestBuilder::SetMergeTexCoordsCreationFunc(MergeTexCoordsCreationFunc fMergeTexCoordsCreation)
{
    mfMergeTexCoordsCreation = fMergeTexCoordsCreation;
}

void ForestBuilder::BeginForest()
{
	assert(!mForestStarted && !mIsValid);
	mForestStarted = true;
    mAvgEdgeLength = 0.0;
	mNumPatches = 1;
}


void ForestBuilder::BeginGeometry(bool ColorsPresent, bool IsNormals, unsigned int NumTextures, NodeIndex NumNodes, TriIndex NumTris)
{
	assert(mForestStarted && !mForestEnded && !mIsValid);
	assert(NumNodes != 0 && NumTris != 0);
	assert(NumTextures <= 1); // currently only single texture supported
//    unsigned int i;
    mColorsPresent = ColorsPresent;
    mNormalsPresent = IsNormals;
    mNumTextures = NumTextures;
	mGeometryStarted = true;	
	ReallocateNodes(NumNodes + 1);
	ReallocateTris(NumTris + 1);
}

void ForestBuilder::EndGeometry()
{
	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
	assert(mNumNodes !=0 && mNumTris != 0);
	mGeometryEnded = true;
    mAvgEdgeLength /= (Float) mNumTris * 3.0;
}

void ForestBuilder::EndForest()
{
	unsigned int i;
	// tighten up the arrays if necessary
	ReallocateNodes(mNumNodes + 1);
	ReallocateTris(mNumTris + 1);

    assert(mGeometryEnded && !mForestEnded && !mIsValid);
	mForestEnded = true;
    RelocateRoot();

	cout << "Reordering Nodes to Depth-First..." << flush;
	ReorderNodesDepthFirst(mFirstLiveTris, mNextLiveTris);
	cout << "finished." << endl;
	BuildSubTriLists();

//	ComputeRadii(iROOT_NODE);
	ForestComputeBBoxes(iROOT_NODE, mFirstLiveTris, mNextLiveTris);
	ComputeViewIndependentErrors();

    SetValid();
	delete[] mFirstLiveTris;
	mFirstLiveTris = NULL;
	for (i = 1; i <= mNumTris; ++i)
		delete[] mNextLiveTris[i];
	delete[] mNextLiveTris;
	mNextLiveTris = NULL;
}

bool ForestBuilder::CheckIfNodesAreDepthFirst(NodeIndex iRoot)
{
	NodeIndex iParent = iRoot;
	NodeIndex iChild = mpNodes[iParent].miFirstChild;
	NodeIndex MaxVal;

	while (iChild != iNIL_NODE)
	{
		if (iChild < iParent)
		{
			cout << "Child index " << iChild << " less than parent index " << iParent << endl;
			return 0;
		}
		else
		{
			MaxVal = CheckForDepthFirst(iChild);
			if (MaxVal == 0)
			{
				cout << "MaxVal = 0" << endl;
				return false;
			}

			if (mpNodes[iChild].miRightSibling != iNIL_NODE)
			{
				if (MaxVal > mpNodes[iChild].miRightSibling)
				{
					cout << "Child index " << iChild << " maxval (" << MaxVal << " ) more than right sibling index " << mpNodes[iChild].miRightSibling << endl;
					return false;
				}
			}
		}

		iChild = mpNodes[iChild].miRightSibling;
		if (iChild == iNIL_NODE)
			return true;
	}
	return true;
}

// utility function used by CheckIfNodesAreDepthFirst()
NodeIndex ForestBuilder::CheckForDepthFirst(NodeIndex iRoot)
{
	NodeIndex iParent = iRoot;
	NodeIndex iChild = mpNodes[iParent].miFirstChild;
	NodeIndex MaxVal;

	while (iChild != iNIL_NODE)
	{
		if (iChild < iParent)
		{
			cout << "Child index " << iChild << " less than parent index " << iParent << endl;
			return 0;
		}
		else
		{
			MaxVal = CheckForDepthFirst(iChild);
			if (MaxVal == 0)
			{
				return 0;
			}

			if (mpNodes[iChild].miRightSibling != iNIL_NODE)
			{
				if (MaxVal > mpNodes[iChild].miRightSibling)
				{
					cout << "Child index " << iChild << " maxval (" << MaxVal << " ) more than right sibling index " << mpNodes[iChild].miRightSibling << endl;
					return false;
				}
			}
		}

		iChild = mpNodes[iChild].miRightSibling;
		if (iChild == iNIL_NODE)
			return MaxVal;
	}
	return iRoot;
}

void ForestBuilder::ReorderNodesBreadthFirst()
{
    unsigned int i;
	NodeIndex SwapSourceIndex = iNIL_NODE;
    NodeIndex SwapTargetIndex = 1;
    NodeIndex ChildIndex = iNIL_NODE;
	ReorderQueueLink *QueueHead = new ReorderQueueLink;
	ReorderQueueLink *QueueTail = QueueHead;
	ReorderQueueLink *Next = NULL;
	QueueHead->mIndex = iROOT_NODE;
	QueueHead->mpNext = NULL;
	
	ReorderQueueLink** LocInQueue = new ReorderQueueLink*[mNumNodes];
	for (i = 0; i < mNumNodes; i++)
		LocInQueue[i] = NULL;
	
	LocInQueue[iROOT_NODE] = QueueHead;
	
	while (QueueHead != NULL)
    {
        SwapSourceIndex = QueueHead->mIndex;
		Next = QueueHead->mpNext;
		if (QueueTail == QueueHead)
			QueueTail = Next;
		LocInQueue[QueueHead->mIndex] = NULL;
		delete QueueHead;
		QueueHead = Next;
        
		if (LocInQueue[SwapTargetIndex] != NULL)
			LocInQueue[SwapTargetIndex]->mIndex = SwapSourceIndex;
		
		ReorderQueueLink *TempLink;
		TempLink = LocInQueue[SwapTargetIndex];
		LocInQueue[SwapTargetIndex] = LocInQueue[SwapSourceIndex];
		LocInQueue[SwapSourceIndex] = TempLink;
		
        SwapNodes(SwapTargetIndex, SwapSourceIndex, mFirstLiveTris);
		
        ChildIndex = mpNodes[SwapTargetIndex].GetFirstChild();
        SwapTargetIndex++;
        
        while(ChildIndex != iNIL_NODE)
        {
			if (QueueHead == NULL)
			{
				QueueHead = new ReorderQueueLink;
				LocInQueue[ChildIndex] = QueueHead;
				QueueHead->mIndex = ChildIndex;
				QueueHead->mpNext = NULL;
				QueueTail = QueueHead;
			}
			else
			{
				Next = new ReorderQueueLink;
				LocInQueue[ChildIndex] = Next;
				Next->mIndex = ChildIndex;
				Next->mpNext = NULL;
				QueueTail->mpNext = Next;
				QueueTail = Next;
			}
			
			ChildIndex = mpNodes[ChildIndex].GetRightSibling();
        }
    }
}

bool ForestBuilder::ReallocateNodes(NodeIndex NewSize)
{
	assert((mGeometryStarted && !mGeometryEnded && !mIsValid) || (mGeometryEnded && !mForestEnded && !mIsValid));
	Node *new_node_array;
	VertexRenderDatum *new_renderdata_array;
	TriIndex *new_firstlivetris_array;

	unsigned int i;
	
	if (0 == NewSize)
	{
		NewSize = 2 * mNumAllocatedNodes;
	}
#ifdef _DEBUG
    //total number of nodes are the nodes added by user + the nil node
	if (NewSize < mNumNodes + 1)
	{
		return false;
	}
#endif
    //Allocate and copy one array at a time to reduce total mem requirement

	new_renderdata_array = new VertexRenderDatum[NewSize];
	if (NULL == new_renderdata_array)
    {
		return false;
	}
    if (mpNodeRenderData != NULL)
    {
		memcpy(new_renderdata_array, mpNodeRenderData, (mNumNodes + 1) * sizeof(VertexRenderDatum));
		delete[] mpNodeRenderData;
    }
	mpNodeRenderData = new_renderdata_array;
	for (i = 1; i <= mNumNodes; ++i)
	{
		mpNodes[i].mpRenderData = &mpNodeRenderData[i-1];
	}

    new_node_array = new Node[NewSize];
    if ((NULL == new_node_array))
	{
		return false;
	}
    if (mpNodes != NULL)
	{
        memcpy(new_node_array, mpNodes, (mNumNodes + 1) * sizeof(Node));
        delete[] mpNodes;
	}
	mpNodes = new_node_array;

	new_firstlivetris_array = new TriIndex[NewSize];
	if ((NULL == new_firstlivetris_array))
	{
		return false;
	}
	if (mFirstLiveTris != NULL)
	{
		memcpy(new_firstlivetris_array, mFirstLiveTris, (mNumNodes + 1) * sizeof(TriIndex));
		delete[] mFirstLiveTris;
	}
	for (i = mNumNodes+1; i < NewSize; ++i)
		new_firstlivetris_array[i] = iNIL_TRI;

	mFirstLiveTris = new_firstlivetris_array;

    mNumAllocatedNodes = NewSize;
	return true;	
}

bool ForestBuilder::ReallocateTris(NodeIndex NewSize)
{
	unsigned int i;
	assert((mGeometryStarted && !mGeometryEnded && !mIsValid) || (mGeometryEnded && !mForestEnded && !mIsValid));
	Tri *new_tri_array;
	TriIndex **new_nextlivetris_array;

	if (0 == NewSize)
	{
		NewSize = 2 * mNumAllocatedTris;
	}    
#ifdef _DEBUG	
    //total number of tris are the tris added by user + the nil tri
	if (NewSize < mNumTris + 1)
	{
		return false;
	}
#endif
    new_tri_array = new Tri[NewSize];
	if (NULL != new_tri_array)
	{
        if (NULL != mpTris)
        {
		    memcpy(new_tri_array, mpTris, (mNumTris + 1) * sizeof(Tri));
		    delete[] mpTris;
        }
		mNumAllocatedTris = NewSize;
		mpTris = new_tri_array;
	}
	else
	{
		return false;
	}

	new_nextlivetris_array = new TriIndex*[NewSize];
	for (i = 1; i < NewSize; ++i)
		new_nextlivetris_array[i] = new TriIndex[3];

	if (NULL != new_nextlivetris_array)
	{
		if (NULL != mNextLiveTris)
		{
			for (i = 1; i <= mNumTris; ++i)
			{
				memcpy(new_nextlivetris_array[i], mNextLiveTris[i], 3*sizeof(TriIndex));
				delete[] mNextLiveTris[i];
			}
			delete[] mNextLiveTris;
			mNextLiveTris = NULL;

			for (i = mNumTris+1; i < NewSize; ++i)
			{
				new_nextlivetris_array[i][0] = iNIL_TRI;
				new_nextlivetris_array[i][1] = iNIL_TRI;
				new_nextlivetris_array[i][2] = iNIL_TRI;
			}
		}
		mNextLiveTris = new_nextlivetris_array;
		return true;
	}
	else
	{
		return false;
	}
}

NodeIndex ForestBuilder::AddNode(const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal, const Point2 &rTexCoords)
{
	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
    assert(mColorsPresent && mNormalsPresent && mNumTextures > 0);
    Vec3 normal;
    NodeIndex node;
//    unsigned int i;

    normal = rNormal;
    normal.Normalize();    
    
    if (mNumNodes + 1 == mNumAllocatedNodes)
	{
		ReallocateNodes();
	}
    ++mNumNodes;
	++mNumNodePositions;
    node = mNumNodes;
	mpNodes[node].mpRenderData = &mpNodeRenderData[node-1];
    mpNodes[node].mpRenderData->Position = rPosition;
    mpNodes[node].mpRenderData->Color = (ByteColorA) rColor;
    mpNodes[node].mpRenderData->Normal = rNormal;
	mpNodes[node].mpRenderData->TexCoords = rTexCoords;
	mpNodes[node].mCoincidentVertex = iNIL_NODE;
	mpNodes[node].mPatchID = 0;
    return mNumNodes;
}

NodeIndex ForestBuilder::AddNode(const Point3 &rPosition, const Vec3 &rNormal, const Point2 &rTexCoords)
{
  	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
    assert(!mColorsPresent && mNormalsPresent && mNumTextures > 0);
    Vec3 normal;
    NodeIndex node;
//    unsigned int i;

    normal = rNormal;
    normal.Normalize();
    
    if (mNumNodes + 1 == mNumAllocatedNodes)
	{
		ReallocateNodes();
    }
    ++mNumNodes;
	++mNumNodePositions;
    node = mNumNodes;
	mpNodes[node].mpRenderData = &mpNodeRenderData[node-1];
    mpNodes[node].mpRenderData->Position = rPosition;
    mpNodes[node].mpRenderData->Normal = rNormal;
	mpNodes[node].mpRenderData->TexCoords = rTexCoords;
	mpNodes[node].mCoincidentVertex = iNIL_NODE;
	mpNodes[node].mPatchID = 0;
    return mNumNodes;
}

NodeIndex ForestBuilder::AddNode(const Point3 &rPosition, const ByteColor &rColor, const Point2 &rTexCoords)
{
	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
    assert(mColorsPresent && !mNormalsPresent && mNumTextures > 0);    
    NodeIndex node;
//    unsigned int i;

    if (mNumNodes + 1== mNumAllocatedNodes)
	{
		ReallocateNodes();
	}     
    ++mNumNodes;
	++mNumNodePositions;
    node = mNumNodes;
	mpNodes[node].mpRenderData = &mpNodeRenderData[node-1];
    mpNodes[node].mpRenderData->Position = rPosition;
    mpNodes[node].mpRenderData->Color = (ByteColorA) rColor;
	mpNodes[node].mpRenderData->TexCoords = rTexCoords;
	mpNodes[node].mCoincidentVertex = iNIL_NODE;
	mpNodes[node].mPatchID = 0;
    return mNumNodes;
}

NodeIndex ForestBuilder::AddNode(const Point3 &rPosition, const Point2 &rTexCoords)
{
	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
    assert(!mColorsPresent && !mNormalsPresent && mNumTextures > 0);
    NodeIndex node;
//    unsigned int i;

    
    if (mNumNodes + 1 == mNumAllocatedNodes)
	{
		ReallocateNodes();
	}
    ++mNumNodes;
	++mNumNodePositions;
    node = mNumNodes;
	mpNodes[node].mpRenderData = &mpNodeRenderData[node-1];
    mpNodes[node].mpRenderData->Position = rPosition;
	mpNodes[node].mpRenderData->TexCoords = rTexCoords;
	mpNodes[node].mCoincidentVertex = iNIL_NODE;
	mpNodes[node].mPatchID = 0;
	return mNumNodes;
}

NodeIndex ForestBuilder::AddNode(const Point3 &rPosition, const Vec3 &rNormal)
{
	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
    assert(!mColorsPresent && mNormalsPresent && mNumTextures == 0);
    Vec3 normal;
    NodeIndex node;

    normal = rNormal;
    normal.Normalize();
        
    if (mNumNodes + 1 == mNumAllocatedNodes)
	{
		ReallocateNodes();
	}    
    ++mNumNodes;
	++mNumNodePositions;
    node = mNumNodes;
	mpNodes[node].mpRenderData = &mpNodeRenderData[node-1];
    mpNodes[node].mpRenderData->Position = rPosition;
    mpNodes[node].mpRenderData->Normal = rNormal;
	mpNodes[node].mCoincidentVertex = iNIL_NODE;
	mpNodes[node].mPatchID = 0;
	return mNumNodes;
}

NodeIndex ForestBuilder::AddNode(const Point3 &rPosition, const ByteColor &rColor)
{
	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
    assert(mColorsPresent && !mNormalsPresent && mNumTextures == 0);    
    NodeIndex node;
    
    if (mNumNodes + 1 == mNumAllocatedNodes)
	{
		ReallocateNodes();
	}
    ++mNumNodes;
	++mNumNodePositions;
    node = mNumNodes;
	mpNodes[node].mpRenderData = &mpNodeRenderData[node-1];
    mpNodes[node].mpRenderData->Position = rPosition;
    mpNodes[node].mpRenderData->Color = (ByteColorA) rColor;
	mpNodes[node].mCoincidentVertex = iNIL_NODE;
	mpNodes[node].mPatchID = 0;
	return mNumNodes;
}

NodeIndex ForestBuilder::AddNode(const Point3 &rPosition)
{
	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
    assert(!mColorsPresent && !mNormalsPresent && mNumTextures == 0);
    NodeIndex node;

    
    if (mNumNodes + 1 == mNumAllocatedNodes)
	{
		ReallocateNodes();
	}
    ++mNumNodes;
	++mNumNodePositions;
    node = mNumNodes;
	mpNodes[node].mpRenderData = &mpNodeRenderData[node-1];
    mpNodes[node].mpRenderData->Position = rPosition;
	mpNodes[node].mCoincidentVertex = iNIL_NODE;
	mpNodes[node].mPatchID = 0;
    return mNumNodes;
}

NodeIndex ForestBuilder::AddNode(const Point3 &rPosition,  const ByteColor &rColor, const Vec3 &rNormal)
{
	assert(mGeometryStarted && !mGeometryEnded && !mIsValid);
    assert(mColorsPresent && mNormalsPresent && mNumTextures == 0);
    Vec3 normal;
    NodeIndex node;

    normal = rNormal;
    normal.Normalize();    
    
    if (mNumNodes + 1 == mNumAllocatedNodes)
	{
		ReallocateNodes();
	}
    ++mNumNodes;
	++mNumNodePositions;
    node = mNumNodes;
	mpNodes[node].mpRenderData = &mpNodeRenderData[node-1];
    mpNodes[node].mpRenderData->Position = rPosition;
    mpNodes[node].mpRenderData->Color = (ByteColorA) rColor;
    mpNodes[node].mpRenderData->Normal = rNormal;
	mpNodes[node].mCoincidentVertex = iNIL_NODE;
	mpNodes[node].mPatchID = 0;
	return mNumNodes;
}

TriIndex ForestBuilder::AddTri(NodeIndex iNode1, NodeIndex iNode2, NodeIndex iNode3)
{
	assert(mGeometryStarted && !mGeometryEnded & !mIsValid);
    assert(iNode1 < 1 + mNumNodes &&
        iNode2 <= 1 + mNumNodes &&
        iNode3 <= 1 + mNumNodes);
	assert(iNode1 != iNode2 && iNode1 != iNode3 && iNode2 != iNode3);
    assert(iNode1 != iNIL_NODE && iNode2 != iNIL_NODE && iNode3 != iNIL_NODE);
    unsigned int i;

    if (mNumTris + 1 == mNumAllocatedTris) //+ 1 is for nilnode
	{
		ReallocateTris();
	}
    mNumTris++;

    mpTris[mNumTris].miCorners[0] = iNode1;
	mpTris[mNumTris].miCorners[1] = iNode2;
	mpTris[mNumTris].miCorners[2] = iNode3;
	mpTris[mNumTris].mPatchID = 0;
    
    // During hierarchy generation, the live tri lists are used to store triangles that each vertex supports
    mpTris[mNumTris].AddToLiveTriListUsingCorners(mNumTris, 0, *this, mFirstLiveTris, mNextLiveTris);
    mpTris[mNumTris].AddToLiveTriListUsingCorners(mNumTris, 1, *this, mFirstLiveTris, mNextLiveTris);
    mpTris[mNumTris].AddToLiveTriListUsingCorners(mNumTris, 2, *this, mFirstLiveTris, mNextLiveTris);

    for (i = 0; i < 3; i++)
    {
		mAvgEdgeLength += mpNodes[mpTris[mNumTris].miCorners[i]].mpRenderData->Position.DistanceTo(mpNodes[mpTris[mNumTris].miCorners[(i + 1) % 3]].mpRenderData->Position);
    }
  return mNumTris;
}

NodeIndex ForestBuilder::SetupMergeNode(const NodeIndexVector &rChildren)
{
    NodeIndex new_node;
    unsigned int i;
    NodeIndex num_children;

    num_children = rChildren.size();

	assert(!mForestEnded  && mGeometryEnded && !mIsValid);

#ifndef NDEBUG //don't perform the loops if asserts are not active
    unsigned int j;
    //:KLUDGE:
    //following loop is O(n^2) ECH! too slow? then stop using debug mode! :)
    for (i = 0; i < num_children; i++)
    {
	    //make sure the node numbers are valid        
		assert(rChildren[i] < 1 + mNumNodes && rChildren[i] >= 1);
        //make sure all nodes to be clustered are unique
        for (j = i + 1; j < num_children; j++)
        {                      
                assert(rChildren[i] != rChildren[j]);
        }
    }
#endif	

    if (mNumNodes + 1 == mNumAllocatedNodes)
	{
		ReallocateNodes();
	}
    ++mNumNodes;
	++mNumNodePositions;
    new_node = mNumNodes;

	mpNodes[new_node].mpRenderData = &mpNodeRenderData[new_node-1];
    mpNodes[new_node].miFirstChild = rChildren[0];
	mpNodes[new_node].mCoincidentVertex = iNIL_NODE;
	mpNodes[new_node].mPatchID = 0;

    //set up all pointers for children
	for(i = 0; i < num_children; i++)
	{
		mpNodes[rChildren[i]].miParent = new_node;
		if (i != num_children - 1) // not the last node in children
		{
			mpNodes[rChildren[i]].miRightSibling = rChildren[i + 1];
		}
        if (i != 0)
        {
            mpNodes[rChildren[i]].miLeftSibling = rChildren[i - 1];
        }
	}
    mpNodes[rChildren[num_children - 1]].miRightSibling = iNIL_NODE;
    mpNodes[rChildren[0]].miLeftSibling = iNIL_NODE;
    return new_node;
}

//THERE ARE 18(!!!) different mergenodes calls

//CALLBACK TO CREATE PARENT********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes)
{
    unsigned int i;
    NodeIndexVector nodes;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes);
}

NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren)
{
  NodeIndex new_node;
//    unsigned int i;

    new_node = SetupMergeNode(rChildren);
    if (NULL == mfMergePositionCreation)
    {
        mpNodes[new_node].mpRenderData->Position = DefaultMergePositionCreation(rChildren);
    }
    else
    {
        mpNodes[new_node].mpRenderData->Position = mfMergePositionCreation(rChildren, *this);
    }
    if (mNormalsPresent)
    {
        if (NULL == mfMergeNormalCreation)
        {
            mpNodes[new_node].mpRenderData->Normal = DefaultMergeNormalCreation(rChildren);
        }
        else
        {
            mpNodes[new_node].mpRenderData->Normal = mfMergeNormalCreation(rChildren, *this);
        }
    }
    if (mColorsPresent)
    {
        if (NULL == mfMergeColorCreation)
        {
            mpNodes[new_node].mpRenderData->Color = (ByteColorA) DefaultMergeColorCreation(rChildren);
        }
        else
        {
            mpNodes[new_node].mpRenderData->Color = (ByteColorA) mfMergeColorCreation(rChildren, *this);
        }
    }
    if (mNumTextures > 0)
    {
        Point2 tex_coords;
        if (NULL == mfMergeColorCreation)
        {
            tex_coords = DefaultMergeTexCoordsCreation(rChildren);
        }
        else
        {
            tex_coords = mfMergeTexCoordsCreation(rChildren, *this);
        }
    }

    return new_node;
  }

//ALL NODE DATA********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal, const Point2 &rTexCoords)
{
    unsigned int i;
    NodeIndexVector nodes;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes, rPosition, rColor, rNormal, rTexCoords);
}

NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren, const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal, const Point2 &rTexCoords)
{
    NodeIndex parent;

    assert(mNumTextures > 0 && mColorsPresent && mNormalsPresent);
//    unsigned int i;
    parent = SetupMergeNode(rChildren);
    mpNodes[parent].mpRenderData->Position = rPosition;
    mpNodes[parent].mpRenderData->Color = (ByteColorA) rColor;
    mpNodes[parent].mpRenderData->Normal = rNormal;
	mpNodes[parent].mpRenderData->TexCoords = rTexCoords;
    return parent;
}

//NO COLOR********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const Vec3 &rNormal, const Point2 &rTexCoords)
{
    NodeIndexVector nodes;
    vector<Point2> tex_coords;

    unsigned int i;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes, rPosition, rNormal, rTexCoords);
}


NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren, const Point3 &rPosition, const Vec3 &rNormal, const Point2 &rTexCoords)
{
    NodeIndex parent;

    assert(mNumTextures > 0 && !mColorsPresent && mNormalsPresent);
//    unsigned int i;
    parent = SetupMergeNode(rChildren);
    mpNodes[parent].mpRenderData->Position = rPosition;
    mpNodes[parent].mpRenderData->Normal = rNormal;
	mpNodes[parent].mpRenderData->TexCoords = rTexCoords;
    return parent;
}

//NO NORMAL********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const ByteColor &rColor, const Point2 &rTexCoords)
{
    NodeIndexVector nodes;
    vector<Point2> tex_coords;

    unsigned int i;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes, rPosition, rColor, rTexCoords);
}

NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren, const Point3 &rPosition, const ByteColor &rColor, const Point2 &rTexCoords)
{
    NodeIndex parent;

    assert(mNumTextures > 0 && mColorsPresent && !mNormalsPresent);
//    unsigned int i;
    parent = SetupMergeNode(rChildren);
    mpNodes[parent].mpRenderData->Position = rPosition;
    mpNodes[parent].mpRenderData->Color = (ByteColorA) rColor;
	mpNodes[parent].mpRenderData->TexCoords = rTexCoords;
    return parent;
}

//NO NORMAL OR COLOR********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const Point2 &rTexCoords)
{
    NodeIndexVector nodes;
    vector<Point2> tex_coords;

    unsigned int i;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes, rPosition, rTexCoords);
}

NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren, const Point3 &rPosition, const Point2 &rTexCoords)
{
    NodeIndex parent;

    assert(mNumTextures > 0 && !mColorsPresent && !mNormalsPresent);
//    unsigned int i;
    parent = SetupMergeNode(rChildren);
    mpNodes[parent].mpRenderData->Position = rPosition;
	mpNodes[parent].mpRenderData->TexCoords = rTexCoords;
    return parent;
}

//NO COLOR OR TEXTURES********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const Vec3 &rNormal)
{
    NodeIndexVector nodes;

    unsigned int i;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes, rPosition, rNormal);
}

NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren, const Point3 &rPosition, const Vec3 &rNormal)
{
    NodeIndex parent;

    assert(mNumTextures == 0 && !mColorsPresent && mNormalsPresent);    
    parent = SetupMergeNode(rChildren);
    mpNodes[parent].mpRenderData->Position = rPosition;
    mpNodes[parent].mpRenderData->Normal = rNormal;
    return parent;
}

//NO NORMAL OR TEXTURES********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const ByteColor &rColor)
{
    unsigned int i;
    NodeIndexVector nodes;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes, rPosition, rColor);
}

NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren, const Point3 &rPosition, const ByteColor &rColor)
{
    NodeIndex parent;

    assert(mNumTextures == 0 && mColorsPresent && !mNormalsPresent);
    parent = SetupMergeNode(rChildren);
    mpNodes[parent].mpRenderData->Position = rPosition;
    mpNodes[parent].mpRenderData->Color = (ByteColorA) rColor;
    return parent;
}

//JUST POSITION********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition)
{
    NodeIndexVector nodes;

    unsigned int i;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes, rPosition);
}

NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren, const Point3 &rPosition)
{
    NodeIndex parent;

    assert(mNumTextures == 0 && !mColorsPresent && !mNormalsPresent);
    parent = SetupMergeNode(rChildren);
    mpNodes[parent].mpRenderData->Position = rPosition;
    return parent;
}

//NO TEXTURES********************************
NodeIndex ForestBuilder::MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal)
{
    NodeIndexVector nodes;

    unsigned int i;
    for (i = 0; i < NumNodes; i++)
    {
        nodes.push_back(piNodes[i]);
    }
    return MergeNodes(nodes, rPosition);
}

NodeIndex ForestBuilder::MergeNodes(const NodeIndexVector &rChildren, const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal)
{
    NodeIndex parent;

    assert(mNumTextures == 0 && mColorsPresent && mNormalsPresent);
    parent = SetupMergeNode(rChildren);
    mpNodes[parent].mpRenderData->Position = rPosition;
    mpNodes[parent].mpRenderData->Color = (ByteColorA) rColor;
    mpNodes[parent].mpRenderData->Normal = rNormal;
    return parent;
}

void ForestBuilder::RelocateRoot()
{
	assert(mForestEnded  && !mIsValid);
	
	NodeIndex root = 1;
	while(mpNodes[root].miParent != iNIL_NODE)
	{
		root = mpNodes[root].miParent;
	}
    SwapNodes(iROOT_NODE, root, mFirstLiveTris);
	TriIndex temptri = mFirstLiveTris[iROOT_NODE];
	mFirstLiveTris[iROOT_NODE] = mFirstLiveTris[root];
	mFirstLiveTris[root] = temptri;
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

//A node's subtris are the additional triangles that must
//be rendered when the node is unfolded.  A tri is first
//rendered when all three of its corners have unique proxies
//above or on the boundary. When the deepest node in the
//Forest that is an ancestor of two or more of the corners unfolds
//all three corners have unique proxies above or on the boundary.
//Therefore the triangle is a subtri of this node.
void ForestBuilder::BuildSubTriLists()
{
    assert(mForestEnded && !mIsValid);
    TriIndex tri;
    NodeIndex a, b, c;
    NodeIndex ab_first_ancestor, bc_first_ancestor;

    //sort corners of each tri such that a<b<c
    //if bc_first_ancestor > a then tri is a subtri of i_bc_first_ancestor
    //otherwise tri is a subtri of i_ab_first_ancestor
    //trust me :)
    
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
}

void ForestBuilder::ComputeViewIndependentErrors()
{
	NodeIndex i;
	int num_interior_nodes = 0;
	for (i = iROOT_NODE; i <= mNumNodes; ++i)
	{
		if (mpNodes[i].miFirstChild != iNIL_NODE)
			++num_interior_nodes;
	}
	mpErrorParams = new float[num_interior_nodes + 1];
	mNumErrorParams = num_interior_nodes + 1;
	mErrorParamSize = 1;
	int error_index_assigned = 1;
	for (i = iROOT_NODE; i <= mNumNodes; ++i)
	{
		if (mpNodes[i].miFirstChild != iNIL_NODE)
		{
			mpNodes[i].miErrorParamIndex = error_index_assigned;
			++error_index_assigned;
		}
		else
			mpNodes[i].miErrorParamIndex = 0;
	}

	for (i = iROOT_NODE; i <= mNumNodes; ++i)
	{
		StdViewIndependentError(i, *this);
	}
}

Float ForestBuilder::DefaultClusterImportance(NodeIndex iNode) const
{
        if (mFirstLiveTris[iNode] != iNIL_TRI)
        {
		    return cos(GreatestFaceAngle(iNode) / 2.0) + LongestNodeEdge(iNode) / (mAvgEdgeLength * 4.0);
        }
        else
        {
            return 0.0;
        }
}

Point3 ForestBuilder::DefaultMergePositionCreation(const NodeIndexVector &NodeVector) const
{
    unsigned int i;
    unsigned int num_nodes;

	Point3 p;
    num_nodes = NodeVector.size();
    p.X = p.Y = p.Z = 0.0;
    for(i = 0; i < num_nodes; i++)
    {
        p.X += mpNodes[NodeVector[i]].mpRenderData->Position.X;
        p.Y += mpNodes[NodeVector[i]].mpRenderData->Position.Y;
        p.Z += mpNodes[NodeVector[i]].mpRenderData->Position.Z;
    }
    p.X /= (Float) num_nodes;
    p.Y /= (Float) num_nodes;
    p.Z /= (Float) num_nodes;
	  return p;
}

Vec3 ForestBuilder::DefaultMergeNormalCreation(const NodeIndexVector &NodeVector) const
{
	unsigned int i;
	unsigned int num_nodes;
    Vec3 normal;
    normal.Set(0.0, 0.0, 0.0);
    num_nodes = NodeVector.size();
    for(i = 0; i < num_nodes; i++)
    {
        normal += mpNodes[NodeVector[i]].mpRenderData->Normal;
    }
    normal /= (Float) num_nodes;
	normal.Normalize();
    return normal;
}

ByteColor ForestBuilder::DefaultMergeColorCreation(const NodeIndexVector &NodeVector) const
{
	unsigned int i;
	unsigned int num_nodes;
    ByteColor c;
    num_nodes = NodeVector.size();
    c.R = c.G = c.B = 0;
    //cannot add all colors first then divide b/c of overflow
    for(i = 0; i < num_nodes; i++)
    {
        c.R = c.R + (BYTE) (mpNodes[NodeVector[i]].mpRenderData->Color.R / (Float) num_nodes);
        c.G = c.G + (BYTE) (mpNodes[NodeVector[i]].mpRenderData->Color.G / (Float) num_nodes);
        c.B = c.B + (BYTE) (mpNodes[NodeVector[i]].mpRenderData->Color.B / (Float) num_nodes);
    }
    return c;
}

Point2 ForestBuilder::DefaultMergeTexCoordsCreation(const NodeIndexVector &NodeVector) const
{
//	unsigned int i;
    unsigned int j;
    unsigned int num_nodes;
    Point2 tex_coords;
    num_nodes = NodeVector.size();
    tex_coords.X = tex_coords.Y = 0.0;
	for (j = 0; j < num_nodes; j++)
	{
		tex_coords.X += mpNodes[NodeVector[j]].mpRenderData->TexCoords.X;
		tex_coords.Y += mpNodes[NodeVector[j]].mpRenderData->TexCoords.Y;
    }
    tex_coords.X /= (Float) num_nodes;
    tex_coords.Y /= (Float) num_nodes;
    return tex_coords;
}

// Finds the longest triangle edge with an endpoint at node iNode
Float ForestBuilder::LongestNodeEdge(NodeIndex iNode) const
{
    assert(mGeometryEnded && !mForestEnded && !mIsValid);
	assert(iNode >= 0 && iNode <= mNumNodes);

	Float max_distance_squared = 0;
    TriIndex tri;
    Float dist1_squared, dist2_squared;
    Float dx, dy, dz;
    int i;

    tri = mFirstLiveTris[iNode];
	while (iNIL_TRI != tri) 
    {
        i = mpTris[tri].GetNodeIndexC(tri, iNode, *this);
   	    dx = mpNodes[iNode].mpRenderData->Position[0] - mpNodes[mpTris[tri].miCorners[(i + 1) % 3]].mpRenderData->Position[0];
        dy = mpNodes[iNode].mpRenderData->Position[1] - mpNodes[mpTris[tri].miCorners[(i + 1) % 3]].mpRenderData->Position[1];
        dz = mpNodes[iNode].mpRenderData->Position[2] - mpNodes[mpTris[tri].miCorners[(i + 1) % 3]].mpRenderData->Position[2];
        dist1_squared = dx*dx + dy*dy + dz*dz;

	    dx = mpNodes[iNode].mpRenderData->Position[0] - mpNodes[mpTris[tri].miCorners[(i + 2) % 3]].mpRenderData->Position[0];
        dy = mpNodes[iNode].mpRenderData->Position[1] - mpNodes[mpTris[tri].miCorners[(i + 2) % 3]].mpRenderData->Position[1];
        dz = mpNodes[iNode].mpRenderData->Position[2] - mpNodes[mpTris[tri].miCorners[(i + 2) % 3]].mpRenderData->Position[2];
        dist2_squared = dx*dx + dy*dy + dz*dz;

		if (dist1_squared > max_distance_squared)
		{
			max_distance_squared = dist1_squared;
		}
		if (dist2_squared > max_distance_squared)
		{
			max_distance_squared = dist2_squared;
		}
        tri = mNextLiveTris[tri][i];
	}
	return sqrt(max_distance_squared);
	return 0.0f;
}

Float ForestBuilder::GreatestFaceAngle(NodeIndex iNode) const
{
    assert(mGeometryEnded && !mForestEnded && !mIsValid);
	Float max_angle;
	Float angle;

    vector<Vec3> normals;
    Point3 v0, v1, v2;
    Vec3 normal;
    TriIndex tri;
    unsigned int num_tris;
    unsigned int i;
    unsigned int j;

    tri = mFirstLiveTris[iNode];


    while (tri != iNIL_TRI)
    {
        v0.Set(mpNodes[mpTris[tri].miCorners[0]].mpRenderData->Position[0],
               mpNodes[mpTris[tri].miCorners[0]].mpRenderData->Position[1],
               mpNodes[mpTris[tri].miCorners[0]].mpRenderData->Position[2]);
        v1.Set(mpNodes[mpTris[tri].miCorners[1]].mpRenderData->Position[0],
               mpNodes[mpTris[tri].miCorners[1]].mpRenderData->Position[1],
               mpNodes[mpTris[tri].miCorners[1]].mpRenderData->Position[2]);
        v2.Set(mpNodes[mpTris[tri].miCorners[2]].mpRenderData->Position[0],
               mpNodes[mpTris[tri].miCorners[2]].mpRenderData->Position[1],
               mpNodes[mpTris[tri].miCorners[2]].mpRenderData->Position[2]);
        
        normal = (v1 - v0) % (v2 - v1);
        normal.Normalize();
        normals.push_back(normal);
        tri = mNextLiveTris[tri][mpTris[tri].GetNodeIndexC(tri, iNode, *this)];
    }
    num_tris = normals.size();
    max_angle = 0.0;
    for (i = 0; i < num_tris; i++)
    {
        for (j = i + 1; j < num_tris; j++)
        {
            angle = 2 * M_PI - acos(normals[i] * normals[j]);
            if (angle > max_angle)
            {
                max_angle = angle;
            }
        }
    }
    return max_angle;
	return 0.0f;
}
}
