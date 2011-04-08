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
#pragma warning(disable: 4305)
#endif

#include <cassert>
#include <iostream>
#include "simplifier.h"
#include "forest.h"
#include "cut.h"

using namespace std;
using namespace VDS;

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif

Simplifier::Simplifier()
{
// public data initialization
	mfErrorFunc = NULL;
	mIsValid = false;
	mBudgetTolerance = 0;
	mSimplificationBreakCount = 0;

// private data initialization
	mpCuts = NULL;
	mNumCuts = 0;
	miCurrentCut = -666666;
	mpCurrentForest = NULL;
	mpFoldQueue = new NodeQueue(this);
	mpFoldQueue->Initialize(48, -FLT_MAX);
	mpUnfoldQueue = new NodeQueue(this);
	mpUnfoldQueue->Initialize(48, -FLT_MAX);

// profiling data initialization
#ifdef _WIN32
	timer_update_budget_errors.LowPart = 0;
	timer_simplification.LowPart = 0;
	fold_time.LowPart = 0;
	unfold_time.LowPart = 0;
	update_budget_errors_calc.LowPart = 0;
	update_budget_errors_queue_manip.LowPart = 0;
#endif
	tris_introduced = 0;
	tris_removed = 0;
	foldqueue_size = 0;
	unfoldqueue_size = 0;
}

Simplifier::~Simplifier()
{
	if (mpCuts != NULL)
		delete[] mpCuts;
	delete mpFoldQueue;
	delete mpUnfoldQueue;
}

void Simplifier::AddCut(Cut *pCut)
{
	mNumCuts++;
	Cut **oldCuts = mpCuts;
	mpCuts = new Cut* [mNumCuts];
	for (int i = 0; i <= mNumCuts-2; ++i)
	{
		mpCuts[i] = oldCuts[i];
	}
	mpCuts[mNumCuts-1] = pCut;
	miCurrentCut = mNumCuts-1;
	
	if (oldCuts != NULL)
		delete[] oldCuts;

	BudgetItem RootNode;
	RootNode.CutID = mNumCuts-1;
	RootNode.miNode = Forest::iROOT_NODE;
	RootNode.miFirstLiveTri = Forest::iNIL_TRI;
	RootNode.mPosition = pCut->mpForest->mpNodes[RootNode.miNode].mpRenderData->Position;
//	RootNode.mRadius = pCut->mpForest->mpNodes[RootNode.miNode].mRadius;
	RootNode.mXBBoxOffset = pCut->mpForest->mpNodes[RootNode.miNode].mXBBoxOffset;
	RootNode.mYBBoxOffset = pCut->mpForest->mpNodes[RootNode.miNode].mYBBoxOffset;
	RootNode.mZBBoxOffset = pCut->mpForest->mpNodes[RootNode.miNode].mZBBoxOffset;
	RootNode.mBBoxCenter = pCut->mpForest->mpNodes[RootNode.miNode].mBBoxCenter;
	RootNode.mError = -mfErrorFunc(&RootNode, pCut);

	RootNode.pVertexRenderDatum = pCut->mpRenderer->AddVertexRenderDatum(RootNode.miNode);
	RootNode.pVertexRenderDatum->Node = RootNode.miNode;

	pCut->mpNodeRefs[RootNode.miNode] = &RootNode;
	mpUnfoldQueue->Insert(&RootNode);
	pCut->mNumActiveNodes = 1;
}

void Simplifier::RemoveCut(Cut *pCut)
{
	cerr << "Simplifier::RemoveCut not implemented yet." << endl;
}

void Simplifier::UpdateNodeErrors()
{
	int i;
	int PQsize;
	BudgetItem *element;

#ifdef TIMING_LEVEL_2
	LARGE_INTEGER time_1, time_2, time_3, time_4, time_5;
	QueryPerformanceCounter(&time_1);
#endif

	PQsize = mpFoldQueue->Size;
	for (i = 1; i <= PQsize; ++i)
	{
		element = mpFoldQueue->GetElement(i);
		element->mError = mfErrorFunc(element, mpCuts[element->CutID]);
	}

#ifdef TIMING_LEVEL_2
	QueryPerformanceCounter(&time_2);
#endif

	PQsize = mpUnfoldQueue->Size;
	for (i = 1; i <= PQsize; ++i)
	{
		element = mpUnfoldQueue->GetElement(i);
		element->mError = -mfErrorFunc(element, mpCuts[element->CutID]);
	}

#ifdef TIMING_LEVEL_2
	QueryPerformanceCounter(&time_3);
#endif

	mpFoldQueue->buildheap();

#ifdef TIMING_LEVEL_2
	QueryPerformanceCounter(&time_4);
#endif

	mpUnfoldQueue->buildheap();

#ifdef TIMING_LEVEL_2
	QueryPerformanceCounter(&time_5);
	times_26.LowPart = (time_2.LowPart - time_1.LowPart);
	times_27.LowPart = (time_3.LowPart - time_2.LowPart);
	times_28.LowPart = (time_4.LowPart - time_3.LowPart);
	times_29.LowPart = (time_5.LowPart - time_4.LowPart);
#endif
#ifdef TIMING_LEVEL_2
	foldqueue_size = mpFoldQueue->Size;
	unfoldqueue_size = mpUnfoldQueue->Size;
#endif
}

