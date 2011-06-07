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
#include "DiscretePatch.h"
#include "xbs.h"

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

#define UPDATE_MINMAX(lo, hi, vecname) \
                if (vecname[0]>hi[0]) hi[0]=vecname[0];\
                if (vecname[1]>hi[1]) hi[1]=vecname[1];\
                if (vecname[2]>hi[2]) hi[2]=vecname[2];\
                if (vecname[0]<lo[0]) lo[0]=vecname[0];\
                if (vecname[1]<lo[1]) lo[1]=vecname[1];\
                if (vecname[2]<lo[2]) lo[2]=vecname[2];

/*****************************************************************************\
 @ DiscretePatchLevel::DiscretePatchLevel
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : Notice that because we match up vertices with patches by
               looking at the triangles, any unused/empty vertices of the
               Model are not carried over into the DiscretePatchLevel.
\*****************************************************************************/
DiscretePatchLevel::DiscretePatchLevel(DiscretePatchHierarchy* hierarchy,  Model *model)
{
    this->hierarchy = hierarchy;
    xbsVec3 v_max(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT), v_min(MAXFLOAT, MAXFLOAT, MAXFLOAT);
    
    char hasColor, hasNormal, hasTexcoord;
    model->hasAttributes(hasColor, hasNormal, hasTexcoord);
    
    numPatches = model->getNumPatches();

    patches = new DiscretePatchPatch[numPatches];
    for(int i =0; i < numPatches; i++) {
        patches[i].Init((DiscretePatchLevel*)this, i, hasColor == 1, hasNormal == 1, hasTexcoord == 1);
    }
    
    numTris = model->getNumTris();

    // count triangles for each patch
    int *patchNumTris = new int[numPatches];
    for (int pnum=0; pnum<numPatches; pnum++)
        patchNumTris[pnum] = 0;
    for (int tnum=0; tnum<numTris; tnum++)
        patchNumTris[model->getTri(tnum)->patchNum]++;

    int **patchTris = new int *[numPatches];
    for (int pnum=0; pnum<numPatches; pnum++)
    {
        patchTris[pnum] = new int[patchNumTris[pnum]];
        patchNumTris[pnum] = 0;
    }
    for (int tnum=0; tnum<numTris; tnum++)
    {
        int patchNum = model->getTri(tnum)->patchNum;
        patchTris[patchNum][patchNumTris[patchNum]++] = tnum;
    }

    // note: we have to be careful, because the same Model vertex may be
    // used in multiple patches!
    
    char *vertIsInPatch = new char[model->getNumVerts()];
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
        vertIsInPatch[vnum] = 0;    
    int *vertIDs = new int[model->getNumVerts()];
    
    for (int pnum=0; pnum<numPatches; pnum++)
    {
        // per-patch bounding-box
        xbsVec3 p_max(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT), p_min(MAXFLOAT, MAXFLOAT, MAXFLOAT);
        DiscretePatchPatch *patch = &(patches[pnum]);
        patch->patchNum = pnum;

        // which verts are in this patch
        int patch_numVerts = 0;
        for (int tnum=0; tnum<patchNumTris[pnum]; tnum++)
            {
                xbsTriangle *tri = model->getTri(patchTris[pnum][tnum]);
                for (int vnum=0; vnum<3; vnum++)
                    if (vertIsInPatch[tri->verts[vnum]->index] == 0) //note, vertmarked always starts out at zero
                        {
                            patch_numVerts++;
                            vertIsInPatch[tri->verts[vnum]->index] = 1;
                        }
            }

        // are we a half-edge-created object or full-edge?
        int *patchVerts = new int[patch_numVerts];
        if(hierarchy->opType == Half_Edge_Collapse && hierarchy->numUsedLODs != 0) {
            DiscretePatchPatch* basePatch = &hierarchy->LODs[0]->patches[pnum];
            // find any new vertices that are in this patch but not the base patch
            // if any are found, create them
            for (int tnum=0; tnum<patchNumTris[pnum]; tnum++) {
                xbsTriangle *tri = model->getTri(patchTris[pnum][tnum]);
                for (int vnum=0; vnum<3; vnum++) {
                    if (vertIsInPatch[tri->verts[vnum]->index] == 0) continue;
                    vertIsInPatch[tri->verts[vnum]->index] = 0;
                    
                    // now.. are they in the base patch?
                    int vertLocalIndex = tri->verts[vnum]->mtIndex;
                    if(vertLocalIndex == -1) {
                        // add the new vert to the base patch
                        int newVertIdx = basePatch->verts.addVert();
                        basePatch->SetFrom(newVertIdx, tri->verts[vnum]);
                        
                        // set the bookkeeping so that we can associate triangles to these...
                        tri->verts[vnum]->mtIndex = newVertIdx; // used for tracking in Half-Edge mode
                        vertLocalIndex = newVertIdx;
                    }
                    
                    float* coord = basePatch->verts.getCoord(vertLocalIndex);
                    UPDATE_MINMAX(v_min, v_max, coord);
                    UPDATE_MINMAX(p_min, p_max, coord);
                }
            }
            
            // build the index list
            patch->indices = new unsigned int[patchNumTris[pnum]*3];
            patch->numIndices = 0;
            for (int tnum=0; tnum<patchNumTris[pnum]; tnum++) {
                xbsTriangle *tri = model->getTri(patchTris[pnum][tnum]);

                patch->indices[patch->numIndices++] = tri->verts[0]->mtIndex;
                patch->indices[patch->numIndices++] = tri->verts[1]->mtIndex;
                patch->indices[patch->numIndices++] = tri->verts[2]->mtIndex;
            }

            // update the per-patch error
            patch->errorCenter = (p_max + p_min) * 0.5;
            patch->errorOffsets= p_max - patch->errorCenter;
            /*
            printf("Patch %i Min:%f %f %f\n", pnum, p_min[0], p_min[1], p_min[2]);
            printf("Patch %i Max:%f %f %f\n", pnum, p_max[0], p_max[1], p_max[2]);
            */

        } else {            // build the base patch if in half_edge and build all patches if in full_edge

            // xxx: is patch_numVerts the correct size already?
            patch->SetSize(patch_numVerts);
            
            // assign verts indices in this patch  (and count verts in patch)
            for (int tnum=0; tnum<patchNumTris[pnum]; tnum++) {
                xbsTriangle *tri = model->getTri(patchTris[pnum][tnum]);
                for (int vnum=0; vnum<3; vnum++) 
                    if (vertIsInPatch[tri->verts[vnum]->index] == 1){
                        int vertID = patch->verts.addVert();
                        vertIDs[tri->verts[vnum]->index] = vertID;
                        
                        if(hierarchy->opType == Half_Edge_Collapse)
                            tri->verts[vnum]->mtIndex = vertID;// used for tracking in Half-Edge mode

                        patchVerts[vertID] = tri->verts[vnum]->index;
                        vertIsInPatch[tri->verts[vnum]->index] = 0; // reset for next iteration
                    }
            }
            
            // copy the vertices
            for (unsigned int vnum=0; vnum<patch_numVerts; vnum++) {
                patch->SetFrom(vnum, model->getVert(patchVerts[vnum]));
                float* coord = patch->verts.getCoord(vnum);
                UPDATE_MINMAX(v_min, v_max, coord);
                UPDATE_MINMAX(p_min, p_max, coord);
            }
            
            // build the index list
            patch->indices = new unsigned int[patchNumTris[pnum]*3];
            patch->numIndices = 0;
            for (int tnum=0; tnum<patchNumTris[pnum]; tnum++) {
                xbsTriangle *tri = model->getTri(patchTris[pnum][tnum]);
                
                patch->indices[patch->numIndices++] =
                    vertIDs[tri->verts[0]->index];
                patch->indices[patch->numIndices++] =
                    vertIDs[tri->verts[1]->index];
                patch->indices[patch->numIndices++] =
                    vertIDs[tri->verts[2]->index];
            }

            // update the per-patch error
            patch->errorCenter = (p_max + p_min) * 0.5;
            patch->errorOffsets= p_max - patch->errorCenter;
        }
        delete patchVerts;
        patchVerts = NULL;
    }

    delete vertIDs;
    delete vertIsInPatch;
    delete patchNumTris;
    for (int pnum=0; pnum<numPatches; pnum++)
        delete patchTris[pnum];
    delete patchTris;
    
    errorCenter = (v_max+v_min)*0.5;
    errorOffsets=v_max-errorCenter;
} /** End of DiscretePatchLevel::DiscretePatchLevel() **/

