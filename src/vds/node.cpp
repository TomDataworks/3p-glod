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

#include <cassert>

#include "forest.h"
#include "node.h"
#include "tri.h"

using namespace VDS;

Node::Node()
{
	mpRenderData = NULL;
    miParent = Forest::iNIL_NODE;
    miLeftSibling= Forest::iNIL_NODE;
    miRightSibling = Forest::iNIL_NODE;
    miFirstChild = Forest::iNIL_NODE;
    miFirstSubTri = Forest::iNIL_TRI;
//    mRadius = 0.0;
	mXBBoxOffset = 0.0f;
	mYBBoxOffset = 0.0f;
	mZBBoxOffset = 0.0f;
	mBBoxCenter.X = 0.0f;
	mBBoxCenter.Y = 0.0f;
	mBBoxCenter.Z = 0.0f;
}

Node::~Node()
{
}

NodeIndex Node::GetParent() const
{
    return miParent;
}

NodeIndex Node::GetLeftSibling() const
{
    return miLeftSibling;
}

NodeIndex Node::GetRightSibling() const
{
    return miRightSibling;
}

NodeIndex Node::GetFirstChild() const
{
    return miFirstChild;
}

TriIndex Node::GetFirstSubTri() const
{
    return miFirstSubTri;
}

//Float Node::GetRadius() const
//{
//    return mRadius;
//}
