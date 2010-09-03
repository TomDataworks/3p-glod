#ifndef MERGEFOREST_H
#define MERGEFOREST_H

#include "forest.h"
#include <vector>

namespace VDS
{

class MergeForest : public Forest
{
public:
	MergeForest();
	void CollapseForest(float changePercent);

private:
	void MergeNodes(NodeIndex parent, NodeIndex child);

};

struct ClusterArc
{
	int PQindex;
	NodeIndex miParent;
	NodeIndex miChild;
	//BudgetItem *Parent;
	//BudgetItem *Child;

	//Float mParentError;
	//Float mChildError;
	Float mErrorDifference;
};

struct ArcCompare
{
	bool operator()(const ClusterArc *left, const ClusterArc *right) const
	{
		return (left->mErrorDifference > right->mErrorDifference);
	}
};

};

#endif