DiscretePatchLevel::DiscretePatchLevel(DiscretePatchHierarchy* h, GLOD_RawObject *raw, unsigned int level)
{
    /* not supported */
    assert(false);
}

/*****************************************************************************\
 @ DiscretePatchHierarchy::initialize(GLOD_RawObject*)
 -----------------------------------------------------------------------------
\*****************************************************************************/

void
DiscretePatchHierarchy::initialize(GLOD_RawObject *raw) 
{
    /* not supported*/
    assert(false);
}

/*****************************************************************************\
 @ DiscretePatchHierarchy::initialize(Model*)
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscretePatchHierarchy::initialize(Model *model)
{
    LODs = new DiscretePatchLevel *[1];
    errors = new xbsReal[1];
    maxLODs = 1;

    LODs[0] = new DiscretePatchLevel(this, model);
    errors[0] = 0.0;
    numUsedLODs = 1;
    
    return;
} /** End of DiscretePatchHierarchy::initialize() **/


/*****************************************************************************\
 @ DiscretePatchHierarchy::finalize
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscretePatchHierarchy::finalize(Model *model)
{
    // update the numLODs array
    if (numLODs == NULL)
        numLODs = new int[LODs[0]->numPatches];
    for (int i=0; i<numUsedLODs; i++)
        for (int j=0; j<LODs[0]->numPatches; j++)
            if (LODs[i]->patches[j].getNumTris()!=0)
                numLODs[j]=i+1;
            else {
                numLODs[j]=i+1;
                break;
            }
    originalErrors = new xbsReal[numUsedLODs];
    for (int i=0; i<numUsedLODs; i++)
        originalErrors[i] = errors[i];
    // our arrays used to store the levels & such might be aggressively large...
    // make them the right size
    if (maxLODs == numUsedLODs)
        return;

    DiscretePatchLevel **newLODs = new DiscretePatchLevel *[numUsedLODs];
    xbsReal *newErrors = new xbsReal[numUsedLODs];
    for (int i=0; i<numUsedLODs; i++)
    {
        newLODs[i] = LODs[i];
        newErrors[i] = errors[i];
    }
    delete [] LODs;
    LODs = newLODs;
    delete [] errors;
    errors = newErrors;
    maxLODs = numUsedLODs;
    return;
       
} /** End of DiscretePatchHierarchy::finalize() **/



