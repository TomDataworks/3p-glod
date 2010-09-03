/*****************************************************************************\
  SimpQueue.C
  --
  Description : The simplification queue provides a cost(error)-driven
                priority queue of simplification operations.
                Sub-classes like the Lazy queue or the Independent
                queue provide some richer behavior beneath the
                hood, while maintaining the same simple interface
                (insert, remove, getNextOperation).

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/SimpQueue.C,v $
  $Revision: 1.6 $
  $Date: 2004/11/11 01:06:28 $
  $Author: gfx_friends $
  $Locker:  $
\*****************************************************************************/
/******************************************************************************
 * Copyright 2003 Jonathan Cohen, Nat Duca, David Luebke, Brenden Schubert    *
 *                Johns Hopkins University and University of Virginia         *
 ******************************************************************************
 * This file is distributed as part of the GLOD library, and as such, falls   *
 * under the terms of the GLOD public license. GLOD is distributed without    *
 * any warranty, implied or otherwise. See the GLOD license for more details. *
 *                                                                            *
 * You should have recieved a copy of the GLOD Open-Source License with this  *
 * copy of GLOD; if not, please visit the GLOD web page,                      *
 * http://www.cs.jhu.edu/~graphics/GLOD/license for more information          *
 ******************************************************************************/


/*----------------------------- Local Includes -----------------------------*/

#include "xbs.h"

/*----------------------------- Local Constants -----------------------------*/


/*------------------------------ Local Macros -------------------------------*/

#if 1
#define RECOMPUTE_INFINITE_COST_OPS
#endif

/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/


/*------------------------ Local Function Prototypes ------------------------*/


/*---------------------------------Functions-------------------------------- */

/*****************************************************************************\
 @ SimpQueue::update
 -----------------------------------------------------------------------------
 description : After a simplification operation is applied, the queue
               update removes some operations from the queue, adds
               some new ones, and modifies some existing ones by
               recomputing their costs.
 input       : Lists of operations to remove, add, and modify
 output      : 
 notes       :
\*****************************************************************************/
void
SimpQueue::update(Model *model,
                  Operation **addOps, int numAddOps,
                  Operation **removeOps, int numRemoveOps,
                  Operation **modOps, int numModOps)
{
    // remove the deleted operations
    for (int opnum=0; opnum<numRemoveOps; opnum++)
    {
        Operation *op = removeOps[opnum];
        remove(op);
    }

    // add the new operations
    for (int opnum=0; opnum<numAddOps; opnum++)
    {
        Operation *op = addOps[opnum];
        op->computeCost(model);
        insert(op);
    }

    // change the modified operations

    // Currently just implement greedy scheme. Later add lazy,
    // independent, etc.
    
    for (int opnum=0; opnum<numModOps; opnum++)
    {
        Operation *op = modOps[opnum];
        op->computeCost(model);
        modify(op);
    }
    return;
} /** End of SimpQueue::update() **/

/*****************************************************************************\
 @ LazySimpQueue::update
 -----------------------------------------------------------------------------
 description : Unlike the update method for the greedy queue, this
               method does not recompute the costs for modified
               operations, but merely flags them as dirty.
 input       : Lists of operations to add, remove, or modify.
 output      : 
 notes       :
\*****************************************************************************/
void
LazySimpQueue::update(Model *model,
                      Operation **addOps, int numAddOps,
                      Operation **removeOps, int numRemoveOps,
                      Operation **modOps, int numModOps)
{
    // remove the deleted operations
    for (int opnum=0; opnum<numRemoveOps; opnum++)
    {
        Operation *op = removeOps[opnum];
        remove(op);
    }

    // add the new operations
    for (int opnum=0; opnum<numAddOps; opnum++)
    {
        Operation *op = addOps[opnum];
        op->computeCost(model);
        insert(op);
    }

    // change the modified operations

    for (int opnum=0; opnum<numModOps; opnum++)
    {
        Operation *op = modOps[opnum];
        op->setDirty();
#ifdef RECOMPUTE_INFINITE_COST_OPS
        // Operations with infinite cost are kept off the queue, but
        // this means that they will never get cleaned
        // (recomputed). One option is to clean them here. This could
        // get expensive if there and lots and lots of such
        // operations, as might occur if the border lock mechanism is
        // used. When there are lots, the work is equivalent to the
        // greedy queue. Another option is to let them sit around
        // until the entire queue is empty and then give them one last
        // try at the end.
        if (op->getCost() == MAXFLOAT)
        {
            op->computeCost(model);
            insert(op);
        }
#endif
    }
    return;
} /** End of SimpQueue::update() **/

void
IndependentSimpQueue::reactivateDependentOps(Model *model)
{
    fprintf(stderr, "Reactivate %d operations.\n",
            dependentOps.size());
    
    while (dependentOps.size() > 0)
    {
        Operation *op = (Operation *)(dependentOps.extractMin()->userData());
        op->computeCost(model);
        insert(op);
    }
}


/*****************************************************************************\
 @ IndependentSimpQueue::update
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
IndependentSimpQueue::update(Model *model,
                             Operation **addOps, int numAddOps,
                             Operation **removeOps, int numRemoveOps,
                             Operation **modOps, int numModOps)
{
    // remove the deleted operations
    for (int opnum=0; opnum<numRemoveOps; opnum++)
    {
        Operation *op = removeOps[opnum];
        // remove from either real heap or dependent heap
        if (op->heapdata.heap() != NULL)
            op->heapdata.heap()->remove(&(op->heapdata));
    }

    // add the new operations
    for (int opnum=0; opnum<numAddOps; opnum++)
    {
        Operation *op = addOps[opnum];
        op->computeCost(model);
        insert(op);
    }

    // change the modified operations

    for (int opnum=0; opnum<numModOps; opnum++)
    {
        Operation *op = modOps[opnum];
        op->setDirty();
    }
    return;
} /** End of IndependentSimpQueue::update() **/

Operation *
IndependentSimpQueue::getNextOperation(Model *model)
{
    if (heap.size() <= 0)
        reactivateDependentOps(model);
    if (heap.size() <= 0)
        return NULL;
    
    Operation *op =
        (Operation *)(heap.extractMin()->userData());
    
    while ((op != NULL) && (op->isDirty() == 1))
    {
        dependentOps.insert(&(op->heapdata));
        if (heap.size() > 0)
            op = (Operation *)(heap.extractMin()->userData());
        else
        {
            reactivateDependentOps(model);
            if (heap.size() > 0)
                op = (Operation *)(heap.extractMin()->userData());
            else
                op = NULL;
        }
    }

    return op;
}

/*****************************************************************************\
  $Log: SimpQueue.C,v $
  Revision 1.6  2004/11/11 01:06:28  gfx_friends
  Improved handling of infinite-cost edges in lazy queue mode. Previously,
  they would never have their costs recomputed, so they never had a chance
  of becoming finite and going back on the queue.

  Revision 1.5  2004/07/30 20:56:51  gfx_friends
  Some documentation and commenting updates

  Revision 1.4  2004/07/08 16:47:49  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.3  2003/07/26 01:17:45  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.2  2003/07/23 19:55:35  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.1  2003/01/13 20:30:16  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/

