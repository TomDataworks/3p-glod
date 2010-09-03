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
#ifndef FREELIST_H
#define FREELIST_H

#include "vds.h"

#define FREELISTSIZE 5000

class VDS::FreeList
{
public:
	FreeList();
	~FreeList();

	NodeIndex GetFreeSlot();
	void AddFreeSlot(NodeIndex slot);
	void Reset();

	int mSlotsCached;

	int head;
	int tail;

	int mIndexOfLargest;
	NodeIndex mLargest;

	NodeIndex mFreeList[FREELISTSIZE];

	friend class Renderer;
};

#endif //#ifndef FREELIST_H

