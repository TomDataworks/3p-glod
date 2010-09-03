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
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#pragma implementation
#endif
#include "mt.h"

int
MT::addVertex(const mtVertex &vert)
{
    if (maxVerts == 0)
    {
	verts = vert.makeNew();
	if (!verts)
	{
	    fprintf(stderr, "Cannot add first vertex to MT.\n");
	    exit(1);
	}
	
	maxVerts = 1;
    }
    else if (numVerts == maxVerts)
    {
	mtVertex *oldverts = verts;
	mtVertex *newverts = vert.makeNew(maxVerts*2);
	if (!newverts)
	{
	    fprintf(stderr, "MT: cannot expand to %d verts.\n",
		    maxVerts*2);
	    exit(1);
	}
	for (int i=0; i<maxVerts; i++)
	{
	    // this is really UGLY

	    // first ugliness: only use getVert() to index verts[]
	    verts = oldverts;
	    mtVertex *oldvert = getVert(i);
	    verts = newverts;
	    mtVertex *newvert = getVert(i);

	    // second ugliness: need special function that knows that the
	    // two instances are the same subclass to do the copying
	    // properly
	    oldvert->copySame(newvert);
	}
	verts = newverts;
	maxVerts *= 2;

	delete oldverts;
    }

    vert.copySame(getVert(numVerts++));
    return numVerts-1;
}

int
MT::addTriangle(int vert0, int vert1, int vert2)
{
    if ((vert0 < 0) || (vert1 < 0) || (vert2 < 0) ||
	(vert0 >= numVerts) || (vert1 >= numVerts) || (vert2 >= numVerts))
    {
	fprintf(stderr, "MT->addTriangle(): vertex ids out of range.\n"
		"numVerts=%d, ids: %d %d %d\n",
		numVerts, vert0, vert1, vert2);
	exit(1);
    }
    
    if (maxTris == 0)
    {
	tris = new mtTriangle;
	if (!tris)
	{
	    fprintf(stderr, "Cannot add first triangle to MT.\n");
	    exit(1);
	}
	
	maxTris = 1;
    }
    else if (numTris == maxTris)
    {
	mtTriangle *oldtris = tris;
	tris = new mtTriangle [maxTris*2];
	if (!tris)
	{
	    fprintf(stderr, "MT: cannot expand to %d triangles.\n",
		    maxTris*2);
	    exit(1);
	}
	for (int i=0; i<maxTris; i++)
	    tris[i] = oldtris[i];
	maxTris *= 2;

	delete oldtris;
    }

    
    tris[numTris++].set(vert0, vert1, vert2);

    return numTris-1;
}


int
MT::addArc()
{
    if (maxArcs == 0)
    {
	arcs = new mtArc;
	if (!arcs)
	{
	    fprintf(stderr, "Cannot add first arc to MT.\n");
	    exit(1);
	}
	
	maxArcs = 1;
    }
    else if (numArcs == maxArcs)
    {
	mtArc *oldarcs = arcs;
	arcs = new mtArc [maxArcs*2];
	if (!arcs)
	{
	    fprintf(stderr, "MT: cannot expand to %d arcs.\n",
		    maxArcs*2);
	    exit(1);
	}
	for (int i=0; i<maxArcs; i++)
	    arcs[i] = oldarcs[i];
	maxArcs *= 2;

	delete oldarcs;
    }

    numArcs++;
    return numArcs - 1;
}

int
MT::addArc(int tri)
{
    if (maxArcs == 0)
    {
	arcs = new mtArc;
	if (!arcs)
	{
	    fprintf(stderr, "Cannot add first arc to MT.\n");
	    exit(1);
	}
	
	maxArcs = 1;
    }
    else if (numArcs == maxArcs)
    {
	mtArc *oldarcs = arcs;
	arcs = new mtArc [maxArcs*2];
	if (!arcs)
	{
	    fprintf(stderr, "MT: cannot expand to %d arcs.\n",
		    maxArcs*2);
	    exit(1);
	}
	for (int i=0; i<maxArcs; i++)
	    arcs[i] = oldarcs[i];
	maxArcs *= 2;

	delete oldarcs;
    }

    arcs[numArcs++].addTri(tri);
    return numArcs - 1;
}

