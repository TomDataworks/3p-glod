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
#pragma warning(disable: 4305)
#endif

#include <iostream>
#include "manager.h"

using namespace std;
using namespace VDS;


Manager::Manager()
{
	mNumRenderers = 0;
//	mNumMemoryBlocks = 0;

//	mpSystemMemoryPool = NULL;
//	mSystemMemoryPoolSize = 0;
	mpFastMemoryPool = NULL;
	mFastMemoryPoolSize = 0;
	mInitialized = false;
}

Manager::~Manager()
{
	Reset();
}

void Manager::Initialize(/*void *pSystemMemory, unsigned int SystemMemorySize,*/ void *pFastMemory, unsigned int FastMemorySize)
{
//	mpSystemMemoryPool = pSystemMemory;
//	mSystemMemoryPoolSize = SystemMemorySize;
	mpFastMemoryPool = pFastMemory;
	mFastMemoryPoolSize = FastMemorySize;
	if (/*(pSystemMemory != NULL) && (SystemMemorySize > 0) && */(pFastMemory != NULL) && (FastMemorySize > 0))
		mInitialized = true;
}

void Manager::Reset()
{
	unsigned int i;
	for (i = 0; i < mNumRenderers; ++i)
	{
		mpMemoryBlocks[i].pRenderer->mpFastVertexRenderData = NULL;
		mpMemoryBlocks[i].pRenderer->mNumVerticesAllocated = 0;
	}
}

/*
bool Manager::AllocateSystemMemory(Renderer *pRenderer, unsigned int SizeOfVertexMemory, unsigned int SizeOfProxyMemory, unsigned int SizeOfProxyBackRefMemory)
{
	unsigned int i, j, r;
	r = GetRendererIndex(pRenderer);
	RenderMemoryBlock *b = &mpMemoryBlocks[mNumMemoryBlocks];
	if (mpRecords[r].SystemMemoryBlock != NULL)
	{
		cerr << "Error - System memory already allocated to renderer" << endl;
		return false;
	}
	mpRecords[r].SystemMemoryBlock = b;

	b->mSizeOfVertexMemoryAllocated = SizeOfVertexMemory;
	b->mSizeOfProxyMemoryAllocated = SizeOfProxyMemory;
	b->mSizeOfProxyBackRefMemoryAllocated = SizeOfProxyBackRefMemory;

	b->mBaseAddressOfVertexMemoryAllocated = mpSystemMemoryPool;
	if (b->mBaseAddressOfVertexMemoryAllocated == NULL)
	{
		b->mSizeOfVertexMemoryAllocated = 0;
		cerr << "Error - memory manager unable to allocate vertex system memory." << endl;
	}
	b->mTopAddressOfVertexMemoryAllocated = (char*)b->mBaseAddressOfVertexMemoryAllocated + b->mSizeOfVertexMemoryAllocated;
// TODO use user-specified memory instead of malloc
	b->mBaseAddressOfProxyMemoryAllocated = (char*)b->mTopAddressOfVertexMemoryAllocated + 1;
	if (b->mBaseAddressOfProxyMemoryAllocated == NULL)
	{
		b->mSizeOfProxyMemoryAllocated = 0;
		cerr << "Error - memory manager unable to allocate Proxy system memory." << endl;
	}
	b->mTopAddressOfProxyMemoryAllocated = (char*)b->mBaseAddressOfProxyMemoryAllocated + b->mSizeOfProxyMemoryAllocated;
// TODO use user-specified memory instead of malloc
	b->mBaseAddressOfProxyBackRefMemoryAllocated = (char*)b->mTopAddressOfProxyMemoryAllocated + 1;
	if (b->mBaseAddressOfProxyBackRefMemoryAllocated == NULL)
	{
		b->mSizeOfProxyBackRefMemoryAllocated = 0;
		cerr << "Error - memory manager unable to allocate ProxyBackRef system memory." << endl;
	}
	b->mTopAddressOfProxyBackRefMemoryAllocated = (char*)b->mBaseAddressOfProxyBackRefMemoryAllocated + b->mSizeOfProxyBackRefMemoryAllocated;

	if (b->mTopAddressOfProxyBackRefMemoryAllocated > ((char*)mpSystemMemoryPool + mSystemMemoryPoolSize))
	{
		cerr << "Error - Not enough system memory given to manager to fulfil requirements of renderer" << endl;
		return false;
	}

//	bool SetRenderDataToSystem = false;
//	if (pRenderer->mpVertexRenderData == pRenderer->mpSystemVertexRenderData)
//		SetRenderDataToSystem = true;
	pRenderer->mpSystemVertexRenderData = (VertexRenderDatum*)b->mBaseAddressOfVertexMemoryAllocated;
//	if (SetRenderDataToSystem)
//		pRenderer->mpVertexRenderData = pRenderer->mpSystemVertexRenderData;
	pRenderer->mVertexRenderDataMemoryAllocated = b->mSizeOfVertexMemoryAllocated;
	pRenderer->mNumVerticesAllocated = pRenderer->mVertexRenderDataMemoryAllocated / sizeof(VertexRenderDatum);

	pRenderer->mTriRenderDataMemoryAllocated = SizeOfProxyMemory + SizeOfProxyBackRefMemory;
	pRenderer->mNumTrisAllocated = pRenderer->mTriRenderDataMemoryAllocated / (sizeof(TriProxy) + sizeof(TriProxyBackRef));
	pRenderer->mpTriProxiesArray = (TriProxy*)b->mBaseAddressOfProxyMemoryAllocated;
	pRenderer->mpTriProxyBackRefs = (TriProxyBackRef*)b->mBaseAddressOfProxyBackRefMemoryAllocated;

	++mNumMemoryBlocks;
	return true;
}
*/

