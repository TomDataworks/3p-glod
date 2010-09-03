/*****************************************************************************\
  Operation.C
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/Operation.C,v $
  $Revision: 1.33 $
  $Date: 2004/10/20 19:33:34 $
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

#include <xbs.h>
#include <Point.h>
#include <glod_error.h>

/*----------------------------- Local Constants -----------------------------*/

#if 0
#define FILTER_ALL_COINCIDENT_TRIANGLES
#endif

#define MARGIN_THRESHOLD 0.1
#define POINT_THRESHOLD 15000

/*------------------------------ Local Macros -------------------------------*/


/*------------------------------- Local Types -------------------------------*/

typedef xbsVertex *vertduple[2];

/*------------------------------ Local Globals ------------------------------*/

static xbsVertex *compare_ops_source_vert = NULL;
static xbsVertex *compare_ops_destination_vert = NULL;

/*------------------------ Local Function Prototypes ------------------------*/

#if 0
static int
compare_ints (const void *a, const void *b);
#endif

static int
compare_ops (const void *a, const void *b);

extern int
compare_tri_end_nodes(const void *a, const void *b);

void TestVdata(Model *model);

/*---------------------------------Functions-------------------------------- */

#if 0
/*****************************************************************************\
 @ Operation::computeSampleCost
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
float
Operation::computeSampleCost(Model *model)
{
    MT *mt = model->mt;
    
    // consumed triangles are just source vertex's tris

    // generated triangles are the subset of the consumed triangles that
    // are not destroyed (and replacing source_vert with destination_vert
    
    // Conservative allocation rather than counting first
    mtTriangle *generatedTris =
        new mtTriangle[model->vdata[source_vert].numTris];
    int numGeneratedTris = 0;
    
    for (int i=0; i<model->vdata[source_vert].numTris; i++)
    {
        int triIndex = model->vdata[source_vert].tris[i];
        mtTriangle *tri = mt->getTri(triIndex);
        
        if (!(((tri->verts[0] == destination_vert) ||
               (tri->verts[1] == destination_vert) ||
               (tri->verts[2] == destination_vert))))
        {
            mtTriangle *genTri = &(generatedTris[numGeneratedTris++]);
            genTri->verts[0] =
                ((tri->verts[0] == source_vert) ?
                 destination_vert : tri->verts[0]);
            genTri->verts[1] =
                ((tri->verts[1] == source_vert) ?
                 destination_vert : tri->verts[1]);
            genTri->verts[2] =
                ((tri->verts[2] == source_vert) ?
                 destination_vert : tri->verts[2]);
        }
    }

    // Compute initial sample radius
    mtVertex *vert0 = mt->getVert(generatedTris[0].verts[0]);
    mtVertex *vert1 = mt->getVert(generatedTris[0].verts[1]);
    mtVertex *vert2 = mt->getVert(generatedTris[0].verts[2]);
    mtVec3 center = (vert0->coord + vert1->coord + vert2->coord) * (1.0/3.0);
    float sampleRadius = (center-vert0->coord).length();
    fprintf(stderr, "sampleRadius = %f\n", sampleRadius);
    
    float hausdorff;
    float distanceBound = MAXFLOAT;
    float margin = MAXFLOAT;
    
    PointSet consumedPoints;
    PointSet generatedPoints;
    while ((margin > MARGIN_THRESHOLD) &&
           (consumedPoints.numPoints < POINT_THRESHOLD) &&
           (generatedPoints.numPoints < POINT_THRESHOLD))
    {
        // sample consumed triangles
        consumedPoints.clear();
        for (int i=0; i<model->vdata[source_vert].numTris; i++)
        {
            mtTriangle *tri =
                mt->getTri(model->vdata[source_vert].tris[i]);
            consumedPoints.addSampledTriangle(
                mt->getVert(tri->verts[0])->coord,
                mt->getVert(tri->verts[1])->coord,
                mt->getVert(tri->verts[2])->coord,
                sampleRadius);
        }
        
        // sample generated triangles
        generatedPoints.clear();
        for (int i=0; i<numGeneratedTris; i++)
        {
            mtTriangle *tri = &(generatedTris[i]);
            generatedPoints.addSampledTriangle(
                mt->getVert(tri->verts[0])->coord,
                mt->getVert(tri->verts[1])->coord,
                mt->getVert(tri->verts[2])->coord,
                sampleRadius);
        }

        // compute margin and hausdorff distance
#if 1
        fprintf(stderr, "SampleRadius: %f, Num Points: %d %d, ",
                sampleRadius, consumedPoints.numPoints,
                generatedPoints.numPoints);
#endif
        
        hausdorff = consumedPoints.hausdorff(generatedPoints);
        distanceBound = hausdorff + 2.0 * sampleRadius;
        margin = (2.0*sampleRadius)/ distanceBound;
        sampleRadius /= 2.0;

#if 1
        fprintf(stderr, "Hausdorff: %f, Bound: %f, Margin: %f, \n",
                hausdorff, distanceBound, margin);
#endif
    }
    fprintf(stderr, "Converged.\n");
    return distanceBound;
} /** End of Operation::computeSampleCost() **/
#endif

//float Operation::calculateError(){
//    return error->halfEdgeCollapseError();
//}




/*****************************************************************************\
 @ Operation::computeCost
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
Operation::computeCost(Model *model)
{
    dirty = 0;

#if 1
    ///float length = (source_vert->coord - destination_vert->coord).length();
    
    //cost = MAX(destination_vert->errorRadius,
    //       source_vert->errorRadius + length);
    if (error==NULL){
        if (model->errorMetric==GLOD_METRIC_SPHERES)
            error = new SphereHalfEdgeError();
        else if (model->errorMetric==GLOD_METRIC_QUADRICS)
            error = new QuadricHalfEdgeError();
        else if (model->errorMetric==GLOD_METRIC_PERMISSION_GRID)
            error = new PermissionGridHalfEdgeError();
    }

    int sourceOnBorder = source_vert->onBorder();
    int destOnBorder = destination_vert->onBorder();
    
    if (model->borderLock == 1)
    {
        // This will prevent ANY movement of geometric border vertices
        if (sourceOnBorder == 1)
        {
            //cost = MAXFLOAT;
            error->setError(MAXFLOAT);
            return;
        }
    }
    
//    if (sourceOnBorder && !(destOnBorder))
//        error->setError(MAXFLOAT);
    
    
    // This tends to discourage borders from merging into interiors and
    // corners from merging to borders or interiors, at least in the case
    // of multiple adjacent patches (won't prevent holes in the bunny from
    // creeping in, for example)

    int nc1 = source_vert->numNonEmptyCoincident() + sourceOnBorder;
    int nc2 = destination_vert->numNonEmptyCoincident() + destOnBorder;
    if (nc1 > nc2)
    {
        error->setError(MAXFLOAT);
        return;
    }

    // It turns out the above heuristic is insufficient. It allows to
    // vertices on different patch boundaries to merge because they both
    // touch 2 patches, even though they are not the same 2. For nice,
    // parameterized models, this can pinch a corner triangle into
    // non-existance, messing up the nice connection between patches

    // Count how many coincident source vertices have triangles
    // touching the destination vertex. If it's only 1, this is
    // probably a non-border edge connecting two border vertices, and
    // should be disallowed.
#if 1
    if ((nc1 == nc2) && (nc1 != 1))
    {
        xbsVertex *current = source_vert;
        xbsVertex *v2min = destination_vert->minCoincident();
        do 
        {
            int count = 0;
            for (int i=0; i<current->numTris; i++)
            {
                xbsTriangle *tri = current->tris[i];
                if ((tri->verts[0]->minCoincident() == v2min) ||
                    (tri->verts[1]->minCoincident() == v2min) ||
                    (tri->verts[2]->minCoincident() == v2min))
                    count++;
                if (count > 1)
                {
                    error->setError(MAXFLOAT);
                    return;
                }
            }
            current = current->nextCoincident;
        } while (current != source_vert);
    }
#endif

    error->calculateError(model, this);

#if 0
    // This additional heurisitic looks to see if the edge touches
    // triangles on two different patches. If so, the edge lies along the
    // boundary of a patch and should be okay. If not, this is a bad
    // collapse. There are still lots of unpleasant situations where a
    // single patch model has multi-attribute vertices like sharp edges on
    // a cylinder where we don't want to pinch, and this heuristic will not
    // fix those.
    if ((nc1 == nc2) && (nc1 != 1))
    {
        int patch_num = -1;
        int patchBoundary = 0;
        xbsVertex *current = source_vert;
        xbsVertex *v2min = destination_vert->minCoincident();
        do 
        {
            for (int i=0; i<current->numTris; i++)
            {
                xbsTriangle *tri = current->tris[i];
                if ((tri->verts[0]->minCoincident() == v2min) ||
                    (tri->verts[1]->minCoincident() == v2min) ||
                    (tri->verts[2]->minCoincident() == v2min))
                {
                    if (patch_num == -1)
                        patch_num = tri->patchNum;
                    else if (patch_num != tri->patchNum)
                        patchBoundary = 1;
                }
            }
            current = current->nextCoincident;
        } while ((current != source_vert) && (patchBoundary == 0));

        if (patchBoundary == 0)
            error->setError(MAXFLOAT);
    }
#endif    
    
#else
    cost = computeSampleCost(model);
#endif
    
} /** End of Operation::computeCost() **/


#if 0
/*****************************************************************************\
 @ compare_ints
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
static int
compare_ints (const void *a, const void *b)
{
    const int *ia = (const int *) a;
    const int *ib = (const int *) b;
    
    return (*ia > *ib) - (*ia < *ib);
} /** End of compare_ints() **/
#endif

/*****************************************************************************\
 @ compare_pointers
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
static int
compare_pointers (const void *a, const void *b)
{
    const void **ia = (const void **) a;
    const void **ib = (const void **) b;
    
    return (*ia > *ib) - (*ia < *ib);
} /** End of compare_pointers() **/

/*****************************************************************************\
 @ compare_ops
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
static int
compare_ops (const void *a, const void *b)
{
    const Operation **op_a = (const Operation **) a;
    const Operation **op_b = (const Operation **) b;

    xbsVertex *source_a = (*op_a)->getSource();
    xbsVertex *source_b = (*op_b)->getSource();
    xbsVertex *destination_a = (*op_a)->getDestination();
    xbsVertex *destination_b = (*op_b)->getDestination();

    if (source_a == compare_ops_source_vert)
        source_a = compare_ops_destination_vert;
    if (source_b == compare_ops_source_vert)
        source_b = compare_ops_destination_vert;
    if (destination_a == compare_ops_source_vert)
        destination_a = compare_ops_destination_vert;
    if (destination_b == compare_ops_source_vert)
        destination_b = compare_ops_destination_vert;
    
    int source_compare = (source_a > source_b) - (source_a < source_b);
    
    if (source_compare != 0)
        return source_compare;

    int destination_compare =
        (destination_a > destination_b) -
        (destination_a < destination_b);
        
    return destination_compare;
} /** End of compare_ops() **/



/*****************************************************************************\
 @ TestVdata
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void TestVdata(Model *model)
{
#if 0
    
#if 1
    if (model->getNumTris() > 41400)
        return;
    
#if 0
    model->verify();
#endif
#endif
    // make sure all triangles belong on the verts
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        for (int tnum=0; tnum<vert->numTris; tnum++)
        {
            xbsTriangle *tri = vert->tris[tnum];
            if ((tri->verts[0] != vert) &&
                (tri->verts[1] != vert) &&
                (tri->verts[2] != vert))
            {
                fprintf(stderr, "Bad tri on vert!\n");
                exit(1);
            }
        }
    }
    
    // make sure all ops are on the correct vert
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        for (int opnum=0; opnum<vert->numOps; opnum++)
        {
            if (vert->ops[opnum]->getSource() != vert)
            {
                fprintf(stderr, "Bad op on vert!\n");
                exit(1);
            }
            xbsVertex *src  = vert->ops[opnum]->getSource();
            xbsVertex *dest = vert->ops[opnum]->getDestination();
            xbsVertex *currentCoincident = src;
            int found = 0;
            do
            {
                for (int tnum=0; tnum<currentCoincident->numTris; tnum++)
                {
                    xbsTriangle *tri = currentCoincident->tris[tnum];
                    if ((tri->verts[0]->minCoincident() == dest) ||
                        (tri->verts[1]->minCoincident() == dest) ||
                        (tri->verts[2]->minCoincident() == dest))
                        found = 1;
                }
                
                currentCoincident = currentCoincident->nextCoincident;
            } while (currentCoincident != src);
            if (found == 0)
            {
                fprintf(stderr, "Op on vert has no corresponding tri\n");
            }
        }
    }

#if 0
    // make sure all verts have tris
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        if (vert->numTris == 0)
        {
            fprintf(stderr, "vertex has no tris!\n");
        }
    }
#endif
    
    // make sure no verts have NULL pointers in their coincident ring
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        int vtnum = vert->coincidentNumTris();
    }
    
#endif
    return;
} /** End of TestVdata() **/


