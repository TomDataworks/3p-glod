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
#ifndef VDSAUX_H
#define VDSAUX_H

#include "forest.h"
#include <vector>

namespace VDS
{

struct ReorderQueueLink
{
	NodeIndex mIndex;
	ReorderQueueLink *mpNext;
};

struct BudgetItem
{
	int PQindex;

	Point3 mPosition;	// position and BBox values do not need to be in
						// BudgetItem - they can be obtained by following back
//	Float mRadius;		// through the cut to the forest and using miNode to 
	Float mXBBoxOffset;	// get the render data from the forest; however,
	Float mYBBoxOffset;	// duplicating them in BudgetItem makes node error
	Float mZBBoxOffset;	// calculation faster
	Point3 mBBoxCenter;

    Float mError;
    NodeIndex miNode;
	VertexRenderDatum *pVertexRenderDatum;
	int CutID; // TODO: this doesn't need to be an int - can save space by making into a char or even fewer bits

	TriIndex miFirstLiveTri;
};

} //namespace VDS

#endif //VDSAUX_H

