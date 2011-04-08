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
#endif

#include <cmath>
#include <limits.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "renderer.h"
#include "simplifier.h"
#include "manager.h"

using namespace VDS;
using namespace std;

Renderer::Renderer(unsigned int NumInitialVerticesToAllocate, unsigned int NumInitialTrisToAllocate)
{
	unsigned int i;
	mIsValid = false;
	mfRender = NULL;
	mpVertexRenderData = NULL;
	mpSystemVertexRenderData = NULL;
	mUseFastMemory = false;
	mCopyDataToFastMemoryPerFrame = false;
	mNumVertices = 0;
	mLastActiveVertex = 0;
	mpVertexActiveFlags = NULL;
	mpVertexAboveParentsOfBoundaryFlags = NULL;
	mpVertexUseCounts = NULL;
	mpVertexNodeIDs = NULL;
	mpCut = NULL;

	mVertexDataStride = sizeof(VertexRenderDatum);
	if ((mVertexDataStride % 4) != 0)
	{
		cerr << "ERROR - to support rendering from AGP/video memory, Vertex Array stride restricted to being a multiple of 4" << endl;
		exit(666);
	}

	mNumPatches = 0;
	mNumTris = 0;
	mpPatchTriData = NULL;

	mpMemoryManager = NULL;
	mSlackBytes = 0;
	mNumVerticesAllocated = 0;

	if (sizeof(ProxyIndex) == 2)
		VertexIndexSizeLimit = 65535;
	else if (sizeof(ProxyIndex) == 4)
		VertexIndexSizeLimit = 0xFFFFFFFF;
	else
	{
		cerr << "Why is your ProxyIndex some weird size?" << endl;
		VertexIndexSizeLimit = (unsigned int) pow(2.0f, 8.0f * (float)sizeof(ProxyIndex));
	}

	mNumInitialTrisToAllocate = NumInitialTrisToAllocate;

	mpSystemVertexRenderData = new VertexRenderDatum[NumInitialVerticesToAllocate];
	mNumVerticesAllocated = NumInitialVerticesToAllocate;
	mpFastVertexRenderData = NULL;
	mpOldFastVertexRenderData = NULL;
	mpTriProxyData = NULL;

	// initialize vertexrenderdatums just allocated
	for (i = 0; i < mNumVerticesAllocated; ++i)
	{
		mpSystemVertexRenderData[i].Node = 0;
	}
	mNumVertexSlotsFree = 0;
	PopulateVertexSlotsCache();
}

Renderer::~Renderer()
{
    // detatch from memory manager...
    mpMemoryManager->RemoveRenderer(this);
    
    unsigned int i;
	if (mpPatchTriData != NULL)
	{
		for (i = 0; i < mNumPatches; ++i)
		{
			free(mpPatchTriData[i].TriMemoryAllocated);
		}
		delete[] mpPatchTriData;
	}
	if (mpVertexActiveFlags != NULL)
		delete[] mpVertexActiveFlags;
	if (mpVertexAboveParentsOfBoundaryFlags != NULL)
		delete[] mpVertexAboveParentsOfBoundaryFlags;
	if (mpVertexUseCounts != NULL)
		delete[] mpVertexUseCounts;
	if (mpSystemVertexRenderData != NULL)
		delete[] mpSystemVertexRenderData;
}

void Renderer::RenderPatch(PatchIndex PatchID)
{
	mfRender(*this, PatchID);
}

void Renderer::SetRenderFunc(RenderFunc fRender)
{
    mfRender = fRender;
}

