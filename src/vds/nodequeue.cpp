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

#include "nodequeue.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#define MinPQSize (1)

using namespace std;
using namespace VDS;

#define PARENT(i) ((i) >> 1)
#define LEFT(i) ((i) << 1)
#define RIGHT(i) (((i) << 1) | 1)

NodeQueue::NodeQueue(Simplifier *pSimplifier)
{
	mpSimplifier = pSimplifier;
}

NodeQueue::~NodeQueue()
{
	free(Elements);
}

void NodeQueue::Initialize(int MaxElements, float MinData)
{
	int i;
#ifdef VERBOSE_MEM_MANAGEMENT
	cerr << "NodeQueue allocating " << (MaxElements+1) << " BudgetItems." << endl;
#endif
	Elements = (VDS::BudgetItem *) malloc((MaxElements+1) * sizeof(BudgetItem));

	Capacity = MaxElements;
	Size = 0;
	Elements[0].mError = MinData;
	Elements[0].miNode = 0;
	Elements[0].PQindex = -666;
	Elements[0].pVertexRenderDatum = NULL;

	for (i = 1; i <= Capacity; ++i)
	{
		Elements[i].mError = MinData;
		Elements[i].miNode = 0;
		Elements[i].PQindex = -666;
		Elements[i].pVertexRenderDatum = NULL;
		Elements[i].miFirstLiveTri = Forest::iNIL_TRI;
	}
}

void NodeQueue::_PQupheap(BudgetItem *moving, int i)
{
    int parent;
    for (parent=i/2; ( (Elements[parent].mError > moving->mError) && (parent >=1) );
         i=parent, parent/=2)
    {
        Elements[i] = Elements[parent];
        Elements[i].PQindex = i;
		mpSimplifier->mpCuts[Elements[i].CutID]->mpNodeRefs[Elements[i].miNode] = &Elements[i];
    }

    Elements[i] = *moving;
    Elements[i].PQindex = i;
	mpSimplifier->mpCuts[Elements[i].CutID]->mpNodeRefs[Elements[i].miNode] = &Elements[i];
}


void NodeQueue::_PQdownheap(BudgetItem *moving, int i)
{
    int child;
    for (child=i*2; child <= Size; i=child, child*=2) {
        if ((child != Size) &&
            (Elements[child+1].mError < Elements[child].mError))
        {
            child++;
        }

        if (moving->mError > Elements[child].mError) 
		{
            Elements[i] = Elements[child];
            Elements[i].PQindex = i;
			mpSimplifier->mpCuts[Elements[i].CutID]->mpNodeRefs[Elements[i].miNode] = &Elements[i];
        }
        else
            break;
    }

    Elements[i] = *moving;
    Elements[i].PQindex = i;
	mpSimplifier->mpCuts[Elements[i].CutID]->mpNodeRefs[Elements[i].miNode] = &Elements[i];
}

void NodeQueue::MakeEmpty()
{
	Size = 0;
}

void NodeQueue::DoubleCapacity()
{
	int i;
	int oldcapacity = Capacity;
	
	Capacity *= 2;
	BudgetItem* OldElements = Elements;

#ifdef VERBOSE_MEM_MANAGEMENT
	cerr << "Reallocating NodeQueue; capacity doubled to " << (Capacity+1) << " BudgetItems." << endl;
#endif
	Elements = (VDS::BudgetItem *) realloc(Elements, (Capacity+1) * sizeof(BudgetItem));
	if (Elements == NULL)
	{
		cerr << "Error - realloc returned null block when increasing nodequeue capacity";
		return;
	}


	for (i = 1; i <= oldcapacity; ++i)
	{
		if (Elements[i].miNode != 0)
			mpSimplifier->mpCuts[Elements[i].CutID]->mpNodeRefs[Elements[i].miNode] = &Elements[i];
	}

	for (i = oldcapacity + 1; i <= Capacity; ++i)
	{
		Elements[i].miNode = 0;
		Elements[i].PQindex = -666;
		Elements[i].pVertexRenderDatum = NULL;
		Elements[i].mError = Elements[0].mError;
		Elements[i].miFirstLiveTri = Forest::iNIL_TRI;
	}
}

void NodeQueue::Insert(BudgetItem* pItem)
{
	if (Size >= Capacity)
		DoubleCapacity();

	_PQupheap(pItem, ++Size);
}