void Simplifier::FlushQueues()
{
	BudgetItem *pItem;
	NodeIndex node;
	Cut *pCurrentCut;
	unsigned int i;

	while (mpFoldQueue->Size > 0)
	{
		pItem = mpFoldQueue->GetElement(mpFoldQueue->Size);
		node = pItem->miNode;
		pCurrentCut = mpCuts[pItem->CutID];
		mpFoldQueue->Remove(pItem);
		pCurrentCut->mpNodeRefs[node] = NULL;
	}
	while (mpUnfoldQueue->Size > 0)
	{
		pItem = mpUnfoldQueue->GetElement(mpUnfoldQueue->Size);
		node = pItem->miNode;
		pCurrentCut = mpCuts[pItem->CutID];
		mpUnfoldQueue->Remove(pItem);
		pCurrentCut->mpNodeRefs[node] = NULL;
	}
	for (miCurrentCut = 0; miCurrentCut < mNumCuts; ++miCurrentCut)
	{
		pCurrentCut = mpCuts[miCurrentCut];
		for (i = 1; i <= pCurrentCut->mpForest->mNumNodes; ++i)
		{
			if (pCurrentCut->mpNodeRefs[i] != NULL)
			{
				delete pCurrentCut->mpNodeRefs[i];
				pCurrentCut->mpNodeRefs[i] = NULL;
			}
		}
		pCurrentCut->mpRenderer->FlushRenderData();
	}

	for (miCurrentCut = 0; miCurrentCut < mNumCuts; ++miCurrentCut)
	{
		pCurrentCut = mpCuts[miCurrentCut];

		BudgetItem RootNode;
		RootNode.CutID = miCurrentCut;
		RootNode.miNode = Forest::iROOT_NODE;
		RootNode.miFirstLiveTri = Forest::iNIL_TRI;
		RootNode.mPosition = pCurrentCut->mpForest->mpNodes[RootNode.miNode].mpRenderData->Position;
//		RootNode.mRadius = pCurrentCut->mpForest->mpNodes[RootNode.miNode].mRadius;
		RootNode.mXBBoxOffset = pCurrentCut->mpForest->mpNodes[RootNode.miNode].mXBBoxOffset;
		RootNode.mYBBoxOffset = pCurrentCut->mpForest->mpNodes[RootNode.miNode].mYBBoxOffset;
		RootNode.mZBBoxOffset = pCurrentCut->mpForest->mpNodes[RootNode.miNode].mZBBoxOffset;
		RootNode.mBBoxCenter = pCurrentCut->mpForest->mpNodes[RootNode.miNode].mBBoxCenter;

		RootNode.mError = -mfErrorFunc(&RootNode, pCurrentCut);

		RootNode.pVertexRenderDatum = pCurrentCut->mpRenderer->AddVertexRenderDatum(RootNode.miNode);
		RootNode.pVertexRenderDatum->Node = RootNode.miNode;

		pCurrentCut->mpNodeRefs[RootNode.miNode] = &RootNode;
		mpUnfoldQueue->Insert(&RootNode);
		pCurrentCut->mNumActiveNodes = 1;
	}

}

void Simplifier::SimplifyBudget(unsigned int Budget, bool UseTriBudget)
{
	unsigned int count, curval, NumTris, BytesUsed;
	tris_introduced = 0;
	tris_removed = 0;

	if (!mIsValid)
		return;

#ifdef TIMING_LEVEL_3
	unfold_time.LowPart = 0;
	fold_time.LowPart = 0;
#endif
#ifdef TIMING_LEVEL_4
	unfold_queue_manip.LowPart = 0;
	fold_queue_manip.LowPart = 0;
#endif

	NumTris = GetTriangleCount();
	BytesUsed = GetMemoryUsage();
	curval = UseTriBudget ? NumTris : BytesUsed;

	mpCurrentForest = mpCuts[miCurrentCut]->mpForest;

	bool done = false;
	BudgetItem *UnfoldNode = NULL;
	NodeIndex UnfoldNodeIndex = 0;
	NodeIndex FoldNodeIndex = 0;
	BudgetItem *FoldNode = NULL;
	NodeIndex LastFold = 0, LastUnfold = 0;
	int LastCutFolded, LastCutUnfolded;
	count = 0;

	LastUnfold = 0xFFFFFFFF;
	while (!done)
	{
		LastUnfold = 0xFFFFFFFD;
		LastCutUnfolded = -1;
		while ((curval < (Budget-mBudgetTolerance)) && (mpUnfoldQueue->Size >= 1))
		{
			UnfoldNode = mpUnfoldQueue->FindMin();
			UnfoldNodeIndex = UnfoldNode->miNode;
			
			if (mpCurrentForest->NodesAreCoincidentOrEqual(UnfoldNode->miNode, LastUnfold) && (UnfoldNode->CutID == LastCutUnfolded))
			{
				break;
			}
			LastUnfold = UnfoldNode->miNode;
			LastCutUnfolded = UnfoldNode->CutID;

			Unfold(UnfoldNode, NumTris, BytesUsed);
			curval = UseTriBudget ? NumTris : BytesUsed;
			if (mSimplificationBreakCount)
			{
				++count;
				if (count >= mSimplificationBreakCount)
					return;
			}
		}
		if (mpCurrentForest->NodesAreCoincidentOrEqual(LastUnfold, LastFold))
			done = true;
		
		LastFold = 0xFFFFFFFE;
		LastCutFolded = -1;
		while ((curval >= Budget+mBudgetTolerance) && (mpFoldQueue->Size >= 1))
		{
			FoldNode = mpFoldQueue->FindMin();
			FoldNodeIndex = FoldNode->miNode;
			
			if (mpCurrentForest->NodesAreCoincidentOrEqual(FoldNode->miNode, LastFold) && mpCurrentForest->NodesAreCoincidentOrEqual(FoldNode->CutID, LastCutFolded))
			{
				break;
			}
			LastFold = FoldNode->miNode;
			LastCutFolded = FoldNode->CutID;

			Fold(FoldNode, NumTris, BytesUsed);
			curval = UseTriBudget ? NumTris : BytesUsed;
			if (mSimplificationBreakCount)
			{
				++count;
				if (count >= mSimplificationBreakCount)
					return;
			}
		}
		if (mpCurrentForest->NodesAreCoincidentOrEqual(LastUnfold, LastFold))
			done = true;

		if ((UnfoldNode == NULL) || (FoldNode == NULL))
			done = true;

	} // while (!done)
	miCurrentCut = 0;
}

