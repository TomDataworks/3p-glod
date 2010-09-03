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
#pragma warning(disable: 4530)
#endif

#include "freelist.h"

using namespace VDS;

FreeList::FreeList()
{
	Reset();
}

FreeList::~FreeList()
{
}

NodeIndex FreeList::GetFreeSlot()
{	
	int retslot = tail;
	tail = (tail + 1) % FREELISTSIZE;
	--mSlotsCached;
	return mFreeList[retslot];
}

void FreeList::AddFreeSlot(NodeIndex slot)
{
//	unsigned int i;
	if (mSlotsCached < FREELISTSIZE)
	{
		++mSlotsCached;
		mFreeList[head] = slot;
		head = (head + 1) % FREELISTSIZE;
	}
}

void FreeList::Reset()
{
	unsigned int i;
	for (i = 0; i < FREELISTSIZE; ++i)
		mFreeList[i] = 0xFFFFFFFF;
	mSlotsCached = 0;
	head = 0;
	tail = 0;
	mLargest = 0;
	mIndexOfLargest = 0;
}
