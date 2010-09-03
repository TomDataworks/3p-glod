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
#pragma warning(disable: 4530)
#include "threads.h"
#include <iostream>

#ifdef WIN32
#include <windows.h>
#endif

int GetNumSystemProcessors()
{
	int n = 0;
//	ztGetNumAvailableProcessors(&n);
	return n;
}

void StartRenderThread(void (*fSimplifyFunc) (void))
{
/*	ThreadParams* gParams = new ThreadParams; // KLUDGE: leaked memory
	gParams->t = t;
	gParams->tv = tv;
	ZThreads::ztCreateThread((ZThreads::ztThreadFunc) fSimplifyFunc);
*/
}

void RenderThreadFunc(void (*fSimplifyFunc) (void))
{
	printf("Simplifyloop running.\n");
		if (fSimplifyFunc != NULL)
		{
			while(1)
			{
				fSimplifyFunc();
			}
		}
		else
#ifdef WIN32
			Sleep(0);
#else
			0;// what do we need to include to get linux Sleep()?
#endif
}