bool Manager::AddRenderer(Renderer *pRenderer)
{
	if (mNumRenderers >= MAX_MEMORY_BLOCKS)
	{
		cerr << "Error - maximum number of renderer memory blocks exceeded" << endl;
		return false;
	}

	pRenderer->mpMemoryManager = this;

	mpMemoryBlocks[mNumRenderers].pRenderer = pRenderer;
// TODO: replace these with pass-through functions to let user specify fast memory per object (renderer)
//	mpMemoryBlocks[mNumRenderers].mBaseAddressOfVertexMemoryAllocated = mpFastMemoryPool;
//	mpMemoryBlocks[mNumRenderers].mTopAddressOfVertexMemoryAllocated = &((char*)mpFastMemoryPool)[mFastMemoryPoolSize - 1];
//	mpMemoryBlocks[mNumRenderers].mSizeOfVertexMemoryAllocated = mFastMemoryPoolSize;
//	mpMemoryBlocks[mNumRenderers].pRenderer->mpFastVertexRenderData = (VertexRenderDatum *)mpMemoryBlocks[mNumRenderers].mBaseAddressOfVertexMemoryAllocated;

// TODO: need to make separate mNumFastVerticesAllocated to allow for there to be different amounts of fast and system vertex render data available
//	mpMemoryBlocks[mNumRenderers].pRenderer->mNumVerticesAllocated = mpMemoryBlocks[mNumRenderers].mSizeOfVertexMemoryAllocated / sizeof(VertexRenderDatum);

	++mNumRenderers;
	return true;
}

void Manager::RemoveRenderer(Renderer *pRenderer) {
    unsigned int i;
    for(i = 0; i < mNumRenderers; i++) {
        if(mpMemoryBlocks[i].pRenderer == pRenderer) {
            pRenderer->mpMemoryManager = NULL;
            memmove(&mpMemoryBlocks[i], &mpMemoryBlocks[i+1], sizeof(RenderMemoryBlock) * mNumRenderers - i);
            mNumRenderers--;
            return;
        }
    }
}

