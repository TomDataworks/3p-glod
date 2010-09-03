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
#ifndef FORESTBUILDER_H
#define FORESTBUILDER_H

#include <vector>

#include "forest.h"
#include "node.h"
#include "tri.h"

#include "primtypes.h"

namespace VDS
{
	
class ForestBuilder : public Forest
{
	// Subtypes
	typedef std::vector<NodeIndex> NodeIndexVector;
	typedef Point3 (*MergePositionCreationFunc)(const NodeIndexVector &, const ForestBuilder &);
	typedef Vec3 (*MergeNormalCreationFunc)(const NodeIndexVector &, const ForestBuilder &);
	typedef ByteColor (*MergeColorCreationFunc)(const NodeIndexVector &, const ForestBuilder &);
	typedef Point2 (*MergeTexCoordsCreationFunc)(const NodeIndexVector &, const ForestBuilder &);
	
public:
    ForestBuilder();
    ForestBuilder(const ForestBuilder &); // not implemented
    virtual ~ForestBuilder();
    virtual void Reset();
	
public:
	
	Point3 GetNodePosition(NodeIndex n) const;
	
    // All building of the Forest takes place between a call to BeginForest and a call to EndForest.
	// After EndForest the Forest cannot be modified.
    void BeginForest();
    void EndForest();
	
    // All nodes and triangles are added after a call to beginGeometry.
    void BeginGeometry(bool IsColors, bool IsNormals, unsigned int NumTextures = 0, NodeIndex NumNodes = STARTING_MAX_NODES, TriIndex NumTris = STARTING_MAX_TRIS);
	
    // EndGeometry is called after all nodes and tris have been added.  Clustering of
    // nodes is performed after this call.
    void EndGeometry();
	
	// Reorders nodes in data structure to breadth first
	void ReorderNodesBreadthFirst();
	
	NodeIndex CheckForDepthFirst(NodeIndex iRoot);
	
	// checks if node indices are currently in depth-first order
	bool CheckIfNodesAreDepthFirst(NodeIndex iRoot);

    // Set merge callback functions
    void SetMergePositionCreationFunc(MergePositionCreationFunc fMergePositionCreation);
    void SetMergeNormalCreationFunc(MergeNormalCreationFunc fMergeNormalCreation);
    void SetMergeColorCreationFunc(MergeColorCreationFunc fMergeColorCreation);
    void SetMergeTexCoordsCreationFunc(MergeTexCoordsCreationFunc fMergeTexCoordsCreation);
	
    // Must be called after beginGeometry and before endGeometry
    NodeIndex AddNode(const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal, const Point2 &rTexCoords);
    NodeIndex AddNode(const Point3 &rPosition, const Vec3 &rNormal, const Point2 &rTexCoords);
    NodeIndex AddNode(const Point3 &rPosition, const ByteColor &rColor, const Point2 &rTexCoords);
    NodeIndex AddNode(const Point3 &rPosition, const Point2 &rTexCoords);
    NodeIndex AddNode(const Point3 &rPosition, const Vec3 &rNormal);
    NodeIndex AddNode(const Point3 &rPosition, const ByteColor &rColor);
    NodeIndex AddNode(const Point3 &rPosition);
    NodeIndex AddNode(const Point3 &rPosition,  const ByteColor &rColor, const Vec3 &rNormal);
	
    // Must be called after beginGeometry and before endGeometry
    TriIndex AddTri(NodeIndex iNode1, NodeIndex iNode2, NodeIndex iNode3);

	// Merge Functions
    // Must be called after endGeometry and before endForest
    
    // these two versions of MergeNodes use a function to create the parent node.  The parent is 
	// created by the user provided function and if none exists the default function.
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector);
	
    // these versions allow the user to explicitly pass in the info for the node to be added as the merge
	// parent.  As with the AddNode function, only the version with the correct node data can be used.
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal, const Point2 &rTexCoords);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector, const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal, const Point2 &rTexCoords);
	
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const Vec3 &rNormal, const Point2 &rTexCoords);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector, const Point3 &rPosition, const Vec3 &rNormal, const Point2 &rTexCoords);
	
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const ByteColor &rColor, const Point2 &rTexCoords);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector, const Point3 &rPosition, const ByteColor &rColor, const Point2 &rTexCoords);
	
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const Point2 &rTexCoords);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector, const Point3 &rPosition, const Point2 &rTexCoords);
	
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const Vec3 &rNormal);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector, const Point3 &rPosition, const Vec3 &rNormal);
	
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const ByteColor &rColor);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector, const Point3 &rPosition, const ByteColor &rColor);
	
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector, const Point3 &rPosition);
	
    NodeIndex MergeNodes(unsigned int NumNodes, NodeIndex *piNodes, const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal);
    NodeIndex MergeNodes(const NodeIndexVector &rNodeVector, const Point3 &rPosition, const ByteColor &rColor, const Vec3 &rNormal);
	
private:
    ForestBuilder &operator =(const ForestBuilder &); // not implemented

    Float DefaultClusterImportance(NodeIndex iNode) const;
    Point3 DefaultMergePositionCreation(const NodeIndexVector &NodeVector) const;
    Vec3 DefaultMergeNormalCreation(const NodeIndexVector &NodeVector) const;
    ByteColor DefaultMergeColorCreation(const NodeIndexVector &NodeVector) const;
    Point2 DefaultMergeTexCoordsCreation(const NodeIndexVector &NodeVector) const;
	
    NodeIndex SetupMergeNode(const NodeIndexVector &rChildrenVector);
    
    Float LongestNodeEdge(NodeIndex iNode) const;
	
	// finds the longest triangle edge with an endpoint at node iNode, returns
	// the face angle opposite it in RADIANS
    Float GreatestFaceAngle(NodeIndex index) const;
	
    void BuildSubTriLists();        
//    void ComputeRadii(NodeIndex iNode);
	void ComputeViewIndependentErrors();
	
    bool ReallocateNodes(NodeIndex NewSize = 0);
    bool ReallocateTris(TriIndex NewSize = 0); 
	
    // Relocates the root to index iROOT_NODE, assumes proper Forest structure
    void RelocateRoot();
	
private:
    bool mForestStarted;
    bool mGeometryStarted;
    bool mGeometryEnded;    
    bool mForestEnded;
    Float mAvgEdgeLength;
    TriIndex mNumAllocatedTris;
    NodeIndex mNumAllocatedNodes;
	
	TriIndex *mFirstLiveTris;
	TriIndex **mNextLiveTris;
	
    MergePositionCreationFunc mfMergePositionCreation;
    MergeNormalCreationFunc mfMergeNormalCreation;
    MergeColorCreationFunc mfMergeColorCreation;
    MergeTexCoordsCreationFunc mfMergeTexCoordsCreation;
	
    // Microsoft compiler does not allow initialization of static
    // const integral types within a class declaration.
    static const NodeIndex STARTING_MAX_NODES;
    static const TriIndex STARTING_MAX_TRIS;
	
}; // class ForestBuilder
}; // namespace VDS


#endif //FORESTBUILDER_H