BudgetItem *NodeQueue::FindMin()
{
	if (Size > 0)
	    return &Elements[1];
	else
		return NULL;
}

BudgetItem *NodeQueue::Find(BudgetItem *pItem)
{
	return &Elements[pItem->PQindex];
}

void NodeQueue::RemoveMin()
{
    BudgetItem *Element;

    Element = &Elements[Size--];
    _PQdownheap(Element, 1);
}

/***************************************************
description: after Remove(), PQ is maintained. 
****************************************************/
void NodeQueue::Remove(BudgetItem *pItem)
{
    int i;

	i = pItem - Elements;
	int parent = i/2;
    pItem = &Elements[Size--];

	if (pItem->mError < Elements[parent].mError)
			 _PQupheap(pItem, i);
	else	_PQdownheap(pItem, i); 
}


void NodeQueue::GiveElementTo(BudgetItem *pElement, NodeQueue *pReceivingQueue)
{
	BudgetItem *tmpElement = pElement;
	int tmpCutID = pElement->CutID;
	unsigned int tmpIndex = pElement->miNode;

	BudgetItem *tmp = new BudgetItem;
	memcpy(tmp, pElement, sizeof(BudgetItem));
	tmp->mError = (-1) * pElement->mError;

	pReceivingQueue->Insert(tmp);
	
	tmpElement = mpSimplifier->mpCuts[tmpCutID]->mpNodeRefs[tmpIndex];
	if (tmpElement == NULL)
		cerr << " giveElementto : NodeRef is null after insert" << endl;

 	Remove(pElement); 	
	mpSimplifier->mpCuts[tmpCutID]->mpNodeRefs[tmpIndex] = tmpElement;
}


BudgetItem *NodeQueue::GetElement(int i)
{
	return &Elements[i];
}

void NodeQueue::heapify(int i)
{
	int l,r,smallest = -1;
	BudgetItem tmp;

	while (1)
	{
		l = LEFT(i);
		r = RIGHT(i);
		
		if (l <= Size && Elements[l].mError <= Elements[i].mError)
		{
			smallest = l;
		}
		else
		{
			smallest = i;
		}
		if (r <= Size && Elements[r].mError <= Elements[smallest].mError)
		{
			smallest = r;
		}
		if (smallest != i)
		{
			tmp = Elements[i];

			Elements[i] = Elements[smallest];
			Elements[i].PQindex = i;
			mpSimplifier->mpCuts[Elements[i].CutID]->mpNodeRefs[Elements[i].miNode] = &Elements[i];

			Elements[smallest] = tmp;
			Elements[smallest].PQindex = smallest;
			mpSimplifier->mpCuts[Elements[smallest].CutID]->mpNodeRefs[Elements[smallest].miNode] = &Elements[smallest];
			i = smallest;
		}
		else
		{
			break;
		}

	}
}


void NodeQueue::buildheap()
{
	int i;
	BudgetItem * pItem;
	for (i = Size / 2; i > 0; --i)
	{
		heapify(i);
	}
}


/****************************************************************************************
description: For DEBUG only. to check is the array maintains PQ property.
*****************************************************************************************/
void NodeQueue::checkProperty()
{
	int i;
	int l,r,smallest = -1;

	cout<<"check:";
	for (i = Size / 2; i > 0; --i)
	{
	
		l = LEFT(i);
		r = RIGHT(i);
		
		if (  (l <= Size && Elements[l].mError < Elements[i].mError )
			||  (r <= Size && Elements[r].mError < Elements[i].mError))
		{
			cout<<"    Priority Q's properties are violated"<<endl;
			break;
		}
	}
	cout << endl;
}


/****************************************************************************************
description: For DEBUG only. to check NodeRef and PQindex's consistency.
*****************************************************************************************/
void NodeQueue::checkqueue()
{
	int temp;
	for (int i = 1; i <= Size; ++i)
	{
		if (mpSimplifier->mpCuts[Elements[i].CutID]->mpNodeRefs[Elements[i].miNode] == NULL)
			temp = 1;
		if (mpSimplifier->mpCuts[Elements[i].CutID]->mpNodeRefs[Elements[i].miNode]->PQindex != i)
		{
			cout << "NodeQueue element " << i << " failed node check test." << endl;
			return;
		}
		
	}
	cout << "check queue correct"<< endl;
}








