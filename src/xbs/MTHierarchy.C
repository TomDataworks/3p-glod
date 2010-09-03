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
/*****************************************************************************\
  MT.C
  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/MTHierarchy.C,v $
  $Revision: 1.4 $
  $Date: 2004/10/20 20:01:38 $
  $Author: gfx_friends $
\*****************************************************************************/

#include "xbs.h"
#include "MTHierarchy.h"

/*****************************************************************************\
 @ compare_tri_end_nodes
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
static int compare_tri_end_nodes(const void *a, const void *b)
{
    const xbsTriangle *tri_a = *(const xbsTriangle **)a;
    const xbsTriangle *tri_b = *(const xbsTriangle **)b;

    int endNodeIndex_a = tri_a->endNodeIndex;
    int endNodeIndex_b = tri_b->endNodeIndex;

    int patch_a = tri_a->patchNum;
    int patch_b = tri_b->patchNum;
    
    return
        ((endNodeIndex_a < endNodeIndex_b) ? -1 :
         ((endNodeIndex_a > endNodeIndex_b) ? 1 :
          ((patch_a < patch_b) ? -1 :
           ((patch_a > patch_b) ? 1 :
            ((tri_a < tri_b) ? -1 :
             ((tri_a > tri_b) ? 1 : 0))))));
    
} /** End of compare_tri_end_nodes() **/

/*****************************************************************************\
 @ MTHierarchy::initialize
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
MTHierarchy::initialize(Model *model)
{
    int drainIndex = mt->addNode();
    
    // add each vertex to the MT
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);

        xbsVertex *cur = vert;
        do {
            mtVertex *mtvert = cur->makeMTVertex();
            cur->mtIndex = mt->addVertex(*mtvert);
            delete mtvert;
            cur = cur->nextCoincident;
        } while(cur != vert);
    }

    // add each triangle to the MT
    for (int tnum=0; tnum<model->getNumTris(); tnum++)
    {
        xbsTriangle *tri = model->getTri(tnum);
        tri->mtIndex = mt->addTriangle(tri->verts[0]->mtIndex,
                                       tri->verts[1]->mtIndex,
                                       tri->verts[2]->mtIndex);
        tri->endNodeIndex = drainIndex;
    }
    
    return;
} /** End of MTHierarchy::initialize() **/



/*****************************************************************************\
 @ MTHierarchy::finalize
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
MTHierarchy::finalize(Model *model)
{
    // XBS is not guaranteed to consume all triangles in the model....
    // Thus, there are orphan triangles... these need to be assigned
    // to arcs between their end_node and the root node... this set of
    // orphan triangles includes (a) tris who'se endnodes have no
    // parents... and (b) triangles whose endnodes have a partial set
    // of parents.

    int numNodes = mt->getNumNodes();
    int numArcs  = mt->getNumArcs();
    
    int rootIndex = mt->addNode();
    mtNode *root = mt->getNode(rootIndex);
    root->setError(MAXFLOAT);
    mt->setRoot(rootIndex);

    int numConsumedTris = model->getNumTris();
    xbsTriangle** consumedTris = new xbsTriangle*[model->getNumTris()];
    for(int i = 0; i < model->getNumTris(); i++)
        consumedTris[i] = model->getTri(i);
    qsort(consumedTris, numConsumedTris, sizeof(xbsTriangle*), compare_tri_end_nodes);    
    
    // build arcs for every group of triangles with the same patch number
    int firstTnum = 0;
    for (int tnum=0; tnum<numConsumedTris; tnum++) {
        xbsTriangle *tri, *nextTri;
        
        tri = consumedTris[tnum];
        if (tnum != numConsumedTris - 1)
            nextTri = consumedTris[tnum+1];
        else
            nextTri = NULL;
        
        if ((nextTri == NULL) ||
            (tri->endNodeIndex != nextTri->endNodeIndex) ||
            (tri->patchNum != nextTri->patchNum)) {
            // create a new arc
            int arcIdx = mt->addArc();
            mtArc *arc = mt->getArc(arcIdx);
            arc->setStart(rootIndex);

            arc->setEnd(tri->endNodeIndex);
            //            fprintf(stderr, "Fixed node %i\n", tri->endNodeIndex);
            
            arc->setPatchNumber(tri->patchNum);
            int *tmp = new int[tnum-firstTnum+1];
            //fprintf(stderr, "  Patch %i:", tri->patchNum);
            for (int tnum2=0; tnum2<tnum-firstTnum+1; tnum2++){
                tmp[tnum2] = consumedTris[firstTnum+tnum2]->mtIndex;
                //fprintf(stderr, "%i ", tmp[tnum2]);
            }
            //fprintf(stderr, "\n");
            arc->addTris(tmp, tnum-firstTnum+1);
            delete [] tmp;
            firstTnum = tnum+1;
        }
    }

    // this may not have connected all of the nodes...
    {
        char *nodeHasParent = new char [numNodes];
        for (int i=0; i<mt->getNumNodes(); i++)
            nodeHasParent[i] = 0;
        for (int arcnum = 0; arcnum<mt->getNumArcs(); arcnum++) {
            mtArc *arc = mt->getArc(arcnum);
            nodeHasParent[arc->getEnd()] = 1;
            if ((arc->getStart() < 0) || (arc->getStart() >= numArcs)) {
                fprintf(stderr, "finalize(): Arc has no parent!\n");
                exit(1);
            }
        }
        
        for (int nodenum=0; nodenum<numNodes; nodenum++) {
            if (nodeHasParent[nodenum] == 1)
                continue;
            //            fprintf(stderr, "Fixed node %i\n", nodenum);
            int arcIndex = mt->addArc();
            mtArc *arc = mt->getArc(arcIndex);
            arc->setEnd(nodenum);
            arc->setStart(rootIndex);
        }

        delete [] nodeHasParent;
    }

    // CONNECT THE MT ARCS
    mt->connectArcs();


    return;
    
} /** End of MTHierarchy::finalize() **/