/*****************************************************************************\
 @ DiscretePatchHierarchy::update
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscretePatchHierarchy::update(Model *model, Operation *op)
{
    if ((model->getNumTris() <= 0) ||
        (model->getNumTris() > (LODs[numUsedLODs-1]->numTris/2)))
        return;

    //
    // add a new LOD
    //

    if (numUsedLODs == maxLODs)
    {
        DiscretePatchLevel **newLODs = new DiscretePatchLevel *[maxLODs*2];
        float *newErrors = new xbsReal[maxLODs*2];
        for (int i=0; i<numUsedLODs; i++)
        {
            newLODs[i] = LODs[i];
            newErrors[i] = errors[i];
        }
        delete LODs;
        LODs = newLODs;
        delete errors;
        errors = newErrors;

        maxLODs *= 2;
    }
    
    LODs[numUsedLODs] = new DiscretePatchLevel(this, model);
    errors[numUsedLODs] = op->getCost();
    numUsedLODs++;
    
    return;
    
} /** End of DiscretePatchHierarchy::update() **/

void
DiscretePatchHierarchy::manualAddLevel(Model *model, unsigned int level, float geometric_error)
{
    assert(false);
}

/*****************************************************************************\
 @ DiscretePatchHierarchy::getReadbackSize
 -----------------------------------------------------------------------------
 description : READ BACK THE ENTIRE OBJECT
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
int  DiscretePatchHierarchy::getReadbackSize() {
    assert(false);
    return 0;
}

/*****************************************************************************\
 @ DiscretePatchHierarchy::readback
 -----------------------------------------------------------------------------
 description : READ BACK THE ENTIRE OBJECT
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void DiscretePatchHierarchy::readback(void* void_dst) {
    assert(false);
}

/*****************************************************************************\
 @ DiscretePatchHierarchy::load
 -----------------------------------------------------------------------------
 description : LOAD UP AN ENTIRE OBJECT FROM THIS FILE
 input       : 
 output      : 
 errors      : GLOD_CORRUPT_BUFFER or GLOD_BAD_MAGIC
\*****************************************************************************/
#if 0
#define GET(dst,size) {memcpy(dst,src,size);src+=size;printf("%i::+%i = %i bytes\n", __LINE__, size, src - (char*)src_data);}
#else
#define GET(dst,size) {memcpy(dst,src,size);src+=size;}
#endif
int DiscretePatchHierarchy::load(void* src_data) {
    assert(false);
    return 0;
}