/*****************************************************************************\
 @ Operation::duplicatedTriangle
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
int
Operation::duplicatedTriangle(xbsTriangle *tri)
{
    xbsVertex *currentDestination = destination_vert;
    do 
    {
        for (int tnum=0; tnum<currentDestination->numTris; tnum++)
        {
            xbsTriangle *dtri = currentDestination->tris[tnum];
            
            if ((dtri->verts[0]->minCoincident() == source_vert) ||
                (dtri->verts[1]->minCoincident() == source_vert) ||
                (dtri->verts[2]->minCoincident() == source_vert))
                continue;
            
            for (int vnum1=0; vnum1<3; vnum1++)
                for (int vnum2=0; vnum2<3; vnum2++)
#ifndef FILTER_ALL_COINCIDENT_TRIANGLES
                    // Only filter out duplicate triangles with the same
                    // orientation (so backface culling still works for closed
                    // models)
                    if ((tri->verts[vnum1]->minCoincident() ==
                         dtri->verts[vnum2]->minCoincident()) &&
                        (tri->verts[(vnum1+1)%3]->minCoincident() ==
                         dtri->verts[(vnum2+1)%3]->minCoincident()))
#else
                        // Filter out duplicate triangles even if the
                        // orientations are reversed. This is more
                        // agressive, removing more triangles, but the
                        // resulting models should not be rendered with
                        // backface culling enabled.
                        if (((tri->verts[vnum1]->minCoincident() ==
                              dtri->verts[vnum2]->minCoincident()) &&
                             (tri->verts[(vnum1+1)%3]->minCoincident() ==
                              dtri->verts[(vnum2+1)%3]->minCoincident())) ||
                            ((tri->verts[vnum1]->minCoincident() ==
                              dtri->verts[vnum2]->minCoincident()) &&
                             (tri->verts[(vnum1+1)%3]->minCoincident() ==
                              dtri->verts[(vnum2+2)%3]->minCoincident())))
#endif
                        {
                            //fprintf(stderr, "duplicate tri!\n");
                            return 1;
                        }
        }
        currentDestination = currentDestination->nextCoincident;
    } while (currentDestination != destination_vert);
    
    return 0;
    
} /** End of Operation::duplicatedTriangle() **/

/*****************************************************************************\
 @ Operation::initQueue
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : keep only 2 half edge collapses between each pair of
               coincident vertex sets
\*****************************************************************************/
void
Operation::initQueue(Model *model, SimpQueue *queue)
{
    // for each vertex, generate a list of operations, then insert them
    // onto the queue

    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        
        if (vert != vert->minCoincident())
            continue;

        if ((model->errorMetric==GLOD_METRIC_SPHERES) && (vert->errorData==NULL))
            vert->errorData = new SphereErrorData(vert);
        if ((model->errorMetric==GLOD_METRIC_QUADRICS) && (vert->errorData==NULL))
            vert->errorData = new QuadricErrorData(vert);
        if ((model->errorMetric==GLOD_METRIC_PERMISSION_GRID) && (vert->errorData==NULL))
            vert->errorData = new PermissionGridErrorData(vert);
        
        // generate list of neighboring vertices (with possible
        // redundancies)

        int numNeighborVerts = vert->coincidentNumTris() * 2;
        xbsVertex **neighborVerts = new xbsVertex *[numNeighborVerts];
        int maxNeighborVerts = numNeighborVerts;
        
        numNeighborVerts = 0;
        xbsVertex *currentCoincident = vert;
        do {
            for (int tnum=0; tnum<currentCoincident->numTris; tnum++)
            {
                xbsTriangle *tri = currentCoincident->tris[tnum];
                
                for (int tvnum=0; tvnum<3; tvnum++)
                    if (tri->verts[tvnum] != currentCoincident)
                        neighborVerts[numNeighborVerts++] =
                            tri->verts[tvnum]->minCoincident();
            }
            currentCoincident = currentCoincident->nextCoincident;
        } while (currentCoincident != vert);
        if (model->errorMetric==GLOD_METRIC_SPHERES){
            for (int i=0; i<numNeighborVerts; i++)
                if (neighborVerts[i]->errorData==NULL)
                    neighborVerts[i]->errorData = new SphereErrorData(neighborVerts[i]);
        }
        else if (model->errorMetric==GLOD_METRIC_QUADRICS) {
            for (int i=0; i<numNeighborVerts; i++)
                if (neighborVerts[i]->errorData==NULL)
                    neighborVerts[i]->errorData = new QuadricErrorData(neighborVerts[i]);
        }
        else if (model->errorMetric==GLOD_METRIC_PERMISSION_GRID) {
            for (int i=0; i<numNeighborVerts; i++)
                if (neighborVerts[i]->errorData==NULL)
                    neighborVerts[i]->errorData = new PermissionGridErrorData(neighborVerts[i]);
        }
        
        if (numNeighborVerts != maxNeighborVerts)
        {
            fprintf(stderr, "Bad numNeighborVerts!\n");
            exit(1);
        }
        
        // sort the neighbor list
        qsort(neighborVerts, numNeighborVerts, sizeof(xbsVertex *),
              compare_pointers);
        
        // compact neighbor list (removing redundancies)
        if (numNeighborVerts > 0)
        {
            int current=0;
            for (int i=1; i<numNeighborVerts; i++)
                if (neighborVerts[i] != neighborVerts[current])
                    neighborVerts[++current] = neighborVerts[i];
            numNeighborVerts = current+1;
        }
        
        vert->ops = new Operation *[numNeighborVerts];
        vert->numOps = 0;

        // should call some function of the error metric to initialize
        // error-specific vertex data
        vert->errorData->init(vert);
        
        // generate operations and insert onto the queue
        for (int opnum=0; opnum<numNeighborVerts; opnum++)
        {
            
            Operation *op = new Operation;
            op->source_vert = vert;
            op->destination_vert = neighborVerts[opnum];
            op->computeCost(model);
            
            queue->insert(op);

            // for the half edge collapse operation, store with each vertex
            // a list of the operations that involve pulling that vertex
            // (modifying or deleting all its neighbor triangles)
            vert->ops[vert->numOps++] = op;
        }

        delete [] neighborVerts;
        neighborVerts = NULL;
        numNeighborVerts = 0;
    }

    return;
} /** End of Operation::initQueue() **/

/*****************************************************************************\
 @ Operation::apply
 ------------------------------------------=-----------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
Operation::apply(Model *model, Hierarchy *hierarchy, SimpQueue *queue)
{
    Operation **addOps, **removeOps, **modOps;
    int numAddOps, numRemoveOps, numModOps;

    if ((source_vert == NULL) || (destination_vert == NULL))
    {
        fprintf(stderr, "NULL vert on op: abort\n");
        return;
    }
    if ((source_vert->index == -1) || (destination_vert->index == -1))
    {
        fprintf(stderr, "non-model vertex on op: abort\n");
        return;
    }
    
//    fprintf(stderr, "Collapsing %d->%d\n",
//          source_vert->index, destination_vert->index);

    TestVdata(model);
    
#if 0
    // (SLOW) temporary test
    // does any vertex have a deleted operation on it?
    for (int i=0; i<model->getNumVerts(); i++)
    {
        xbsVertex *vert = model->getVert(i);
        for (int j=0; j<vert->numOps; j++)
        {
            if (vert->ops[j]->source_vert != vert)
            {
                fprintf(stderr, "BEFORE APPLY: Bad op on vert!\n");
            }
            if (vert->ops[j]->destination_vert->index == -1)
            {
                fprintf(stderr, "BEFORE APPLY: Deleted op on vert!\n");
            }
        }
        
        if (vert->numTris == 0)
            fprintf(stderr, "BEFORE APPLY: No tris on vert!\n");
        if ((vert->minCoincident() == vert) && (vert->numOps == 0))
            fprintf(stderr, "BEFORE APPLY: No ops on min vert\n");
    }
#endif
    
    
    getNeighborOps(model,
                   &addOps, &numAddOps,
                   &removeOps, &numRemoveOps,
                   &modOps, &numModOps);
    
    // temporary test
    TestVdata(model);

    updateModel(model, hierarchy,
                &addOps, &numAddOps,
                &removeOps, &numRemoveOps,
                &modOps, &numModOps);
    
    // temporary test
    TestVdata(model);

    queue->update(model,
                  addOps, numAddOps,
                  removeOps, numRemoveOps,
                  modOps, numModOps);

    // temporary test
    TestVdata(model);

    
    for (int i=0; i<numRemoveOps; i++)
    {
        // careful -- don't delete "this"
        if (removeOps[i] != this)
            delete removeOps[i];
    }
    
    delete [] addOps;
    addOps = NULL;
    numAddOps = 0;

    delete [] removeOps;
    removeOps = NULL;
    numRemoveOps = 0;

    delete [] modOps;
    modOps = NULL;
    numModOps = 0;

#if 0
    // (SLOW) temporary test
    // does any vertex have a deleted operation on it?
    for (int i=0; i<model->getNumVerts(); i++)
    {
        xbsVertex *vert = model->getVert(i);
        for (int j=0; j<vert->numOps; j++)
        {
            if (vert->ops[j]->source_vert != vert)
            {
                fprintf(stderr, "AFTEr APPLY: Bad op on vert!\n");
            }
            if (vert->ops[j]->destination_vert->index == -1)
            {
                fprintf(stderr, "AFTER APPLY: Deleted op on vert!\n");
            }
        }
        if (vert->numTris == 0)
            fprintf(stderr, "AFTER APPLY: No tris on vert!\n");
        if ((vert->minCoincident() == vert) && (vert->numOps == 0))
            fprintf(stderr, "AFTER APPLY: No ops on min vert\n");
    }
#endif
    
#if 0
    // (SLOW) temporary test
    // does every triangle appear on its three vertices?
    if (model->getNumTris() < 3000)
    {
        fprintf(stderr, "Tris: %d\n", model->getNumTris());
        
        for (int i=0; i<model->getNumTris(); i++)
        {
            xbsTriangle *tri = model->getTri(i);
            for (int j=0; j<3; j++)
            {
                xbsVertex *vert = tri->verts[j];
                int found = 0;
                for (int k=0; k<vert->numTris; k++)
                {
                    if (vert->tris[k] == tri)
                        found = 1;
                }
                if (found == 0)
                {
                    fprintf(stderr, "Tri not on vert!\n");
                    exit(1);
                }
            }
        }
    }
#endif
    
    return;
} /** End of Operation::apply() **/