/*****************************************************************************\
 @ MTHierarchy::update
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
MTHierarchy::update(Model *model, Operation *op,
                    xbsVertex **sourceMappings,
                    xbsTriangle **changedTris, int numChangedTris,
                    xbsTriangle **destroyedTris, int numDestroyedTris)
{
    xbsVertex *source_vert = op->getSource();
    xbsVertex *destination_vert = op->getDestination();
    
    // create new mtNode for this operation
    int nodeIndex = mt->addNode();
    mtNode *node = mt->getNode(nodeIndex);
    node->setError(op->getCost());
    
    // we need to build a list of triangles...
    int numConsumedTris = numChangedTris + numDestroyedTris;
    xbsTriangle **consumedTris = new xbsTriangle *[numConsumedTris];
    memcpy(consumedTris, changedTris, numChangedTris * sizeof(xbsTriangle*));
    memcpy(consumedTris + numChangedTris, destroyedTris, numDestroyedTris * sizeof(xbsTriangle*));
    
    // sort consumed triangles so we can create merged arcs
    qsort(consumedTris, numConsumedTris,
          sizeof(xbsTriangle *), compare_tri_end_nodes);

    if(nodeIndex == 1408) {
        //        printf("Foom!\n");
    }
    
    // create arcs - one for each unique end node index, and add the
    // triangles
    int firstTnum = 0;
    for (int tnum=0; tnum<numConsumedTris; tnum++)
    {
        xbsTriangle *tri, *nextTri;

        tri = consumedTris[tnum];
        if (tnum != numConsumedTris - 1)
            nextTri = consumedTris[tnum+1];
        else
            nextTri = NULL;
        
        if ((nextTri == NULL) ||
            (tri->endNodeIndex != nextTri->endNodeIndex) ||
            (tri->patchNum != nextTri->patchNum))
        {
            // create a new arc
            int arcIdx = mt->addArc();
            mtArc *arc = mt->getArc(arcIdx);
            arc->setStart(nodeIndex);
            arc->setEnd(tri->endNodeIndex);
            
            arc->setPatchNumber(tri->patchNum);
            int *tris = new int[tnum-firstTnum+1];
            //            printf("Arc %5i to node %5i (patch %5i):\n", arcIdx,
            //tri->endNodeIndex, tri->patchNum);
            for (int tnum2=0; tnum2<tnum-firstTnum+1; tnum2++){
                tris[tnum2] = consumedTris[firstTnum+tnum2]->mtIndex;
                /*printf(" tri %i (p=%i) (end=%i)\n", 
                       consumedTris[firstTnum+tnum2]->mtIndex,
                       consumedTris[firstTnum+tnum2]->patchNum,
                       consumedTris[firstTnum+tnum2]->endNodeIndex);*/
                       
            }
            //printf("\n");

            arc->addTris(tris, tnum-firstTnum+1);
            delete [] tris;
            firstTnum = tnum+1;
        }
    }
    
    //    printf("\n");
    // create new mtTriangle for each triangle produced
    // (modified) by this operation
    
    for (int tnum=0; tnum<numChangedTris; tnum++)
    {   
        xbsTriangle *tri = changedTris[tnum];
        int indices[3];
        for (int tvnum=0; tvnum<3; tvnum++)
        {
            xbsVertex* curVert = tri->verts[tvnum];
            xbsVertex* realVert;
            int matchOffset;
            if (source_vert->equals(curVert, &matchOffset)) {
                // but this vert maps to sourceMappings[matchOffset]
                realVert = sourceMappings[matchOffset];
            } else
                realVert = curVert;
            if(realVert->mtIndex == -1) {
                mtVertex *mtvert = realVert->makeMTVertex();
                realVert->mtIndex = mt->addVertex(*mtvert);
                delete mtvert;
            }
            indices[tvnum] = realVert->mtIndex;
        }
        
        tri->mtIndex = mt->addTriangle(indices[0],
                                       indices[1],
                                       indices[2]);
        tri->endNodeIndex = nodeIndex;
    }
    
    for (int i = 0; i < numDestroyedTris; i++)
        destroyedTris[i]->mtIndex = -1;
    delete [] consumedTris;
    return;
} /** End of MTHierarchy::update() **/

