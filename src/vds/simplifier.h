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
#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "vds.h"
#include "nodequeue.h"
#include "primtypes.h"
#include "tri.h"

namespace VDS
{

class Simplifier
{
public: // PUBLIC FUNCTIONS
	Simplifier();
	virtual ~Simplifier();

	void AddCut(Cut *pCut);
	void RemoveCut(Cut *pCut);
	void UpdateNodeErrors();

	// simplify cut(s) assigned to this simplifier
	void SimplifyBudget(unsigned int Budget, bool UseTriBudget);
	void SimplifyThreshold(float Threshold);
	void SimplifyBudgetAndThreshold(unsigned int Budget, bool UseTriBudget, float Threshold);

	// set budget mode error callback
	void SetErrorFunc(ErrorFunc fError);

	// get current memory usage by this simplifier's cuts
	unsigned int GetMemoryUsage();

	// get current total rendered triangles of this simplifier's cuts
	unsigned int GetTriangleCount();

	// these functions for use by error callbacks to get the position and radius of nodes
	const Point3& GetNodePosition(BudgetItem *pItem) const;
//	const Float& GetNodeRadius(BudgetItem *pItem) const;
//	const Plane3& GetTopPlane(BudgetItem *pItem) const;
//	const Plane3& GetBottomPlane(BudgetItem *pItem) const;
//	const Plane3& GetRightPlane(BudgetItem *pItem) const;
//	const Plane3& GetLeftPlane(BudgetItem *pItem) const;
//	const Point3& GetViewpoint(BudgetItem *pItem) const;
//	const Float& GetInvTanFov(BudgetItem *pItem) const;

protected: // PRIVATE FUNCTIONS
	void Unfold(BudgetItem *pItem, unsigned int &NumTris, unsigned int &BytesUsed);
	void Fold(BudgetItem *pItem, unsigned int &NumTris, unsigned int &BytesUsed);

	// removes all nodes from fold queue and removes all nodes except root node from unfold queue
	// deletes BudgetItems of all pruned and reverse-pruned nodes
	void FlushQueues();

public: // DEBUG FUNCTIONS
	void DisplayQueues();
	void CheckLiveTrisProxies(Forest *pForest, Renderer *pRenderer);
	void CheckNodeAndChildrensLiveTrisProxies(NodeIndex iNode, Forest *pForest, Renderer *pRenderer);
	void CheckTrisForNullProxy(Forest *pForest, Renderer *pRenderer);

public: // PUBLIC DATA
	ErrorFunc mfErrorFunc;
//	Float mThreshold;
//	Float mSin2Threshold;
	int mBudgetTolerance;
	unsigned int mSimplificationBreakCount;
	bool mIsValid;
	Cut **mpCuts;	// dynamically allocated array of pointers to cuts this simplifier simplifies

protected: // PRIVATE DATA
public: // making these public for access by glod VDSCut
	int mNumCuts;	// number of cuts the simplifier is simplifying
	int miCurrentCut;
	Forest *mpCurrentForest;
	NodeQueue *mpFoldQueue;
	NodeQueue *mpUnfoldQueue;

public: // profiling information
#ifdef _WIN32
	LARGE_INTEGER timer_update_budget_errors;
	LARGE_INTEGER timer_simplification;
	LARGE_INTEGER fold_time;
	LARGE_INTEGER unfold_time;
	LARGE_INTEGER fold_check;
	LARGE_INTEGER fold_remove_children;
	LARGE_INTEGER fold_remove_subtris;
	LARGE_INTEGER fold_queue_manip;
	LARGE_INTEGER unfold_activate_children;
	LARGE_INTEGER unfold_move_livetris;
	LARGE_INTEGER unfold_activate_subtris;
	LARGE_INTEGER unfold_queue_manip;
	LARGE_INTEGER unfold_finding_trirenderdatum_slot;
	LARGE_INTEGER unfold_adding_trirenderdatum;
	LARGE_INTEGER unfold_finding_vertexrenderdatum_slot;
	LARGE_INTEGER unfold_adding_vertexrenderdatum;
	LARGE_INTEGER update_budget_errors_calc;
	LARGE_INTEGER update_budget_errors_queue_manip;

	LARGE_INTEGER times_26;
	LARGE_INTEGER times_27;
	LARGE_INTEGER times_28;
	LARGE_INTEGER times_29;
#endif

	unsigned int tris_introduced;
	unsigned int tris_removed;
	unsigned int foldqueue_size;
	unsigned int unfoldqueue_size;

//Friends
	friend class Tree;
	friend class Tri;
	friend class ForestBuilder;
	friend class Cut;
}; // class Simplifier

} // namespace VDS

#endif // SIMPLIFIER_H