/*****************************************************************************\
 @ Operation::getNeighborOps
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
Operation::getNeighborOps(Model *model,
                          Operation ***addOps, int *numAddOps,
                          Operation ***removeOps, int *numRemoveOps,
                          Operation ***modOps, int *numModOps)
{
    // For the half edge collapse operation and an error stored with the
    // destination vertex, the operations which are affected are those
    // include this operation's source or destination vertex as the source
    // or destination of the operation in question.
    
    int maxAffectedVerts = source_vert->coincidentNumTris() * 3 +
        destination_vert->coincidentNumTris() * 3;
    
    xbsVertex **affectedVerts = new xbsVertex *[maxAffectedVerts];
    int numAffectedVerts = 0;

    if (source_vert->minCoincident() != source_vert)
    {
        fprintf(stderr, "source_vert is not a minCoincident vertex\n");
        exit(1);
    }
    if (destination_vert->minCoincident() != destination_vert)
    {
        fprintf(stderr, "destination_vert is not a minCoincident vertex\n");
        exit(1);
    }

    xbsVertex *currentCoincident = source_vert;
    do
    {
        for (int i=0; i<currentCoincident->numTris; i++)
        {
            xbsTriangle *tri = currentCoincident->tris[i];
            affectedVerts[numAffectedVerts++] = tri->verts[0]->minCoincident();
            affectedVerts[numAffectedVerts++] = tri->verts[1]->minCoincident();
            affectedVerts[numAffectedVerts++] = tri->verts[2]->minCoincident();
        }
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != source_vert);
    
    currentCoincident = destination_vert;
    do
    {
        for (int i=0; i<currentCoincident->numTris; i++)
        {
            xbsTriangle *tri = currentCoincident->tris[i];
            affectedVerts[numAffectedVerts++] = tri->verts[0]->minCoincident();
            affectedVerts[numAffectedVerts++] = tri->verts[1]->minCoincident();
            affectedVerts[numAffectedVerts++] = tri->verts[2]->minCoincident();
        }
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != destination_vert);
    
    
    // sort and remove duplicates
    qsort(affectedVerts, numAffectedVerts, sizeof(xbsVertex *),
          compare_pointers);

    int current = 0;
    for (int i=1; i<numAffectedVerts; i++)
    {
        if (affectedVerts[i] != affectedVerts[current])
            affectedVerts[++current] = affectedVerts[i];
    }
    numAffectedVerts = current + 1;


    // Create list of affected operations

    // count, allocate, and fill
    int numAffectedOps = 0;
    for (int i=0; i<numAffectedVerts; i++)
        numAffectedOps += affectedVerts[i]->numOps;

    Operation **affectedOps = new Operation *[numAffectedOps];

    numAffectedOps = 0;
    for (int i=0; i<numAffectedVerts; i++)
        for (int opnum=0; opnum<affectedVerts[i]->numOps; opnum++)
            affectedOps[numAffectedOps++] = affectedVerts[i]->ops[opnum];

    delete [] affectedVerts;
    affectedVerts = NULL;
    numAffectedVerts = 0;
    

    // debugging test
    for (int i=0; i<numAffectedOps; i++)
        if (affectedOps[i]->source_vert == NULL)
            fprintf(stderr, "NULL vert on op\n");
    
    // sort the affected operations - for purpose of sort, consider all
    // occurances of the source vertex to be equal to the destination
    // vertex
    compare_ops_source_vert = source_vert;
    compare_ops_destination_vert = destination_vert;
    qsort(affectedOps, numAffectedOps, sizeof(Operation *), compare_ops);

    // classify operations as modified or removed

    // just allocate conservatively rather than counting first
    *removeOps = new Operation *[numAffectedOps];
    *modOps    = new Operation *[numAffectedOps];
    *numRemoveOps = *numModOps = 0;

    for (int i=0; i<numAffectedOps; i++)
    {
        Operation *op = affectedOps[i];
        xbsVertex *mapped_source =
            ((op->source_vert == source_vert) ? destination_vert :
             op->source_vert);
        xbsVertex *mapped_destination =
            ((op->destination_vert == source_vert) ? destination_vert :
             op->destination_vert);

        // now I can't remember why this can happen...
        if ((mapped_source != destination_vert) &&
            (mapped_destination != destination_vert))
        {
            // fprintf(stderr, "unnecessary op affected!\n");
            continue;
        }
        
        if (mapped_source == mapped_destination)
        {
            (*removeOps)[(*numRemoveOps)++] = op;
            continue;
        }

        int different;
        
        if (i==0)
            different = 1;
        else
        {
            Operation *prev_op = affectedOps[i-1];
            different = compare_ops((void *)(&prev_op),
                                    (void *)(&op));
        }

        if (different == 0)
            (*removeOps)[(*numRemoveOps)++] = op;
        else
            (*modOps)[(*numModOps)++] = op;
    }

    // for half edge collapse, no operations are ever added
    *numAddOps = 0;
    *addOps = NULL;

    delete [] affectedOps;
    affectedOps = NULL;
    numAffectedOps = 0;
    
    return;
} /** End of Operation::getNeighborOps() **/

#if 0
/*****************************************************************************\
 @ Operation::getNeighborOpsOld
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
Operation::getNeighborOpsOld(Model *model,
                             Operation ***addOps, int *numAddOps,
                             Operation ***removeOps, int *numRemoveOps,
                             Operation ***modOps, int *numModOps)
{
    MT *mt = model->mt;
    VertexData *vdata = model->vdata;

    // For the half edge collapse operation, the only operations which are
    // effected are those which modify a triangle around the source vertex
    // of this current operation. This should be any operation which has
    // as its source vertex any of these vertices around the current
    // operation's source vertex

    // Actually, the above statement assumed that the error is stored with
    // the triangles. If the error is stored with the vertices, then any
    // operation including any of these modified triangles' vertices could
    // need to be updated.

    // NEED TO FIX THIS!!!!
    
    int maxAffectedVerts = vdata[source_vert].numTris * 3;
    
    int *affectedVerts = new int[maxAffectedVerts];
    int numAffectedVerts = 0;

    for (int i=0; i<vdata[source_vert].numTris; i++)
    {
        int triIndex = vdata[source_vert].tris[i];
        mtTriangle *tri = mt->getTri(triIndex);
        affectedVerts[numAffectedVerts++] = tri->verts[0];
        affectedVerts[numAffectedVerts++] = tri->verts[1];
        affectedVerts[numAffectedVerts++] = tri->verts[2];
    }

    // sort and remove duplicates
    qsort(affectedVerts, numAffectedVerts, sizeof(int), compare_ints);

    int current = 0;
    for (int i=1; i<numAffectedVerts; i++)
    {
        if (affectedVerts[i] != affectedVerts[current])
            affectedVerts[++current] = affectedVerts[i];
    }
    numAffectedVerts = current + 1;


    // Create list of affected operations

    // count, allocate, and fill
    int numAffectedOps = 0;
    for (int i=0; i<numAffectedVerts; i++)
        numAffectedOps += vdata[affectedVerts[i]].numOps;

    Operation **affectedOps = new Operation *[numAffectedOps];

    numAffectedOps = 0;
    for (int i=0; i<numAffectedVerts; i++)
        for (int opnum=0; opnum<vdata[affectedVerts[i]].numOps; opnum++)
            affectedOps[numAffectedOps++] =
                vdata[affectedVerts[i]].ops[opnum];

    delete [] affectedVerts;
    affectedVerts = NULL;
    numAffectedVerts = 0;
    
    
    // sort the affected operations - for purpose of sort, consider all
    // occurances of the source vertex to be equal to the destination
    // vertex
    compare_ops_source_vert = source_vert;
    compare_ops_destination_vert = destination_vert;
    qsort(affectedOps, numAffectedOps, sizeof(Operation *), compare_ops);

    // classify operations as modified or removed

    // just allocate conservatively rather than counting first
    *removeOps = new Operation *[numAffectedOps];
    *modOps    = new Operation *[numAffectedOps];
    *numRemoveOps = *numModOps = 0;

    for (int i=0; i<numAffectedOps; i++)
    {
        Operation *op = affectedOps[i];
        int mapped_source =
            ((op->source_vert == source_vert) ? destination_vert :
             op->source_vert);
        int mapped_destination =
            ((op->destination_vert == source_vert) ? destination_vert :
             op->destination_vert);
            
        if (mapped_source == mapped_destination)
        {
            (*removeOps)[(*numRemoveOps)++] = op;
            continue;
        }

        int different;
        
        if (i==0)
            different = 1;
        else
        {
            Operation *prev_op = affectedOps[i-1];
            different = compare_ops((void *)(&prev_op),
                                    (void *)(&op));
        }

        if (different == 0)
            (*removeOps)[(*numRemoveOps)++] = op;
        else
            (*modOps)[(*numModOps)++] = op;
    }

    // for half edge collapse, no operations are ever added
    *numAddOps = 0;
    *addOps = NULL;

    delete [] affectedOps;
    affectedOps = NULL;
    numAffectedOps = 0;
    
    return;
} /** End of Operation::getNeighborOpsOld() **/
#endif
    
    
/*****************************************************************************\
 @ Operation::updateModel
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
Operation::updateModel(Model *model, Hierarchy *hierarchy,
                       Operation ***addOps, int *numAddOps,
                       Operation ***removeOps, int *numRemoveOps,
                       Operation ***modOps, int *numModOps)
{

    // debugging
    //    if (model->getNumVerts() == 1928)
    //      fprintf(stderr, "Break!\n");
    //    if (model->getNumTris() == 2220)
    //      fprintf(stderr, "Break2!\n");
        
    // If everything is REALLY kept up to date and we keep around all the
    // information necessary for propagating the error, we may not need to
    // do this cost computation here.
#if 0// moved this test up into the XBSSimplifier
    float oldcost = error->getError(); //cost;

    computeCost(model);

    if (/*cost*/ error->getError() != oldcost)
        fprintf(stderr, "Cost not kept up to date! (%g != %g)\n",
                oldcost, error->getError());
#endif
    