/*****************************************************************************\
 @ MTHierarchy::update
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
MTHierarchy::update(Model *model, EdgeCollapse *op, 
                    xbsVertex **sourceMappings, xbsVertex **destMappings,
                    xbsTriangle **changedTris, int numChangedTris,
                    xbsTriangle **destroyedTris, int numDestroyedTris,
                    xbsVertex *generated_vert)
{
    xbsVertex *source_vert = op->getSource();
    xbsVertex *destination_vert = op->getDestination();
    
    // create new mtNode for this operation

    int nodeIndex = mt->addNode();
    mtNode *node = mt->getNode(nodeIndex);
    node->setError(op->getCost());

    // sort consumed triangles so we can create merged arcs
    int numConsumedTris = numChangedTris + numDestroyedTris;
    xbsTriangle **consumedTris = new xbsTriangle *[numConsumedTris];
    memcpy(consumedTris, changedTris, numChangedTris * sizeof(xbsTriangle*));
    memcpy(consumedTris + numChangedTris, destroyedTris, numDestroyedTris * sizeof(xbsTriangle*));
    
    /*    int current = 1;
    for (int tnum=1; tnum<numConsumedTris; tnum++)
    {
        if (consumedTris[current-1] != consumedTris[tnum])
            consumedTris[current++] = consumedTris[tnum];
    }
    numConsumedTris = current;*/
    
        
    // create arcs - one for each unique end node index, and add the
    // triangles

    int firstTnum = 0;
    for (int tnum=0; tnum<numConsumedTris; tnum++)
    {
        xbsTriangle *tri, *nextTri;
        
        tri = consumedTris[tnum];
        if (tnum != numConsumedTris - 1)
            nextTri = consumedTris[tnum+1];
        else
            nextTri = NULL;

        if ((nextTri == NULL) ||
            (tri->endNodeIndex != nextTri->endNodeIndex) ||
            (tri->patchNum != nextTri->patchNum))
        {
            // create a new arc
            mtArc *arc = mt->getArc(mt->addArc());
            arc->setStart(nodeIndex);
            arc->setEnd(tri->endNodeIndex);
            arc->setPatchNumber(tri->patchNum);
            int *tris = new int[tnum-firstTnum+1];
            for (int tnum2=0; tnum2<tnum-firstTnum+1; tnum2++)
                tris[tnum2] = consumedTris[firstTnum+tnum2]->mtIndex;
            arc->addTris(tris, tnum-firstTnum+1);
            delete [] tris;
            firstTnum = tnum+1;
        }
    }
    
    
    // Make lists of which triangles are changed and which are destoyed
    // (all are actually consumed by this operation, but some are not
    // replaced by new triangles)

    /*    mtVertex *mtvert = generated_vert->makeMTVertex();
    generated_vert->mtIndex = mt->addVertex(*mtvert);
    delete mtvert;*/
    
    // create new mtTriangle for each triangle produced
    // (modified) by this operation
    for (int tnum=0; tnum<numChangedTris; tnum++)
    {   
        xbsTriangle *tri = changedTris[tnum];
        int indices[3];
        for (int tvnum=0; tvnum<3; tvnum++)
        {
            xbsVertex* curVert = tri->verts[tvnum];
            xbsVertex* realVert;
            int matchOffset;
            if (source_vert->equals(curVert, &matchOffset)) {
                realVert = sourceMappings[matchOffset];                
            } else if(destination_vert->equals(curVert, &matchOffset)) {
                realVert = destMappings[matchOffset];                
            } else
                realVert = curVert;
            if(realVert->mtIndex == -1) {
                mtVertex *mtvert = realVert->makeMTVertex();
                realVert->mtIndex = mt->addVertex(*mtvert);
                delete mtvert;
            }
            indices[tvnum] = realVert->mtIndex;
        }
        
        tri->mtIndex = mt->addTriangle(indices[0],
                                       indices[1],
                                       indices[2]);
        tri->endNodeIndex = nodeIndex;
    }
    
    delete [] consumedTris;

    return;
} /** End of MTHierarchy::update() **/


/***************************************************************************
 $Log: MTHierarchy.C,v $
 Revision 1.4  2004/10/20 20:01:38  gfx_friends
 Full edge collapses on MT work, too.

 Revision 1.3  2004/10/20 19:43:13  gfx_friends
 MT patch is complete. XBS standalone works. -nat

 Revision 1.2  2004/10/20 19:33:31  gfx_friends
 Rewrote MTHierarchy driver, made the VDS hack in Operation.C vds-specific

 Revision 1.1  2004/08/23 16:47:09  gfx_friends
 Pulled MT into a new file so that I can do some experiments with the shortcut algorithm.--nat

 ***************************************************************************/