void Renderer::FlushRenderData()
{
	unsigned int i, j;

	for (i = 0; i < mNumPatches; ++i)
	{
		mpPatchTriData[i].TriFreeSlots.Reset();
		mpPatchTriData[i].NumTris = 0;
		mpPatchTriData[i].LastActiveTri = 0;

		for (j = 0; j < mpPatchTriData[i].NumTrisAllocated; ++j)
		{
			mpPatchTriData[i].TriProxyBackRefs[j][0] = Forest::iNIL_NODE;
			mpPatchTriData[i].TriProxyBackRefs[j].miNextLiveTris[0] = Forest::iNIL_TRI;
			mpPatchTriData[i].TriProxyBackRefs[j].miNextLiveTris[1] = Forest::iNIL_TRI;
			mpPatchTriData[i].TriProxyBackRefs[j].miNextLiveTris[2] = Forest::iNIL_TRI;
			mpPatchTriData[i].TriProxiesArray[j][0] = 0;
			mpPatchTriData[i].TriProxiesArray[j][1] = 0;
			mpPatchTriData[i].TriProxiesArray[j][2] = 0;
		}
		mpPatchTriData[i].NumTriSlotsFree = mpPatchTriData[i].NumTrisAllocated;
		mpPatchTriData[i].TriFreeSlots.Reset();
		PopulateTriSlotsCache(i);
	}

	mNumVertices = 0;
	mLastActiveVertex = 0;
	mNumVertexSlotsFree = 0;
	mNumTris = 0;
	mpCut->mNumActiveTris = 0;
	for (i = 0; i < mNumVerticesAllocated; ++i)
	{
		mpVertexActiveFlags[i] = false;
		mpVertexAboveParentsOfBoundaryFlags[i] = false;
		mpVertexUseCounts[i] = 0;
	}
	mVertexFreeSlots.Reset();
	PopulateVertexSlotsCache();

	mpCut->mBytesUsed = 0;
}

void Renderer::AddCut(Cut *pCut)
{
	unsigned int i, j;
	if (mfRender == NULL)
	{
		fprintf(stderr, "Must set a renderfunction before adding cuts to renderer\n");
		return;
	}
	if (mpCut != NULL)
	{
		fprintf(stderr, "Tried to add a cut to a renderer that already has a cut\n");
		return;
	}

	mNumInitialTrisToAllocate = pCut->mpForest->mNumTris;
	mpVertexRenderData = mpSystemVertexRenderData;

	mpCut = pCut;

	if (pCut->mpForest->mNumPatches == 0)
	{
		fprintf(stderr, "Error - cut being added to renderer has 0 patches\n");
		return;
	}

	mNumPatches = pCut->mpForest->mNumPatches;
	mpPatchTriData = new PatchRenderTris[mNumPatches];
	unsigned int TrisAllocatedPerPatch = mNumInitialTrisToAllocate / mNumPatches;
	
	unsigned int MemoryPerPatch;
		MemoryPerPatch = (TrisAllocatedPerPatch) * (sizeof(TriProxy) + sizeof(TriProxyBackRef));

	for (i = 0; i < mNumPatches; ++i)
	{
		mpPatchTriData[i].NumTrisAllocated = TrisAllocatedPerPatch;
		mpPatchTriData[i].ColorsPresent = pCut->mpForest->mColorsPresent;
		mpPatchTriData[i].NormalsPresent = pCut->mpForest->mNormalsPresent;
		mpPatchTriData[i].NumTris = 0;
		mpPatchTriData[i].LastActiveTri = 0;
#ifdef VERBOSE_MEM_MANAGEMENT
	cerr << "Allocating Patch " << i << " Tri RenderData; capacity is " << TrisAllocatedPerPatch << " Tris." << endl;
#endif
		mpPatchTriData[i].TriMemoryAllocated = malloc(MemoryPerPatch);
		void *BackRefsStart;
		mpPatchTriData[i].TriProxiesArray = (TriProxy *) mpPatchTriData[i].TriMemoryAllocated;
		BackRefsStart = &mpPatchTriData[i].TriProxiesArray[mpPatchTriData[i].NumTrisAllocated];
		mpPatchTriData[i].TriProxyBackRefs = (TriProxyBackRef *) BackRefsStart;
		for (j = 0; j < mpPatchTriData[i].NumTrisAllocated; ++j)
		{
			mpPatchTriData[i].TriProxyBackRefs[j][0] = Forest::iNIL_NODE;
			mpPatchTriData[i].TriProxyBackRefs[j].miNextLiveTris[0] = Forest::iNIL_TRI;
			mpPatchTriData[i].TriProxyBackRefs[j].miNextLiveTris[1] = Forest::iNIL_TRI;
			mpPatchTriData[i].TriProxyBackRefs[j].miNextLiveTris[2] = Forest::iNIL_TRI;
			mpPatchTriData[i].TriProxiesArray[j][0] = 0;
			mpPatchTriData[i].TriProxiesArray[j][1] = 0;
			mpPatchTriData[i].TriProxiesArray[j][2] = 0;
		}
		mpPatchTriData[i].NumTriSlotsFree = mpPatchTriData[i].NumTrisAllocated;
		PopulateTriSlotsCache(i);
	}

	mpVertexActiveFlags = new bool[mNumVerticesAllocated];
	mpVertexAboveParentsOfBoundaryFlags = new bool[mNumVerticesAllocated];
	mpVertexUseCounts = new unsigned int[mNumVerticesAllocated];
	for (i = 0; i < mNumVerticesAllocated; ++i)
	{
		mpVertexActiveFlags[i] = false;
		mpVertexAboveParentsOfBoundaryFlags[i] = false;
		mpVertexUseCounts[i] = 0;
	}

	pCut->mBytesUsed = 0;
	if (pCut->mIsValid)
		mIsValid = true;
}

