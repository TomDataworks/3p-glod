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
#ifndef RENDERER_H
#define RENDERER_H

#include "vds.h"
#include "cut.h"
#include "node.h"
#include "freelist.h"
#include "primtypes.h"

#ifndef APIENTRY
#ifdef __APPLE__
#define APIENTRY
#else
#define APIENTRY extern
#endif
#endif

#ifdef _WIN32
typedef void (APIENTRY *PFNGLVERTEXARRAYRANGEPROC) (GLsizei , void *);
typedef void (APIENTRY *PFNGLFLUSHVERTEXARRAYRANGENVPROC) ();
typedef void (APIENTRY *PFNGLGENFENCESNVPROC) (GLsizei , GLuint *);
typedef void (APIENTRY *PFNGLDELETEFENCESNVPROC) (GLsizei , const GLuint *);
typedef void (APIENTRY *PFNGLFSETFENCENVPROC) (GLuint fence, GLuint);
typedef GLboolean (APIENTRY *PFNGLTESTFENCENVPROC) (GLuint);
typedef void (APIENTRY *PFNGLFINISHFENCENVPROC) (GLuint);
typedef GLboolean (APIENTRY *PFNGLISFENCENVPROC) (GLuint);
typedef void (APIENTRY *PFNGLGETFENCEIVNVPROC) (GLuint, GLenum GLpname, GLint *params);
#else
typedef void (APIENTRY *PFNGLGETFENCEIVNVPROC) (GLuint, GLenum, GLint *params);
#endif

namespace VDS
{

struct PatchRenderTris
{
	TriProxyBackRef *TriProxyBackRefs;
	TriProxy *TriProxiesArray;
	TriIndex NumTris;
	TriIndex LastActiveTri;
	void *TriMemoryAllocated;
	
	// number of triangles whose proxies can fit in TriProxiesArray
	TriIndex NumTrisAllocated;
	int NumTriSlotsFree;
	FreeList TriFreeSlots;

	bool NormalsPresent;
	bool ColorsPresent;
};	
	
class Renderer 
{
public: // PUBLIC FUNCTIONS
	Renderer(unsigned int NumInitialVerticesToAllocate, unsigned int NumInitialTrisToAllocate);
	virtual ~Renderer();

	void RenderPatch(PatchIndex PatchID);
	void SetRenderFunc(RenderFunc fRender);
	void AddCut(Cut *pCut);
	void RemoveCut(Cut *pCut);
	void FlushRenderData();
	VertexRenderDatum *AddVertexRenderDatum(NodeIndex iNode);
	void RemoveVertexRenderDatum(VertexRenderDatum *pVertexRenderDatum);
	void AddTriRenderDatum(TriIndex iTri, PatchIndex PatchID);
	void RemoveTriRenderDatum(TriIndex TriRenderDatum, PatchIndex PatchID);
	NodeIndex GetVertexRenderDatumIndex(VertexRenderDatum *pVRD);
	NodeIndex GetVertexCacheBackRef(NodeIndex iVertexCacheIndex, Forest *pForest);
	inline ProxyIndex GetProxy(TriIndex i, int k) const;
	inline NodeIndex GetProxyBackRef(TriIndex i, int k) const;
	void SetVertexRenderDatumAboveParentsOfBoundary(VertexRenderDatum *pVRD, bool newflag);
	void IncrementVertexUseCount(VertexRenderDatum *pVRD);
	void DecrementVertexUseCount(VertexRenderDatum *pVRD);
	unsigned int GetVertexUseCount(VertexRenderDatum *pVRD);
	void ZeroVertexUseCount(VertexRenderDatum *pVRD);

protected: // PRIVATE FUNCTIONS
	bool ReallocateTriRenderData(PatchIndex PatchID, unsigned int newTrisAllocated);

	void PopulateVertexSlotsCache();
	void PopulateTriSlotsCache(VDS::PatchIndex PatchID);
	VertexRenderDatum *CacheVertex(NodeIndex iVertexArrayLocation, Node *pNode);
	void UseSystemMemoryVertexData();
	void UseFastMemoryVertexData();

public:	// PUBLIC DATA