/*****************************************************************************\
 @ DiscretePatchHierarchy::write
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/


#if 0 //unused
void
DiscretePatchHierarchy::debugWrite(char *basename)
{
    assert(false);
} /** End of DiscretePatchHierarchy::debugWrite() **/
#endif

/*****************************************************************************\
 @ DiscreteHierarchy::write
 -----------------------------------------------------------------------------
description : 
input       : 
output      : 
notes       : 
\*****************************************************************************/
void DiscretePatchHierarchy::changeQuadricMultiplier(GLfloat multiplier){
    for (int i=0; i<numUsedLODs; i++)
        errors[i] = originalErrors[i]*multiplier;
}


GLOD_Cut *
DiscretePatchHierarchy::makeCut()
{
    return new DiscretePatchCut(this, numUsedLODs-1);
};


void
DiscretePatchCut::computeBoundingSphere()
{
    DiscretePatchLevel *obj = hierarchy->LODs[0];

    xbsVec3 vmin(MAXFLOAT, MAXFLOAT, MAXFLOAT);
    xbsVec3 vmax(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);

    for (int pnum=0; pnum<obj->numPatches; pnum++)
    {
        DiscretePatchPatch *patch = &(obj->patches[pnum]);
        for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
        {
            float* coord = patch->verts.getCoord(vnum);            
            UPDATE_MINMAX(vmin, vmax, coord);
        }
    }
    center = (vmax+vmin)*0.5;
#if 1
    radius=0;
    for (int pnum=0; pnum<obj->numPatches; pnum++)
    {
        DiscretePatchPatch *patch = &(obj->patches[pnum]);
        for (unsigned int vnum=0; vnum<patch->getNumVerts(); vnum++)
        {
            float* coord = patch->verts.getCoord(vnum);
            float length=(center-xbsVec3(coord[0], coord[1], coord[2])).length();
            if (length>radius) radius=length;
        }
    }
#else
    radius = (vmax-center).length();
#endif
}



/*****************************************************************************\
 @ DiscretePatchCut::adaptObjectSpaceErrorThreshold
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscretePatchCut::adaptObjectSpaceErrorThreshold(float threshold)
{
    //fprintf(stderr, "adaptObjectSpaceErrorThreshold(): ");
    int level;
    for (level=1; level<hierarchy->numUsedLODs; level++)
        if ((view.computePixelsOfError(hierarchy->LODs[level]->errorCenter, hierarchy->LODs[level]->errorOffsets, hierarchy->errors[level], -1) > 0.f ? hierarchy->errors[level] : 0.f) > threshold)
            break;
    
    level--;
    LODNumber = level;
    for (int i=0; i<hierarchy->LODs[0]->numPatches; i++)
        patchLevel[i]=level;
    updateStats();
    modeChange = 1;
    //fprintf(stderr, "threshold: %f, LOD: %d\n", threshold, LODNumber);
    return;
} /** End of DiscretePatchCut::adaptObjectSpaceErrorThreshold() **/