void Renderer::RemoveCut(Cut *pCut)
{
	cerr << "Renderer::RemoveCut() not implemented yet." << endl;
}

VertexRenderDatum *Renderer::AddVertexRenderDatum(NodeIndex iNode)
{
	NodeIndex CacheLocation;
	if (mVertexFreeSlots.mSlotsCached > 0)
	{
		mNumVertexSlotsFree--;
		CacheLocation = mVertexFreeSlots.GetFreeSlot();
		mSlackBytes -= mpCut->mBytesPerNode;
	}
	else
	{
		if (mNumVertexSlotsFree > 0)
		{
			PopulateVertexSlotsCache();
			assert(mVertexFreeSlots.mSlotsCached > 0);
			mNumVertexSlotsFree--;
			CacheLocation = mVertexFreeSlots.GetFreeSlot();
			mSlackBytes -= mpCut->mBytesPerNode;
		}
		else
		{
			if (mNumVertices == mNumVerticesAllocated)
			{
// TODO: fix for new memory allocation scheme
//				if (!ReallocateMemoryForRenderData(2 * mNumVerticesAllocated, 2 * mNumTrisAllocated))
//				{
					cerr << "Error - couldn't reallocate renderdata memory; AddVertexRenderDatum failed" << endl;
					return NULL;
//				}
			}
			CacheLocation = mNumVertices;
			++mNumVertices;
		}
	}
	if (mpVertexActiveFlags[CacheLocation])
	{
		cerr << "we got an active one" << endl;
		return NULL;
	}

	if (mpVertexUseCounts[CacheLocation] > 0)
	{
		cerr << "we got a live one" << endl;
		return NULL;
	}

	VertexRenderDatum *pNewVertexRenderDatum = CacheVertex(CacheLocation, &(mpCut->mpForest->mpNodes[iNode]));

	mpVertexActiveFlags[CacheLocation] = true;
	mpVertexUseCounts[CacheLocation] = 0;
	mpVertexAboveParentsOfBoundaryFlags[CacheLocation] = false;

	mpCut->mBytesUsed += mpCut->mBytesPerNode;

	if (CacheLocation > mLastActiveVertex)
	{
		mLastActiveVertex = CacheLocation;
	}

	return pNewVertexRenderDatum;	
}

VertexRenderDatum *Renderer::CacheVertex(NodeIndex iVertexArrayLocation, Node *pNode)
{
	mpVertexRenderData[iVertexArrayLocation].Position = pNode->mpRenderData->Position;
	mpVertexRenderData[iVertexArrayLocation].Color = pNode->mpRenderData->Color;
	mpVertexRenderData[iVertexArrayLocation].Normal = pNode->mpRenderData->Normal;
	mpVertexRenderData[iVertexArrayLocation].TexCoords = pNode->mpRenderData->TexCoords;
	return &mpVertexRenderData[iVertexArrayLocation];
}