//    destination_vert->errorData->setError(error->getError());
    destination_vert->errorData->update(this);

    TestVdata(model);
    
    // Make lists of which triangles are changed and which are destoyed
    // (all are actually consumed by this operation, but some are not
    // replaced by new triangles)

    // Conservative allocation rather than counting first
    xbsTriangle **destroyedTris =
        new xbsTriangle *[source_vert->coincidentNumTris()];
    xbsTriangle **changedTris =
        new xbsTriangle *[source_vert->coincidentNumTris()];
    int numDestroyedTris = 0;
    int numChangedTris = 0;

    xbsVertex *currentCoincident = source_vert;
    do 
    {
        for (int i=0; i<currentCoincident->numTris; i++)
        {
            xbsTriangle *tri = currentCoincident->tris[i];
            
            // filter out degenerate tris
            if (((tri->verts[0]->minCoincident() == destination_vert) ||
                 (tri->verts[1]->minCoincident() == destination_vert) ||
                 (tri->verts[2]->minCoincident() == destination_vert)) ||
                duplicatedTriangle(tri))
                destroyedTris[numDestroyedTris++] = tri;
            else
                changedTris[numChangedTris++] = tri;
        }
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != source_vert);


    // list of vertices who's tris may be reduced (and thus may possibly
    // end up with zero tris)
    int numReducedVerts = (numDestroyedTris+numChangedTris)*3
        + source_vert->numCoincident() + destination_vert->numCoincident();
    xbsVertex **reducedVerts = new xbsVertex *[numReducedVerts];
    numReducedVerts = 0;
    for (int i=0; i<numDestroyedTris; i++)
    {
        xbsTriangle *tri = destroyedTris[i];
        for (int j=0; j<3; j++)
            reducedVerts[numReducedVerts++] = tri->verts[j];
    }
    for (int i=0; i<numChangedTris; i++)
    {
        xbsTriangle *tri = changedTris[i];
        for (int j=0; j<3; j++)
            reducedVerts[numReducedVerts++] = tri->verts[j];
    }
    // Empty verts on source or destination that we have carried around for
    // VDS are now ripe for removal as well!
    currentCoincident = source_vert;
    do
    {
        if (currentCoincident->numTris == 0)
            reducedVerts[numReducedVerts++] = currentCoincident;
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != source_vert);
    currentCoincident = destination_vert;
    do
    {
        if (currentCoincident->numTris == 0)
            reducedVerts[numReducedVerts++] = currentCoincident;
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != destination_vert);
    //remove duplicates
    qsort(reducedVerts, numReducedVerts, sizeof(xbsVertex *),
          compare_pointers);
    int current=0;
    for (int i=1; i<numReducedVerts; i++)
        if (reducedVerts[i] != reducedVerts[current])
            reducedVerts[++current] = reducedVerts[i];
    numReducedVerts = current+1;

    
    
    // Compute mapping of ids from source_vert to destination_vert. This
    // mapping would trivially say to replace all occurances of source_vert
    // with destination_vert, except we have to also worry about any number
    // of coincident vertices for each of these. So for each coincident
    // vertex of the source_vert, figure out which coincident vertex of the
    // destination_vert it maps to. We do this by looking at the
    // destroyedTris. Each destroyed triangle (except those filtered out by
    // duplicatedTriangle()) has an edge between a source and a destination
    // vertex, and collapsing that edge implies a mapping. If a coincident
    // vertex of the source has no such triangle, then we need to make a
    // new vertex with the coordinates of the destination_vert and all the
    // attributes of the source. Or perhaps this half edge collapse should
    // be disallowed? This is a bit strange because half edge collapses are
    // not typically supposed to generate new vertices.

    int numSourceCoincident = source_vert->numCoincident();
    xbsVertex **vertMappings = new xbsVertex *[numSourceCoincident];
    for (int i=0; i<numSourceCoincident; i++)
        vertMappings[i] = NULL;
    for (int i=0; i<numDestroyedTris; i++)
    {
        xbsTriangle *tri = destroyedTris[i];
        int index = -1;
        xbsVertex *mapping = NULL;
        for (int j=0; j<3; j++)
        {
            xbsVertex *vert=tri->verts[j];
            xbsVertex *min = vert->minCoincident();
            if (min == source_vert)
                index = vert->coincidentIndex();
            else if (min == destination_vert)
                mapping = vert;
        }
        if ((index == -1) || (mapping == NULL))
        {
            // This is a triangle filtered out by duplicatedTriangle()
            // rather than one with an edge that is collapsed
            continue;
            
            //fprintf(stderr,
            //   "Destroyed tri doesn't connect source to destination vertex!\n");
            //exit(1);
        }
#if 0
        if ((vertMappings[index] != NULL) && (vertMappings[index] != mapping))
        {
            // good for debugging
            fprintf(stderr, "Source vert has multiple mappings to destination!\n");
        }
#endif
        vertMappings[index] = mapping;
    }

    
    currentCoincident = source_vert;
    for (int i=0; i<numSourceCoincident;
         i++, currentCoincident = currentCoincident->nextCoincident)
    {
        // make sure we found a mapping
        if (vertMappings[i] != NULL)
            continue;

        // If the source vertex is empty, just keep the mapping NULL
        // There can be empty vertices on the source that we have been
        // keeping around for VDS...
        if (currentCoincident->numTris == 0)
            continue;
        
        // If not, create a new vertex. The new vertex has the attributes
        // of this coincident source vertex and the coordinates of the
        // destination vertex
        xbsVertex *newVertex = currentCoincident->makeNew();
        currentCoincident->copySame(newVertex);
            newVertex->coord = destination_vert->coord;
#if 0 // this is a holdover from a previous iteration... we don't need to do this 
      // anymore because there is a mincoindent in the ring already that has errordata
        if (newVertex->errorData==NULL){
            if (model->errorMetric==GLOD_METRIC_SPHERES)
                newVertex->errorData = new SphereErrorData(newVertex);
            else if (model->errorMetric==GLOD_METRIC_QUADRICS)
                newVertex->errorData = new QuadricErrorData(newVertex);
            else if (model->errorMetric==GLOD_METRIC_PERMISSION_GRID)
                newVertex->errorData = new PermissionGridErrorData(newVertex);
        }
#endif
        newVertex->index = -1;
        newVertex->ops = NULL;
        newVertex->numOps = 0;
        newVertex->errorData = NULL; //->setError(MAXFLOAT);
        newVertex->mtIndex = -1;
//      fprintf(stderr, "here1\n");
        newVertex->nextCoincident = newVertex;
        newVertex->tris = NULL;
        newVertex->numTris = 0;
        
        vertMappings[i] = newVertex;

        
        // Should we add the vertex to the model and the coincident ring
        // of destination_vert now or later? Whenever we add it, it may
        // become the minCoincident instead of this operation's current
        // destination_vert! Then we would have to migrate all the
        // operations from the old minCoincident to this one, etc.
        model->addVert(newVertex);

        
        //fprintf(stderr, "newVertex index: %d\n", newVertex->index);
        newVertex->nextCoincident = destination_vert->nextCoincident;
        destination_vert->nextCoincident = newVertex;
    }
    if (destination_vert->minCoincident() != destination_vert)
    {
        // Actually change the destination_vert to be the new
        // minCoincident. The new minCoincident gets all the ops. All its
        // ops should get the new vertex reference, etc.
        
        xbsVertex *dv = destination_vert;
        xbsVertex *min = dv->minCoincident();
        min->ops = dv->ops;
        min->numOps = dv->numOps;
        /*
        if (min->errorData == NULL){
            if (model->errorMetric==GLOD_METRIC_SPHERES)
                min->errorData = new SphereErrorData(min);
            else if (model->errorMetric==GLOD_METRIC_QUADRICS)
                min->errorData = new QuadricErrorData(min);
        }
    */
        //printf("%f %f\n", min->errorData->getError(), dv->errorData->getError());
        //min->errorData->setError(dv->errorData->getError());
        //min->errorData->setError(dv->errorData->getError());
        min->errorData = dv->errorData;
        dv->errorData = NULL;
        dv->ops = NULL;
        dv->numOps = 0;

        for (int i=0; i<*numModOps; i++)
        {
            Operation *op = (*modOps)[i];
            if (op->source_vert == dv)
                op->source_vert = min;
            else if (op->destination_vert == dv)
                op->destination_vert = min;
        }
        for (int i=0; i<*numRemoveOps; i++)
        {
            Operation *op = (*removeOps)[i];
            if (op->source_vert == dv)
                op->source_vert = min;
            else if (op->destination_vert == dv)
                op->destination_vert = min;
        }
        // the above should also have changed this->destination_vert !!

        if (destination_vert->minCoincident() != destination_vert)
        {
            fprintf(stderr, "minCoincident is still screwed up!\n");
            exit(1);
        }
    }
    


    

    hierarchy->update(model, this, vertMappings,
                      changedTris, numChangedTris,
                      destroyedTris, numDestroyedTris);



    
    
    // remove changed and destroyed tris from the vdata of all their
    // adjacent vertices

    for (int i=0; i<numDestroyedTris; i++)
    {
        xbsTriangle *tri = destroyedTris[i];
        
        for (int j=0; j<3; j++)
        {
            if (tri->verts[j]->removeTri(tri) != 1)
            {
                fprintf(stderr,
                        "Error removing destroyed tri from vdata.\n");
                exit(1);
            }
        }
    }

    for (int i=0; i<numChangedTris; i++)
    {
        xbsTriangle *tri = changedTris[i];
        
        for (int j=0; j<3; j++)
        {
            if (tri->verts[j]->removeTri(tri) != 1)
            {
                fprintf(stderr,
                        "Error removing changed tri from vdata.\n");
                exit(1);
            }
        }
    }


    currentCoincident = source_vert;
    do
    {
        currentCoincident->freeTris();
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != source_vert);


    // Rather than loop over the destination_vert's coincident ring
    // directly, loop over the mapping array we created earlier. This may
    // avoid a problem if we created a new vertex and haven't added it to
    // the ring yet.
    for (int i=0; i<numSourceCoincident; i++)
    {
        currentCoincident = vertMappings[i];

        // Ignore empty verts (which we are keeping around for VDS)
        if (currentCoincident == NULL)
            continue;
        
        
        currentCoincident->reallocTris(currentCoincident->numTris +
                                       numChangedTris);
    }


    //
    // change the vertices of all the changed tris in the model
    //
    for (int tnum=0; tnum<numChangedTris; tnum++)
    {
        xbsTriangle *tri = changedTris[tnum];

        for (int tvnum=0; tvnum<3; tvnum++)
            if (tri->verts[tvnum]->minCoincident() == source_vert)
            {
                // match this vertex to an appropriate coincident vertex of
                // the destination_vert
                tri->verts[tvnum] =
                    vertMappings[tri->verts[tvnum]->coincidentIndex()];
            }
        
        // add new tri to index data of all its vertices
        for (int tvnum=0; tvnum<3; tvnum++)
            tri->verts[tvnum]->addTri(tri);
    }


    for (int tnum=0; tnum<numDestroyedTris; tnum++)
    {
        model->removeTri(destroyedTris[tnum]);
        delete destroyedTris[tnum];
    }
    
    delete [] destroyedTris;
    destroyedTris = NULL;
    numDestroyedTris = 0;

    delete [] changedTris;
    changedTris = NULL;
    numChangedTris = 0;


    
    
    // Concatenate operation lists for destination vertex.
    // This should work even with coincident vertices, because all ops are
    // stored on the minCoincident (and every source and destination vertex
    // should be a minCoincident)
    int newNumOps = source_vert->numOps + destination_vert->numOps;
    Operation **newOps = new Operation*[newNumOps];

    newNumOps = 0;
    for (int opnum=0; opnum<destination_vert->numOps; opnum++)
        newOps[newNumOps++] = destination_vert->ops[opnum];
    for (int opnum=0; opnum<source_vert->numOps; opnum++)
        newOps[newNumOps++] = source_vert->ops[opnum];

    delete [] source_vert->ops;
    source_vert->ops = NULL;
    source_vert->numOps = 0;

    delete [] destination_vert->ops;
    destination_vert->ops = newOps;
    destination_vert->numOps = newNumOps;

    
    // Map all occurances of the current operation's source vertex to its
    // destination vertex
    // DANGER - overwrites source_vert variable of current operation!
    // don't use this->source_vert again for current operation
    xbsVertex *sv = source_vert;
    xbsVertex *dv = destination_vert;
    for (int i=0; i<*numModOps; i++)
    {
        Operation *op = (*modOps)[i];
        if (op->source_vert == sv)
            op->source_vert = dv;
        if (op->destination_vert == sv)
            op->destination_vert = dv;
    }
    for (int i=0; i<*numRemoveOps; i++)
    {
        Operation *op = (*removeOps)[i];
        if (op->source_vert == sv)
            op->source_vert = dv;
        if (op->destination_vert == sv)
            op->destination_vert = dv;
    }



    // After the collapse, some of the neighborhood vertices may no longer
    // have any triangles, so we plan to delete them. But first, make sure
    // we will remove all operations using those vertices!

    // I think all such ops would have to already be listed in the mod or
    // remove ops. If it's currently a mod op, make it a remove op instead.
    
    for (int i=0; i<*numModOps; i++)
    {
        Operation *op = (*modOps)[i];

        // Make sure there is actually still a triangle with an edge
        // connecting the source to the destination
        if (op->source_vert->coincidentIsAdjacent(op->destination_vert))
            continue;

        // We need to move the operation from mod list to remove list

        // remove from mod list
        (*modOps)[i] = (*modOps)[*numModOps-1];
        (*numModOps)--;
        i--;

        // add to remove list
        Operation **newRemoveOps =
            new Operation *[(*numRemoveOps)+1];
        for (int opnum=0; opnum<(*numRemoveOps); opnum++)
            newRemoveOps[opnum] = (*removeOps)[opnum];
        newRemoveOps[(*numRemoveOps)++] = op;
        delete (*removeOps);
        (*removeOps) = newRemoveOps;
    }
    

    
    
    // Now we have to update the ops list of all the affected vertices. The
    // list is not changed by modOps, and we know for this operation type
    // that (numAddOps == 0), so just remove the removeOps from the
    // appropriate vertex lists
    
    // (We could do this more efficiently, but for now, just do it by brute
    // force.)

    for (int opnum=0; opnum<*numRemoveOps; opnum++)
    {
        Operation *op = (*removeOps)[opnum];
        xbsVertex *vert = op->source_vert;

        int current = 0;
        for (int i=0; i<vert->numOps; i++)
        {
            if (vert->ops[i] != op)
                vert->ops[current++] = vert->ops[i];
        }
        
        if (current != (vert->numOps - 1))
        {
            fprintf(stderr, "Error removing op from vdata.\n");
            exit(1);
        }
        
        vert->numOps = current;

        if (vert->numOps == 0)
        {
            delete [] vert->ops;
            vert->ops = NULL;
        }
    }