void Simplifier::SimplifyThreshold(float Threshold)
{
	unsigned int count, NumTris, BytesUsed;
	tris_introduced = 0;
	tris_removed = 0;

	count = 0;
	if (!mIsValid)
		return;

#ifdef TIMING_LEVEL_3
	unfold_time.LowPart = 0;
	fold_time.LowPart = 0;
#endif
#ifdef TIMING_LEVEL_4
	unfold_queue_manip.LowPart = 0;
	fold_queue_manip.LowPart = 0;
#endif

	BudgetItem *UnfoldNode = NULL;
	NodeIndex UnfoldNodeIndex = 0;
	NodeIndex FoldNodeIndex = 0;
	BudgetItem *FoldNode = NULL;
	NodeIndex LastUnfold;
	int LastCutUnfolded;

	while (mpFoldQueue->Size >= 1)
	{
		FoldNode = mpFoldQueue->FindMin();
		FoldNodeIndex = FoldNode->miNode;
		float FoldNodeErr = FoldNode->mError;
		
		if (FoldNodeErr >= Threshold)
		{
			break;
		}
		Fold(FoldNode, NumTris, BytesUsed);
		if (mSimplificationBreakCount)
		{
			++count;
			if (count >= mSimplificationBreakCount)
				return;
		}
	}
	LastUnfold = 0;
	LastCutUnfolded = -1;
	while (mpUnfoldQueue->Size >= 1)
	{
		UnfoldNode = mpUnfoldQueue->FindMin();
		UnfoldNodeIndex = UnfoldNode->miNode;
		float UnfoldNodeErr = UnfoldNode->mError;
		
		if ((-UnfoldNodeErr) < Threshold)
		{
			break;
		}
		Unfold(UnfoldNode, NumTris, BytesUsed);
		if (mSimplificationBreakCount)
		{
			++count;
			if (count >= mSimplificationBreakCount)
				return;
		}
	}
}

void Simplifier::SimplifyBudgetAndThreshold(unsigned int Budget, bool UseTriBudget, float Threshold)
{
	unsigned int count, curval, NumTris, BytesUsed;
	tris_introduced = 0;
	tris_removed = 0;

	count = 0;
	if (!mIsValid)
		return;

	NumTris = GetTriangleCount();
	BytesUsed = GetMemoryUsage();
	curval = UseTriBudget ? NumTris : BytesUsed;
	int expanding = (curval < Budget) ? 1 : 0;
	if (curval == Budget)
		expanding = -1;

	mpCurrentForest = mpCuts[miCurrentCut]->mpForest;

	bool done = false;
	BudgetItem *UnfoldNode = NULL;
	NodeIndex UnfoldNodeIndex = 0;
	NodeIndex FoldNodeIndex = 0;
	BudgetItem *FoldNode = NULL;
	NodeIndex LastFold, LastUnfold;
	int LastCutFolded, LastCutUnfolded;

	LastUnfold = 0;
	while (!done)
	{
		LastUnfold = 0xFFFFFFFD;
		LastCutUnfolded = -1;
		while ((curval <= Budget-mBudgetTolerance) && (mpUnfoldQueue->Size >= 1))
		{
			UnfoldNode = mpUnfoldQueue->FindMin();
			UnfoldNodeIndex = UnfoldNode->miNode;
			
			float UnfoldNodeErr = UnfoldNode->mError;
			
			if (mpCurrentForest->NodesAreCoincidentOrEqual(UnfoldNode->miNode, LastUnfold) && (UnfoldNode->CutID == LastCutUnfolded))
			{
				break;
			}
			if (((-UnfoldNodeErr) <= Threshold) && (expanding == 1))
			{
				UnfoldNode = NULL;
				break;
			}
			LastUnfold = UnfoldNode->miNode;
			LastCutUnfolded = UnfoldNode->CutID;
			
			Unfold(UnfoldNode, NumTris, BytesUsed);
			curval = UseTriBudget ? NumTris : BytesUsed;
			if (mSimplificationBreakCount)
			{
				++count;
				if (count >= mSimplificationBreakCount)
					return;
			}
		}
		if (mpCurrentForest->NodesAreCoincidentOrEqual(LastUnfold, LastFold))
			done = true;

		LastFold = 0xFFFFFFFE;
		LastCutFolded = -1;
		while ((curval > Budget+mBudgetTolerance) && (mpFoldQueue->Size >= 1))
		{
			FoldNode = mpFoldQueue->FindMin();
			FoldNodeIndex = FoldNode->miNode;
			
			float FoldNodeErr = FoldNode->mError;
			
			if (mpCurrentForest->NodesAreCoincidentOrEqual(FoldNode->miNode, LastFold) && (FoldNode->CutID == LastCutFolded))
			{
				break;
			}
			if ((FoldNodeErr >= Threshold) && (expanding == 0))
			{
				FoldNode = NULL;
				break;
			}

			LastFold = FoldNode->miNode;
			LastCutFolded = FoldNode->CutID;

			Fold(FoldNode, NumTris, BytesUsed);
			curval = UseTriBudget ? NumTris : BytesUsed;
			if (mSimplificationBreakCount)
			{
				++count;
				if (count >= mSimplificationBreakCount)
					return;
			}
		}
		if (mpCurrentForest->NodesAreCoincidentOrEqual(LastUnfold, LastFold))
			done = true;
		
		if ((UnfoldNode == NULL) || (FoldNode == NULL))
			done = true;
	} // while (!done)
	miCurrentCut = 0;
}

void Simplifier::SetErrorFunc(ErrorFunc fError)
{
	if (fError != mfErrorFunc)
	{
		mfErrorFunc = fError;
		FlushQueues();
	}
}

unsigned int Simplifier::GetMemoryUsage()
{
	unsigned int TotalMemUsage = 0;
	for (miCurrentCut = 0; miCurrentCut < mNumCuts; ++miCurrentCut)
	{
		TotalMemUsage += mpCuts[miCurrentCut]->mBytesUsed;
	}
	miCurrentCut = 0;
	return TotalMemUsage;
}