/*
bool Manager::AllocateFastMemory(Renderer *pRenderer, unsigned int SizeOfVertexMemory)
{
	unsigned int i, j, r;
	r = GetRendererIndex(pRenderer);
	RenderMemoryBlock *b = &mpMemoryBlocks[mNumMemoryBlocks];
	if (mpRecords[r].FastMemoryBlock != NULL)
	{
		cerr << "Error - Fast memory already allocated to renderer" << endl;
		return false;
	}

//	cerr << "Error - still need to implment Manager::AllocateFastMemory()" << endl;
//	return false;

	mpRecords[r].FastMemoryBlock = b;


	b->mBaseAddressOfProxyBackRefMemoryAllocated = NULL;
	b->mBaseAddressOfProxyMemoryAllocated = NULL;
	b->mSizeOfProxyBackRefMemoryAllocated = 0;
	b->mSizeOfProxyMemoryAllocated = 0;
	b->mTopAddressOfProxyBackRefMemoryAllocated = NULL;
	b->mTopAddressOfProxyMemoryAllocated = NULL;

	b->mBaseAddressOfVertexMemoryAllocated = mpFastMemoryPool;

cout << "Allocating " << SizeOfVertexMemory << " bytes of fast memory." << endl;
printf("Address of memory is %x.\n", b->mBaseAddressOfVertexMemoryAllocated);
cout << "Vertex capacity is " << SizeOfVertexMemory / sizeof(VertexRenderDatum) << "." << endl;

	b->mSizeOfVertexMemoryAllocated = SizeOfVertexMemory;

	pRenderer->mpFastVertexRenderData = (VertexRenderDatum *)b->mBaseAddressOfVertexMemoryAllocated;

	if (b->mBaseAddressOfVertexMemoryAllocated == NULL)
	{
		b->mSizeOfVertexMemoryAllocated = 0;
		cerr << "Error - memory manager unable to allocate chunk of fast memory." << endl;
	}




	++mNumMemoryBlocks;
	return true;
}

void Manager::DeallocateSystemMemory(Renderer *pRenderer)
{
TODO: need to heavily redo memory deallocation

	int r = GetRendererIndex(pRenderer);

	pRenderer->mNumTrisAllocated = 0;
	pRenderer->mNumVerticesAllocated = 0;
	pRenderer->mpSystemVertexRenderData = NULL;
	pRenderer->mpVertexRenderData = NULL;
	pRenderer->mpTriProxiesArray = NULL;
	pRenderer->mpTriProxyBackRefs = NULL;
	pRenderer->mVertexRenderDataMemoryAllocated = 0;
	pRenderer->mTriRenderDataMemoryAllocated = 0;
}

void Manager::DeallocateFastMemory(Renderer *pRenderer)
{
	int r = GetRendererIndex(pRenderer);

//	pRenderer->mNumVerticesAllocated = 0;
//	pRenderer->mpVertexRenderData = NULL;
//	pRenderer->mVertexRenderDataMemoryAllocated = NULL;

}

bool Manager::IsSystemMemoryAllocated(Renderer *pRenderer)
{
	int r = GetRendererIndex(pRenderer);
	return mpRecords[r].SystemMemoryBlock;
}

bool Manager::IsFastMemoryAllocated(Renderer *pRenderer)
{
	int r = GetRendererIndex(pRenderer);
	return mpRecords[r].FastMemoryBlock;
}

void Manager::ReceiveSystemRenderMemory(void *pSysRenderMem, unsigned int SysRenderMemSize)
{
}

void Manager::ReceiveFastRenderMemory(void *pFastRenderMem, unsigned int FastRenderMemSize)
{
}
*/

int Manager::GetRendererIndex(Renderer *pRenderer)
{
	unsigned int i;
	for (i = 0; i < mNumRenderers; ++i)
	{
		if (mpMemoryBlocks[i].pRenderer == pRenderer)
			return i;
	}
	cerr << "Error - GetRendererIndex didn't find renderer pointer" << endl;
	return -666;
}