#if 0
    // debugging test (SLOW)
    // make sure no vertices have ops except minCoincident verts
    for (int i=0; i<model->getNumVerts(); i++)
    {
        xbsVertex *vert = model->getVert(i);
        if ((vert->numOps > 0) && (vert->minCoincident() != vert))
        {
            fprintf(stderr, "Vert with ops is not minCoincident.\n");
        }
        if ((vert->minCoincident() == vert) && (vert->numOps == 0))
        {
            //fprintf(stderr, "minCoincident vert has no ops\n");
        }
    }
#endif

//    TestVdata(model);
    
    // remove any empty vertices caused by the destroyed tris
    for (int reducedVertNum=0; reducedVertNum<numReducedVerts;
         reducedVertNum++)
    {
        xbsVertex *vert = reducedVerts[reducedVertNum];
        if (vert->numTris > 0)
            continue;

        // For the sake of VDS, leave around empty vertices unless they are
        // on the source or destination vert





        // Okay, this is ugly, too. Because the current error metric uses a
        // heuristic based on how many coincident vertices there are, if we
        // change the number of non-empty coincident vertices of some
        // vertex, it can change the cost of all its adjacent edges (even
        // if this vertex was not the source or
        // destination). Unfortunately, it is hard to tell whether this
        // vertex was source or destination, or how many nonempty
        // coincident vertices there were in its coincident ring when we
        // started. So just make sure any ops referring to this vertex's
        // ring as source or destination are on the modOp list

        // are there any ops referring to this vert's ring?
        if (vert->coincidentNumTris() > 0)
        {
            currentCoincident = vert;
            do
            {
                for (int tnum=0; tnum<currentCoincident->numTris;
                     tnum++)
                {
                    xbsTriangle *tri = currentCoincident->tris[tnum];
                    for (int vnum=0; vnum<3; vnum++)
                    {
                        xbsVertex *tvert =
                            tri->verts[vnum]->minCoincident();
                        for (int opnum=0; opnum<tvert->numOps; opnum++)
                        {
                            Operation *op = tvert->ops[opnum];

                            // is op already on remove list?
                            int found = 0;
                            for (int i=0; i<*numRemoveOps; i++)
                            {
                                if ((*removeOps)[i] == op)
                                {
                                    found = 1;
                                    break;
                                }
                            }
                            if (found == 1)
                                continue;

                            // is op already on mod list?
                            found = 0;
                            for (int i=0; i<*numModOps; i++)
                            {
                                if ((*modOps)[i] == op)
                                {
                                    found = 1;
                                    break;
                                }
                            }
                            if (found == 1)
                                continue;


                            // add op to mod list
#if 1
                            //fprintf(stderr, "Adding to mod list\n");
                            Operation **newModOps =
                                new Operation *[(*numModOps)+1];
                            for (int opnum2=0; opnum2<(*numModOps); opnum2++)
                                newModOps[opnum2] = (*modOps)[opnum2];
                            newModOps[(*numModOps)++] = op;
                            delete (*modOps);
                            (*modOps) = newModOps;
#endif
                        }
                    }
                }
                currentCoincident = currentCoincident->nextCoincident;
            } while (currentCoincident != vert);
        }


        
#if 0
        // this doesn't seem to get everything...
        // I guess once the original source or destination are removed from
        // the ring, there is no way left to figure out which ones are on
        // that ring
        if ((vert->minCoincident() != sv) &&
            (vert->minCoincident() != dv))
            continue;
#else
        // Okay, a bit of a hack for now
        // Only delete if VDS says we can. Discrete leaves this set to -1
        // all the time, so that will have the happy side effect of letting
        // us clean up properly as we go
        if (hierarchy->getHierarchyType() == VDS_Hierarchy &&
            vert->mtIndex != -1)
            continue;
#endif
        
        // remove any responsibilities of this vertex as a minCoincident
        // -- move any operations to the new minCoincident
                
        if (vert->minCoincident() == vert)
        {
            // are there any ops referring to this vertex as a source or a
            // destination?
            if (vert->coincidentNumTris() > 0)
            {
                // find the vertex who will become minCoincident
                xbsVertex *min = vert->minCoincidentExceptThis();
                if (min == NULL)
                {
                    fprintf(stderr, "No new coincident available!\n");
                    exit(1);
                }
                /*
                if (min->errorData == NULL){
                    if (model->errorMetric==GLOD_METRIC_SPHERES)
                        min->errorData = new SphereErrorData(min);
                    else if (model->errorMetric==GLOD_METRIC_QUADRICS)
                        min->errorData = new QuadricErrorData(min);
                }
                 */
                // re-map all ops that may refer to this vertex to the new
                // minCoincident
                int foundCount = 0;
                currentCoincident = vert;
                do
                {
                    for (int tnum=0; tnum<currentCoincident->numTris;
                         tnum++)
                    {
                        xbsTriangle *tri = currentCoincident->tris[tnum];
                        for (int vnum=0; vnum<3; vnum++)
                        {
                            xbsVertex *tvert =
                                tri->verts[vnum]->minCoincident();
                            for (int opnum=0; opnum<tvert->numOps; opnum++)
                            {
                                Operation *op = tvert->ops[opnum];
                                if (op->source_vert == vert)
                                {
                                    op->source_vert = min;
                                    foundCount++;
                                }
                                if (op->destination_vert == vert)
                                {
                                    op->destination_vert = min;
                                    foundCount++;
                                }
                            }
                        }
                    }
                    currentCoincident = currentCoincident->nextCoincident;
                } while (currentCoincident != vert);

                if (foundCount > 2*vert->numOps)
                    fprintf(stderr, "Found count is too big\n");
                
                if (foundCount < 2*vert->numOps)
                {
                    fprintf(stderr, "Didn't find all ops\n");
                    for (int i=0; i<model->getNumVerts(); i++)
                    {
                        xbsVertex *vert1 = model->getVert(i);
                        for (int j=0; j<vert1->numOps; j++)
                        {
                            Operation *op = vert1->ops[j];
                            if ((op->source_vert == vert) ||
                                (op->destination_vert == vert))
                            {
                                fprintf(stderr,
                                        "Found vertex to be deleted still");
                                fprintf(stderr, " on an op.\n");
                            }
                        }
                    }
                }
                
                min->ops = vert->ops;
                min->numOps = vert->numOps;
                //printf("%f %f\n", min->errorData->getError(), vert->errorData->getError());
                //min->errorData->setError(vert->errorData->getError());
                min->errorData = vert->errorData;
                vert->errorData = NULL;
                vert->ops = NULL;
                vert->numOps = 0;
            }
        }
        
        // remove the vert from the coincident ring
        for (currentCoincident = vert;
             currentCoincident->nextCoincident != vert;
             currentCoincident = currentCoincident->nextCoincident);
//      fprintf(stderr, "here2\n");
        currentCoincident->nextCoincident = vert->nextCoincident;
        vert->nextCoincident = vert;

#if 0
        // debugging test (SLOW)
        for (int i=0; i<model->getNumVerts(); i++)
        {
            xbsVertex *vert = model->getVert(i);
            for (int j=0; j<vert->numOps; j++)
            {
                Operation *op = vert->ops[j];
                if ((op->source_vert->index == -1) ||
                    (op->destination_vert->index == -1))
                    fprintf(stderr, "Deleted vert on op\n");
            }
        }
#endif
        
        // remove vertex from model and delete it
        model->removeVert(vert);
        delete vert;
        
#if 0
        // debugging test (SLOW)
        for (int i=0; i<model->getNumVerts(); i++)
        {
            xbsVertex *vert1 = model->getVert(i);
            for (int j=0; j<vert1->numOps; j++)
            {
                Operation *op = vert1->ops[j];
                if ((op->source_vert->index == -1) ||
                    (op->destination_vert->index == -1))
                {
                    fprintf(stderr, "Deleted vert on op\n");
                    if (op->destination_vert == vert)
                        fprintf(stderr,
                                "It's the one just deleted!\n");
                }
            }
        }
#endif

    }
    delete [] reducedVerts;
    delete [] vertMappings;
    TestVdata(model);

    return;
} /** End of Operation::updateModel() **/


/*****************************************************************************\
 @ EdgeCollapse::initQueue
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
EdgeCollapse::initQueue(Model *model, SimpQueue *queue)
{
    // for each vertex, generate a list of operations, then insert them
    // onto the queue

    // This is similar to half edge collapse, but each edge has only 1
    // possible operation instead of 2. We're storing in same data
    // structure as half edge collapse, so only take operations with
    // source address < destination address (somewhat arbitrary choice)
    
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        if (vert != vert->minCoincident())
            continue;   
        if (vert->errorData==NULL){
            if (model->errorMetric==GLOD_METRIC_SPHERES)
                vert->errorData = new SphereErrorData(vert);
            else if (model->errorMetric==GLOD_METRIC_QUADRICS)
                vert->errorData = new QuadricErrorData(vert);
            else if (model->errorMetric==GLOD_METRIC_PERMISSION_GRID)
                vert->errorData = new PermissionGridErrorData(vert);
        }
        // generate list of neighboring vertices (with possible
        // redundancies)

        int numNeighborVerts = vert->coincidentNumTris() * 2;
        xbsVertex **neighborVerts = new xbsVertex *[numNeighborVerts];
        int maxNeighborVerts = numNeighborVerts;

        numNeighborVerts = 0;
        xbsVertex *currentCoincident = vert;
        do {
            for (int tnum=0; tnum<currentCoincident->numTris; tnum++)
            {
                xbsTriangle *tri = currentCoincident->tris[tnum];
                
                for (int tvnum=0; tvnum<3; tvnum++)
                    if ((tri->verts[tvnum] != currentCoincident) &&
                        (tri->verts[tvnum]->minCoincident() > vert))
                        neighborVerts[numNeighborVerts++] =
                            tri->verts[tvnum]->minCoincident();
            }
            currentCoincident = currentCoincident->nextCoincident;
        } while (currentCoincident != vert);
        
        if (model->errorMetric==GLOD_METRIC_SPHERES){
            for (int i=0; i<numNeighborVerts; i++)
                if (neighborVerts[i]->errorData==NULL)
                    neighborVerts[i]->errorData = new SphereErrorData(neighborVerts[i]);
        }
        else if (model->errorMetric==GLOD_METRIC_QUADRICS) {
            for (int i=0; i<numNeighborVerts; i++)
                if (neighborVerts[i]->errorData==NULL)
                    neighborVerts[i]->errorData = new QuadricErrorData(neighborVerts[i]);
        }
        else if (model->errorMetric==GLOD_METRIC_PERMISSION_GRID) {
            for (int i=0; i<numNeighborVerts; i++)
                if (neighborVerts[i]->errorData==NULL)
                    neighborVerts[i]->errorData = new PermissionGridErrorData(neighborVerts[i]);
        }
        
        if (numNeighborVerts > maxNeighborVerts)
        {
            fprintf(stderr, "Bad numNeighborVerts!\n");
            exit(1);
        }
        
        // sort the neighbor list
        qsort(neighborVerts, numNeighborVerts, sizeof(xbsVertex *),
              compare_pointers);
        
        // compact neighbor list (removing redundancies)
        if (numNeighborVerts > 0)
        {           
            int current=0;
            for (int i=1; i<numNeighborVerts; i++)
                if (neighborVerts[i] != neighborVerts[current])
                    neighborVerts[++current] = neighborVerts[i];
            numNeighborVerts = current+1;
        }
        
        vert->ops = new Operation *[numNeighborVerts];
        vert->numOps = 0;

        // should call some function of the error metric to initialize
        // error-specific vertex data

        vert->errorData->init(vert);
        //vert->errorData->setError(0.0);
        
        // generate operations and insert onto the queue
        for (int opnum=0; opnum<numNeighborVerts; opnum++)
        {
            // For full edge collapse, which vertex we call the source and
            // which the destination is arbitrary. We have restricted to
            // source id < destination id
        
            EdgeCollapse *op = new EdgeCollapse;
            op->source_vert = vert;
            op->destination_vert = neighborVerts[opnum];
            op->computeCost(model);
            
            queue->insert(op);

            // PROBABLY NEED TO MODIFY FOR FULL EDGE COLLAPSE!!!
            
            // Actually, as in the half edge collapse, we can store each
            // operation with only one vertex rather than two. The
            // getNeighborOps() method will still find all the correct
            // operations.
            vert->ops[vert->numOps++] = op;
        }

        delete [] neighborVerts;
        neighborVerts = NULL;
        numNeighborVerts = 0;

    }
    return;
} /** End of EdgeCollapse::initQueue() **/