int
MT::addArc(int *tris, int numTris)
{
    if (maxArcs == 0)
    {
	arcs = new mtArc;
	if (!arcs)
	{
	    fprintf(stderr, "Cannot add first arc to MT.\n");
	    exit(1);
	}
	
	maxArcs = 1;
    }
    else if (numArcs == maxArcs)
    {
	mtArc *oldarcs = arcs;
	arcs = new mtArc [maxArcs*2];
	if (!arcs)
	{
	    fprintf(stderr, "MT: cannot expand to %d arcs.\n",
		    maxArcs*2);
	    exit(1);
	}
	for (int i=0; i<maxArcs; i++)
	    arcs[i] = oldarcs[i];
	maxArcs *= 2;

	delete oldarcs;
    }

    arcs[numArcs++].addTris(tris, numTris);
    return numArcs - 1;
}

int
MT::addNode()
{
    if (maxNodes == 0)
    {
	nodes = new mtNode;
	if (!nodes)
	{
	    fprintf(stderr, "Cannot add first node to MT.\n");
	    exit(1);
	}
	
	maxNodes = 1;
    }
    else if (numNodes == maxNodes)
    {
	mtNode *oldnodes = nodes;
	nodes = new mtNode [maxNodes*2];
	if (!nodes)
	{
	    fprintf(stderr, "MT: cannot expand to %d nodes.\n",
		    maxNodes*2);
	    exit(1);
	}
	for (int i=0; i<maxNodes; i++)
	    nodes[i] = oldnodes[i];
	maxNodes *= 2;

	delete oldnodes;
    }

    numNodes++;
    return numNodes-1;
}

int
MT::addPoint(const mtPoint &p)
{
    if (maxPoints == 0)
    {
	points = new mtPoint;
	if (!points)
	{
	    fprintf(stderr, "Cannot add first point to MT.\n");
	    exit(1);
	}
	
	maxPoints = 1;
    }
    else if (numPoints == maxPoints)
    {
	mtPoint *oldpoints = points;
	points = new mtPoint [maxPoints*2];
	if (!points)
	{
	    fprintf(stderr, "MT: cannot expand to %d points.\n",
		    maxPoints*2);
	    exit(1);
	}
	for (int i=0; i<maxPoints; i++)
	    points[i] = oldpoints[i];
	maxPoints *= 2;

	delete oldpoints;
    }

    points[numPoints++] = p;
    return numPoints-1;
}

int compareArcs(const void *e1, const void *e2)
{
    const mtArc *arc1 = (const mtArc *)e1;
    const mtArc *arc2 = (const mtArc *)e2;

    if (arc1->getStart() < arc2->getStart())
	return -1;
    if (arc1->getStart() > arc2->getStart())
	return 1;
    if (arc1->getEnd() < arc2->getEnd())
	return -1;
    if (arc1->getEnd() > arc2->getEnd())
	return 1;
    if (arc1->getPatchNumber() < arc2->getPatchNumber())
	return -1;
    if (arc1->getPatchNumber() > arc2->getPatchNumber())
	return 1;
    if (arc1->isBorder() == 1)
	return -1;
    if (arc2->isBorder() == 1)
	return 1;
    if (arc1 < arc2)
	return -1;
    return 1;
}


void
MT::mergeArcs()
{
    if (numArcs < 2)
	return;
    
    // sort the arcs
    qsort(arcs, numArcs, sizeof(mtArc), compareArcs);

    // do the merging
    int next, current;
    mtArc *nextArc, *currentArc;
    
    for (next=1, current=0,
	 currentArc = &arcs[current],
	 nextArc = &arcs[next];
	 next < numArcs;
	 next++, nextArc = &arcs[next])
    {
	if ((currentArc->getStart() == nextArc->getStart()) &&
	    (currentArc->getEnd() == nextArc->getEnd()) &&
	    (currentArc->getPatchNumber() == nextArc->getPatchNumber()))
	{
	    for (int tnum=0; tnum < nextArc->getNumTris(); tnum++)
		currentArc->addTri(nextArc->getTri(tnum));
	    nextArc->deleteTris();
	}
	else
	{
	    current++;
	    currentArc = &arcs[current];
	    if (current < next)
		*currentArc = *nextArc;
	}
    }

    int newNumArcs = current + 1;
    
    // try to shrink the space
    mtArc *oldarcs = arcs;
    arcs = new mtArc [newNumArcs];
    if (arcs == 0)
    {
	arcs = oldarcs;
	numArcs = newNumArcs;
	return;
    }

    for (int i=0; i<newNumArcs; i++)
	arcs[i] = oldarcs[i];
    numArcs = newNumArcs;
    delete oldarcs;
    maxArcs = numArcs;
    
    return;
}