unsigned int Simplifier::GetTriangleCount()
{
	unsigned int TotalTriangleCount = 0;
	for (miCurrentCut = 0; miCurrentCut < mNumCuts; ++miCurrentCut)
	{
		TotalTriangleCount += mpCuts[miCurrentCut]->mNumActiveTris;
	}
	miCurrentCut = 0;
	return TotalTriangleCount;
}

const Point3& Simplifier::GetNodePosition(BudgetItem *pItem) const
{
	return mpCuts[pItem->CutID]->mpForest->mpNodes[pItem->miNode].mpRenderData->Position;
}

//const Float& Simplifier::GetNodeRadius(BudgetItem *pItem) const
//{
//	return mpCuts[pItem->CutID]->mpForest->mpNodes[pItem->miNode].mRadius;
//}
/*
const Plane3& Simplifier::GetTopPlane(BudgetItem *pItem) const
{
	return mpCuts[pItem->CutID]->mTopPlane;
}

const Plane3& Simplifier::GetBottomPlane(BudgetItem *pItem) const
{
	return mpCuts[pItem->CutID]->mBottomPlane;
}

const Plane3& Simplifier::GetRightPlane(BudgetItem *pItem) const
{
	return mpCuts[pItem->CutID]->mRightPlane;
}

const Plane3& Simplifier::GetLeftPlane(BudgetItem *pItem) const
{
	return mpCuts[pItem->CutID]->mLeftPlane;
}
*/
/*
const Point3& Simplifier::GetViewpoint(BudgetItem *pItem) const
{
	return mpCuts[pItem->CutID]->mViewpoint;
}

const Float& Simplifier::GetInvTanFov(BudgetItem *pItem) const
{
	return mpCuts[pItem->CutID]->mFov;
}
*/