/*****************************************************************************\
 @ DiscretePatchCut::adaptScreenSpaceErrorThreshold
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscretePatchCut::adaptScreenSpaceErrorThreshold(float threshold)
{
    //fprintf(stderr, "adaptScreenSpaceErrorThreshold(): ");
    for (int p=0; p<hierarchy->LODs[0]->numPatches; p++){
        patchLevel[p]=1;
        int level;
        for (level=1; level<hierarchy->numUsedLODs; level++)
            if (view.computePixelsOfError(hierarchy->LODs[level]->patches[p].errorCenter, hierarchy->LODs[level]->patches[p].errorOffsets, hierarchy->errors[level], -1) //view.computePixelsOfError(center, hierarchy->errors[level])
                > threshold)
                break;
        patchLevel[p]=level-1;
    }
    updateStats();
    modeChange = 1;
    //fprintf(stderr, "threshold: %f, LOD: %d\n", threshold, LODNumber);
    return;
} /** End of DiscretePatchCut::adaptScreenSpaceErrorThreshold() **/


/*****************************************************************************\
 @ DiscretePatchCut::coarsen
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscretePatchCut::coarsen(ErrorMode mode, int triTermination, float errorTermination)
{
//    printf("coarsening %i\n", triTermination);
    //if (errorTermination==MAXFLOAT)
      //  errorTermination=0;
    int curTris=currentNumTris;
//    printf("error term %f triTerm %i\n", errorTermination, triTermination);
    int numPatches=hierarchy->LODs[0]->numPatches;
    if (modeChange||(lastMode!=mode)) {//(coarsenQueue->size()==0) || (mode==ScreenSpace)){
        coarsenQueue->clear();
        curTris=0;
        for (int i=0; i<numPatches; i++){
            DiscretePatchPatch *patch=&hierarchy->LODs[0]->patches[i];
            patchLevel[i]=0;
            float error = (mode==ScreenSpace) ? 
                view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[patchLevel[i]]):
                hierarchy->errors[patchLevel[i]];
            if (patch->getNumTris()==0) error=MAXFLOAT/2.0f;
            budgetCoarsenHeapData[i]->setKey(error);
            coarsenQueue->insert(budgetCoarsenHeapData[i]);
            refineQueue->changeKey(budgetRefineHeapData[i], -error);
            curTris+=patch->getNumTris();
            
        }
        lastMode=mode;
        modeChange=0;
    }
    //else
     //   for (int i=0; i<numPatches; i++){
     //       DiscretePatchPatch *patch=&hierarchy->LODs[patchLevel[i]]->patches[i];
     //       curTris+=patch->getNumTris();
     //   }
    
    int adapting=1;
    float error=0;
    //printf("%i : %i %i %f %f %i\n", adapting, curTris, triTermination, error, errorTermination, coarsenQueue->size());
//    printf("%i : %i %i\n", adapting, curTris, triTermination);
    DiscretePatchPatch *previousCoarsen=NULL;
    DiscretePatchPatch *coarsenPatch=NULL;
    while ((adapting) && (coarsenQueue->size()>0)){
        previousCoarsen = coarsenPatch;
        coarsenPatch = (DiscretePatchPatch*)coarsenQueue->extractMin()->userData();
        coarsenPatch = &hierarchy->LODs[patchLevel[coarsenPatch->patchNum]]->patches[coarsenPatch->patchNum];
        if (budgetCoarsenHeapData[coarsenPatch->patchNum]->key()==MAXFLOAT) break;
//        printf("inside1 %i : %i %.5f %.5f %i\n", coarsenPatch->patchNum, adapting, error, errorTermination, coarsenQueue->size());
//        printf("inside1 %i : %i %i %i %i\n", coarsenPatch->patchNum, adapting, curTris, triTermination, coarsenPatch->getNumTris());
        curTris-=coarsenPatch->getNumTris();
        patchLevel[coarsenPatch->patchNum]++;
        if (patchLevel[coarsenPatch->patchNum]>(hierarchy->numLODs[coarsenPatch->patchNum]-1)) {
            patchLevel[coarsenPatch->patchNum]=hierarchy->numLODs[coarsenPatch->patchNum]-1;
            coarsenPatch = &hierarchy->LODs[patchLevel[coarsenPatch->patchNum]]->patches[coarsenPatch->patchNum];
            //error = (mode==ScreenSpace) ? 
            //    view.computePixelsOfError(coarsenPatch->errorCenter, coarsenPatch->errorOffsets, hierarchy->errors[coarsenPatch->patchNum]):hierarchy->errors[patchLevel[coarsenPatch->patchNum]];
            curTris+=coarsenPatch->getNumTris();
            budgetCoarsenHeapData[coarsenPatch->patchNum]->setKey(MAXFLOAT);
            refineQueue->changeKey(budgetRefineHeapData[coarsenPatch->patchNum], -MAXFLOAT);
            coarsenQueue->insert(budgetCoarsenHeapData[coarsenPatch->patchNum]);
            adapting = ((curTris>=triTermination) && (error<=errorTermination));
        }
        else {
            if (coarsenQueue->size()==0)
                break;
            coarsenPatch = &hierarchy->LODs[patchLevel[coarsenPatch->patchNum]]->patches[coarsenPatch->patchNum];
            error = (mode==ScreenSpace) ? 
                view.computePixelsOfError(coarsenPatch->errorCenter, coarsenPatch->errorOffsets, hierarchy->errors[patchLevel[coarsenPatch->patchNum]]):hierarchy->errors[patchLevel[coarsenPatch->patchNum]];
            if (coarsenPatch->getNumTris()==0) error=MAXFLOAT;
            curTris+=coarsenPatch->getNumTris();
            budgetCoarsenHeapData[coarsenPatch->patchNum]->setKey(error);
            refineQueue->changeKey(budgetRefineHeapData[coarsenPatch->patchNum], -error);
            coarsenQueue->insert(budgetCoarsenHeapData[coarsenPatch->patchNum]);
            adapting = ((curTris>=triTermination) && (error<=errorTermination));
        }
//        printf("inside2 %i : %i %.5f %.5f %i\n", coarsenPatch->patchNum, adapting, error, errorTermination, coarsenQueue->size());
//        printf("inside2 %i : %i %i %i %i\n", coarsenPatch->patchNum, adapting, curTris, triTermination, coarsenPatch->getNumTris());
        if (coarsenQueue->size()==0)
            adapting=0;
    }
    
    lastCoarsen = coarsenPatch;
    /*
    coarsenQueue->clear();
    
    curTris-=refinePatch->getNumTris();
    patchLevel[refinePatch->patchNum]++;
    if (patchLevel[refinePatch->patchNum]==hierarchy->numLODs[refinePatch->patchNum]) patchLevel[refinePatch->patchNum]--;
    refinePatch = &hierarchy->LODs[patchLevel[refinePatch->patchNum]]->patches[refinePatch->patchNum];
    curTris+=refinePatch->getNumTris();
    if (previousRefine==NULL)
        previousRefine=refinePatch;
     */
    /*
     int pl = patchLevel[previousRefine->patchNum];
     pl++;
     if (pl==hierarchy->numLODs[previousRefine->patchNum]) pl--;
     previousRefine = &hierarchy->LODs[pl]->patches[previousRefine->patchNum];
     float error = (mode==ScreenSpace) ? 
     view.computePixelsOfError(previousRefine->errorCenter, previousRefine->errorOffsets, hierarchy->errors[pl]):
     hierarchy->errors[pl];
     */
    //budgetCoarsenHeapData[previousRefine->patchNum]->setKey(0);
    //coarsenQueue->insert(budgetCoarsenHeapData[previousRefine->patchNum]);
    
    /*
     
     //for (int i=0; i<numPatches; i++){
     //    DiscretePatchPatch *patch=&hierarchy->LODs[patchLevel[i]]->patches[i];
     float error = (mode==ScreenSpace) ? 
     view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[patchLevel[i]]):
     hierarchy->errors[patchLevel[i]];
     if (patch->getNumTris()==0) error=MAXFLOAT/2.0f;
     budgetCoarsenHeapData[i]->setKey(error);
     coarsenQueue->insert(budgetCoarsenHeapData[i]);
     //}
     */
    printf("done\n");
    updateStats();
    
    return;
    
    
} /** End of DiscretePatchCut::coarsen() **/