void
MT::connectArcs()
{
    // assumes that all nodes have 0 parents and children connected

    // let's also go ahead and compact the main element lists now
    if (maxVerts > numVerts)
    {
	mtVertex *oldverts = verts;
	mtVertex *newverts = verts[0].makeNew(numVerts);
	for (int i=0; i<numVerts; i++)
	{
	    // this is really UGLY

	    // first ugliness: only use getVert() to index verts[]
	    verts = oldverts;
	    mtVertex *oldvert = getVert(i);
	    verts = newverts;
	    mtVertex *newvert = getVert(i);

	    // second ugliness: need special function that knows that the
	    // two instances are the same subclass to do the copying
	    // properly
	    oldvert->copySame(newvert);
	}
	verts = newverts;
	delete oldverts;
	maxVerts = numVerts;
    }
    
    if (maxTris > numTris)
    {
	mtTriangle *oldtris = tris;
	tris = new mtTriangle[numTris];
	for (int i=0; i<numTris; i++)
	    tris[i] = oldtris[i];
	delete oldtris;
	maxTris = numTris;
    }

    if (maxNodes > numNodes)
    {
	mtNode *oldnodes = nodes;
	nodes = new mtNode[numNodes];
	for (int i=0; i<numNodes; i++)
	    nodes[i] = oldnodes[i];
	delete oldnodes;
	maxNodes = numNodes;
    }

    // check if merging has been done
    if (numArcs == numTris)
	mergeArcs();
    
    if (maxArcs > numArcs)
    {
	mtArc *oldarcs = arcs;
	arcs = new mtArc[numArcs];
	for (int i=0; i<numArcs; i++)
	    arcs[i] = oldarcs[i];
	delete oldarcs;
	maxArcs = numArcs;
    }

    
    // rather than getting access to numParents and numChildren,
    //   just make temporary counters
    int *numParents = new int[numNodes];
    int *numChildren = new int[numNodes];
    for (int i=0; i<numNodes; i++)
	numParents[i] = numChildren[i] = 0;

    // count parents and children for all nodes    
    for (int arcID=0; arcID<numArcs; arcID++)
    {
	int end = arcs[arcID].getEnd();
	int start = arcs[arcID].getStart();

	if ((start < 0) || (start >= numNodes))
	{
	    fprintf(stderr, "Invalid arc start: %d\n", start);
	    exit(1);
	}
	if ((end < 0) || (end >= numNodes))
	{
	    fprintf(stderr, "Invalid arc end: %d\n", end);
	    exit(1);
	}
	
	numParents[end]++;
	numChildren[start]++;
    }

    // allocate parent and child arrays
    for (int nodeID=0; nodeID<numNodes; nodeID++)
    {
	nodes[nodeID].allocateParents(numParents[nodeID]);
	nodes[nodeID].allocateChildren(numChildren[nodeID]);
    }

    // add parents and children
    for (int arcID=0; arcID<numArcs; arcID++)
    {
	nodes[arcs[arcID].getEnd()].addParentNoAllocate(this, arcID);
	nodes[arcs[arcID].getStart()].addChildNoAllocate(this, arcID);
    }

    // free up temporary counters
    free(numParents);
    free(numChildren);
}

void
MT::removeUnusedTris()
{
    // mark which tris are used in some arc
    char *used = new char[numTris];
    for (int i=0; i<numTris; i++)
	used[i] = 0;

    for (int i=0; i<numArcs; i++)
    {
	mtArc *arc = &arcs[i];
	for (int j=0; j<arc->getNumTris(); j++)
	    used[arc->getTri(j)] = 1;
    }

    // compact tris and create mapping from old ids to new
    int *map = new int[numTris];
    for (int i=0; i<numTris; i++)
	map[i] = -1;
    
    int current = 0;
    for (int i=0; i<numTris; i++)
    {
	if (used[i] == 1)
	{
	    tris[current] = tris[i];
	    map[i] = current;
	    current++;
	}
    }
    numTris = current;

    // finally, map the triangle ids in all arcs
    for (int i=0; i<numArcs; i++)
    {
	mtArc *arc = &arcs[i];
	for (int j=0; j<arc->getNumTris(); j++)
	    arc->setTri(j, map[arc->getTri(j)]);
    }
}