void Simplifier::Unfold(BudgetItem *pItem, unsigned int &NumTris, unsigned int &BytesUsed)
{
	NodeIndex iChild;
	TriIndex iLiveTri, iNextLiveTri;
	TriIndex iSubTri;
	int NumSubTris = 0, NumChildren = 0;
	int k;
	VertexRenderDatum *newVertexRenderDatum;
	BudgetItem newBudgetItem;

	if (pItem == NULL)
	{
		cerr << "tried to unfold a null element" << endl;
		return;
	}

#ifdef TIMING_LEVEL_3
	LARGE_INTEGER time_before_unfold;
	QueryPerformanceCounter(&time_before_unfold);
#endif

	miCurrentCut = pItem->CutID;
	Cut *pCurrentCut = mpCuts[miCurrentCut];
	mpCurrentForest = pCurrentCut->mpForest;

	NodeIndex iNode = pItem->miNode;
	Node *pNodes = mpCurrentForest->mpNodes;
	Tri *pTris = mpCurrentForest->mpTris;
	BudgetItem **pNodeRefs = pCurrentCut->mpNodeRefs;
	TriProxyBackRef **pTriRefs = pCurrentCut->mpTriRefs;
	Renderer *pRenderer = pCurrentCut->mpRenderer;
	NodeIndex iParent = pNodes[iNode].miParent;

//cout << " unfolding node " << iNode << endl;

	if ((pNodeRefs[pNodes[iNode].miFirstChild] == NULL) && (pNodes[iNode].miFirstChild != Forest::iNIL_NODE))
	{
		if (pNodes[iNode].miParent != Forest::iNIL_NODE)
			pRenderer->SetVertexRenderDatumAboveParentsOfBoundary(pNodeRefs[pNodes[iNode].miParent]->pVertexRenderDatum, true);

		// for each child of iNode:
		iChild = pNodes[iNode].miFirstChild;
		while (iChild != Forest::iNIL_NODE)
		{
			// add VertexRenderDatum to renderer
			newVertexRenderDatum = pRenderer->AddVertexRenderDatum(iChild);
			newVertexRenderDatum->Node = iChild;
			pCurrentCut->mNumActiveNodes++;
			NumChildren++;
			
			// construct BudgetItem for child
			newBudgetItem.miNode = iChild;
			newBudgetItem.CutID = miCurrentCut;
			newBudgetItem.miFirstLiveTri = Forest::iNIL_TRI;

			newBudgetItem.mPosition = pCurrentCut->mpForest->mpNodes[iChild].mpRenderData->Position;
//			newBudgetItem.mRadius = pCurrentCut->mpForest->mpNodes[iChild].mRadius;
			newBudgetItem.mXBBoxOffset = pCurrentCut->mpForest->mpNodes[iChild].mXBBoxOffset;
			newBudgetItem.mYBBoxOffset = pCurrentCut->mpForest->mpNodes[iChild].mYBBoxOffset;
			newBudgetItem.mZBBoxOffset = pCurrentCut->mpForest->mpNodes[iChild].mZBBoxOffset;
			newBudgetItem.mBBoxCenter = pCurrentCut->mpForest->mpNodes[iChild].mBBoxCenter;

			newBudgetItem.mError = -mfErrorFunc(&newBudgetItem, pCurrentCut);

			// set BudgetItem.RenderData pointer to address of child's RenderData
			newBudgetItem.pVertexRenderDatum = newVertexRenderDatum;

			pNodeRefs[iChild] = &newBudgetItem;

#ifdef PRUNING
			// only put child's budgetitem in unfoldqueue if child is not a leaf node
			if (pNodes[iChild].miFirstChild != Forest::iNIL_NODE)
			{
#endif
				// TODO: find place in queue first instead of making budgetitem and then copying into place in queue
				// copy BudgetItem data into unfold queue
				mpUnfoldQueue->Insert(&newBudgetItem);

				// this is needed because the location of pItem could have changed if the unfoldqueue was enlarged during the Insert call
				pItem = pNodeRefs[iNode];
#ifdef PRUNING
			}
			else
			{
				pNodeRefs[iChild] = new BudgetItem;
				if (pNodeRefs[iChild] == NULL)
				{
					cerr << "Memory Error - new returned NULL when creating BudgetItem for pruned child." << endl;
				}
				else
					memcpy(pNodeRefs[iChild], &newBudgetItem, sizeof(BudgetItem));
			}
#endif			
			iChild = pNodes[iChild].miRightSibling;
		}

		BytesUsed += NumChildren * pCurrentCut->mBytesPerNode;

		// for each livetri of iNode:
		for (iLiveTri = pNodeRefs[iNode]->miFirstLiveTri; iLiveTri != Forest::iNIL_TRI; iLiveTri = iNextLiveTri)
		{
			// update proxy which currently points to iNode to point to one of iNode's children 
			// (use children nodeIDs and nodeID of livetri's corner to determine which child)

/* this apparently not needed anymore, but leaving it in until i get around to figuring out why
			// TODO: why need to call GetNodeIndex twice?
			k = pTris[iLiveTri].GetNodeIndex(iLiveTri, Forest::iNIL_NODE, mpCurrentForest, pCurrentCut->mpRenderer);
			if (pTriRefs[iLiveTri]->backrefs[k] != Forest::iNIL_NODE)
			{
				k = pTris[iLiveTri].GetNodeIndex(iLiveTri, iNode, mpCurrentForest, pCurrentCut->mpRenderer);
			}
*/
			k = pTris[iLiveTri].GetNodeIndex(iLiveTri, iNode, mpCurrentForest, pCurrentCut->mpRenderer);
			iNextLiveTri = pTriRefs[iLiveTri]->miNextLiveTris[k];
			
			pTris[iLiveTri].RemoveFromLiveTriList(iLiveTri, iNode, *mpCurrentForest, pCurrentCut->mpRenderer);
			pTris[iLiveTri].MoveProxyDown(iLiveTri, k, *mpCurrentForest, pRenderer);

			// update the proxy - MoveProxyDown actually moves the ProxyBackRef down
			unsigned int tri_index = pTriRefs[iLiveTri] - pRenderer->mpPatchTriData[pTris[iLiveTri].mPatchID].TriProxyBackRefs;
			pRenderer->mpPatchTriData[pTris[iLiveTri].mPatchID].TriProxiesArray[tri_index].proxies[k] = pRenderer->GetVertexRenderDatumIndex(pNodeRefs[pTriRefs[iLiveTri]->backrefs[k]]->pVertexRenderDatum);

			pTris[iLiveTri].AddToLiveTriList(iLiveTri, k, *mpCurrentForest, pRenderer);
		}

		// for each subtri of iNode:
		for (iSubTri = pNodes[iNode].miFirstSubTri; iSubTri != Forest::iNIL_TRI; iSubTri = pTris[iSubTri].miNextSubTri)
		{
			//cout << "\tAdding Tri " << iSubTri << endl;
			++NumSubTris;
			// add TriRenderDatum to renderer
			pRenderer->AddTriRenderDatum(iSubTri, pTris[iSubTri].mPatchID);
		}

		// Add number of subtris to Cut.ActiveTris
		pCurrentCut->mNumActiveTris += NumSubTris;
		NumTris += NumSubTris;
		BytesUsed += NumSubTris * pCurrentCut->mBytesPerTri;
		tris_introduced += NumSubTris;
		
		// because of the addition of the children, pItem likely now points to a different
		// element of the queue than iNode's, so update it to point to iNode's entry again
		pItem = mpUnfoldQueue->Find(pNodeRefs[iNode]);

		mpUnfoldQueue->GiveElementTo(pItem, mpFoldQueue);

		// pItem still points to iNode's old element in UnfoldQueue, so update it
		pItem = mpFoldQueue->Find(pNodeRefs[iNode]);

	//	pItem->mError = mfErrorFunc(pItem, pCurrentCut);

#ifdef REVERSE_PRUNING
		BudgetItem *ParentItem;
		BudgetItem *NewItem;
		if(iParent != Forest::iNIL_NODE)
		{
			bool alreadyremoved = false;

			NodeIndex testnode = iParent;
			do
			{
				iChild = pNodes[testnode].miFirstChild;
				while (iChild != Forest::iNIL_NODE)
				{
					if (pNodes[iChild].miFirstChild != Forest::iNIL_NODE)
					{
						if ((pNodeRefs[pNodes[iChild].miFirstChild] != NULL) && (iChild != iNode))
						{
							alreadyremoved = true;
							break;
						}
					}
					iChild = pNodes[iChild].miRightSibling;
				}
				if (alreadyremoved)
					break;
				testnode = pNodes[testnode].mCoincidentVertex;
			}
			while ((testnode != Forest::iNIL_NODE) && (testnode != iParent));

			if (!alreadyremoved)
			{
				testnode = iParent;
				do
				{
					ParentItem = pNodeRefs[testnode];
					NewItem = new BudgetItem;
					memcpy(NewItem, ParentItem, sizeof(BudgetItem));
					mpFoldQueue->Remove(ParentItem);
					pNodeRefs[testnode] = NewItem;
					testnode = pNodes[testnode].mCoincidentVertex;
				}
				while ((testnode != Forest::iNIL_NODE) && (testnode != iParent));
			}
		}
#endif
		
#ifdef TIMING_LEVEL_4
	LARGE_INTEGER time_before_heapify;
	QueryPerformanceCounter(&time_before_heapify);
#endif
#ifdef TIMING_LEVEL_4
	LARGE_INTEGER time_after_heapify;
	QueryPerformanceCounter(&time_after_heapify);
	unfold_queue_manip.LowPart += (time_after_heapify.LowPart - time_before_heapify.LowPart);
#endif


#ifdef TIMING_LEVEL_3
	LARGE_INTEGER time_after_unfold;
	QueryPerformanceCounter(&time_after_unfold);
	unfold_time.LowPart += (time_after_unfold.LowPart - time_before_unfold.LowPart);
#endif

		if (pNodes[iNode].mCoincidentVertex)
			Unfold(pNodeRefs[pNodes[iNode].mCoincidentVertex], NumTris, BytesUsed);
	}
	miCurrentCut = 0;

}

