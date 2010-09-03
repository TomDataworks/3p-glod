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
#ifndef NODE_H
#define NODE_H
#include "vds.h"

class VDS::Node
{
public:
    Node();
    Node(const Node &); //not implemented
    virtual ~Node();
	
	// Following access functions simply return a data member
    NodeIndex GetParent() const;
	NodeIndex GetLeftSibling() const;
	NodeIndex GetRightSibling() const;
	NodeIndex GetFirstChild() const;
	TriIndex GetFirstSubTri() const;
//	Float GetRadius() const;
	
private:
	Node &operator =(const Node &);//not implemented
	
public:		// making these public for "debugging convenience"
	NodeIndex miParent;
	NodeIndex miLeftSibling;
	NodeIndex miRightSibling;
	NodeIndex miFirstChild;
	TriIndex miFirstSubTri;
//	Float mRadius;
	PatchIndex mPatchID;
	NodeIndex mCoincidentVertex;
//	ViewIndependentError mViewIndependentError;
	unsigned int miErrorParamIndex;
	VertexRenderDatum *mpRenderData;

	Point3 mBBoxCenter;
	Float mXBBoxOffset;
	Float mYBBoxOffset;
	Float mZBBoxOffset;
	
	// Friends
	friend class VDS::Forest;
	friend class VDS::Tri;
	friend class VDS::ForestBuilder;
};

#endif