void
MT::removeUnusedVerts()
{
    // mark which tris are used in some arc
    char *used = new char[numVerts];
    for (int i=0; i<numVerts; i++)
	used[i] = 0;

    for (int i=0; i<numTris; i++)
    {
	mtTriangle *tri = &tris[i];
	for (int j=0; j<3; j++)
	    used[tri->verts[j]] = 1;
    }

    // compact verts and create mapping from old ids to new
    int *map = new int[numTris];
    for (int i=0; i<numTris; i++)
	map[i] = -1;
    
    int current = 0;
    for (int i=0; i<numVerts; i++)
    {
	if (used[i] == 1)
	{
	    getVert(i)->copySame(getVert(current));
	    map[i] = current;
	    current++;
	}
    }
    numVerts = current;

    // finally, map the vertex ids in all triangles
    for (int i=0; i<numTris; i++)
    {
	mtTriangle *tri = &tris[i];
	for (int j=0; j<3; j++)
	    tri->verts[j] = map[tri->verts[j]];
    }

    delete used;
    delete map;
}

void
MT::removeEmptyArcs()
{
    // mark which arcs are used (not empty)
    char *used = new char[numArcs];
    for (int i=0; i<numArcs; i++)
    {
        mtArc *arc = getArc(i);
	if ((arc->getNumTris() == 0) && (arc->getNumPoints() == 0))
	    used[i] = 0;
	else
	    used[i] = 1;
    }

    // compact arcs and create mapping from old ids to new
    int *map = new int[numArcs];
    for (int i=0; i<numArcs; i++)
	map[i] = -1;
    
    int current = 0;
    for (int i=0; i<numArcs; i++)
    {
	if (used[i] == 1)
	{
	    arcs[current] = arcs[i];
	    map[i] = current;
	    current++;
	}
    }
    numArcs = current;

    // finally, map the arc ids in all nodes
    for (int i=0; i<numNodes; i++)
    {
	mtNode *node = &nodes[i];

	// make new parent list
	int *parentIDs = new int[node->getNumParents()];
	int numParents = 0;
	for (int j=0; j<node->getNumParents(); j++)
	    if (used[node->getParent(j)] == 1)
	        parentIDs[numParents++] = map[node->getParent(j)];

	node->clearParents();
	node->allocateParents(numParents);
	for (int j=0; j<numParents; j++)
	    node->addParentNoAllocate(this, parentIDs[j]);
	delete parentIDs;
	parentIDs = NULL;

	// make new child list
	int *childIDs = new int[node->getNumChildren()];
	int numChildren = 0;
	for (int j=0; j<node->getNumChildren(); j++)
	    if (used[node->getChild(j)] == 1)
	        childIDs[numChildren++] = map[node->getChild(j)];

	node->clearChildren();
	node->allocateChildren(numChildren);
	for (int j=0; j<numChildren; j++)
	    node->addChildNoAllocate(this, childIDs[j]);
	delete childIDs;
	childIDs = NULL;

    }

    delete used;
    delete map;  
}

mtCut *
MT::newErrorCut(mtReal error)
{
    currentCut.newErrorCut(this, error);
    return &currentCut;
}

void
mtArc::addTri(int tri)
{
    addTris(&tri, 1);
}

void
mtArc::addTris(int *newTris, int num)
{
    int *oldtris = tris;
    tris = new int[numTris+num];
    for (int i=0; i<numTris; i++)
	tris[i] = oldtris[i];
    for (int i=0; i<num; i++)
	tris[numTris++] = newTris[i];
    delete oldtris;
}

void
mtArc::addPoints(int *newPoints, int num)
{
    int *oldpoints = points;
    points = new int[numPoints+num];
    for (int i=0; i<numPoints; i++)
	points[i] = oldpoints[i];
    for (int i=0; i<num; i++)
	points[numPoints++] = newPoints[i];
    delete oldpoints;
}


