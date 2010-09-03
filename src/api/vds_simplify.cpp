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

// SIMPLIFICATION CALLBACKS ******************************************

#include "vds_callbacks.h"
#include "simplifier.h"

bool StdCheckUnfold(BudgetItem *pItem, const Simplifier &rSimplifier)
{
//    const StandardBudget *std_Budget = &static_cast<const StandardBudget&> (rBudget);
//	const Node *node = &rBudget.GetNode(iNode);

	Point3 center = rSimplifier.GetNodePosition(pItem->miNode);
	float radius = rSimplifier.GetNodeRadius(pItem->miNode);

//if (pItem->miNode != 1)
//	return false;
//else
//	printf("node 1\n");	

	float topdist, bottomdist, rightdist, leftdist;
	

	// Fold nodes entirely outside any of the four frustum planes
	if (rSimplifier.GetTopPlane() * center > radius || 
		rSimplifier.GetBottomPlane() * center > radius ||
		rSimplifier.GetRightPlane() * center > radius || 
		rSimplifier.GetLeftPlane() * center > radius)
	{
		return false;
	}

return true;

	// Create a vector going from eye point to node
	Vec3 v(center, rSimplifier.GetViewPoint());
	float length2 = v * v; // length squared of v
	float radius2 = radius * radius;

//	 Unfold the node if it contains eye:
//	 	length < radius, OR 
//	 	length2 < radius2
//	 or if theta (angle subtended by node) >= threshold:
//	 	sin(theta) < sin(threshold)
//	 	sin2(theta) < sin2(threshold)
//	 	radius2 / length2 < sin2(threshold)
	
	if (length2 < radius2 || radius2 / length2 > rSimplifier.GetSin2Threshold())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool StdCheckUnfoldNoFrustum(BudgetItem *pItem, const Simplifier &rSimplifier)
{
//    const StandardBudget *std_Budget = &static_cast<const StandardBudget&> (rBudget);
//	const Node *node = &rBudget.GetNode(iNode);
	Point3 center = rSimplifier.GetNodePosition(pItem->miNode);
	float radius = rSimplifier.GetNodeRadius(pItem->miNode);

	// Create a vector going from eye point to node
	Vec3 v(center, rSimplifier.GetViewPoint());
	float length2 = v * v; // length squared of v
	float radius2 = radius * radius;

//	 Unfold the node if it contains eye:
//	 	length < radius, OR 
//	 	length2 < radius2
//	 or if theta (angle subtended by node) >= threshold:
//	 	sin(theta) < sin(threshold)
//	 	sin2(theta) < sin2(threshold)
//	 	radius2 / length2 < sin2(threshold)

	if (length2 < radius2 || radius2 / length2 > rSimplifier.GetSin2Threshold())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//ONLY USE WITH StandardBudget!!!
bool OldCheckUnfold(BudgetItem *pItem, const Simplifier &rSimplifier)
{/*
    const Point3 *node_position;
    const Node *node;        
    static const sqrttwo = sqrt(2);
    Float distance;     //distance of vector from eye to node centers
    Float inv_distance; //1/distance
    Float theta;        //angle from D to viewing direction
    Float phi;           //angle subtended by sphere
    Float d_dot_v;      //dot product of d with view vector
    Float radius;
    
    node_position = &rBudget.GetNodePosition(iNode);
    node = &rBudget.GetNode(iNode);
    
    Vec3 d(*node_position);
    d -= (Vec3) (static_cast<const StandardBudget &> (rBudget)).GetViewPoint();    
    distance = d.Length();
    inv_distance = 1.0 / distance;
    
    radius = node->GetRadius();
    if (distance < radius) //camera inside node
    {
        return true;
    }
    phi = atan(radius * inv_distance);
    
    d_dot_v = d * (static_cast<const StandardBudget &> (rBudget)).GetLookVector();
    theta = acos(d_dot_v * inv_distance);
        
    if (theta - phi > (static_cast<const StandardBudget &> (rBudget)).GetFov() * sqrttwo / 2.0)
    {
        return false; //node outside Fov cone
    }
       
    if (radius * (static_cast<const StandardBudget &> (rBudget)).GetInvTanFov() * inv_distance < (static_cast<const StandardBudget &> (rBudget)).GetThreshold())
    {
        return false;
    }
	*/    
    return true;
}

bool OldCheckUnfoldNoFrustum(BudgetItem *pItem, const Simplifier &rSimplifier)
{/*
    const Point3 *node_position;
    const Node *node;        
    static const sqrttwo = sqrt(2);
    Float distance;     //distance of vector from eye to node centers
    Float inv_distance; //1/distance
    Float theta;        //angle from D to viewing direction
    Float phi;           //angle subtended by sphere
    Float d_dot_v;      //dot product of d with view vector
    Float radius;
    
    node_position = &rBudget.GetNodePosition(iNode);
    node = &rBudget.GetNode(iNode);
    
    Vec3 d(*node_position);
    d -= (Vec3) (static_cast<const StandardBudget &> (rBudget)).GetViewPoint();    
    distance = d.Length();
    inv_distance = 1.0 / distance;
    
    radius = node->GetRadius();
    if (distance < radius) //camera inside node
    {
        return true;
    }
    phi = atan(radius * inv_distance);
    
    d_dot_v = d * (static_cast<const StandardBudget &> (rBudget)).GetLookVector();
    theta = acos(d_dot_v * inv_distance);
              
    if (radius * (static_cast<const StandardBudget &> (rBudget)).GetInvTanFov() * inv_distance < (static_cast<const StandardBudget &> (rBudget)).GetThreshold())
    {
        return false;
    }
	*/    
    return true;
}
