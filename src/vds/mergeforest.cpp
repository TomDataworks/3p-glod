#include "mergeforest.h"
#include <queue>
#include <vector>

namespace VDS {

MergeForest::MergeForest()
{}

void MergeForest::MergeNodes(NodeIndex parent, NodeIndex child)
{
		NodeIndex leftmostGrandchild = mpNodes[child].miFirstChild;
		NodeIndex rightmostGrandchild = mpNodes[child].miFirstChild;
		while (mpNodes[rightmostGrandchild].miRightSibling != iNIL_NODE)
		{
			mpNodes[rightmostGrandchild].miParent = mpNodes[child].miParent;
			rightmostGrandchild = mpNodes[rightmostGrandchild].miRightSibling;
		}
		if (mpNodes[parent].miFirstChild = child)
			mpNodes[parent].miFirstChild = leftmostGrandchild;
		mpNodes[leftmostGrandchild].miLeftSibling = mpNodes[child].miLeftSibling;
		mpNodes[mpNodes[child].miLeftSibling].miRightSibling = leftmostGrandchild;
		mpNodes[rightmostGrandchild].miRightSibling = mpNodes[child].miRightSibling;
		mpNodes[mpNodes[child].miRightSibling].miLeftSibling = rightmostGrandchild;
}

void MergeForest::CollapseForest(float changePercent)
{
	std::priority_queue<ClusterArc*, std::vector<ClusterArc*>, ArcCompare> *clusterQueue = 
		new std::priority_queue<ClusterArc*, std::vector<ClusterArc*>, ArcCompare>;
	ClusterArc *arc;

	// calculate errors
	for (int i = 1; i <= mNumNodes; ++i)
		StdViewIndependentError(i, *this);

	// fill the PQ with edges (starting past the root since it doesn't end any edges)
	for (i = 2; i <= mNumNodes; ++i)
	{
		arc = new ClusterArc;
		arc->miChild = i;
		arc->miParent = mpNodes[i].miParent;
		arc->mErrorDifference = mpErrorParams[mpNodes[arc->miParent].miErrorParamIndex] - mpErrorParams[mpNodes[arc->miChild].miErrorParamIndex];
		clusterQueue->push(arc);
	}
	
	int numChanges = 0;
	ClusterArc *minArc;
	int numArcs = clusterQueue->size();
	
	while (numChanges < ((float)numArcs * (changePercent/100.0f)) && numChanges < numArcs)
	{
		minArc = clusterQueue->top();
		MergeNodes(minArc->miParent, minArc->miChild);
		clusterQueue->pop();
		
		++numChanges;
	}
	
	delete clusterQueue;
}


};