mtReal
mtArc::getError(MT *mt, mtCut *cut, float *ret_d)
{
   mtReal error = mt->getNode(end)->getError();
   mtVec3 v = center - cut->getView()->eye;
   mtReal d;

    if(cut->errorMode == SCRERROR)
    {
        d = v.dot(cut->getView()->viewd);
        if(d < -radius)
        { // All of the object must be clipped
	    d = MAXFLOAT;
        } else
            d -= radius;
        if(d < .01)
        { // Too close. Just say the full object is at hither.
	    d = .01f; // Close to hither. Should we maintain hither?
        }
        d = cut->getView()->zp/d;
    } else {
#if 1
      d = 1;
#else
#define ARBITRARY  1.0
      d = ARBITRARY/v.length();
#endif
    }

    if (ret_d) *ret_d = d;

    if(cut->dumpMode==2)
        printf("[ArcError:%f, Scale:%f (Eff:%f)]", error, d, error*d);
    return error*d;
}

void
mtNode::allocateParents(int numPar)
{
    if (parents != NULL)
    {
	delete parents;
	numParents = 0;
    }
    
    if (numPar > 0)
	parents = new int [numPar];
    return;
}

void
mtNode::allocateChildren(int numChil)
{
    if (children != NULL)
    {
	delete children;
	numChildren = 0;
    }

    if (numChil > 0)
	children = new int [numChil];
    return;
}

void
mtNode::addParentNoAllocate(MT *mt, int arc)
{
    parents[numParents++] = arc;
}

void
mtNode::addChildNoAllocate(MT *mt, int arc)
{
    children[numChildren++] = arc;
}

void
mtNode::addParent(MT *mt, int arc)
{
    int *oldparents = parents;
    parents = new int [numParents+1];
    
    for (int i=0; i<numParents; i++)
	parents[i] = oldparents[i];
    delete oldparents;
    
    parents[numParents++] = arc;
}

void
mtNode::addChild(MT *mt, int arc)
{
    int *oldchildren = children;
    children = new int [numChildren+1];
    
    for (int i=0; i<numChildren; i++)
	children[i] = oldchildren[i];
    delete oldchildren;
    
    children[numChildren++] = arc;
}

void
mtCut::addArc(MT *mt, int arcID, float d)
{
    // printf("Add Arc %d.. \n", arcID);
    if (maxArcs == 0)
    {
	arcs   = new int;
	depths = new float;
	maxArcs = 1;
    }
    else if (numArcs == maxArcs)
    {
	int *oldarcs = arcs;
	float *olddepths = depths;
	arcs = new int[maxArcs*2];
	depths = new float[maxArcs*2];
	for (int i=0; i<numArcs; i++) {
	    arcs[i] = oldarcs[i];
	    depths[i] = olddepths[i];
        }
	delete oldarcs;
	delete olddepths;
	maxArcs *= 2;
    }
    arcs[numArcs] = arcID;
    depths[numArcs] = d;
    numArcs++;
}

/*****************************************************************************\
 @ mtCut::raiseNode 
 -----------------------------------------------------------------------------
 description : Lift a node above the cut
 input       : 
 output      : 
 notes       : We know coming into this that one of the node's ancestors
               has too much error, requiring this lift.
\*****************************************************************************/
void
mtCut::raiseNode(MT *mt, int nodeID, mtReal error)
{
    float d;

    // check if node is already visited
    if (nodeAboveCut[nodeID] != 0)
	return;

    stat.gNodeStat ++;
    // printf("Really visiting node %d\n", nodeID);

    mtNode *node = mt->getNode(nodeID);

    // mark node visited
    nodeAboveCut[nodeID] = 1;
    
    // visit ancestor nodes first to make sure they are all lifted above
    // the cut as well
    for (int i=0; i<node->getNumParents(); i++)
    {
	mtArc *arc = mt->getArc(node->getParent(i));
	int parentID = arc->getStart();
	raiseNode(mt, parentID, error);
    }
    
    // add node's child arcs
    //   visit descendants if they do not meet error bound
    for (int i=0; i<node->getNumChildren(); i++)
    {
	int arcID = node->getChild(i);
	mtArc *arc = mt->getArc(arcID);
	if (arc->getError(mt, this, &d) > error)
	{
	    if(dumpMode==2) printf("ERROR: Visit Down .. \n");
	    raiseNode(mt, arc->getEnd(), error);
	}
	else
	{
	    if(dumpMode==2) printf("ARC OK: Add Arc .. \n");
	    addArc(mt, arcID, d);
	}
    }
    
    return;
} /** End of mtCut::raiseNode() **/

