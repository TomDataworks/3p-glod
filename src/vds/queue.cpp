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
#include "queue.h"
#include <iostream>
using namespace std;

void Set::Init()
{
	long i;

	size = 0;
	
	for (i = 0; i < 10; ++i)
	{
		a[i].lock = &locks[i];
	}
}

void Set::Insert(unsigned long n)
{
	long i;
	for (i = 0; i < size; ++i)
	{
		if (a[i].x == n)
		{
			cerr << "DUPINSERT:" << n << " " << flush;
			return;
		}
	}
	a[size].x = n;
	size++;
}

void Set::Remove(unsigned long n)
{
	long i;
	bool success = false;
	for (i = 0; i < size; ++i)
	{
		if (a[i].x == n)
		{
			a[i].x = a[size-1].x;
			a[i].thread = a[size-1].thread;
			ztLock *temp;
			temp = a[i].lock;
			a[i].lock =	a[size-1].lock;
			a[size-1].lock = temp;
			size--;
			success = true;
		}
	}
	if (success == false)
	{
		cerr << "REMOVEBAD:" << n << " " << flush;
		return;
	}
}

bool Set::IsAMember(unsigned long n)
{
	long i;
	bool result = false;
	for (i = 0; i < size; ++i)
	{
		if (a[i].x == n)
		{
			result = true;
		}
	}
	return result;
}

void Set::Lock(unsigned long n, int thread)
{
	long i;
#ifdef _WIN32
try {
#endif
	for (i = 0; i < size; ++i)
	{
		if (a[i].x == n)
		{
			a[i].lock->Lock();
			a[i].thread = thread;
			return;
		}
	}
#ifdef _WIN32
}
#include "terseexceptionhandling.cpp"
#endif
	cerr << "LOCKNOTEXIST:" << n << " " << flush;
}

void Set::Unlock(unsigned long n, int thread)
{
	long i;
#ifdef _WIN32
try {
#endif
	for (i = 0; i < size; ++i)
	{
		if (a[i].x == n)
		{
			if (a[i].thread != thread)
			{
				cerr << "UNLOCKBYWRONGTHREAD:" << n << " " << flush;
			}
			a[i].lock->Unlock();
			return;
		}
	}
#ifdef _WIN32
}
#include "terseexceptionhandling.cpp"
#endif
	cerr << "UNLOCKNOTEXIST:" << n << " " << flush;
}