/*****************************************************************************\
 @ DiscretePatchCut::refine
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscretePatchCut::refine(ErrorMode mode, int triTermination, float errorTermination)
{
    //errorTermination=-errorTermination;
//    printf("refining %i\n", triTermination);
    
    int curTris=currentNumTris;
//    printf("error term %f triTerm %i\n", errorTermination, triTermination);
    int numPatches=hierarchy->LODs[0]->numPatches;
    if (modeChange||(lastMode!=mode)){//((refineQueue->size()==0) || (mode==ScreenSpace)){
        refineQueue->clear();
        curTris=0;
        for (int i=0; i<numPatches; i++){
            DiscretePatchPatch *patch=&hierarchy->LODs[hierarchy->numLODs[i]-1]->patches[i];
            patchLevel[i]=hierarchy->numLODs[i]-1;
            float error = (mode==ScreenSpace) ? 
                view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[patchLevel[i]]):
                hierarchy->errors[patchLevel[i]];
            if (patch->getNumTris()==0) error=MAXFLOAT/2.0f;
            budgetRefineHeapData[i]->setKey(-error);
            refineQueue->insert(budgetRefineHeapData[i]);
            coarsenQueue->changeKey(budgetCoarsenHeapData[i], error);
            curTris+=patch->getNumTris();
        }
        lastMode=mode;
        modeChange=0;
    }
 //   else
   //     for (int i=0; i<numPatches; i++){
    //        DiscretePatchPatch *patch=&hierarchy->LODs[patchLevel[i]]->patches[i];
    //        curTris+=patch->getNumTris();
   //     }
    
    int adapting=1;
    float error=0;
    //printf("%i : %i %i %f %f %i\n", adapting, curTris, triTermination, error, errorTermination, refineQueue->size());
//    printf("%i : %i %i\n", adapting, curTris, triTermination);
    DiscretePatchPatch *previousRefine=NULL;
    DiscretePatchPatch *refinePatch=NULL;
    while ((adapting) && (refineQueue->size()>0)){
        previousRefine = refinePatch;
        refinePatch = (DiscretePatchPatch*)refineQueue->extractMin()->userData();
        refinePatch = &hierarchy->LODs[patchLevel[refinePatch->patchNum]]->patches[refinePatch->patchNum];
        //printf("inside1 %i : %i %i %i %i\n", refinePatch->patchNum, adapting, curTris, triTermination, refinePatch->getNumTris());
        curTris-=refinePatch->getNumTris();
        patchLevel[refinePatch->patchNum] --;
        if (patchLevel[refinePatch->patchNum]<0) {
            patchLevel[refinePatch->patchNum]=0;
            refinePatch = &hierarchy->LODs[patchLevel[refinePatch->patchNum]]->patches[refinePatch->patchNum];
            budgetRefineHeapData[refinePatch->patchNum]->setKey(0);
            refineQueue->insert(budgetRefineHeapData[refinePatch->patchNum]);
            coarsenQueue->changeKey(budgetCoarsenHeapData[refinePatch->patchNum], 0);
            curTris+=refinePatch->getNumTris();
            adapting = ((curTris<=triTermination) && (error>=errorTermination));
        }
        else {
            if (refineQueue->size()==0)
                break;
            refinePatch = &hierarchy->LODs[patchLevel[refinePatch->patchNum]]->patches[refinePatch->patchNum];
            error = (mode==ScreenSpace) ? 
                view.computePixelsOfError(refinePatch->errorCenter, refinePatch->errorOffsets, hierarchy->errors[patchLevel[refinePatch->patchNum]]):hierarchy->errors[patchLevel[refinePatch->patchNum]];
            if (refinePatch->getNumTris()==0) error=MAXFLOAT/2.0f;
            curTris+=refinePatch->getNumTris();
            budgetRefineHeapData[refinePatch->patchNum]->setKey(-error);
            refineQueue->insert(budgetRefineHeapData[refinePatch->patchNum]);
            coarsenQueue->changeKey(budgetCoarsenHeapData[refinePatch->patchNum], error);
            adapting = ((curTris<triTermination) && (error>errorTermination));
        }
//        printf("inside2 %i : %i %.5f %.5f %i\n", refinePatch->patchNum, adapting, error, errorTermination, refineQueue->size());
//        printf("inside2 %i : %i %i %i %i\n", refinePatch->patchNum, adapting, curTris, triTermination, refinePatch->getNumTris());
        if (refineQueue->size()==0)
            adapting=0;
    }
    
    //coarsenQueue->clear();
    lastRefine = refinePatch;
    /*
     int pl = patchLevel[previousRefine->patchNum];
     pl++;
     if (pl==hierarchy->numLODs[previousRefine->patchNum]) pl--;
     previousRefine = &hierarchy->LODs[pl]->patches[previousRefine->patchNum];
     
     float error = (mode==ScreenSpace) ? 
     view.computePixelsOfError(previousRefine->errorCenter, previousRefine->errorOffsets, hierarchy->errors[pl]):
     hierarchy->errors[pl];
     */
    //budgetCoarsenHeapData[previousRefine->patchNum]->setKey(0);
    //coarsenQueue->insert(budgetCoarsenHeapData[previousRefine->patchNum]);
    
    /*
   
    //for (int i=0; i<numPatches; i++){
    //    DiscretePatchPatch *patch=&hierarchy->LODs[patchLevel[i]]->patches[i];
        float error = (mode==ScreenSpace) ? 
            view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[patchLevel[i]]):
            hierarchy->errors[patchLevel[i]];
        if (patch->getNumTris()==0) error=MAXFLOAT/2.0f;
        budgetCoarsenHeapData[i]->setKey(error);
        coarsenQueue->insert(budgetCoarsenHeapData[i]);
    //}
    */
    printf("done\n");
    updateStats();
    
    return;
} /** End of DiscretePatchCut::refine() **/