/*****************************************************************************\
 @ mtCut::lowerNode
 -----------------------------------------------------------------------------
 description : Attempt to move a node below the cut. This may not be
               possible if some dependency prevents it.
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
mtCut::lowerNode(MT *mt, int nodeID, mtReal error)
{
    float *d;

    // check if node is already below the cut
    if (nodeAboveCut[nodeID] == 0)
	return;

    stat.gNodeStat ++;

    mtNode *node = mt->getNode(nodeID);

    // Verify that all the child nodes are already below the cut
    int i = node->getNumChildren();
    while(i--)
    {
	int arcID = node->getChild(i);
	mtArc *arc = mt->getArc(arcID);
        if(nodeAboveCut[arc->getEnd()] != 0)
        {
            return; // Found an active child. Cannot move up.
        }
    }

    i = node->getNumParents();
    
    if (i == 0)
	return; // Am at the root. Cannot move up.

    // Verify that the error of all the parent arcs is small enough to be
    // placed on the cut
    d = new float[i];
    while(i--)
    {
	int arcID = node->getParent(i);
	mtArc *arc = mt->getArc(arcID);
        if (arc->getError(mt, this, &d[i]) > error)
        {
	    if(dumpMode==2) printf("ERROR: Cannot raise cut ..\n");
            delete d;
	    return; // Some incoming arc has too much error
        } else { if(dumpMode==2) printf("ARC OK: Raise cut now ..\n"); }
    }

    // printf("I shouldn't actually need to unvisit anyone ..\n");

    nodeAboveCut[nodeID] = 0;

    // Actually add the node's parent arcs to the cut, then attempt to
    // lower the ancestor nodes below the cut as well
    i = node->getNumParents();
    while(i--)
    {
        int arcID = node->getParent(i);
        addArc(mt, arcID, d[i]);
	lowerNode(mt, (mt->getArc(arcID))->getStart(), error);
    }

    delete d;

    return;
} /** End of mtCut::lowerNode() **/

/*****************************************************************************\
 @ mtCut::cleanArcList
 -----------------------------------------------------------------------------
 description : Remove all arcs that do not belong on the cut (they were
               placed there temporarily, but now their start and end nodes
	       lie on the same side of the cut)
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
mtCut::cleanArcList(MT *mt)
{
    int next, current;
    
    for (next=current=0; next<numArcs; next++)
    {
	if ((nodeAboveCut[mt->getArc(arcs[next])->getStart()] !=
	     nodeAboveCut[mt->getArc(arcs[next])->getEnd()]) &&
            (mt->getArc(arcs[next])->getNumTris() > 0 ||
             mt->getArc(arcs[next])->getNumPoints() > 0))

        {
	    arcs[current] = arcs[next];
	    depths[current] = depths[next];
            current++;
        }
    }
    numArcs = current;
} /** End of mtCut::cleanArcList() **/


/*****************************************************************************\
 @ mtCut::newErrorCut
 -----------------------------------------------------------------------------
 description : Create a new cut from an MT by starting at the root and
               lowering the cut as far as possible within the given error
	       bound.
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
mtCut::newErrorCut(MT *mt, mtReal error)
{
    if (nodeAboveCut == NULL)
    {
	nodeAboveCut = new char[mt->getNumNodes()];
	memset(nodeAboveCut, 0, mt->getNumNodes()*sizeof(char));
    }
    
    numArcs = 0;
    
    raiseNode(mt, mt->getRoot(), error);
    cleanArcList(mt);
} /** End of mtCut::newErrorCut() **/



/*****************************************************************************\
 @ mtCut::lowerErrorCut
 -----------------------------------------------------------------------------
 description : Push the cut down wherever necessary, refining the
               tesselation to meet the error bound.
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
mtCut::lowerErrorCut(MT *mt, mtReal error)
{
    int lnumArcs = numArcs;

    while(lnumArcs--)
    {
        // This can result in visiting a node several times, but they will
	// return immediately.
	
	mtArc *arcptr = mt->getArc(arcs[lnumArcs]);
        if (arcptr->getError(mt, this) > error)
        {
	   if(dumpMode==2) printf("ERROR: Lower ..\n");
           raiseNode(mt, arcptr->getEnd(), error);
        }
	else { if(dumpMode==2) printf("ARC OK: Don't lower..\n"); }
    }

    cleanArcList(mt);
} /** End of mtCut::lowerErrorCut() **/