void Simplifier::Fold(BudgetItem *pItem, unsigned int &NumTris, unsigned int &BytesUsed)
{
	NodeIndex iChild, iParent;
	int numSubTris=0, NumChildren = 0;
	TriIndex iLiveTri, iNextLiveTri;
	TriIndex iSubTri;
	int k;

	if (pItem == NULL)
	{
		cerr << "tried to fold a null element" << endl;
		return;
	}

#ifdef TIMING_LEVEL_3
	LARGE_INTEGER time_before_fold;
	QueryPerformanceCounter(&time_before_fold);
#endif

	miCurrentCut = pItem->CutID;
	Cut *pCurrentCut = mpCuts[miCurrentCut];
	mpCurrentForest = pCurrentCut->mpForest;

	Node *pNodes = pCurrentCut->mpForest->mpNodes;
	Tri *pTris = pCurrentCut->mpForest->mpTris;
	BudgetItem **pNodeRefs = pCurrentCut->mpNodeRefs;
	TriProxyBackRef **pTriRefs = pCurrentCut->mpTriRefs;
	Renderer *pRenderer = pCurrentCut->mpRenderer;
	NodeIndex iNode = pItem->miNode;
	iParent = pNodes[iNode].miParent;

	// check that all of node's children are on boundary
	for (iChild = pNodes[pItem->miNode].miFirstChild; iChild != Forest::iNIL_NODE; iChild = pNodes[iChild].miRightSibling)
	{
		if (pNodeRefs[iChild] == NULL)
		{
//			cout << "Folding node " << pItem->miNode << " failed because child " << iChild << " has null NodeRef." << endl;
			return;
		}
#ifndef REVERSE_PRUNING
		else if (pNodeRefs[pNodes[iChild].miFirstChild] != NULL)
		{
			cout << "Folding node " << pItem->miNode << " failed because child " << iChild << " has first child with non-null NodeRef..." << endl;
			cout << "Forcing fold of node " << iChild << " first." << endl;
			Fold(pNodeRefs[iChild], NumTris, BytesUsed);
			Fold(pItem, NumTris, BytesUsed);
			return;
		}
#endif
	}

//cout << " folding node " << iNode << endl;

	if (iParent != Forest::iNIL_NODE)
		pRenderer->SetVertexRenderDatumAboveParentsOfBoundary(pNodeRefs[iParent]->pVertexRenderDatum, false);

	// for each child of iNode:
	for (iChild = pNodes[iNode].miFirstChild; iChild != Forest::iNIL_NODE; iChild = pNodes[iChild].miRightSibling)
	{
		// for each livetri of the child
		for (iLiveTri = pNodeRefs[iChild]->miFirstLiveTri; iLiveTri != Forest::iNIL_TRI; iLiveTri = iNextLiveTri)
		{
			k = pTris[iLiveTri].GetNodeIndex(iLiveTri, iChild, mpCurrentForest, pCurrentCut->mpRenderer);
			iNextLiveTri = pTriRefs[iLiveTri]->miNextLiveTris[k];

			// move proxy up to parent node
			unsigned int tri_index = pTriRefs[iLiveTri] - pRenderer->mpPatchTriData[pTris[iLiveTri].mPatchID].TriProxyBackRefs;
			pRenderer->mpPatchTriData[pTris[iLiveTri].mPatchID].TriProxiesArray[tri_index][k] = pRenderer->GetVertexRenderDatumIndex(pNodeRefs[iNode]->pVertexRenderDatum);
			pTriRefs[iLiveTri]->backrefs[k] = iNode;

			// add tri to livetri list of parent IF it isn't already a livetri of parent - if it is, then
			// it's a degenerate triangle now anyway and will be removed as a subtri of iNode
			if ((*pTriRefs[iLiveTri])[(k+1)%3] == iNode) 
			{
				pTriRefs[iLiveTri]->miNextLiveTris[k] = pTriRefs[iLiveTri]->miNextLiveTris[(k+1)%3];
				
				if ((*pTriRefs[iLiveTri])[(k+2)%3] == iNode)
				{
					pTriRefs[iLiveTri]->miNextLiveTris[k] = pTriRefs[iLiveTri]->miNextLiveTris[(k+2)%3];
				}
			}
			else if ((*pTriRefs[iLiveTri])[(k+2)%3] == iNode)
			{
				pTriRefs[iLiveTri]->miNextLiveTris[k] = pTriRefs[iLiveTri]->miNextLiveTris[(k+2)%3];
			}
			else
			{
				pTris[iLiveTri].AddToLiveTriList(iLiveTri, k, *mpCurrentForest, pRenderer);
			}
		}
		// now that we've moved all proxies up to parent node, iChild has no livetris and thus a usecount of 0
		pCurrentCut->mpNodeRefs[iChild]->miFirstLiveTri = Forest::iNIL_TRI;
		pRenderer->ZeroVertexUseCount(pCurrentCut->mpNodeRefs[iChild]->pVertexRenderDatum);

		pRenderer->RemoveVertexRenderDatum(pNodeRefs[iChild]->pVertexRenderDatum);
		++NumChildren;

#ifdef PRUNING
		// if child is not a leaf node, remove its BudgetItem from unfoldqueue
		if (pNodes[iChild].miFirstChild != Forest::iNIL_NODE)
		{
#endif
			// remove child from the unfoldqueue
			mpUnfoldQueue->Remove(mpUnfoldQueue->Find(pNodeRefs[iChild]));

			// set child's NodeRef to NULL, because NodeQueue->Remove() does not
			pNodeRefs[iChild] = NULL;
#ifdef PRUNING
		}
		else // if child is a leaf node, its budgetitem was never put into the unfoldqueue, so just delete it
		{
			delete pNodeRefs[iChild];
			pNodeRefs[iChild] = NULL;
		}
#endif

		pCurrentCut->mNumActiveNodes--;
	}

	BytesUsed -= NumChildren * pCurrentCut->mBytesPerNode;

	for (iSubTri = pNodes[iNode].miFirstSubTri; iSubTri != Forest::iNIL_TRI; iSubTri = pTris[iSubTri].miNextSubTri)
	{
		pTris[iSubTri].RemoveFromLiveTriList(iSubTri, pTriRefs[iSubTri]->backrefs[0], *mpCurrentForest, pRenderer);

		if (pTriRefs[iSubTri]->backrefs[1] != pTriRefs[iSubTri]->backrefs[0])
		{
			pTris[iSubTri].RemoveFromLiveTriList(iSubTri, pTriRefs[iSubTri]->backrefs[1], *mpCurrentForest, pRenderer);
		}
		if ((pTriRefs[iSubTri]->backrefs[2] != pTriRefs[iSubTri]->backrefs[0]) && (pTriRefs[iSubTri]->backrefs[2] != pTriRefs[iSubTri]->backrefs[1]))
		{
			pTris[iSubTri].RemoveFromLiveTriList(iSubTri, pTriRefs[iSubTri]->backrefs[2], *mpCurrentForest, pRenderer);
		}
	}

	// TODO: could move this up before the elevation of the proxies of the children's livetris and 
	// then check for a null triref pointer before elevating the proxy
	// for each subtri of iNode:
	numSubTris = 0;
	for (iSubTri = pNodes[iNode].miFirstSubTri; iSubTri != Forest::iNIL_TRI; iSubTri = pTris[iSubTri].miNextSubTri)
	{
		//cout << "\tRemoving Tri " << iSubTri << endl;
		// remove subtri's Proxies entry from Renderer's Trilist's ProxiesArray
		unsigned int tri_index = pTriRefs[iSubTri] - pRenderer->mpPatchTriData[pTris[iSubTri].mPatchID].TriProxyBackRefs;
		pCurrentCut->mpRenderer->RemoveTriRenderDatum(tri_index, pTris[iSubTri].mPatchID);
		pTriRefs[iSubTri] = NULL;
		++numSubTris;
	}
	pCurrentCut->mNumActiveTris -= numSubTris;
	NumTris -= numSubTris;
	BytesUsed -= numSubTris * pCurrentCut->mBytesPerTri;
	tris_removed += numSubTris;

	// because of the removal of the children, pItem possibly points to an
	// element of the queue other than iNode's, so update it
	pItem = mpFoldQueue->Find(pNodeRefs[iNode]);

	mpFoldQueue->GiveElementTo(pItem, mpUnfoldQueue);

	// pItem still points to iNode's old element in FoldQueue, so update it
	pItem = mpUnfoldQueue->Find(pNodeRefs[iNode]);


#ifdef REVERSE_PRUNING
	if(iParent != Forest::iNIL_NODE)
	{
		bool lastchildfolded = true;

		NodeIndex testnode = iParent;
		do
		{
			iChild = pNodes[testnode].miFirstChild;
			while (iChild != Forest::iNIL_NODE)
			{
				if (pNodes[iChild].miFirstChild != Forest::iNIL_NODE)
				{
					if (pNodeRefs[pNodes[iChild].miFirstChild] != NULL)
					{
						lastchildfolded = false;
						break;
					}
				}
				iChild = pNodes[iChild].miRightSibling;
			}
			if (!lastchildfolded)
				break;
			testnode = pNodes[testnode].mCoincidentVertex;
		}
		while ((testnode != Forest::iNIL_NODE) && (testnode != iParent));
		if (lastchildfolded)
		{
			testnode = iParent;
			do
			{
				BudgetItem *OldParentItem = pNodeRefs[testnode];
				mpFoldQueue->Insert(pNodeRefs[testnode]);
				delete OldParentItem;
				testnode = pNodes[testnode].mCoincidentVertex;
			}
			while ((testnode != Forest::iNIL_NODE) && (testnode != iParent));
		}
	}
#endif	


#ifdef TIMING_LEVEL_4
	LARGE_INTEGER time_before_heapify;
	QueryPerformanceCounter(&time_before_heapify);
#endif
#ifdef TIMING_LEVEL_4
	LARGE_INTEGER time_after_heapify;
	QueryPerformanceCounter(&time_after_heapify);
	fold_queue_manip.LowPart += (time_after_heapify.LowPart - time_before_heapify.LowPart);
#endif

#ifdef TIMING_LEVEL_3
	LARGE_INTEGER time_after_fold;
	QueryPerformanceCounter(&time_after_fold);
	fold_time.LowPart += (time_after_fold.LowPart - time_before_fold.LowPart);
#endif

	if (pNodes[iNode].mCoincidentVertex)
		Fold(pNodeRefs[pNodes[iNode].mCoincidentVertex], NumTris, BytesUsed);
}

