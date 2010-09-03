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

#include "tri.h"
#include "forest.h"
#include "node.h"
#include "renderer.h"

#include <cassert>
#include <iostream>

using namespace std;
using namespace VDS;

Tri::Tri()
{
    miNextSubTri = Forest::iNIL_TRI;
}

Tri::~Tri()
{
}

// TODO: make these functions less unintelligible

void Tri::InitializeProxiesAndLiveTris(TriIndex iTri, const Forest &rForest, Renderer *pRenderer, PatchIndex PatchID)
{
// TODO: pass (locations of) proxy back refs in directly instead of recalculating through forest.mpTriRefs
	TriProxyBackRef **pTriRefs = pRenderer->mpCut->mpTriRefs;
	BudgetItem **pNodeRefs = pRenderer->mpCut->mpNodeRefs;

    for (int i = 0; i < 3; i++)
    {
		NodeIndex *proxy = &(*pTriRefs[iTri])[i];
        *proxy = rForest.iROOT_NODE;
        while ((*proxy != miCorners[i]) && 
			(pNodeRefs[rForest.mpNodes[*proxy].miFirstChild] != NULL))
        {
            MoveProxyDown(iTri, i, rForest, pRenderer);
        }
		unsigned int tri_index = pTriRefs[iTri] - pRenderer->mpPatchTriData[PatchID].TriProxyBackRefs;
		pRenderer->mpPatchTriData[PatchID].TriProxiesArray[tri_index][i] = pRenderer->GetVertexRenderDatumIndex(pNodeRefs[*proxy]->pVertexRenderDatum);
        AddToLiveTriList(iTri, i, rForest, pRenderer);
    }
}

// TODO: just pass in the pointer to the proxy
void Tri::MoveProxyDown(TriIndex iTri, int iProxy, const Forest &rForest, Renderer *pRenderer)
{
	TriProxyBackRef **pTriRefs = pRenderer->mpCut->mpTriRefs;
	const Node *nodes = rForest.mpNodes;
	NodeIndex *pProxy = &((*pTriRefs[iTri])[iProxy]);
	*pProxy = nodes[*pProxy].miFirstChild;

	// need additional termination test that sees if current proxy is an ancestor of or is the proxy needed
	// terminate if corner is greater than proxy->rightsibling

    while ((nodes[*pProxy].miRightSibling != Forest::iNIL_NODE)
        && (miCorners[iProxy] >= nodes[*pProxy].miRightSibling))
    {
        *pProxy = rForest.mpNodes[*pProxy].miRightSibling;
    }
    assert((*pTriRefs[iTri])[iProxy] <= miCorners[iProxy]);
}

int Tri::GetNodeIndex(TriIndex iTri, NodeIndex iNode, const Forest *pForest, Renderer *pRenderer) const
{
	TriProxyBackRef **pTriRefs = pRenderer->mpCut->mpTriRefs;

#ifdef _DEBUG
	if (pTriRefs[iTri] == NULL)
	{
		cerr << "Error - Trying to get node proxy index of inactive triangle " << iTri << endl;
	}
#endif
	
	if (pTriRefs[iTri]->backrefs[0] == iNode)
	{
		return 0;
	}
	else if (pTriRefs[iTri]->backrefs[1] == iNode)
	{
		return 1;
	}
	else if (pTriRefs[iTri]->backrefs[2] == iNode)
	{
		return 2;
	}
	cerr << endl << "GetNodeIndex couldn't find proxy matching Node " << iNode << endl << "Triangle proxies: " 
		<< (*pTriRefs[iTri])[0] << " " 
		<< (*pTriRefs[iTri])[1] << " "
		<< (*pTriRefs[iTri])[2] << endl;
	return -666666;
}

int Tri::GetNodeIndexC(TriIndex iTri, NodeIndex iNode, const Forest &rForest) const
{
    if (rForest.mpTris[iTri].miCorners[0] == iNode)
    {
        return 0;
    }
    else if (rForest.mpTris[iTri].miCorners[1] == iNode)
    {
        return 1;
    }
    else if (rForest.mpTris[iTri].miCorners[2] == iNode)	// can eventually take out this test and assume that if not 0 or 1 then 2
    {
        return 2;
    }
	cerr << "GetNodeIndex couldn't find proxy matching iNode " << iNode << endl;
	return -666666;
}