/*****************************************************************************\
 @ mtCut::raiseErrorCut
 -----------------------------------------------------------------------------
 description : Raise the cut wherever possible without violating the error
               bound, coarsening the tesselation.
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
mtCut::raiseErrorCut(MT *mt, mtReal error)
{
    
    int lnumArcs = numArcs;

    while(lnumArcs--)
    {
        // This can result in visiting a node several times.
 	// Need to fix that.
	int parentID = (mt->getArc(arcs[lnumArcs]))->getStart();
	
        // printf(" Arc %d: Trying to raise at Start node %d\n",
	//         lnumArcs, parentID);
	
        lowerNode(mt, parentID, error);
    }

    cleanArcList(mt);
} /** End of mtCut::raiseErrorCut() **/


/*****************************************************************************\
 @ mtCut::adaptErrorCut
 -----------------------------------------------------------------------------
 description : Adapt an existing cut to a new error bound. When the error
               is changing coherently, this is pretty fast.
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
mtCut::adaptErrorCut(MT *mt, mtReal error)
{
    stat.start.getTime();
    stat.gNodeStat = 0;
    
    raiseErrorCut(mt, error);
    lowerErrorCut(mt, error);
    
    stat.gTimeUpdate = stat.start.since();
    
    return;
} /** End of mtCut::adaptErrorCut() **/


int
mtCut::getNumTris(MT *mt) const
{
    int numTris = 0;
    
    for (int i=0; i<numArcs; i++)
	numTris += mt->getArc(arcs[i])->getNumTris();
    return numTris;
}

unsigned int
mtArc::getdlistID(MT *mt) const
{
    return mt->arcIndex(*this) + mt->getDlistBase();
};

int
mtBVNode::getNumTris(MT *mt)
{
    int numTris = 0;
    
    mtNode *node = mt->getNode(mt->bvnodeIndex(*this));
    for (int i=0; i<node->getNumChildren(); i++)
	numTris += mt->getArc(node->getChild(i))->getNumTris();
    return numTris;
}
    
int
mtBVNode::getTri(MT *mt, int index)
{
    int triIndex;
    
    mtNode *node = mt->getNode(mt->bvnodeIndex(*this));
    int trinum=0;
    for (int i=0; i<node->getNumChildren(); i++)
    {
	mtArc *arc = mt->getArc(node->getChild(i));
	if ((arc->getNumTris() + trinum) > index)
	{
	    triIndex = arc->getTri(index - trinum);
	    return triIndex;
	}
	trinum += arc->getNumTris();
    }
    fprintf(stderr, "mtBVNode::getTri(): invalid index\n");
    exit(1);
}
    
int
mtBVNode::getNumChildren(MT *mt)
{
    return mt->getNode(mt->bvnodeIndex(*this))->getNumChildren();
}
    
int
mtBVNode::getChild(MT *mt, int index)
{
    return mt->getArc(mt->getNode(mt->bvnodeIndex(*this))->
		       getChild(index))->getEnd();
}

void MT::calcMaxHeight(int nodeID, int *nodeMax)
{
    mtNode *node = &(nodes[nodeID]);

    nodeMax[nodeID] = 0;

    for (int i=0; i<node->getNumChildren(); i++)
    {
	int arcID = node->getChild(i);
	mtArc *arc = &(arcs[arcID]);
	int childID = arc->getEnd();
	
	if (nodeMax[childID] == -1)
	    calcMaxHeight(childID, nodeMax);
	
	if (nodeMax[nodeID] < (nodeMax[childID] + 1))
	    nodeMax[nodeID] = nodeMax[childID] + 1;
    }
    
    return;
}

int MT::calcMaxHeight()
{
    int *nodeMax = new int[numNodes];
    for (int i=0; i<numNodes; i++)
	nodeMax[i] = -1;
    
    calcMaxHeight(root, nodeMax);

    // verify that all nodes were reached
    for (int i=0; i<numNodes; i++)
	if (nodeMax[i] == -1)
	    fprintf(stderr,
		    "calcMaxHeight did not reach node %d\n",i);
    
    int maxHeight = nodeMax[root];
    delete nodeMax;

    return maxHeight;
}