void Simplifier::DisplayQueues()
{
	int i,j;
	cout << "Unfold Queue: " << flush;
	for (i = 1; i <= mpUnfoldQueue->Size; ++i)
	{
		cout << mpUnfoldQueue->Elements[i].miNode << "(" << mpUnfoldQueue->Elements[i].mError << ") " << flush;
		for (j = 1; j <= mpUnfoldQueue->Size; ++j)
		{
			if ((mpUnfoldQueue->Elements[j].miNode == mpUnfoldQueue->Elements[i].miNode) && (i != j))
				cout << " (duplicate queue entry detected) " << flush;
		}
	}
	cout << endl;

	cout << "Fold Queue: " << flush;
	for (i = 1; i <= mpFoldQueue->Size; ++i)
	{
		cout << mpFoldQueue->Elements[i].miNode << "(" << mpFoldQueue->Elements[i].mError << ") " << flush;
		for (j = 1; j <= mpFoldQueue->Size; ++j)
		{
			if ((mpFoldQueue->Elements[j].miNode == mpFoldQueue->Elements[i].miNode) && (i != j))
				cout << " (duplicate queue entry detected) " << flush;
		}
	}
	cout << endl << endl;
}

void Simplifier::CheckLiveTrisProxies(Forest *pForest, Renderer *pRenderer)
{
	unsigned int i, k, proxy, l, cached_tri_index;
    int j;
	TriIndex livetri, nextlivetri;
	bool proxyfound;
	NodeIndex cached_vertex_index;

	for (i = 1; i < pForest->mNumTris; ++i)
	{
		for (j = 0; j < mNumCuts; ++j)
		{
			if (mpCuts[j]->mpTriRefs[i] != NULL)
			{
				for (k = 0; k < 3; ++k)
				{
					proxy = mpCuts[j]->mpTriRefs[i]->backrefs[k];
					proxyfound = false;

if ((i == 6) && (proxy == 37))
	cout << "break" << endl;

					livetri = pRenderer->mpCut->mpNodeRefs[proxy]->miFirstLiveTri;
					while (livetri != 0)
					{
						l = pForest->mpTris[livetri].GetNodeIndex(livetri, proxy, pForest, pRenderer);
						nextlivetri = pRenderer->mpCut->mpTriRefs[livetri]->miNextLiveTris[l];

						if (livetri == i)
							proxyfound = true;

						livetri = nextlivetri;
					}

					if (!proxyfound)
					{
						cerr << "node " << proxy << " = proxy " << k << " of tri " << i << "." << endl;
						cerr << "triangle " << i << "was not found in node " << proxy << "'s livetri list." << endl;
					}

					cached_vertex_index = pRenderer->GetVertexRenderDatumIndex(mpCuts[miCurrentCut]->mpNodeRefs[proxy]->pVertexRenderDatum);
					cached_tri_index = mpCuts[j]->mpTriRefs[i] - pRenderer->mpPatchTriData[pForest->mpTris[i].mPatchID].TriProxyBackRefs;
					if (cached_vertex_index != pRenderer->mpPatchTriData[pForest->mpTris[i].mPatchID].TriProxiesArray[cached_tri_index][k])
					{
						cerr << "triangle " << i << "'s proxy index " << k << " is " 
							<< pRenderer->mpPatchTriData[pForest->mpTris[i].mPatchID].TriProxiesArray[cached_tri_index][k] 
							<< ", which is not equal to the cached index of its corresponding proxy backref (" 
							<< cached_vertex_index << ")" << endl;
						cerr << "triangle " << i << "'s cached index is " << cached_tri_index << endl;
					}
				}
			}
		}
	}

	for (i = 1; i < pForest->mNumNodes; ++i)
	{
		for (j = 0; j < mNumCuts; ++j)
		{
			if (mpCuts[j]->mpNodeRefs[i] != NULL)
			{
				livetri = mpCuts[j]->mpNodeRefs[i]->miFirstLiveTri;
				while (livetri != 0)
				{
					l = pForest->mpTris[livetri].GetNodeIndex(livetri, i, pForest, pRenderer);
					nextlivetri = pRenderer->mpCut->mpTriRefs[livetri]->miNextLiveTris[l];

					if (
						(pRenderer->mpCut->mpTriRefs[livetri]->backrefs[0] != i) &&
						(pRenderer->mpCut->mpTriRefs[livetri]->backrefs[1] != i) &&
						(pRenderer->mpCut->mpTriRefs[livetri]->backrefs[2] != i))
					{
						cerr << "node " << i << "'s livetri, tri " << livetri << "does not have " << i << " as a proxy." << endl;
						cerr << "\ttri " << livetri << "'s proxies: " 
							<< pRenderer->mpCut->mpTriRefs[livetri]->backrefs[0] << " "
							<< pRenderer->mpCut->mpTriRefs[livetri]->backrefs[1] << " "
							<< pRenderer->mpCut->mpTriRefs[livetri]->backrefs[2] << endl;
						cerr << "";
					}

					livetri = nextlivetri;
				}
			}
		}
	}
}