void Renderer::RemoveVertexRenderDatum(VertexRenderDatum *pVertexRenderDatum)
{
	unsigned int index = pVertexRenderDatum - mpVertexRenderData;
#ifdef _DEBUG
	if (mpVertexUseCounts[index] != 0)
		cout << "node going inactive has positive usecount" << endl;
#endif

	mpCut->mBytesUsed -= mpCut->mBytesPerNode;
	mSlackBytes += mpCut->mBytesPerNode;

	mpVertexActiveFlags[index] = false;
	mVertexFreeSlots.AddFreeSlot(index);
	mNumVertexSlotsFree++;

	if (index == mLastActiveVertex)
	{
		int i = index - 1;
		while ((i > 0) && (mpVertexActiveFlags[i] == false))
		{
			--i;
			mLastActiveVertex = i;
		}
	}
}

void Renderer::AddTriRenderDatum(TriIndex iTri, PatchIndex PatchID)
{
	unsigned int NextFreeSlot = 0xFFFFFFFF;
	TriIndex iTriArrayLocation;
	Forest *pForest = mpCut->mpForest;
	
	if (mpPatchTriData[PatchID].TriFreeSlots.mSlotsCached > 0)
	{
		mpPatchTriData[PatchID].NumTriSlotsFree--;
		NextFreeSlot = mpPatchTriData[PatchID].TriFreeSlots.GetFreeSlot();
		mSlackBytes -= mpCut->mBytesPerTri;
	}
	else
	{
		if (mpPatchTriData[PatchID].NumTriSlotsFree > 0)
		{
			PopulateTriSlotsCache(PatchID);
			assert(mpPatchTriData[PatchID].TriFreeSlots.mSlotsCached > 0);
			mpPatchTriData[PatchID].NumTriSlotsFree--;
			NextFreeSlot = mpPatchTriData[PatchID].TriFreeSlots.GetFreeSlot();
			mSlackBytes -= mpCut->mBytesPerTri;
		}
		else
		{
			if (mpPatchTriData[PatchID].NumTris == mpPatchTriData[PatchID].NumTrisAllocated)
			{
				if (!ReallocateTriRenderData(PatchID, (unsigned int) (1.5 * mpPatchTriData[PatchID].NumTrisAllocated)))
				{
					cerr << "Error - unable to reallocate memory for renderdata; AddTriRenderDatum failed" << endl;
					return;
				}
			}
			NextFreeSlot = mpPatchTriData[PatchID].NumTris;
		}
	}
	iTriArrayLocation = NextFreeSlot;
	mpCut->mpTriRefs[iTri] = &mpPatchTriData[PatchID].TriProxyBackRefs[iTriArrayLocation];
	pForest->mpTris[iTri].InitializeProxiesAndLiveTris(iTri, *pForest, this, pForest->mpTris[iTri].mPatchID);
#ifdef _DEBUG
	int k;
	for (k = 0; k < 3; ++k)
	{
		if (mpPatchTriData[PatchID].TriProxyBackRefs[iTriArrayLocation][k] > mpCut->mpForest->mNumNodes)
		{
			cerr << "proxy out of range" << endl;
		}
		if (mpCut->mpNodeRefs[mpPatchTriData[PatchID].TriProxyBackRefs[iTriArrayLocation][k]] == NULL)
		{
			cerr << "proxy is inactive node" << endl;
		}
	}
#endif
	mpCut->mBytesUsed += mpCut->mBytesPerTri;
	++mpPatchTriData[PatchID].NumTris;
	++mNumTris;
	if (iTriArrayLocation > mpPatchTriData[PatchID].LastActiveTri)
		mpPatchTriData[PatchID].LastActiveTri = iTriArrayLocation;
}

void Renderer::RemoveTriRenderDatum(TriIndex iTri, PatchIndex PatchID)
{
	unsigned int i;
	mpPatchTriData[PatchID].TriProxyBackRefs[iTri][0] = Forest::iNIL_NODE;
	mpPatchTriData[PatchID].TriProxyBackRefs[iTri].miNextLiveTris[0] = Forest::iNIL_TRI;
	mpPatchTriData[PatchID].TriProxyBackRefs[iTri].miNextLiveTris[1] = Forest::iNIL_TRI;
	mpPatchTriData[PatchID].TriProxyBackRefs[iTri].miNextLiveTris[2] = Forest::iNIL_TRI;
	mpPatchTriData[PatchID].TriProxiesArray[iTri][0] = 0;
	mpPatchTriData[PatchID].TriProxiesArray[iTri][1] = 0;
	mpPatchTriData[PatchID].TriProxiesArray[iTri][2] = 0;
	
	mSlackBytes += mpCut->mBytesPerTri;
	mpCut->mBytesUsed -= mpCut->mBytesPerTri;

	mpPatchTriData[PatchID].TriFreeSlots.AddFreeSlot(iTri);
	mpPatchTriData[PatchID].NumTriSlotsFree++;
	--mpPatchTriData[PatchID].NumTris;
	--mNumTris;
	if (iTri == mpPatchTriData[PatchID].LastActiveTri)
	{
		i = iTri;
		while ((i > 0) && (mpPatchTriData[PatchID].TriProxyBackRefs[i][0] == Forest::iNIL_NODE))
		{
			--i;
		}
		mpPatchTriData[PatchID].LastActiveTri = i;
	}
}