/*****************************************************************************\
 @ EdgeCollapse::updateModel
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
EdgeCollapse::updateModel(Model *model, Hierarchy *hierarchy,
                          Operation ***addOps, int *numAddOps,
                          Operation ***removeOps, int *numRemoveOps,
                          Operation ***modOps, int *numModOps)
{

#if 0
    // debugging
    if (model->getNumTris() == 41402)
        fprintf(stderr, "break here.\n");
#endif

    // If everything is REALLY kept up to date and we keep around all the
    // information necessary for propagating the error, we may not need to
    // do this cost computation here.
#if 0
    float oldcost = error->getError();

    computeCost(model);

    if (error->getError() != oldcost)
        fprintf(stderr, "Cost not kept up to date!\n");
#endif


    // merge consumed triangles from source and destination, removing
    // duplicates
    // sort consumed triangles 
    xbsTriangle **consumedTris =
        new xbsTriangle *[source_vert->coincidentNumTris() +
                          destination_vert->coincidentNumTris()]; 
    
    int numConsumedTris = 0;
    xbsVertex *currentCoincident = source_vert;
    do
    {
        for (int tnum=0; tnum<currentCoincident->numTris; tnum++)
            consumedTris[numConsumedTris++] =
                currentCoincident->tris[tnum];
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != source_vert);

    currentCoincident = destination_vert;
    do
    {
        for (int tnum=0; tnum<currentCoincident->numTris; tnum++)
            consumedTris[numConsumedTris++] =
                currentCoincident->tris[tnum];
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != destination_vert);
    
    qsort(consumedTris, numConsumedTris,
          sizeof(xbsTriangle *), compare_tri_end_nodes);

    int current = 1;
    for (int tnum=1; tnum<numConsumedTris; tnum++)
    {
        if (consumedTris[current-1] != consumedTris[tnum])
            consumedTris[current++] = consumedTris[tnum];
    }
    numConsumedTris = current;
    
        
    
    // Make lists of which triangles are changed and which are destoyed
    // (all are actually consumed by this operation, but some are not
    // replaced by new triangles)

    // Conservative allocation rather than counting first
    xbsTriangle **destroyedTris = new xbsTriangle *[numConsumedTris];
    xbsTriangle **changedTris = new xbsTriangle *[numConsumedTris];
    int numDestroyedTris = 0;
    int numChangedTris = 0;
    
    for (int i=0; i<numConsumedTris; i++)
    {
        xbsTriangle *tri = consumedTris[i];

        // filter out degenerate tris

        // notice that this is exactly the same as for half edge collapse,
        // except we first ensure that this triangle does indeed touch the
        // source vertex. No triangles that do not touch the source vertex
        // are degenerate, and we will not count them as duplicates (if a
        // duplicate pair is created, we eliminate the one that is touching
        // the source vertex but not the destination)
        if (((tri->verts[0]->minCoincident() == source_vert) ||
             (tri->verts[1]->minCoincident() == source_vert) ||
             (tri->verts[2]->minCoincident() == source_vert)) &&
            (((tri->verts[0]->minCoincident() == destination_vert) ||
              (tri->verts[1]->minCoincident() == destination_vert) ||
              (tri->verts[2]->minCoincident() == destination_vert)) ||
             duplicatedTriangle(tri)))
            destroyedTris[numDestroyedTris++] = tri;
        else
            changedTris[numChangedTris++] = tri;
    }


    // list of vertices who's tris may be reduced (and thus may possibly
    // end up with zero tris)
    int numReducedVerts = numConsumedTris*3;
    xbsVertex **reducedVerts = new xbsVertex *[numReducedVerts];
    numReducedVerts = 0;
    for (int i=0; i<numConsumedTris; i++)
    {
        xbsTriangle *tri = consumedTris[i];
        for (int j=0; j<3; j++)
            reducedVerts[numReducedVerts++] = tri->verts[j];
    }
    qsort(reducedVerts, numReducedVerts, sizeof(xbsVertex *),
          compare_pointers);
    current=0;
    for (int i=1; i<numReducedVerts; i++)
        if (reducedVerts[i] != reducedVerts[current])
            reducedVerts[++current] = reducedVerts[i];
    numReducedVerts = current+1;






#if 0
    // Find all unique edge connecting coincident vertices of the source
    // vertex to coincident vertices of the destination vertex. The common
    // case should be a 1-1 mapping. But it is also possible for a
    // coincident vertex of source or destination to either have 0 mappings
    // or more than 1 mapping (should be rare).

    VertDuple *edges = new VertDuple[numDestroyedTris];
    for (int i=0; i<numDestroyedTris; i++)
    {
        xbsTriangle *tri = destroyedTris[i];
        edges[i][0] = edges[i][1] = NULL;
        for (int j=0; j<3; j++)
        {
            xbsVertex *vert = tri->verts[j];
            min = vert->minCoincident();
            if (min == source_vert)
                edges[i][0] = vert;
            else if (min == destination_vert)
                edges[i][1] = vert;
        }
        if ((edges[i][0] == NULL) || (duples[i][1] == NULL))
        {
            fprintf(stderr,
                    "Couldn't find source or destination on destroyed tri!\n");
            exit(1);
        }
    }

    // sort and remove duplicates
    qsort(edges, numDestroyedTris, sizeof(VertDuple), compare_vertduples);
    int current = 0;
    for (int i=1; i<numDestroyedTris; i++)
    {
        if ((edges[i][0] != edges[current][0]) ||
            (edges[i][1] != edges[current][1]))
        {
            current++;
            edges[current][0] = edges[i][0];
            edges[current][1] = edges[i][1];
        }
    }
    int numEdges = current + 1;
#endif
    

    // Compute mapping of ids from source_vert and destination_vert to
    // generated vert. This mapping would trivially say to replace all
    // occurances of source_vert and destination_vert with generated vert,
    // except we have to also worry about any number of coincident vertices
    // for each of these. So for each coincident vertex of the source_vert,
    // figure out which coincident vertex of the destination_vert it maps
    // to. We do this by looking at the destroyedTris. Each destroyed
    // triangle has an edge between a source and a destination vertex, and
    // collapsing that edge implies a mapping. If a coincident vertex of
    // the source or destination has no such triangle, then we need to make
    // a new generated vertex.
    

    xbsVertex *generated_vert = NULL;
    
    int numSourceCoincident = source_vert->numCoincident();
    int numDestCoincident = destination_vert->numCoincident();
    xbsVertex **sourceMappings = new xbsVertex *[numSourceCoincident];
    xbsVertex **destMappings = new xbsVertex *[numDestCoincident];
    for (int i=0; i<numSourceCoincident; i++)
        sourceMappings[i] = NULL;
    for (int i=0; i<numDestCoincident; i++)
        destMappings[i] = NULL;
    for (int i=0; i<numDestroyedTris; i++)
    {
        xbsTriangle *tri = destroyedTris[i];
        xbsVertex *smap = NULL;
        xbsVertex *dmap = NULL;
        for (int j=0; j<3; j++)
        {
            xbsVertex *vert = tri->verts[j];
            xbsVertex *min = vert->minCoincident();
            if (min == source_vert)
                smap = vert;
            else if (min == destination_vert)
                dmap = vert;
        }
        if ((smap == NULL) || (dmap == NULL))
        {
            // This is a triangle filtered out by duplicatedTriangle()
            // rather than one with an edge that is collapsed
            continue;
            
            //fprintf(stderr,
            //   "Destroyed tri doesn't connect source to destination vertex!\n");
            //exit(1);
        }
        int sindex = smap->coincidentIndex();
        int dindex = dmap->coincidentIndex();
        if ((sourceMappings[sindex] != NULL) &&
            (destMappings[dindex] != NULL))
            continue;
        if ((sourceMappings[sindex] != NULL) ||
            (destMappings[dindex] != NULL))
        {
            // good for debugging
//          fprintf(stderr,
//                  "warning: Source vert or destination vert has multiple mappings.\n");
        }

        xbsVertex *newvert = generateVertex(model, smap, dmap);
        if (generated_vert != NULL)
        {
            newvert->nextCoincident = generated_vert->nextCoincident;
            generated_vert->nextCoincident = newvert;
        }
        generated_vert = newvert;
        sourceMappings[sindex] = destMappings[dindex] = generated_vert;
    }
    currentCoincident = source_vert;
    for (int i=0; i<numSourceCoincident;
         i++, currentCoincident = currentCoincident->nextCoincident)
    {
        if (sourceMappings[i] != NULL)
            continue;

        // If the vertex is empty, just keep the mapping NULL
        // There can be empty vertices on the vertex that we have been
        // keeping around for VDS...
        if (currentCoincident->numTris == 0)
            continue;
        
        
        xbsVertex *newvert = generateVertex(model, currentCoincident, NULL);
        
        if (generated_vert != NULL)
        {
            newvert->nextCoincident = generated_vert->nextCoincident;
            generated_vert->nextCoincident = newvert;
        }
        generated_vert = newvert;
        sourceMappings[i] = generated_vert;
    }
    currentCoincident = destination_vert;
    for (int i=0; i<numDestCoincident;
         i++, currentCoincident = currentCoincident->nextCoincident)
    {
        if (destMappings[i] != NULL)
            continue;

        // If the vertex is empty, just keep the mapping NULL
        // There can be empty vertices on the vertex that we have been
        // keeping around for VDS...
        if (currentCoincident->numTris == 0)
            continue;

        xbsVertex *newvert = generateVertex(model, NULL, currentCoincident);
        
        if (generated_vert != NULL)
        {
            newvert->nextCoincident = generated_vert->nextCoincident;
            generated_vert->nextCoincident = newvert;
        }
        generated_vert = newvert;
        destMappings[i] = generated_vert;
    }


    generated_vert = generated_vert->minCoincident();

    // generateVertex() actually makes a new error data for every coincident vertex around the 
    // ring, but our policy is to only have an error data on the minCoincident vertex
    for (xbsVertex *current = generated_vert->nextCoincident;
         current != generated_vert;
         current = current->nextCoincident)
    {  
        delete current->errorData;
        current->errorData = NULL;
    }

    // add generated verts to the list of reduced verts, in case any are
    // generated with 0 tris
    xbsVertex **newReduced = new xbsVertex *[numReducedVerts+generated_vert->numCoincident()];
    for (int i=0; i<numReducedVerts; i++)
        newReduced[i] = reducedVerts[i];
    delete [] reducedVerts;
    reducedVerts = newReduced;
    xbsVertex *currentGen = generated_vert;
    do
    {
        reducedVerts[numReducedVerts++] = currentGen;
        currentGen = currentGen->nextCoincident;
    } while (currentGen != generated_vert);


    

    hierarchy->update(model, this, sourceMappings, destMappings,
                      changedTris, numChangedTris,
                      destroyedTris, numDestroyedTris,
                      generated_vert);
    
    
    // remove consumed tris from the vdata of all their adjacent vertices

    for (int i=0; i<numConsumedTris; i++)
    {
        xbsTriangle *tri = consumedTris[i];
        
        for (int j=0; j<3; j++)
        {
            if (tri->verts[j]->removeTri(tri) != 1)
            {
                fprintf(stderr,
                        "Error removing consumed tri from vdata.\n");
                exit(1);
            }
        }
    }


    currentCoincident = source_vert;
    do
    {
        currentCoincident->freeTris();
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != source_vert);
    currentCoincident = destination_vert;
    do
    {
        currentCoincident->freeTris();
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != destination_vert);
    

    delete [] consumedTris;
    consumedTris = NULL;
    numConsumedTris = 0;

    // conservatively allocate space for new tri lists
    currentCoincident = generated_vert;
    do
    {
        currentCoincident->reallocTris(numChangedTris);
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != generated_vert);
        
        

    // change the vertices of all the changed tris in the model
    for (int tnum=0; tnum<numChangedTris; tnum++)
    {
        xbsTriangle *tri = changedTris[tnum];

        for (int tvnum=0; tvnum<3; tvnum++)
        {
            xbsVertex *vert = tri->verts[tvnum];
            if (vert->minCoincident() == source_vert)
                tri->verts[tvnum] = sourceMappings[vert->coincidentIndex()];
            else if (vert->minCoincident() == destination_vert)
                tri->verts[tvnum] = destMappings[vert->coincidentIndex()];
        }
        
        // add new tri to index data of all its vertices
        for (int tvnum=0; tvnum<3; tvnum++)
            tri->verts[tvnum]->addTri(tri);
    }
    


    for (int tnum=0; tnum<numDestroyedTris; tnum++)
    {
        model->removeTri(destroyedTris[tnum]);
        delete destroyedTris[tnum];
    }

    delete [] destroyedTris;
    destroyedTris = NULL;
    numDestroyedTris = 0;

    delete [] changedTris;
    changedTris = NULL;
    numChangedTris = 0;


    
    // concatenate operation lists for generated vertex
    int newNumOps = source_vert->numOps + destination_vert->numOps;
    Operation **newOps = new Operation*[newNumOps];

    newNumOps = 0;
    for (int opnum=0; opnum<destination_vert->numOps; opnum++)
        newOps[newNumOps++] = destination_vert->ops[opnum];
    for (int opnum=0; opnum<source_vert->numOps; opnum++)
        newOps[newNumOps++] = source_vert->ops[opnum];

    delete [] source_vert->ops;
    source_vert->ops = NULL;
    source_vert->numOps = 0;

    delete [] destination_vert->ops;
    destination_vert->ops = NULL;
    destination_vert->numOps = 0;

    generated_vert->ops = newOps;
    generated_vert->numOps = newNumOps;
    
    // Map all occurances of the current operation's source and destination
    // vertex to the generated vertex.

    // DANGER - overwrites source_vert and destination_vert variables of
    // current operation!  don't use this->source_vert or
    // this->destination_vert again for current operation
    xbsVertex *sv = source_vert;
    xbsVertex *dv = destination_vert;
    for (int i=0; i<*numModOps; i++)
    {
        EdgeCollapse *op = (EdgeCollapse *)(*modOps)[i];
        if ((op->source_vert == sv) || (op->source_vert == dv))
            op->source_vert = generated_vert;
        if ((op->destination_vert == sv) || (op->destination_vert == dv))
            op->destination_vert = generated_vert;
    }
    for (int i=0; i<*numRemoveOps; i++)
    {
        EdgeCollapse *op = (EdgeCollapse *)(*removeOps)[i];
        if ((op->source_vert == sv) || (op->source_vert == dv))
            op->source_vert = generated_vert;
        if ((op->destination_vert == sv) || (op->destination_vert == dv))
            op->destination_vert = generated_vert;
    }


    // After the collapse, some of the neighborhood vertices may no longer
    // have any triangles, so we plan to delete them. But first, make sure
    // we will remove all operations using those vertices!

    // I think all such ops would have to already be listed in the mod or
    // remove ops. If it's currently a mod op, make it a remove op instead.
    
    for (int i=0; i<*numModOps; i++)
    {
        Operation *op = (*modOps)[i];
        xbsVertex *v1 = op->getSource();
        xbsVertex *v2 = op->getDestination();

        // Make sure there is actually still a triangle with an edge
        // connecting the source to the destination
        if (v1->coincidentIsAdjacent(v2))
            continue;

        // We need to move the operation from mod list to remove list

        // remove from mod list
        (*modOps)[i] = (*modOps)[*numModOps-1];
        (*numModOps)--;
        i--;

        // add to remove list
        Operation **newRemoveOps =
            new Operation *[(*numRemoveOps)+1];
        for (int opnum=0; opnum<(*numRemoveOps); opnum++)
            newRemoveOps[opnum] = (*removeOps)[opnum];
        newRemoveOps[(*numRemoveOps)++] = op;
        delete (*removeOps);
        (*removeOps) = newRemoveOps;
    }
    



    
    // Now we have to update the ops list of all the affected vertices. The
    // list is not changed by modOps, and we know for this operation type
    // that (numAddOps == 0), so just remove the removeOps from the
    // appropriate vertex lists

    // We could do this more efficiently, but for now, just do it by brute
    // force.

    for (int opnum=0; opnum<*numRemoveOps; opnum++)
    {
        Operation *op = (*removeOps)[opnum];
        xbsVertex *vert = op->getSource();

        int current = 0;
        for (int i=0; i<vert->numOps; i++)
        {
            if (vert->ops[i] != op)
                vert->ops[current++] = vert->ops[i];
        }
        
        if (current != (vert->numOps - 1))
        {
            fprintf(stderr, "Error removing op from vdata.\n");
            exit(1);
        }
        
        vert->numOps = current;

        if (vert->numOps == 0)
        {
            delete [] vert->ops;
            vert->ops = NULL;
        }
    }



    // Add generated vertex (vertices) to the model
    currentCoincident = generated_vert;
    do
    {
        model->addVert(currentCoincident);
        currentCoincident = currentCoincident->nextCoincident;
    } while (currentCoincident != generated_vert);
    
    
    
    // remove any empty vertices caused by the destroyed tris
    for (int reducedVertNum=0; reducedVertNum<numReducedVerts;
         reducedVertNum++)
    {
        xbsVertex *vert = reducedVerts[reducedVertNum];
        if (vert->numTris > 0)
            continue;



        // Okay, this is ugly, too. Because the current error metric uses a
        // heuristic based on how many coincident vertices there are, if we
        // change the number of non-empty coincident vertices of some
        // vertex, it can change the cost of all its adjacent edges (even
        // if this vertex was not the source or
        // destination). Unfortunately, it is hard to tell whether this
        // vertex was source or destination, or how many nonempty
        // coincident vertices there were in its coincident ring when we
        // started. So just make sure any ops referring to this vertex's
        // ring as source or destination are on the modOp list

        // are there any ops referring to this vert's ring?
        if (vert->coincidentNumTris() > 0)
        {
            currentCoincident = vert;
            do
            {
                for (int tnum=0; tnum<currentCoincident->numTris;
                     tnum++)
                {
                    xbsTriangle *tri = currentCoincident->tris[tnum];
                    for (int vnum=0; vnum<3; vnum++)
                    {
                        xbsVertex *tvert =
                            tri->verts[vnum]->minCoincident();
                        for (int opnum=0; opnum<tvert->numOps; opnum++)
                        {
                            Operation *op = tvert->ops[opnum];

                            // is op already on remove list?
                            int found = 0;
                            for (int i=0; i<*numRemoveOps; i++)
                            {
                                if ((*removeOps)[i] == op)
                                {
                                    found = 1;
                                    break;
                                }
                            }
                            if (found == 1)
                                continue;

                            // is op already on mod list?
                            found = 0;
                            for (int i=0; i<*numModOps; i++)
                            {
                                if ((*modOps)[i] == op)
                                {
                                    found = 1;
                                    break;
                                }
                            }
                            if (found == 1)
                                continue;


                            // add op to mod list
#if 1
                            //fprintf(stderr, "Adding to mod list\n");
                            Operation **newModOps =
                                new Operation *[(*numModOps)+1];
                            for (int opnum2=0; opnum2<(*numModOps); opnum2++)
                                newModOps[opnum2] = (*modOps)[opnum2];
                            newModOps[(*numModOps)++] = op;
                            delete (*modOps);
                            (*modOps) = newModOps;
#endif
                        }
                    }
                }
                currentCoincident = currentCoincident->nextCoincident;
            } while (currentCoincident != vert);
        }


        // For the sake of VDS, leave around empty vertices unless they are
        // on the source or destination vert


        
        
#if 0
        // this doesn't seem to get everything...
        // I guess once the original source or destination are removed from
        // the ring, there is no way left to figure out which ones are on
        // that ring
        if ((vert->minCoincident() != sv) &&
            (vert->minCoincident() != dv))
            continue;
#else
        // Okay, a bit of a hack for now
        // Only delete if VDS says we can. Discrete leaves this set to -1
        // all the time, so that will have the happy side effect of letting
        // us clean up properly as we go
        if (hierarchy->getHierarchyType() == VDS_Hierarchy &&
            vert->mtIndex != -1)
            continue;
#endif

        // remove any responsibilities of this vertex as a minCoincident
        // -- move any operations to the new minCoincident
                
        if (vert->minCoincident() == vert)
        {
            if (vert->coincidentNumTris() > 0)
            {
                // There are still some ops for this vertex, either
                // directly on it or on one of its adjacent vertices. We
                // need to move them to the next available vertex.

                // find the vertex who will become minCoincident
                xbsVertex *min = vert->minCoincidentExceptThis();
                /*
                if (min->errorData == NULL){
                    if (model->errorMetric==GLOD_METRIC_SPHERES)
                        min->errorData = new SphereErrorData(min);
                    else if (model->errorMetric==GLOD_METRIC_QUADRICS)
                        min->errorData = new QuadricErrorData(min);
                }
                 */
                if (min == NULL)
                {
                    fprintf(stderr, "No new coincident available!\n");
                    exit(1);
                }

                // re-map all ops that may refer to this vertex to the new
                // minCoincident
                int foundCount = 0;
                currentCoincident = vert;
                do
                {
                    for (int tnum=0; tnum<currentCoincident->numTris;
                         tnum++)
                    {
                        xbsTriangle *tri = currentCoincident->tris[tnum];
                        for (int vnum=0; vnum<3; vnum++)
                        {
                            xbsVertex *tvert =
                                tri->verts[vnum]->minCoincident();
                            for (int opnum=0; opnum<tvert->numOps; opnum++)
                            {
                                EdgeCollapse *op = (EdgeCollapse*) tvert->ops[opnum];
                                if (op->source_vert == vert)
                                {
                                    op->source_vert = min;
                                    foundCount++;
                                }
                                if (op->destination_vert == vert)
                                {
                                    op->destination_vert = min;
                                    foundCount++;
                                }
                            }
                        }
                    }
                    currentCoincident = currentCoincident->nextCoincident;
                } while (currentCoincident != vert);

                // For the full edge collapse, we actually have no idea how
                // big the foundCount should be compared to the
                // vert->numOps (for the half edge collapse, each edge gets
                // two operations, so foundCount is always 2*vert->numOps
                
                min->ops = vert->ops;
                min->numOps = vert->numOps;
                //min->errorData->setError(vert->errorData->getError());
                min->errorData = vert->errorData;
                vert->errorData = NULL;
                vert->ops = NULL;
                vert->numOps = 0;
            }
        }

        // remove the vert from the coincident ring
        for (currentCoincident = vert;
             currentCoincident->nextCoincident != vert;
             currentCoincident = currentCoincident->nextCoincident);
