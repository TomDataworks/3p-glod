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
#ifndef MANAGER_H
#define MANAGER_H

#include "vds.h"
#include "renderer.h"

#ifndef GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV 0x8520
#endif

#ifndef GL_VERTEX_ARRAY_RANGE_NV
#define GL_VERTEX_ARRAY_RANGE_NV 0x851D
#endif

#ifndef GL_ALL_COMPLETED_NV
#define GL_ALL_COMPLETED_NV 0x84F2
#endif

#ifndef GL_FENCE_STATUS_NV
#define GL_FENCE_STATUS_NV 0x84F3
#endif

#ifndef GL_FENCE_CONDITION_NV
#define GL_FENCE_CONDITION_NV 0x84F4
#endif

#define MAX_MEMORY_BLOCKS 64
#define MAX_RECORDS 8

namespace VDS
{

struct RenderMemoryBlock
{
	int mTypeOfMemory; // 0 = system memory, 1 = agp memory, 2 = video memory
	void *mBaseAddressOfVertexMemoryAllocated;
	void *mTopAddressOfVertexMemoryAllocated;
	unsigned int mSizeOfVertexMemoryAllocated;
//	void *mBaseAddressOfProxyMemoryAllocated;
//	void *mTopAddressOfProxyMemoryAllocated;
//	unsigned int mSizeOfProxyMemoryAllocated;
//	void *mBaseAddressOfProxyBackRefMemoryAllocated;
//	void *mTopAddressOfProxyBackRefMemoryAllocated;
//	unsigned int mSizeOfProxyBackRefMemoryAllocated;
	Renderer *pRenderer;
};

//struct RendererMemoryRecord
//{
//	RenderMemoryBlock *SystemMemoryBlock;
//	RenderMemoryBlock *FastMemoryBlock;
//	Renderer *pRenderer;
//};

class Manager
{
public:
	Manager();
	virtual ~Manager();

	// releases all memory allocated
	void Reset();

	void Initialize(/*void *pSystemMemory, unsigned int SystemMemorySize,*/ void *pFastMemory, unsigned int FastMemorySize);

	// Add renderer for memory management
	// returns an ID which is the index in the manager's arrays for this renderer
	bool AddRenderer(Renderer *pRenderer);
    void RemoveRenderer(Renderer *pRenderer);
//	bool AllocateSystemMemory(Renderer *pRenderer, unsigned int SizeOfVertexMemory, unsigned int SizeOfProxyMemory, unsigned int SizeOfProxyBackRefMemory);
//	bool AllocateFastMemory(Renderer *pRenderer, unsigned int SizeOfVertexMemory);


	// Allocate memory for given number of vertices and tris
	// returns true if allocation succeeded, false if it didn't
//	bool AllocateMemory(Renderer *pRenderer, unsigned int NumVerts, unsigned int NumTris);


//	void DeallocateSystemMemory(Renderer *pRenderer);
//	void DeallocateFastMemory(Renderer *pRenderer);

//	bool IsSystemMemoryAllocated(Renderer *pRenderer);
//	bool IsFastMemoryAllocated(Renderer *pRenderer);

// to be used for receiving additional memory from app
//	void ReceiveSystemRenderMemory(void *pSysRenderMem, unsigned int SysRenderMemSize);
//	void ReceiveFastRenderMemory(void *pFastRenderMem, unsigned int FastRenderMemSize);

protected:
	int GetRendererIndex(Renderer *pRenderer);
	

public:
	bool mInitialized;

	RenderMemoryBlock mpMemoryBlocks[MAX_MEMORY_BLOCKS];
	unsigned int mNumRenderers;
//	unsigned int mNumMemoryBlocks;

//	RendererMemoryRecord mpRecords[MAX_RECORDS];

//	void *mpSystemMemoryPool;
//	unsigned int mSystemMemoryPoolSize;
	void *mpFastMemoryPool;
	unsigned int mFastMemoryPoolSize;

}; // class Manager

} // namespace VDS

#endif // #ifndef MANAGER_H