NodeIndex Renderer::GetVertexRenderDatumIndex(VertexRenderDatum *pVRD)
{
#ifdef _DEBUG
	if (pVRD == NULL)
	{
		cerr << "error - null pVRD" << endl;
		return -1;
	}
#endif
	return (pVRD - mpVertexRenderData);
}

NodeIndex Renderer::GetVertexCacheBackRef(NodeIndex iVertexCacheIndex, Forest *pForest)
{
	NodeIndex i;
	for (i = pForest->iROOT_NODE; i <= pForest->mNumNodes; ++i)
	{
		if (mpCut->mpNodeRefs[i] != NULL)
		{
			if (GetVertexRenderDatumIndex(mpCut->mpNodeRefs[i]->pVertexRenderDatum) == iVertexCacheIndex)
				return i;
		}
	}
	return 0xFFFFFFFF;
}

bool Renderer::ReallocateTriRenderData(PatchIndex PatchID, unsigned int newTrisAllocated)
{
	unsigned int i, index;
	unsigned int newTriProxiesSize = newTrisAllocated * sizeof(TriProxy);
	unsigned int newTriProxyBackRefsSize = newTrisAllocated * sizeof(TriProxyBackRef);
	void *newTriMemory = malloc(newTriProxiesSize + newTriProxyBackRefsSize);

	if (newTriMemory == NULL)
	{
		cerr << "Error allocating more memory for Tri renderdata" << endl;
		return false;
	}
#ifdef VERBOSE_MEM_MANAGEMENT
	cerr << "Reallocating Patch " << PatchID << " Tri RenderData; capacity increased to " << newTrisAllocated << " Tris." << endl;
#endif

	TriProxy *newTriProxiesArray = (TriProxy *) newTriMemory;
	memcpy(newTriProxiesArray, mpPatchTriData[PatchID].TriProxiesArray, mpPatchTriData[PatchID].NumTrisAllocated * sizeof(TriProxy));
	void *newBackRefsStart = &newTriProxiesArray[newTrisAllocated];
	TriProxyBackRef *newTriProxyBackRefs = (TriProxyBackRef *)newBackRefsStart;
	memcpy(newTriProxyBackRefs, mpPatchTriData[PatchID].TriProxyBackRefs, mpPatchTriData[PatchID].NumTrisAllocated * sizeof(TriProxyBackRef));
	
	for (i = mpPatchTriData[PatchID].NumTrisAllocated; i < newTrisAllocated; ++i)
	{
		newTriProxyBackRefs[i].backrefs[0] = Forest::iNIL_NODE;
		newTriProxyBackRefs[i].miNextLiveTris[0] = Forest::iNIL_TRI;
		newTriProxyBackRefs[i].miNextLiveTris[1] = Forest::iNIL_TRI;
		newTriProxyBackRefs[i].miNextLiveTris[2] = Forest::iNIL_TRI;
		newTriProxiesArray[i].proxies[0] = 0;
		newTriProxiesArray[i].proxies[1] = 0;
		newTriProxiesArray[i].proxies[2] = 0;
	}
	
	for (i = 1; i <= mpCut->mpForest->mNumTris; ++i)
	{
		if (mpCut->mpTriRefs[i] != NULL)
		{
			index = mpCut->mpTriRefs[i] - mpPatchTriData[PatchID].TriProxyBackRefs;
			if ((index >= 0) && (index < mpPatchTriData[PatchID].NumTrisAllocated))
			{
				mpCut->mpTriRefs[i] = &newTriProxyBackRefs[index];
			}
		}
	}

	free(mpPatchTriData[PatchID].TriProxiesArray);
	mpPatchTriData[PatchID].TriProxiesArray = newTriProxiesArray;
	mpPatchTriData[PatchID].TriProxyBackRefs = newTriProxyBackRefs;
	mpPatchTriData[PatchID].NumTrisAllocated = newTrisAllocated;
	return true;
}