	bool mIsValid;
	RenderFunc mfRender;

	VertexRenderDatum *mpSystemVertexRenderData; // coherent array of vertex coordinates of active nodes
	VertexRenderDatum *mpFastVertexRenderData; // vertex data in fast memory
	VertexRenderDatum *mpOldFastVertexRenderData; // address of vertex data fast memory before buffer unbound for rendering
	VertexRenderDatum *mpVertexRenderData; // vertex data pointer used when manipulating (adding/removing) vertices
										   // can point to either system vertex data or fast vertex data depending
										   // on whether adapting in-place or copying to fast memory per frame
	TriProxy *mpTriProxyData;
	bool mUseFastMemory;
	bool mCopyDataToFastMemoryPerFrame;
	NodeIndex mNumVertices;
	NodeIndex mLastActiveVertex;
	NodeIndex *mpVertexNodeIDs;
	unsigned int mVertexDataStride;

	bool *mpVertexActiveFlags;
	bool *mpVertexAboveParentsOfBoundaryFlags;
	unsigned int *mpVertexUseCounts;

	Cut *mpCut;	// pointer to cut this renderer renders
	VDS::PatchIndex mNumPatches;	// number of patches this renderer is in charge of rendering
	PatchRenderTris *mpPatchTriData; // Triangle index structs, one per patch

	TriIndex mNumTris;	// total number of tris in all of this renderer's patches

	unsigned int VertexIndexSizeLimit;


public: // MEMORY MANAGEMENT FUNCTIONS AND DATA

	bool InitializeNVidiaExtensions();

	// asks that render memory hold NumNodes VertexRenderDatums and NumTris triangles' proxies+backrefs
	bool ReallocateMemoryForRenderData(unsigned int NumNodes, unsigned int NumTris);

	// Called in AddCut() call
	// allocates initial vertex and tri memory
	// if memory was successfully allocated:
		// mpVertexRenderData, mVertexRenderDataMemoryAllocated, mpTriProxiesArray, 
		// and mTriRenderDataMemoryAllocated are set to their updated values
		// returns true
	// if memory was not successfully allocated, returns false
//	bool AllocateSystemMemoryForRenderData();

	// allocates fast memory for vertex renderdata
//	bool AllocateFastMemoryForVertexRenderData();

	// copies vertex render data from system memory to fast memory
	void CopyVertexDataToFastMemory();

	// frees all memory allocated for the vertex data array and proxies array for this renderer
	void FreeSystemRenderMemory();

	// sets the flag to use fast memory when rendering
	void SetUseFastMemory(bool UseFastMemory);

	// sets the flag to copy system memory to fast memory each frame (rather than adapting in-place in fast memory)
	void SetCopyPerFrame(bool CopyPerFrame);

	Manager *mpMemoryManager; // is set by Manager::AddRenderer() call

	// memory which is getting sent to the card (i.e. is in the vertex or tri array) but does not contain vertex or tri information
	unsigned int mSlackBytes; 

	// total number of bytes of memory allocated to this renderer for the vertex data array
//	unsigned int mVertexRenderDataMemoryAllocated;

	// number of VertexRenderDatums which can fit in mVertexRenderDataMemoryAllocated
	NodeIndex mNumVerticesAllocated;

	// total number of bytes of memory allocated to this renderer for the proxies array
//	unsigned int mTriRenderDataMemoryAllocated;

//	unsigned int mNumInitialVerticesToAllocate;
	unsigned int mNumInitialTrisToAllocate;

	int VertexIndexHWLimit; // hardware suggested limit to number of vertices per vertex array

protected: // PRIVATE DATA
public:
	FreeList mVertexFreeSlots;

	int mNumVertexSlotsFree;

public: // DEBUG DATA
	float *ACMR;
	unsigned int *vertex_cache_size;

#ifdef _WIN32
	LARGE_INTEGER times_30;

#endif
}; // Class Renderer

} //namespace VDS

#endif //RENDERER_H