//      fprintf(stderr, "here2\n");
        currentCoincident->nextCoincident = vert->nextCoincident;
        vert->nextCoincident = vert;

#if 0
        // debugging test (SLOW)
        for (int i=0; i<model->getNumVerts(); i++)
        {
            xbsVertex *vert = model->getVert(i);
            for (int j=0; j<vert->numOps; j++)
            {
                Operation *op = vert->ops[j];
                if ((op->source_vert->index == -1) ||
                    (op->destination_vert->index == -1))
                    fprintf(stderr, "Deleted vert on op\n");
            }
        }
#endif
        
        // remove vertex from model and delete it
        model->removeVert(vert);
        delete vert;

#if 0
        // debugging test (SLOW)
        for (int i=0; i<model->getNumVerts(); i++)
        {
            xbsVertex *vert1 = model->getVert(i);
            for (int j=0; j<vert1->numOps; j++)
            {
                Operation *op = vert1->ops[j];
                if ((op->source_vert->index == -1) ||
                    (op->destination_vert->index == -1))
                {
                    fprintf(stderr, "Deleted vert on op\n");
                    if (op->destination_vert == vert)
                        fprintf(stderr,
                                "It's the one just deleted!\n");
                }
            }
        }
#endif

    }

    delete [] reducedVerts;
    //delete [] vertMappings;
    delete [] sourceMappings;
    delete [] destMappings;
    
    
    return;
} /** End of EdgeCollapse::updateModel() **/