/*****************************************************************************\
 @ DiscretePatchCut::getReadbackSizes
 -----------------------------------------------------------------------------
 description :  READ BACK A PATCH!!!
 input       : 
 output      : 
 notes       :  
\*****************************************************************************/
void DiscretePatchCut::getReadbackSizes(int patch, GLuint* nindices, GLuint* nverts) {
    assert(false);
}

/*****************************************************************************\
 @ DiscretePatchCut::readback
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :  *patch.data_flags will be set for what should be produced.
                If we don't have a certain one of these, then don't procduce
                it and unset the flag.
\*****************************************************************************/
void DiscretePatchCut::readback(int npatch, GLOD_RawPatch* raw) {
    assert(false);
}

/*****************************************************************************\
 @DiscretePatchCut::draw
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void DiscretePatchCut::draw(int patchnum) {

    if ((patchnum < 0) || (patchnum >= hierarchy->LODs[LODNumber]->numPatches))
    {
        fprintf(stderr, "DiscretePatchCut::draw(): invalid patch number.\n");
        return;
    }

    //DiscretePatchPatch *patch = &(hierarchy->LODs[LODNumber]->patches[patchnum]);
    DiscretePatchPatch *patch = &(hierarchy->LODs[patchLevel[patchnum]]->patches[patchnum]);
    DiscretePatchPatch *basepatch;
    if(hierarchy->opType == Half_Edge_Collapse)
        basepatch = &(hierarchy->LODs[0]->patches[patchnum]);
    else
        basepatch = &(hierarchy->LODs[LODNumber]->patches[patchnum]);

    for (int area=0; area<GLOD_NUM_TILES; area++){
        if (view.computePixelsOfError(hierarchy->LODs[LODNumber]->errorCenter, 
                                      hierarchy->LODs[LODNumber]->errorOffsets, 
                                      hierarchy->errors[LODNumber],area) < 0.000001f)
            continue;
        
        if (patch->numIndices==0) return;
        
#ifdef GLOD_USE_LISTS
        if (! basepatch->verts.glInitialized()) {
            int id = glGenLists(1);
            basepatch->verts.glInit(false); // force use of VA

            glNewList(id, GL_COMPILE);
            basepatch->verts.glBind(false);
            glDrawElements(GL_TRIANGLES, patch->numIndices, GL_UNSIGNED_INT,
                           patch->indices);
            basepatch->verts.glUnbind(false);
            glEndList();
            basepatch->verts.m_VBOid = id;
        }
        glCallList(id);
#else
        if(! basepatch->verts.glInitialized())
            basepatch->verts.glInit();
        
        basepatch->verts.glBind();
        glDrawElements(GL_TRIANGLES, patch->numIndices, GL_UNSIGNED_INT,
                       patch->indices);
        basepatch->verts.glUnbind();
#endif
    }
}
