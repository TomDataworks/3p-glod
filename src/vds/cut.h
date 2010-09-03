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
#ifndef CUT_H
#define CUT_H

#include "vds.h"
#include "renderer.h"
#include "simplifier.h"
#include "forest.h"

class VDS::Cut
{
public: // PUBLIC FUNCTIONS
	Cut();
	virtual ~Cut();

	void GiveContents(Cut *pCut);
	void SetRenderer(Renderer *pRenderer);
	void SetSimplifier(Simplifier *pSimplifier);
	void SetForest(Forest *pForest);

//	void SetLookVector(const Vec3 &rLookVector);
//	void SetUpVector(const Vec3 &rUpVector);
//	void SetFov(Float Fov); // takes FOV in degrees
//	void SetViewPoint(const Point3 &rViewpoint);
//	void SetAspectRatio(Float AspectRatio); // takes viewport width / height

//	void CalcFrustum(); // recalculates frustum planes based on current view information

	// set the transformation matrix from a COLUMN MAJOR float[16]
	void SetTransformationMatrix(float *ColMajorFloat16Matrix);
	void SetTransformationMatrix(Mat4 ColMajorFloat16Matrix);

	// Updates the cut's simplifiers view parameters from cut's transform matrix
	// Call after SetTransformationMatrix() to update the view-dependent simplification parameters using the new matrix
//	void UpdateViewParametersFromMatrix();

	// initializes NodeRefs and TriRefs
	void InitializeRefs();
	
// DEBUG FUNCTIONS
	void CheckForDuplicateNodeRefs();

public: // DEBUG FUNCTIONS
	void PrintHighlightedNodeInfo();
	void HighlightRootNode();
	void UnhighlightNode();
	void HighlightParent();
	void HighlightFirstChild();
	void HighlightRightSibling();
	void HighlightLeftSibling();
	void FoldHighlightedNode();
	void FullyFoldHighlightedNode();
	void FullyFoldNode(NodeIndex node, unsigned int &NumTris, unsigned int &BytesUsed);
	void FullyUnfoldHighlightedNode();
	void FullyUnfoldNode(NodeIndex node, unsigned int &NumTris, unsigned int &BytesUsed);
	void UnfoldHighlightedNode();
	void PrintHighlightedNodeStructure();
	void HighlightFirstLiveTri();
	void HighlightNextLiveTri();
	void PrintHighlightedTriInfo();

public: // PUBLIC DATA
	Forest *mpForest;
	Renderer *mpRenderer;
	Simplifier *mpSimplifier;
	bool mIsValid;
	NodeIndex mNumActiveNodes;
	TriIndex mNumActiveTris;
	unsigned int mBytesUsed;
	unsigned int mBytesPerTri;
	unsigned int mBytesPerNode;
	Mat4 mTransformMatrix; // object-to-eye transformation matrix

	// Refs
	BudgetItem **mpNodeRefs;
	TriProxyBackRef **mpTriRefs;

	// view information:
//	Point3 mViewpoint;
//	Vec3 mForwardVector;
//	Vec3 mUpVector;
//	Vec3 mRightVector;
//	Float mFov; // in degrees
//	Float mInvTanFov;
//	Float mAspectRatio; // viewport width / height
	unsigned int mDisplayWidth;
	unsigned int mDisplayHeight;

	// pointer to external view class (NULL if not used)
	void *mpExternalViewClass;
	
	// plane equations of view frustum
//	Plane3 mTopPlane;
//	Plane3 mBottomPlane;
//	Plane3 mRightPlane;
//	Plane3 mLeftPlane;

	// Node/Tri Highlighting
	NodeIndex miHighlightedNode;
	TriIndex miHighlightedTri;

	//Friends
	friend class Tree;
	friend class Tri;
	friend class ForestBuilder;
};

#endif // #ifndef CUT_H

