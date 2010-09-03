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
#ifndef TRI_H
#define TRI_H
#include "vds.h"

class VDS::Tri
{
// PUBLIC FUNCTIONS
public:
	Tri();
	Tri(const Tri &); //not implemented
	virtual ~Tri();

// PRIVATE FUNCTIONS
private:
	//Moves proxies to lowest nodes above boundary.  Adds Tri to live tri
	//lists of these new proxies.
	void InitializeProxiesAndLiveTris(TriIndex iTri, const Forest &rForest, Renderer *pRenderer, PatchIndex PatchID);
	
	//Moves a proxy down one level of the Forest.  Does not alter live tri list
	//Does not check for going below boundary
	void MoveProxyDown(TriIndex iTri, int iProxy, const Forest &rForest, Renderer *pRenderer);  
	
	//Returns an index i either 0, 1, 2. miProxies[i] == iNODE representing
	//mCorners[i], and miNextLiveTris[i] is this nodes live tri link.  Assumes
	//that this node is in fact one of the proxies.
	int GetNodeIndex(TriIndex iTri, NodeIndex iNode, const Forest *pForest, Renderer *pRenderer) const;
	
	// same as GetNodeIndex but assumes that all proxies are corners
	// (i.e. doesn't even use proxies, uses corners) - for preprocessing
	int GetNodeIndexC(TriIndex iTri, NodeIndex iNode, const Forest &rForest) const;
	
	//Adds this node to the live tri list of miProxies[iProxy] in rForest
	void AddToLiveTriList(TriIndex iTri, int iProxy, const Forest &rForest, Renderer *pRenderer);
	
	//Adds this node to the live tri list of miProxies[iProxy] in rForest
	void AddToLiveTriListUsingCorners(TriIndex iTri, int iProxy, const Forest &rForest, TriIndex *FirstLiveTris, TriIndex **NextLiveTris);
	
	//Removes the node from the live tri list of node iNode
	//undefined behaviour if the Tri is not actually part of the live tri
	//list of the node
	void RemoveFromLiveTriList(TriIndex iTri, NodeIndex iNode, const Forest &rForest, Renderer *pRenderer);
	
	//Inserts this Tri into the sub tri list of node iNode
	void AddToSubTriList(TriIndex iTri, NodeIndex iNode, const Forest &rForest);
	
	TriIndex GetNextSubTri() const;
	NodeIndex GetCorner(int iCorner) const;

	Tri &operator =(const Tri &); //not implemented

// DEBUG FUNCTIONS
	void CheckLiveTriLists(TriIndex i, const Forest &rForest);

// DATA
public: // making these public for convenience
	TriIndex miNextSubTri;
	NodeIndex miCorners[3];
	PatchIndex mPatchID;
	
//Friends
	friend class Forest;
	friend class Node;
	friend class ForestBuilder;
	friend class Simplifier;
	friend class Renderer;
};

#endif
