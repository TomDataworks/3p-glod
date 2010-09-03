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

// This class started out as a queue but is actually now a set; i just never bothered to change the filenames

#ifndef SET_H
#define SET_H

#include "zthreads.h"

using namespace ZThreads;

struct IndexLockPair
{
	unsigned long x;
	ztLock *lock;
	int thread;
};

class Set
{
public:
	void Init();
	void Insert(unsigned long n);
	void Remove(unsigned long n);
	bool IsAMember(unsigned long n);
	void Lock(unsigned long n, int thread);
	void Unlock(unsigned long n, int thread);
	int lastremoved;

private:
	long size;				// number of indices published (must never be less than the size of the arrays below)
	IndexLockPair a[10];	// array to hold published indices
	ztLock locks[10];		// locks for each published index to ensure publishing semantics
};




#endif // #ifdef SET_H