void MT::printStats()
{
    // number of various elements
    fprintf(stderr, "Verts:   %d\n", numVerts);
    fprintf(stderr, "Tris:    %d\n", numTris);
    fprintf(stderr, "Arcs:    %d\n", numArcs);
    fprintf(stderr, "Nodes:   %d\n", numNodes);
//  fprintf(stderr, "BVNodes: %d\n", numNodes);
    fprintf(stderr, "\n");
    
    // average tris/arc
    fprintf(stderr, "Average tris per arc: %f\n",
	    (mtReal)numTris/(mtReal)numArcs);

    // average tris/arc, not counting root or sink
    int nonRootSinkTris=0;
    int nonRootSinkArcs=0;
    for (int i=0; i<numArcs; i++)
    {
	mtArc *arc = &arcs[i];
	if ((arc->getStart() == root) || (arc->getEnd() == 0))
	    continue;
	nonRootSinkArcs++;
	nonRootSinkTris += arc->getNumTris();
    }
    fprintf(stderr, "Average tris per non-root, non-sink arc: %f\n",
	    (mtReal)nonRootSinkTris/(mtReal)nonRootSinkArcs);
    
    // average numChildren (and numParents)
    //   each arc appears as one child and one parent
    fprintf(stderr, "Average parents/children per node: %f\n",
	    (mtReal)numArcs/(mtReal)numNodes);

    // average parents and children arcs per node,
    //   not counting arcs connected to root or sink
    int nonRootSinkNodes = numNodes - 2;
    int nonRootSinkParents = 0;
    int nonRootSinkChildren = 0;
    for (int i=0; i<numNodes; i++)
    {
	mtNode *node = &nodes[i];
	if ((i == 0) || (i == root))
	    continue;
	nonRootSinkParents += node->getNumParents();
	nonRootSinkChildren += node->getNumChildren();
    }
    fprintf(stderr,
	    "Average parents per non-root, non-sink node: %f\n",
	    (mtReal)nonRootSinkParents/(mtReal)nonRootSinkNodes);
    fprintf(stderr,
	    "Average children per non-root, non-sink node: %f\n",
	    (mtReal)nonRootSinkChildren/(mtReal)nonRootSinkNodes);
    

    // maximum height
    int height = calcMaxHeight();
    fprintf(stderr, "Maximum height of dag: %d\n", height);
}

void MT::buildBVH()
{
    // one mtBVNode for each mtNode, with corresponding indices
    bvnodes = new mtBVNode[numNodes];
    for (int i=0; i<numNodes; i++)
	bvnodes[i].constructBV();
}

void
MT::allocateVerts(int num, mtVertex &sampleVert)
{
    if (verts != NULL)
    {
	delete verts;
	numVerts = 0;
	maxVerts = 0;
    }

    if (num <= 0)
	return;
    
    verts = sampleVert.makeNew(num);
    maxVerts = num;
    return;
}

void
MT::allocateTris(int num)
{
    if (tris != NULL)
    {
	delete tris;
	numTris = 0;
	maxTris = 0;
    }

    if (num <= 0)
	return;
    
    tris = new mtTriangle[num];
    maxTris = num;
    return;
}

void
MT::allocateNodes(int num)
{
    if (nodes != NULL)
    {
	delete nodes;
	numNodes = 0;
	maxNodes = 0;
    }

    if (num <= 0)
	return;
    
    nodes = new mtNode[num];
    maxNodes = num;
    return;
}

void
MT::allocatePoints(int num)
{
    if (points != NULL)
    {
	delete points;
	numNodes = 0;
	maxNodes = 0;
    }

    if (num <= 0)
	return;
    
    points = new mtPoint[num];
    maxPoints = num;
    return;
}

void
MT::allocateArcs(int num)
{
    if (arcs != NULL)
    {
	delete arcs;
	numArcs = 0;
	maxArcs = 0;
    }

    if (num <= 0)
	return;
    
    arcs = new mtArc[num];
    maxArcs = num;
    return;
}


void MT::computeNORM()
{
   int lnumTri = numTris;
   mtTriangle *ltris = tris;
   int *vertIds;

   while(lnumTri--)
   { // We know tris will not change midway somewhere
      vertIds = ltris->verts;
      mtVec3 v1 = getVert(vertIds[0])->coord -
	  getVert(vertIds[1])->coord;
      mtVec3 v2 = getVert(vertIds[1])->coord -
	  getVert(vertIds[2])->coord;
      ltris->setNormal((v1^v2).normalize());
      ltris++;
   }
}