void Tri::AddToLiveTriList(TriIndex iTri, int iProxy, const Forest &rForest, Renderer *pRenderer)
{
    pRenderer->mpCut->mpTriRefs[iTri]->miNextLiveTris[iProxy] = pRenderer->mpCut->mpNodeRefs[(*pRenderer->mpCut->mpTriRefs[iTri])[iProxy]]->miFirstLiveTri;
    pRenderer->mpCut->mpNodeRefs[(*pRenderer->mpCut->mpTriRefs[iTri])[iProxy]]->miFirstLiveTri = iTri;

	pRenderer->IncrementVertexUseCount(pRenderer->mpCut->mpNodeRefs[(*pRenderer->mpCut->mpTriRefs[iTri])[iProxy]]->pVertexRenderDatum);
}

void Tri::AddToLiveTriListUsingCorners(TriIndex iTri, int iProxy, const Forest &rForest, TriIndex *FirstLiveTris, TriIndex **NextLiveTris)
{
	NextLiveTris[iTri][iProxy] = FirstLiveTris[rForest.mpTris[iTri].miCorners[iProxy]];
	FirstLiveTris[rForest.mpTris[iTri].miCorners[iProxy]] = iTri;
}


void Tri::RemoveFromLiveTriList(TriIndex iTri, NodeIndex iNode, const Forest &rForest, Renderer *pRenderer)
{
    TriIndex live_tri;
    TriIndex prev_live_tri;
    TriIndex first_live_tri;
    int k;
    int prev_k;
    Tri *const tris = rForest.mpTris;

	if (pRenderer->mpCut->mpNodeRefs[iNode] == NULL)
	{
		cerr << "Tri being removed's proxy has null NodeRef" << endl;
		return;
	}
    first_live_tri = pRenderer->mpCut->mpNodeRefs[iNode]->miFirstLiveTri;
	assert(first_live_tri != Forest::iNIL_TRI);
    if (first_live_tri == iTri)
    {
        k = tris[first_live_tri].GetNodeIndex(first_live_tri, iNode, &rForest, pRenderer);
		pRenderer->mpCut->mpNodeRefs[iNode]->miFirstLiveTri = pRenderer->mpCut->mpTriRefs[first_live_tri]->miNextLiveTris[k];
    }
    else
    {
        prev_live_tri = first_live_tri;
        prev_k = tris[prev_live_tri].GetNodeIndex(prev_live_tri, iNode, &rForest, pRenderer);
        live_tri = pRenderer->mpCut->mpTriRefs[prev_live_tri]->miNextLiveTris[prev_k];
        k = tris[live_tri].GetNodeIndex(live_tri, iNode, &rForest, pRenderer);
        while (live_tri != iTri)
        {
            prev_live_tri = live_tri;
            prev_k = k;
			live_tri = pRenderer->mpCut->mpTriRefs[live_tri]->miNextLiveTris[k];
            k = tris[live_tri].GetNodeIndex(live_tri, iNode, &rForest, pRenderer);
            assert(live_tri != Forest::iNIL_NODE);
        }
		pRenderer->mpCut->mpTriRefs[prev_live_tri]->miNextLiveTris[prev_k] = pRenderer->mpCut->mpTriRefs[live_tri]->miNextLiveTris[k];
    }
	pRenderer->DecrementVertexUseCount(pRenderer->mpCut->mpNodeRefs[iNode]->pVertexRenderDatum);
}

void Tri::CheckLiveTriLists(TriIndex i, const Forest &rForest)
{

}

void Tri::AddToSubTriList(TriIndex iTri, NodeIndex iNode, const Forest &rForest)
{
    Node *const nodes = rForest.mpNodes;
    Tri *const tris = rForest.mpTris;
    //first subtri's prev pointer actually points to the node that the subtri list is for
    tris[iTri].miNextSubTri = nodes[iNode].miFirstSubTri;
    nodes[iNode].miFirstSubTri = iTri;
}

TriIndex Tri::GetNextSubTri() const
{
    return miNextSubTri;
}

NodeIndex Tri::GetCorner(int iCorner) const
{
    return miCorners[iCorner];
}