/*
bool Renderer::ReallocateMemoryForRenderData(unsigned int NumNodes, unsigned int NumTris)
{
	cerr << "Memory reallocation not currently implemented" << endl;

	return false;
}

bool Renderer::AllocateSystemMemoryForRenderData()
{
	unsigned int i;

	if (mpMemoryManager->AllocateSystemMemory(this, mNumInitialVerticesToAllocate * sizeof(VertexRenderDatum), mNumInitialTrisToAllocate * sizeof(TriProxy), mNumInitialTrisToAllocate * sizeof(TriProxyBackRef)))
		{
#ifdef VERBOSE_MEM_MANAGEMENT
//	cerr << "Allocated initial system memory to hold " << mNumVerticesAllocated << " vertices and " << mNumTrisAllocated << " triangles." << endl;
#endif
		// initialize vertexrenderdatums just allocated
		for (i = 0; i < mNumVerticesAllocated; ++i)
		{
			mpSystemVertexRenderData[i].UseCount = 0;
			mpSystemVertexRenderData[i].Active = false;
			mpSystemVertexRenderData[i].Node = 0;
		}


//		// initialize proxy backrefs just allocated
//		for (i = 0; i < mNumTrisAllocated; ++i)
//		{
//			mpTriProxyBackRefs[i][0] = Forest::iNIL_NODE;
//			mpTriProxyBackRefs[i][1] = Forest::iNIL_NODE;
//			mpTriProxyBackRefs[i][2] = Forest::iNIL_NODE;
//		}
		PopulateVertexSlotsCache();
	}
	else
	{
		cerr << "Error allocating initial memory for renderdata" << endl;
		return false;
	}
	return true;
}

bool Renderer::AllocateFastMemoryForVertexRenderData()
{
	unsigned int SizeOfVertexMemory = mNumVerticesAllocated * sizeof(VertexRenderDatum);
	return false;

	if (mpMemoryManager->AllocateFastMemory(this, SizeOfVertexMemory))
	{
//		if (mCopyDataToFastMemoryPerFrame)
//			UseSystemMemoryVertexData();			
//		else
//			UseFastMemoryVertexData();

		return true;
	}
	else
	{
		return false;
	}

}
*/
void Renderer::CopyVertexDataToFastMemory()
{
	memcpy(mpFastVertexRenderData, mpSystemVertexRenderData, (mLastActiveVertex + 1) * sizeof(VertexRenderDatum));
}

void Renderer::FreeSystemRenderMemory()
{
//	mpMemoryManager->FreeSystemMemory(this);
}

void Renderer::SetUseFastMemory(bool UseFastMemory)
{
	if (UseFastMemory && (!mUseFastMemory))
	{
//		unsigned int SizeOfVertexMemory = mNumVerticesAllocated * sizeof(VertexRenderDatum);
		cout << "\nUsing fast memory for vertex data" << endl;

		if (!mCopyDataToFastMemoryPerFrame)
		{
			UseFastMemoryVertexData();
		}
	}
	else if ((!UseFastMemory) && mUseFastMemory)
	{
		cout << "\nUsing system memory for vertex data" << endl;

		if (!mCopyDataToFastMemoryPerFrame)
		{
			UseSystemMemoryVertexData();
		}
	}
	mUseFastMemory = UseFastMemory;
}