EdgeCollapseCase
EdgeCollapse::computeCase(Model *model)
{
    int sourceOnBorder = source_vert->onBorder();
    int destOnBorder = destination_vert->onBorder();
    
    int nc1 = source_vert->numNonEmptyCoincident() + sourceOnBorder;
    int nc2 = destination_vert->numNonEmptyCoincident() + destOnBorder;

    if ((nc1 == nc2) &&
        ((model->borderLock == 0) || 
         ((sourceOnBorder==0) && (destOnBorder==0)))) 
    {
        // general case -- both vertices can move
        
        if ((nc1 == nc2) && (nc1 != 1))
        {
            xbsVertex *current = source_vert;
            xbsVertex *v2min = destination_vert->minCoincident();
            do 
            {
                int count = 0;
                for (int i=0; i<current->numTris; i++)
                {
                    xbsTriangle *tri = current->tris[i];
                    if ((tri->verts[0]->minCoincident() == v2min) ||
                        (tri->verts[1]->minCoincident() == v2min) ||
                        (tri->verts[2]->minCoincident() == v2min))
                        count++;
                    if (count > 1)
                        return MoveNeither;
                }
                current = current->nextCoincident;
            } while (current != source_vert);

            current = destination_vert;
            v2min = source_vert->minCoincident();
            do 
            {
                int count = 0;
                for (int i=0; i<current->numTris; i++)
                {
                    xbsTriangle *tri = current->tris[i];
                    if ((tri->verts[0]->minCoincident() == v2min) ||
                        (tri->verts[1]->minCoincident() == v2min) ||
                        (tri->verts[2]->minCoincident() == v2min))
                        count++;
                    if (count > 1)
                        return MoveNeither;
                }
                current = current->nextCoincident;
            } while (current != destination_vert);
        }
            
        return MoveBoth;
        

#if 0
        if (source_vert->numNonEmptyCoincident() != 1)
        {
            int patch_num = -1;
            int patchBoundary = 0;
            xbsVertex *current = source_vert;
            xbsVertex *v2min = destination_vert->minCoincident();
            do 
            {
                for (int i=0; i<current->numTris; i++)
                {
                    xbsTriangle *tri = current->tris[i];
                    if ((tri->verts[0]->minCoincident() == v2min) ||
                        (tri->verts[1]->minCoincident() == v2min) ||
                        (tri->verts[2]->minCoincident() == v2min))
                    {
                        if (patch_num == -1)
                            patch_num = tri->patchNum;
                        else if (patch_num != tri->patchNum)
                            patchBoundary = 1;
                    }
                }
                current = current->nextCoincident;
            } while ((current != source_vert) && (patchBoundary == 0));
            
            if (patchBoundary == 0)
                error->setError(MAXFLOAT); // = MAXFLOAT;
            
        }
#endif
    }
    else if ((nc1 <= nc2) &&
             ((model->borderLock == 0) || (sourceOnBorder == 0)))
    {
        // only source_vert moves
        
        return MoveSource;
    }
    else if ((nc1 >= nc2) &&
             ((model->borderLock == 0) ||
              (destOnBorder == 0)))
    {
        // only destination_vert moves
        
        return MoveDestination;
    }
//    else
//    {
        // no vertices can move :-(

        return MoveNeither;
//    }
}


/*****************************************************************************\
 @ EdgeCollapse::computeCost
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
EdgeCollapse::computeCost(Model *model)
{
    if (error==NULL){
        if (model->errorMetric==GLOD_METRIC_SPHERES)
            error = new SphereEdgeError();
        else if (model->errorMetric==GLOD_METRIC_QUADRICS)
            error = new QuadricEdgeError();
        else if (model->errorMetric==GLOD_METRIC_PERMISSION_GRID)
            error = new PermissionGridEdgeError();
    }

    EdgeCollapseCase ECcase = computeCase(model);

    switch (ECcase)
    {
        case MoveBoth:
        {
            error->calculateError(model, this);
            break;
        }
        case MoveSource:
        {
            Operation::computeCost(model);
            break;
        }
        case MoveDestination:
        {
            xbsVertex *temp;
            temp = destination_vert;
            destination_vert = source_vert;
            source_vert = temp;
            
            Operation::computeCost(model);
            
            temp = destination_vert;
            destination_vert = source_vert;
            source_vert = temp;     
            break;
        }
        case MoveNeither:
        {
            error->setError(MAXFLOAT);
            break;
        }
        default:
        {
            fprintf(stderr, "Invalid EdgeCollapseCase\n");
            exit(1);
            break;
        }
    }
    
    dirty = 0;
} /** End of EdgeCollapse::computeCost() **/

/*****************************************************************************\
 @ EdgeCollapse::generateVertex
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
xbsVertex *
EdgeCollapse::generateVertex(Model *model,
                             xbsVertex *v1, xbsVertex *v2)
{
    return error->genVertex(model, v1,v2, this, 1);
} /** End of EdgeCollapse::generateVertex() **/



/*****************************************************************************\
  $Log: Operation.C,v $
  Revision 1.33  2004/10/20 19:33:34  gfx_friends
  Rewrote MTHierarchy driver, made the VDS hack in Operation.C vds-specific

  Revision 1.32  2004/07/22 16:38:01  jdt6a
  prelimary permission grid stuff is set up.  now to integrate this into glod for real.

  Revision 1.31  2004/07/19 16:43:21  gfx_friends
  Memory leak patches. -n

  Revision 1.30  2004/07/19 16:03:07  gfx_friends
  Fixed a memory leak: errordata being allocated in an unnecessary place.

  Revision 1.29  2004/07/15 12:52:29  gfx_friends
  Moved check for cost out of date up the call stack so we can avoid
  performing operations out of order. Also, shut up the warning about out of
  date costs so everyone can go back to thinking that xbs just works. :-P

  Revision 1.28  2004/07/14 18:43:33  gfx_friends
  Nothing to see here...

  Revision 1.27  2004/07/14 14:59:51  gfx_friends
  Made handling of border heuristics more consistent. Cylinder now
  simplifies again, and the torus patch borders work pretty well, too.
  The case where borders are not preserved too well right now is using
  full edge collapses and error quadrics, because the location of the
  generated vertex does not in general lie along the collapsed edge
  (it is chosen by an optimization involving an inversion of the quadric
  matrix, yada yada yada). We may improve this by adding additional border
  edge planes into the quadric, as done in some papers by Garland and
  Lindstrom.

  Revision 1.26  2004/07/08 20:18:45  gfx_friends
  couple small changes to the error metric code potentially fixing up problems with errors not getting assigned to new vertices properly in special cases

  Revision 1.25  2004/07/08 16:47:49  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.24  2004/07/07 14:45:16  gfx_friends
  another fix for un-allocated error data.. kinda hacky, but oddly enough only gives problems in windows

  Revision 1.23  2004/07/06 23:27:53  gfx_friends
  fix for a problem rich holloway reported. turns out it was possible for edge collapse mode not to initialize an error value... hopefully this will set it straight

  Revision 1.22  2004/07/06 19:36:52  gfx_friends
  edge collapse memory leak fix

  Revision 1.21  2004/07/01 03:40:03  gfx_friends
  fix for some unalocated memory problems with the new error code

  Revision 1.20  2004/06/30 16:00:17  gfx_friends
  more memory leaks fixed

  Revision 1.19  2004/06/30 02:11:11  gfx_friends
  first of many memory leak fixes

  Revision 1.18  2004/06/24 21:49:24  gfx_friends
  Added a new metric, quadric errors. Also a major redesign of the error calculation/storage functions, which are now in their own class

  Revision 1.17  2004/06/11 18:30:08  gfx_friends
  Remove all sources of warnings in xbs directory when compiled with -Wall

  Revision 1.16  2004/06/03 19:04:08  gfx_friends
  Added a "border lock" mode to prevent xbs from moving/removing any
  vertices on a geometric border. The determination of whether or not
  something is on a geometric border is somewhat heuristic. It is not
  clear what we want to call a border in the presence of various sorts
  of non-manifold vertices (which may be created by xbs, even if the
  orinal model is manifold).

  To use border lock mode, set the object's GLOD_BUILD_BORDER_MODE to
  GLOD_BORDER_LOCK before building.

  Revision 1.15  2004/03/12 17:00:53  gfx_friends
  First stab at using bounding boxes for error calculations. VDS can finally once again be checked out (i hope...)

  Revision 1.14  2004/01/30 03:10:15  gfx_friends
  Jon Cohen -

  Modified heuristics which favor patch corners over patch edges over
  patch interiors. It now preserves corners and edges to work better
  with seamless grid atlases. I still do not entirely prevent merges
  which combine two corners of a patch, so patches can ultimately
  collapse away. We could probably add parameters to make this more
  configurable.

  Note: This will make the igea normal mapping demo in the SIGGRAPH
  submission video look better because the normal maps will remain
  seamless until patches really begin to collapse to nothing.

  Revision 1.13  2003/07/26 01:17:44  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.12  2003/07/25 03:00:15  gfx_friends
  Jon Cohen -

  Cleaned up sharing a bit more -- now remove vertices which no longer
  have any triangles.

  Removed all #define VDS_CONTEXT nonsense. VDS no longer #includes
  anything from GLOD, so it is not necessary.

  Fixed some subtle problems with simplifying complicated
  multi-attribute models, like brain-gear.normals.ply (which has normals
  computed with sharp angle splitting). This model still trips some
  warning messages (oops4 and oops2) when I simplify to VDS_Hierarchy
  with Half_Edge_Collapse, but for now the program seems to recover well
  enough to generate a working VDS. (the warnings basically indicate
  that some vertices were not removed from the Model even after they
  have been merged in the VIF). But it's a sign that something still
  goes wrong in xbs occasionally.

  Shut up warning messages about things that are now considered okay.

  Revision 1.11  2003/07/23 19:55:34  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.10  2003/07/22 21:35:22  gfx_friends
  Removed outdated debugging call that was now causing crashes under Windows.

  Revision 1.9  2003/07/19 23:50:11  gfx_friends
  Jon Cohen:

  Fixed up output to VDS hierarchy from half edge collapse and edge
  collapse operators. Seems to work, but it's still subject to review by
  Brenden. Also fixed a bug in vertex sharing.

  Revision 1.8  2003/07/18 22:19:43  gfx_friends
  Fixed most of the build problems. The lights have mysteriously gone off in the simple test program... I'm trying to figure out why. But the rest works, I think

  Revision 1.7  2003/07/16 03:12:29  gfx_friends
  Added xbs support for "multi-attribute vertices". These are
  geometrically coincident vertices that may have different
  attributes. Geometric coincidence is maintained throughout the
  simplification process and attributes are correctly propagated along.

  For the full edge collapse, the heuristics for preventing attribute
  seams along patch boundaries could still use a little work.

  Things seem to work for the DiscreteHierarchy output. VDS hierarchy
  has not been integrated yet.

  Revision 1.6  2003/06/12 16:55:56  gfx_friends
  Fixed minor bug with xbs that delete memory that still had pointers to it floating around.

  Revision 1.5  2003/06/05 17:39:01  gfx_friends
  Patches to build on Win32.

  Revision 1.4  2003/01/15 20:28:23  gfx_friends
  Fixed problem with HalfEdgeCollapse (turned back on something that
  had been commented out)

  Revision 1.3  2003/01/14 23:48:05  gfx_friends
  Part of my grand scheme to take over the world.

  Revision 1.2  2003/01/14 00:06:20  gfx_friends
  Added destructors.

  Revision 1.1  2003/01/13 20:30:15  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.3  2003/01/08 05:19:14  cohen
  Added first version of full edge collapse.

  Revision 1.2  2003/01/05 22:40:35  cohen
  no changes

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/
