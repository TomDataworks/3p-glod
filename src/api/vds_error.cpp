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
#include "vds_callbacks.h"
#include "simplifier.h"
#include <stdio.h>


Float StdBudgetError(BudgetItem *pItem, const Simplifier &rSimplifier)
{
//    const StandardBudget *std_Budget = &static_cast<const StandardBudget&> (rSimplifier);
//	const Node *node = &rBudget.GetNode(iNode);
	Point3 center = rSimplifier.GetNodePosition(pItem->miNode);
	float radius = rSimplifier.GetNodeRadius(pItem->miNode);

	// no error for folding a node completely outside fov
	if (rSimplifier.GetTopPlane() * center > radius || 
		rSimplifier.GetBottomPlane() * center > radius ||
		rSimplifier.GetRightPlane() * center > radius || 
		rSimplifier.GetLeftPlane() * center > radius)
	{
		return 0.0;
	}

	// Create a vector going from eye point to node
	Vec3 v(center, rSimplifier.GetViewPoint());
	float length2 = v * v; // length squared of v
	float radius2 = radius * radius;
    if (length2 < radius2) //camera inside node
    {
		return BIGFLOAT / 2;
    }
    return radius2 / length2;
}

Float StdBudgetErrorViewDistOnly(BudgetItem *pItem, const Simplifier &rSimplifier)
{
	Point3 center = rSimplifier.GetNodePosition(pItem->miNode);

/*
	// no error for folding a node completely outside fov
	if (rSimplifier.GetTopPlane() * center > radius || 
		rSimplifier.GetBottomPlane() * center > radius ||
		rSimplifier.GetRightPlane() * center > radius || 
		rSimplifier.GetLeftPlane() * center > radius)
	{
		return 0.0;
	}
*/
	// Create a vector going from eye point to node
	Vec3 v(center, rSimplifier.GetViewPoint());
	float length2 = v * v; // length squared of v
    return length2;
}

Float StdBudgetErrorNodeSizeOnly(BudgetItem *pItem, const Simplifier &rSimplifier)
{
//    const StandardBudget *std_Budget = &static_cast<const StandardBudget&> (rSimplifier);
//	const Node *node = &rBudget.GetNode(iNode);
	Point3 center = rSimplifier.GetNodePosition(pItem->miNode);
	float radius = rSimplifier.GetNodeRadius(pItem->miNode);

	// no error for folding a node completely outside fov
	if (rSimplifier.GetTopPlane() * center > radius || 
		rSimplifier.GetBottomPlane() * center > radius ||
		rSimplifier.GetRightPlane() * center > radius || 
		rSimplifier.GetLeftPlane() * center > radius)
	{
		return 0.0;
	}

	float radius2 = radius * radius;
    return radius2;
}

Float StdBudgetErrorNoFrustum(BudgetItem *pItem, const Simplifier &rSimplifier)
{
//    const StandardBudget *std_Budget = &static_cast<const StandardBudget&> (rSimplifier);
//	const Node *node = &rBudget.GetNode(iNode);
	Point3 center = rSimplifier.GetNodePosition(pItem->miNode);
	float radius = rSimplifier.GetNodeRadius(pItem->miNode);

	// Create a vector going from eye point to node
	Vec3 v(center, rSimplifier.GetViewPoint());
	float length2 = v * v; // length squared of v
	float radius2 = radius * radius;
    if (length2 < radius2) //camera inside node
    {
		return BIGFLOAT / 2;
    }
    return radius2 / length2;
}

Float OldBudgetError(BudgetItem *pItem, const Simplifier &rSimplifier)
{

//	if (pItem->miNode == 746)
//		printf("calculating error for node 746\n");

	Point3 node_position = rSimplifier.GetNodePosition(pItem->miNode);
	float radius = rSimplifier.GetNodeRadius(pItem->miNode);

    static const float sqrttwo = sqrt(2.0);
	Float distance;     //distance of said vector
	Float inv_distance; //1/distance

	Vec3 d(node_position, rSimplifier.GetViewPoint());
    distance = d.Length();
    inv_distance = 1.0 / distance;
    
	if (rSimplifier.GetTopPlane() * node_position > radius || 
		rSimplifier.GetBottomPlane() * node_position > radius ||
		rSimplifier.GetRightPlane() * node_position > radius || 
		rSimplifier.GetLeftPlane() * node_position > radius)
	{
		return 0.0;
	}

    if (distance < radius) //camera inside node
    {
        return BIGFLOAT / 2;
    }

//	printf("Old Error: %f\n", radius * rSimplifier.GetInvTanFov() * inv_distance);

	float invtanfov = rSimplifier.GetInvTanFov();
	float tmperror = radius * invtanfov * inv_distance;
	if (tmperror < 0)
		printf("error < 0\n");

    return radius * rSimplifier.GetInvTanFov() * inv_distance;
}

Float OldBudgetErrorNoFrustum(BudgetItem *pItem, const Simplifier &rSimplifier)
{
//	if (pItem->miNode == 746)
//		printf("calculating error for node 746\n");

	Point3 node_position = rSimplifier.GetNodePosition(pItem->miNode);
	float radius = rSimplifier.GetNodeRadius(pItem->miNode);

    static const float sqrttwo = sqrt(2.0);
	Float distance;     //distance of said vector
	Float inv_distance; //1/distance

	Vec3 d(node_position, rSimplifier.GetViewPoint());
    distance = d.Length();
    inv_distance = 1.0 / distance;
    
//	if (rSimplifier.GetTopPlane() * node_position > radius || 
//		rSimplifier.GetBottomPlane() * node_position > radius ||
//		rSimplifier.GetRightPlane() * node_position > radius || 
//		rSimplifier.GetLeftPlane() * node_position > radius)
//	{
//		return 0.0;
//	}

    if (distance < radius) //camera inside node
    {
        return BIGFLOAT / 2;
    }

//	printf("Old Error: %f\n", radius * rSimplifier.GetInvTanFov() * inv_distance);

	float invtanfov = rSimplifier.GetInvTanFov();
	float tmperror = radius * invtanfov * inv_distance;
	if (tmperror < 0)
		printf("error < 0\n");

    return radius * rSimplifier.GetInvTanFov() * inv_distance;
}