void Renderer::SetCopyPerFrame(bool CopyPerFrame)
{
	if (mUseFastMemory)
	{
		if (CopyPerFrame && (!mCopyDataToFastMemoryPerFrame))
		{
//			cout << "\nUsing copy-per-frame vertex data simplification" << endl;
			UseSystemMemoryVertexData();
		}
		else if ((!CopyPerFrame) && mCopyDataToFastMemoryPerFrame)
		{
//			cout << "\nUsing in-place vertex data simplification" << endl;
			UseFastMemoryVertexData();
		}
	}
	else
	{
//		if (CopyPerFrame)
//			cout << "\nCopying vertex render data to fast memory per frame" << endl;
//		else
//			cout << "\nAdapting vertex render data in-place in fast memory" << endl;
	}
	mCopyDataToFastMemoryPerFrame = CopyPerFrame;
	
}

void Renderer::UseSystemMemoryVertexData()
{
	unsigned int j, index;

	if (mpVertexRenderData == mpSystemVertexRenderData)
		return;

	memcpy(mpSystemVertexRenderData, mpFastVertexRenderData, mNumVertices * sizeof(VertexRenderDatum));
	for (j = 1; j <= mpCut->mpForest->mNumNodes; ++j)
	{
		if (mpCut->mpNodeRefs[j] != NULL)
		{
			index = mpCut->mpNodeRefs[j]->pVertexRenderDatum - mpFastVertexRenderData;
			mpCut->mpNodeRefs[j]->pVertexRenderDatum = &mpSystemVertexRenderData[index];
		}
	}
	mpVertexRenderData = mpSystemVertexRenderData;
}

void Renderer::UseFastMemoryVertexData()
{
	unsigned int j, index;

	if (mpVertexRenderData == mpFastVertexRenderData)
		return;

	memcpy(mpFastVertexRenderData, mpSystemVertexRenderData, mNumVertices * sizeof(VertexRenderDatum));
	for (j = 1; j <= mpCut->mpForest->mNumNodes; ++j)
	{
		if (mpCut->mpNodeRefs[j] != NULL)
		{
			index = mpCut->mpNodeRefs[j]->pVertexRenderDatum - mpSystemVertexRenderData;
			mpCut->mpNodeRefs[j]->pVertexRenderDatum = &mpFastVertexRenderData[index];
		}
	}
	mpVertexRenderData = mpFastVertexRenderData;
}

void Renderer::PopulateVertexSlotsCache()
{
	unsigned int k;
	for (k = 0; k < mNumVertices; ++k)
	{
		if (mpVertexActiveFlags[k] == false)
		{
			mVertexFreeSlots.AddFreeSlot(k);
			if ((mVertexFreeSlots.mSlotsCached >= mNumVertexSlotsFree) || (mVertexFreeSlots.mSlotsCached == FREELISTSIZE))
				return;
		}
	}
}

void Renderer::PopulateTriSlotsCache(VDS::PatchIndex PatchID)
{
	unsigned int k;
	for (k = 0; k < mpPatchTriData[PatchID].NumTrisAllocated; ++k)
	{
		if (mpPatchTriData[PatchID].TriProxyBackRefs[k][0] == Forest::iNIL_NODE)
		{
			mpPatchTriData[PatchID].TriFreeSlots.AddFreeSlot(k);
			if (mpPatchTriData[PatchID].TriFreeSlots.mSlotsCached >= mpPatchTriData[PatchID].NumTriSlotsFree)
				return;
		}
	}
}

void Renderer::SetVertexRenderDatumAboveParentsOfBoundary(VertexRenderDatum *pVRD, bool newflag)
{
	unsigned int index = pVRD - mpVertexRenderData;
	mpVertexAboveParentsOfBoundaryFlags[index] = newflag;
}

void Renderer::IncrementVertexUseCount(VertexRenderDatum *pVRD)
{
	unsigned int index = pVRD - mpVertexRenderData;
	++mpVertexUseCounts[index];
}

void Renderer::DecrementVertexUseCount(VertexRenderDatum *pVRD)
{
	unsigned int index = pVRD - mpVertexRenderData;
	--mpVertexUseCounts[index];
}

unsigned int Renderer::GetVertexUseCount(VertexRenderDatum *pVRD)
{
	unsigned int index = pVRD - mpVertexRenderData;
	return mpVertexUseCounts[index];
}

void Renderer::ZeroVertexUseCount(VertexRenderDatum *pVRD)
{
	unsigned int index = pVRD - mpVertexRenderData;
	mpVertexUseCounts[index] = 0;
}
