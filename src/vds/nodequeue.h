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
#ifndef NODEQUEUE_H
#define NODEQUEUE_H

// this class is used as a queue of the nodes to be folded and a queue of
// the nodes to be unfolded

#include "vds.h"
#include "vdsaux.h"

class VDS::NodeQueue
{
public:
	NodeQueue(Simplifier *pSimplifier);
	~NodeQueue();
	void Initialize(int MaxElements, float MinData);
	void MakeEmpty();
	void Insert(BudgetItem *pItem);
	BudgetItem *FindMin();
	BudgetItem *Find(BudgetItem *pItem);

	// Removes given BudgetItem from queue
	// NOTE: NodeRef pointing to removed BudgetItem is not automatically set
	// to NULL - you must manually set it to NULL after the call to Remove()
	void Remove(BudgetItem *pItem);
	
	void RemoveMin();
	void Update(BudgetItem *pItem);
	int Size;
	void GiveElementTo(BudgetItem *pElement, NodeQueue *pReceivingQueue);
	BudgetItem *GetElement(int i);
	void heapify(int i);
	void buildheap();
	void checkqueue();
	void DoubleCapacity();
	void checkProperty();
	
protected:	
	int Capacity;
	BudgetItem *Elements;
	void _PQupheap(BudgetItem *moving, int i);
	void _PQdownheap(BudgetItem *moving, int i);

	Simplifier *mpSimplifier;

friend class Simplifier;
};

#endif