void Simplifier::CheckNodeAndChildrensLiveTrisProxies(NodeIndex iNode, Forest *pForest, Renderer *pRenderer)
{
	cerr << "checking " << iNode << endl;

	unsigned int child, k;
	TriIndex livetri, nextlivetri;

	livetri = pRenderer->mpCut->mpNodeRefs[iNode]->miFirstLiveTri;
	while (livetri != 0)
	{
		k = pForest->mpTris[livetri].GetNodeIndex(livetri, iNode, pForest, pRenderer);
		nextlivetri = pRenderer->mpCut->mpTriRefs[livetri]->miNextLiveTris[k];
		livetri = nextlivetri;
	}

	child = pForest->mpNodes[iNode].miFirstChild;
	while (child != Forest::iNIL_NODE)
	{
		livetri = pRenderer->mpCut->mpNodeRefs[child]->miFirstLiveTri;
		while (livetri != 0)
		{
			k = pForest->mpTris[livetri].GetNodeIndex(livetri, child, pForest, pRenderer);
			nextlivetri = pRenderer->mpCut->mpTriRefs[livetri]->miNextLiveTris[k];
			livetri = nextlivetri;
		}
		child = pForest->mpNodes[child].miRightSibling;
	}
}

void Simplifier::CheckTrisForNullProxy(Forest *pForest, Renderer *pRenderer)
{
	unsigned int i;
	for (i = 1; i <= pForest->mNumTris; ++i)
	{
		if (pRenderer->mpCut->mpTriRefs[i] != NULL)
		{
			if (pRenderer->mpCut->mpTriRefs[i]->backrefs[0] == Forest::iNIL_NODE)
			{
				cerr << "Error - tri " << i << " has proxies " << pRenderer->mpCut->mpTriRefs[i]->backrefs[0] << " " << pRenderer->mpCut->mpTriRefs[i]->backrefs[1] << " " << pRenderer->mpCut->mpTriRefs[i]->backrefs[2] << endl;
				cerr << endl;
			}
		}
	}
}
