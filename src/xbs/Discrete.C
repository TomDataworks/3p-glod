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
#include "Model.h"  /* for xbsVertex */
#include "glod_core.h"
#include "Discrete.h"
#include "xbs.h" /* for Operator */

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

#define UPDATE_MINMAX(vecname) \
                if (vecname[0]>v_max[0]) v_max[0]=vecname[0];\
                if (vecname[1]>v_max[1]) v_max[1]=vecname[1];\
                if (vecname[2]>v_max[2]) v_max[2]=vecname[2];\
                if (vecname[0]<v_min[0]) v_min[0]=vecname[0];\
                if (vecname[1]<v_min[1]) v_min[1]=vecname[1];\
                if (vecname[2]<v_min[2]) v_min[2]=vecname[2];


AttribSetArray&
DiscretePatch::getVerts()
{
    return (((level->hierarchy->opType == Half_Edge_Collapse) &&
             (level->hierarchy->LODs[0] != NULL)) ?
            level->hierarchy->LODs[0]->patches[patchNum].verts :
            verts);
}


int DiscreteHierarchy::GetVertIdx(int patchNum, xbsVertex* vert) {
#ifndef XBS_SPLIT_BORDER_VERTS
    if(vert->mtIndex == -1)
        return -1;
    PatchVertPair* p = (PatchVertPair*) vert->mtIndex;
    while(p != NULL) {
        if(p->patch == patchNum)
            return p->vertNum;
        p = p->pNext;
    }
    return -1;
#else
    return vert->mtIndex;
#endif
}

void DiscreteHierarchy::SetVertIdx(int patchNum, xbsVertex* vert, int vertNum) {
#ifndef XBS_SPLIT_BORDER_VERTS
    PatchVertPair* p;
    PatchVertPair* pLast = NULL;

    if(vert->mtIndex == -1)
        p = NULL;
    else
        p = (PatchVertPair*) vert->mtIndex;    
    
    while(p != NULL) {
        if(p->patch == patchNum)
            break;
        pLast = p;
        p = p->pNext;
    }
    if(p != NULL) {
        p->vertNum = vertNum;
        return;
    } else {
        p = new PatchVertPair;
        p->patch = patchNum;
        p->vertNum = vertNum;

        if(this->nPatchPairs == this->maxPatchPairs) {
            maxPatchPairs *= 2;
            PatchVertPair** newAry = new PatchVertPair*[maxPatchPairs];
            for(int i = 0; i < nPatchPairs; i++)
                newAry[i] = patchPairs[i];
            delete patchPairs;
            patchPairs = newAry;
        }
        patchPairs[this->nPatchPairs++] = p;
        
        p->pNext = NULL;
        if(pLast != NULL)
            pLast->pNext = p;
        else
            vert->mtIndex = (int)(p);
    }
#else
    vert->mtIndex = vertNum;
    return;
#endif
}

/*****************************************************************************\
 @ DiscreteLevel::DiscreteLevel
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : Notice that because we match up vertices with patches by
               looking at the triangles, any unused/empty vertices of the
               Model are not carried over into the DiscreteLevel.
\*****************************************************************************/
DiscreteLevel::DiscreteLevel(DiscreteHierarchy* hierarchy, Model *model)
{
    this->hierarchy = hierarchy;
    xbsVec3 v_max(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT), v_min(MAXFLOAT, MAXFLOAT, MAXFLOAT);

    char hasColor, hasNormal, hasTexcoord;
    model->hasAttributes(hasColor, hasNormal, hasTexcoord);
    
    numPatches = model->getNumPatches();

    patches = new DiscretePatch[numPatches];
    for(int i =0; i < numPatches; i++) {
        patches[i].Init((DiscreteLevel*)this, i, hasColor == 1, hasNormal == 1, hasTexcoord == 1);
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
        // which verts are in this patch
        DiscretePatch *patch = &(patches[pnum]);
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
        
        if(hierarchy->opType == Half_Edge_Collapse && hierarchy->numLODs != 0) {
            DiscretePatch* basePatch = &hierarchy->LODs[0]->patches[pnum];
            AttribSetArray& verts = basePatch->getVerts();
            // find any new vertices that are in this patch but not the base patch
            // if any are found, create them
            for (int tnum=0; tnum<patchNumTris[pnum]; tnum++) {
                xbsTriangle *tri = model->getTri(patchTris[pnum][tnum]);
                for (int vnum=0; vnum<3; vnum++) {
                    if (vertIsInPatch[tri->verts[vnum]->index] == 0) continue;
                    vertIsInPatch[tri->verts[vnum]->index] = 0;
                    
                    // now.. are they in the base patch?
                    int vertLocalIndex = hierarchy->GetVertIdx(pnum, tri->verts[vnum]);
                    if(vertLocalIndex == -1) {
                        // add the new vert to the base patch
                        int newVertIdx = verts.addVert();
                        basePatch->SetFrom(newVertIdx, tri->verts[vnum]);
                        
                        // set the bookkeeping so that we can associate triangles to these...
                        hierarchy->SetVertIdx(pnum, tri->verts[vnum], newVertIdx); // used for tracking in Half-Edge mode
                        vertLocalIndex = newVertIdx;
                    }
                    
                    float* coord = verts.getCoord(vertLocalIndex--);
                    UPDATE_MINMAX(coord);
                }
            }
            
            // build the index list
            patch->indices = new unsigned int[patchNumTris[pnum]*3];
            patch->numIndices = 0;
            for (int tnum=0; tnum<patchNumTris[pnum]; tnum++) {
                xbsTriangle *tri = model->getTri(patchTris[pnum][tnum]);

                patch->indices[patch->numIndices++] = hierarchy->GetVertIdx(pnum, tri->verts[0]);
                patch->indices[patch->numIndices++] = hierarchy->GetVertIdx(pnum, tri->verts[1]);
                patch->indices[patch->numIndices++] = hierarchy->GetVertIdx(pnum, tri->verts[2]);
            }
        } else {            // build the base patch if in half_edge and build all patches if in full_edge

            // xxx: is patch_numVerts the correct size already?
            patch->SetSize(patch_numVerts);
            
            // assign verts indices in this patch  (and count verts in patch)
            for (int tnum=0; tnum<patchNumTris[pnum]; tnum++) {
                xbsTriangle *tri = model->getTri(patchTris[pnum][tnum]);
                for (int vnum=0; vnum<3; vnum++) 
                    if (vertIsInPatch[tri->verts[vnum]->index] == 1){
                        int vertID = patch->getVerts().addVert();
                        vertIDs[tri->verts[vnum]->index] = vertID;
                        
                        if(hierarchy->opType == Half_Edge_Collapse)
                            hierarchy->SetVertIdx(pnum, tri->verts[vnum], vertID);// used for tracking in Half-Edge mode

                        patchVerts[vertID] = tri->verts[vnum]->index;
                        vertIsInPatch[tri->verts[vnum]->index] = 0; // reset for next iteration
                    }
            }
            
            // copy the vertices
            for (unsigned int vnum=0; vnum<patch_numVerts; vnum++) {
                patch->SetFrom(vnum, model->getVert(patchVerts[vnum]));
                float* coord = patch->getVerts().getCoord(vnum);
                UPDATE_MINMAX(coord);
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
    
} /** End of DiscreteLevel::DiscreteLevel() **/

#ifdef GLOD
DiscreteLevel::DiscreteLevel(DiscreteHierarchy* hierarchy, GLOD_RawObject *raw, unsigned int level)
{
    this->hierarchy = hierarchy;
 
    int curLevelPatches = 0;
    numTris=0;
    bool hasColor, hasNormal, hasTexcoord;
    hasColor = hasNormal= hasTexcoord = 0;

    int maxPatchesPerLevel=0;
    xbsVec3 v_max(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT), v_min(MAXFLOAT, MAXFLOAT, MAXFLOAT);
    for (unsigned int i=0; i<raw->num_patches; i++){
        if (raw->patches[i]->name>maxPatchesPerLevel)
            maxPatchesPerLevel=raw->patches[i]->name;
        if (raw->patches[i]->level==level){
            curLevelPatches++;
            numTris+=raw->patches[i]->num_triangles;
            if (raw->patches[i]->data_flags & GLOD_HAS_VERTEX_COLORS_3) hasColor=1;
            if (raw->patches[i]->data_flags & GLOD_HAS_VERTEX_NORMALS) hasNormal=1;
            if (raw->patches[i]->data_flags & GLOD_HAS_TEXTURE_COORDS_2) hasTexcoord=1;
        }
    }
    maxPatchesPerLevel++;
    patches = new DiscretePatch[maxPatchesPerLevel];
    for (int i=0; i<maxPatchesPerLevel; i++) {
        patches[i].Init(this, i, hasColor, hasNormal, hasTexcoord);
    }
    //  numTris = model->getNumTris();
    numPatches=maxPatchesPerLevel;
    int counter=0;
    for (int pnum=0; pnum<curLevelPatches; pnum++)
    {
        int doPatch=0;
        DiscretePatch *patch;
        GLOD_RawPatch *raw_patch = NULL;
        for (unsigned int i=counter; i<raw->num_patches; i++)
            if (raw->patches[i]->level==level){
                counter=i+1;
                raw_patch = raw->patches[i];
                doPatch=1;
                break;
            }
        patch = &(patches[raw_patch->name]);

        // set patch size...
        patch->SetSize(raw_patch->num_vertices);
        for (unsigned int vnum=0; vnum<raw_patch->num_vertices; vnum++) { 
            int index = vnum;
            
            patch->getVerts().setFrom(index, raw_patch, vnum);
            float* coord=patch->getVerts().getCoord(index);
            UPDATE_MINMAX(coord);
        }

        patch->indices = new unsigned int[raw_patch->num_triangles*3];
        patch->numIndices = 0;
        for (unsigned int tnum=0; tnum<raw_patch->num_triangles; tnum++)
        {
            patch->indices[patch->numIndices++] = raw_patch->triangles[3*tnum];
            patch->indices[patch->numIndices++] = raw_patch->triangles[3*tnum+1];
            patch->indices[patch->numIndices++] = raw_patch->triangles[3*tnum+2];
        }
    }
    errorCenter = (v_max+v_min)*0.5;
    errorOffsets=v_max-errorCenter;
}
#endif


/*****************************************************************************\
 @ DiscreteHierarchy::initialize(GLOD_RawObject*)
 -----------------------------------------------------------------------------
 input       : the rawobject is unsorted in terms of levels and patches
               the only thing that is known is that the patches range from 0 to
               some max number and that, hopefully, the levels range from 0 to 
               some number as well such that their union gives a continuous range
               from zero to n.
\*****************************************************************************/

#ifdef GLOD
void
DiscreteHierarchy::initialize(GLOD_RawObject *raw) { 
    /* determine the number of levels */

    unsigned int n_levels=0;
    int i;
    GLOD_RawPatch *patch;
    for (unsigned i = 0; i < raw->num_patches; i++) {
        patch=raw->patches[i];
        if (patch->level>n_levels) 
            n_levels=patch->level;
        if(n_levels < 0) {
            // ERRORR!!!
            printf("You used discrete manual incorrectly!\n");
            exit(1);
        }
    }
    n_levels++;

    // allocate levels & such
    LODs = new DiscreteLevel *[n_levels];
    errors = new xbsReal[n_levels];
    maxLODs = n_levels;
    numLODs=n_levels;

    // now .... watf?
    int l=0;
    for (unsigned i=0; i<n_levels; i++){
        bool make_level=false;
        for (unsigned int j=0; j<raw->num_patches; j++)
            if (raw->patches[j]->level==i){
                make_level=true;
                errors[l]+=raw->patches[j]->geometric_error;
            }
        if (make_level){
            LODs[l]=new DiscreteLevel(this, raw, i);
            l++;
        }
    }
    DiscreteLevel **tLODs= new DiscreteLevel *[l];
    xbsReal *tErrors=new xbsReal[l];
    for (i=0; i<l; i++){
        tLODs[i]=LODs[i];
        tErrors[i]=errors[i];
    }
    delete LODs;
    delete errors;
    errors=tErrors;
    LODs=tLODs;
    maxLODs=l;
    numLODs=l;

}
#endif

/*****************************************************************************\
 @ DiscreteHierarchy::initialize(Model*)
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteHierarchy::initialize(Model *model)
{
    LODs = new DiscreteLevel *[1];
    LODs[0] = NULL;
    errors = new xbsReal[1];
    maxLODs = 1;
    numLODs = 0;

#ifndef XBS_SPLIT_BORDER_VERTS
    maxPatchPairs = 4;
    nPatchPairs = 0;
    patchPairs = new PatchVertPair*[maxPatchPairs];
#endif

    if ((model->snapMode == PercentReduction) ||
        ((model->snapMode == ManualTriSpec) &&
         (model->numSnapshotSpecs >= 1) &&
         (model->getNumTris() <= model->snapshotTriSpecs[0])))
    {
        LODs[0] = new DiscreteLevel(this, model);
        errors[0] = 0.0;
        numLODs = 1;
    }
    
    return;
} /** End of DiscreteHierarchy::initialize() **/


/*****************************************************************************\
 @ DiscreteHierarchy::finalize
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteHierarchy::finalize(Model *model)
{
#ifndef XBS_SPLIT_BORDER_VERTS
    // cleanup patch-vert pairs
    for(int i = 0; i < nPatchPairs; i++) {
        if(patchPairs[i]->pNext != NULL)
            printf("Has multiple patches!\n");
        delete patchPairs[i];
    }
    delete [] patchPairs;
#endif
    
    if (numLODs==0){
        LODs[numLODs] = new DiscreteLevel(this, model);
        errors[numLODs] = MAXFLOAT;
        numLODs++;
        
    }
    
    
    originalErrors = new xbsReal[numLODs];
    for (int i=0; i<numLODs; i++)
        originalErrors[i] = errors[i];
    // optimize or apply one final hierarchy
    if (maxLODs == numLODs) {
        Optimize();
        return;
    }
    DiscreteLevel **newLODs = new DiscreteLevel *[numLODs];
    xbsReal *newErrors = new xbsReal[numLODs];
    
    for (int i=0; i<numLODs; i++)
    {
        newLODs[i] = LODs[i];
        newErrors[i] = errors[i];
        originalErrors[i] = errors[i];
    }
    delete [] LODs;
    LODs = newLODs;
    delete [] errors;
    errors = newErrors;
    maxLODs = numLODs;
    

    Optimize();

    return;
       
} /** End of DiscreteHierarchy::finalize() **/



/*****************************************************************************\
 @ DiscreteHierarchy::update
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteHierarchy::update(Model *model, Operation *op)
{
    // did any verts get deleted from the model?
    // if they did, and we're in half-edge-collapse-mode
    //    then we've got to deallocate the linked-list used at mtIndex
//    Operation** addOps, remOps, modOps;
//    Operation** nadd, nrem, nmod;
/*
    if(opType == Half_Edge_Collapse &&
       op->getSource()->mtIndex != -1) {
        UnsetVertIdx(op->getSource());
    }
*/
    // process snapshots
    switch (model->snapMode)
    {
        case PercentReduction:
        {            
            if ((model->getNumTris() <= 0) ||
                (model->getNumTris() >
                 (LODs[numLODs-1]->numTris * (1.0-model->reductionPercent))))
                return;
            break;
        }
        case ManualTriSpec:
        {
            if ((model->getNumTris() <= 0) ||
                (numLODs >= model->numSnapshotSpecs) ||
                (model->getNumTris() > model->snapshotTriSpecs[numLODs]))
                return;
            break;
        }
        case ManualErrorSpec:
        {
            if ((model->getNumTris() <= 0) ||
                (numLODs >= model->numSnapshotErrorSpecs) ||
                (op->getCost() < model->snapshotErrorSpecs[numLODs]))
                return;
            break;
        }
        default:
        {
#ifdef GLOD
            GLOD_SetError(GLOD_INVALID_STATE, "Invalid snapshot mode.");
#endif
            return;
            break;
        }
    }
    
    //
    // add a new LOD
    //

    if (numLODs == maxLODs)
    {
        DiscreteLevel **newLODs = new DiscreteLevel *[maxLODs*2];
        float *newErrors = new xbsReal[maxLODs*2];
        for (int i=0; i<numLODs; i++)
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
    
    LODs[numLODs] = new DiscreteLevel(this, model);
    errors[numLODs] = op->getCost();
    numLODs++;
    
    return;
    
} /** End of DiscreteHierarchy::update() **/

/*****************************************************************************\
 @ DiscreteHierarchy::manualAddLevel
 -----------------------------------------------------------------------------
 description : ADD A LEVEL TO THE DISCRETE OBJECT
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteHierarchy::manualAddLevel(Model *model, unsigned int level, float geometric_error)
{
    //    if ((model->getNumTris() <= 0) ||
    //    (model->getNumTris() > (LODs[numLODs-1]->numTris/2)))
    //    return;

    //
    // add a new LOD
    //

    if (numLODs == maxLODs)
    {
        DiscreteLevel **newLODs = new DiscreteLevel *[maxLODs*2];
        float *newErrors = new xbsReal[maxLODs*2];
        for (int i=0; i<numLODs; i++)
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
    LODs[level] = new DiscreteLevel(this, model);
    errors[level] = geometric_error;
    numLODs++;
    return;
    
}

/*****************************************************************************\
 @ DiscreteHierarchy::getReadbackSize
 -----------------------------------------------------------------------------
 description : READ BACK THE ENTIRE OBJECT
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
#define APPEND(value,amount) {size+=amount;}
int  DiscreteHierarchy::getReadbackSize() {
    int size=0;

    APPEND(&DISCRETE_MAGIC, sizeof(int));  // magic for the hierarchy
    APPEND(&numLODs,        sizeof(int)); // LOD count
    APPEND(&opType,         sizeof(OperationType)); // operator type

    // errors
    APPEND(errors,          sizeof(xbsReal)*numLODs); // errors

    for(int i = 0; i < numLODs; i++) { // FOR EACH LOD
        DiscreteLevel* o = LODs[i];

        APPEND(&o->numPatches, sizeof(int)); // patch count
        for(int j = 0; j < o->numPatches; j++) { // FOR EACH PATCH
            DiscretePatch* p = &o->patches[j];
            
            if(opType == Half_Edge_Collapse) {
                if(i == 0)
                    size += p->getVerts().getStateSize();
            } else {
                size += p->getVerts().getStateSize();
            }
            
            APPEND(&p->numIndices, sizeof(unsigned int)); // indices
            APPEND(p->indices, sizeof(unsigned int) * p->numIndices); // indices      
        }
    }

    return size;
#undef APPEND
}

/*****************************************************************************\
 @ DiscreteHierarchy::readback
 -----------------------------------------------------------------------------
 description : READ BACK THE ENTIRE OBJECT
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
static const int DISCRETE_MAGIC = 80175;
#define APPEND(value,amount) {memcpy(dst,(value),(amount)); dst+=(amount);}
void DiscreteHierarchy::readback(void* void_dst) {
    char* dst = (char*) void_dst;
    char zero = 0;

    APPEND(&DISCRETE_MAGIC, sizeof(int));  // magic for the hierarchy
    APPEND(&numLODs,        sizeof(int)); // LOD count
    APPEND(&opType,        sizeof(OperationType)); // operator type

    // errors
    APPEND(errors,          sizeof(xbsReal)*numLODs); // errors

    for(int i = 0; i < numLODs; i++) { // FOR EACH LOD
        DiscreteLevel* o = LODs[i];
    
        APPEND(&o->numPatches, sizeof(int)); // patch count
        for(int j = 0; j < o->numPatches; j++) { // FOR EACH PATCH
            DiscretePatch* p = &o->patches[j];

            if(opType == Half_Edge_Collapse) {
                if(i == 0)
                    dst += p->getVerts().copyState(dst);
            } else {
                dst += p->getVerts().copyState(dst);
            }
            
            APPEND(&p->numIndices, sizeof(unsigned int)); // indices
            APPEND(p->indices, sizeof(unsigned int) * p->numIndices); // indices
        }
    }
#undef APPEND
}

/*****************************************************************************\
 @ DiscreteHierarchy::load
 -----------------------------------------------------------------------------
 description : LOAD UP AN ENTIRE OBJECT FROM THIS FILE
 input       : 
 output      : 
 errors      : GLOD_CORRUPT_BUFFER or GLOD_BAD_MAGIC
\*****************************************************************************/
#define GET(dst,size) {memcpy(dst,src,size);src+=size;}
int DiscreteHierarchy::load(void* src_data) {
    char* src = (char*) src_data;
    int magic; 
    GET(&magic,sizeof(int));
    if(magic != DISCRETE_MAGIC) {
//    GLOD_SetError(GLOD_BAD_MAGIC, "Load of discrete object failed. Invalid magic number: ", magic);
        return 0;
    }

    GET(&numLODs,        sizeof(int)); // LOD count
    GET(&opType,         sizeof(OperationType)); // LOD count

    // errors
    errors = new xbsReal[numLODs];
    GET(errors,          sizeof(xbsReal)*numLODs); // errors

    LODs = new DiscreteLevel*[numLODs];
    for(int i = 0; i < numLODs; i++) { // FOR EACH LOD
        LODs[i] = new DiscreteLevel();
        DiscreteLevel* o = LODs[i];
        o->numTris = 0;
        GET(&o->numPatches, sizeof(int)); // patch count
        o->patches = new DiscretePatch[o->numPatches];
        for(int j = 0; j < o->numPatches; j++) { // FOR EACH PATCH
            DiscretePatch* p = &o->patches[j];
            p->SetLevel(o);
            p->SetPatchNum(j);
            if(opType == Half_Edge_Collapse) {
                if(i == 0)
                    src += p->getVerts().create(src);
            } else {
                src += p->getVerts().create(src);
            }
            
            GET(&p->numIndices, sizeof(unsigned int)); // indices
            p->indices = new unsigned int[p->numIndices];
            GET(p->indices, sizeof(unsigned int) * p->numIndices); // indices
            
            o->numTris += p->numIndices / 3;
        }
        o->hierarchy=this;
    }
    return 1;
}

/*****************************************************************************\
 @ DiscreteHierarchy::write
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteHierarchy::debugWrite(char *basename)
{
    fprintf(stderr, "DiscreteHierarchy has %d LODs.\n", numLODs);

    if (numLODs < 1)
        return;
    
    //fprintf(stderr, "Sorry, can't write DiscreteHierarchy yet.\n");

    // Compute bounding box diagonal
    xbsVec3 extentMin(MAXFLOAT,MAXFLOAT,MAXFLOAT);
    xbsVec3 extentMax(-MAXFLOAT,-MAXFLOAT,-MAXFLOAT);

    for (int pnum=0; pnum<LODs[0]->numPatches; pnum++)
    {
        DiscretePatch *patch = &(LODs[0]->patches[pnum]);
        for (unsigned int vnum=0; vnum<patch->getVerts().getSize(); vnum++)
        {
            xbsVec3 *coord = (xbsVec3*)patch->getVerts().getCoord(vnum);
            for (int dim=0; dim<3; dim++)
            {
                extentMin[dim] = MIN(extentMin[dim], (*coord)[dim]);
                extentMax[dim] = MAX(extentMax[dim], (*coord)[dim]);
            }
        }
    }
    
    xbsVec3 diagonal = extentMax - extentMin;
    double diagonalLength = diagonal.length();

    for (int i=0; i<numLODs; i++)
    {
        char filenamebase[1024];
        char filename[1024];
        sprintf(filenamebase, "%s.PctErr%010.6f.AbsErr%f.NumTris%%0%dd.ply",
                ((basename != NULL) ? basename : "xbsModel"),
                errors[i]/diagonalLength*100.0, errors[i],
                (int)(log10((float)LODs[0]->numTris))+1);
        sprintf(filename, filenamebase, LODs[i]->numTris);

        Model *model = new Model(LODs[i]);
        
        model->writePly(filename);
        delete model;
    }
    
    return;    
} /** End of DiscreteHierarchy::debugWrite() **/

/*****************************************************************************\
 @ DiscreteHierarchy::write
 -----------------------------------------------------------------------------
description : 
input       : 
output      : 
notes       : 
\*****************************************************************************/
void DiscreteHierarchy::changeQuadricMultiplier(GLfloat multiplier){
    for (int i=0; i<numLODs; i++)
        errors[i] = originalErrors[i]*multiplier;
}

GLOD_Cut *
DiscreteHierarchy::makeCut()
{
    return new DiscreteCut(this, numLODs-1);
};


void
DiscreteCut::computeBoundingSphere()
{
    DiscreteLevel *obj = hierarchy->LODs[0];

    xbsVec3 v_min(MAXFLOAT, MAXFLOAT, MAXFLOAT);
    xbsVec3 v_max(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);

    for (int pnum=0; pnum<obj->numPatches; pnum++)
    {
        DiscretePatch *patch = &(obj->patches[pnum]);
        for (unsigned int vnum=0; vnum<patch->getVerts().getSize(); vnum++)
        {
            float* coord = patch->getVerts().getCoord(vnum);
            UPDATE_MINMAX(coord);
        }
    }
    center = (v_max+v_min)*0.5;
#if 1
    radius=0;
    for (int pnum=0; pnum<obj->numPatches; pnum++)
    {
        DiscretePatch *patch = &(obj->patches[pnum]);
        for (unsigned int vnum=0; vnum<patch->getVerts().getSize(); vnum++)
        {
            xbsVec3* coord = (xbsVec3*)patch->getVerts().getCoord(vnum);
            float length=(center-*coord).length();
            if (length>radius) radius=length;
        }
    }
#else
    radius = (v_max-center).length();
#endif
}



/*****************************************************************************\
 @ DiscreteCut::adaptObjectSpaceErrorThreshold
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteCut::adaptObjectSpaceErrorThreshold(float threshold)
{
    //fprintf(stderr, "adaptObjectSpaceErrorThreshold(): ");
    int level;
    for (level=1; level<hierarchy->numLODs; level++)
        //if ((view.computePixelsOfError(hierarchy->LODs[level]->errorCenter, hierarchy->LODs[level]->errorOffsets, hierarchy->errors[level], -1)!=0?hierarchy->errors[level]:0) > threshold)
        if (hierarchy->errors[level]>threshold)
            break;
    
    level--;
    LODNumber = level;

    updateStats();
        
    //fprintf(stderr, "threshold: %f, LOD: %d\n", threshold, LODNumber);
    return;
} /** End of DiscreteCut::adaptObjectSpaceErrorThreshold() **/


/*****************************************************************************\
 @ DiscreteCut::adaptScreenSpaceErrorThreshold
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteCut::adaptScreenSpaceErrorThreshold(float threshold)
{
    //fprintf(stderr, "adaptScreenSpaceErrorThreshold(): ");
    int level;
    for (level=1; level<hierarchy->numLODs; level++)
        if (view.computePixelsOfError(hierarchy->LODs[level]->errorCenter, hierarchy->LODs[level]->errorOffsets, hierarchy->errors[level], -1) //view.computePixelsOfError(center, hierarchy->errors[level])
            > threshold)
            break;
    level--;
    LODNumber = level;
    updateStats();
        
    //fprintf(stderr, "threshold: %f, LOD: %d\n", threshold, LODNumber);
    return;
} /** End of DiscreteCut::adaptScreenSpaceErrorThreshold() **/


/*****************************************************************************\
 @ DiscreteCut::coarsen
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteCut::coarsen(ErrorMode mode, int triTermination, float errorTermination)
{
    int level;
    for (level=LODNumber; level<hierarchy->numLODs; level++)
    {
        if ((((mode == ObjectSpace) ? (view.computePixelsOfError(hierarchy->LODs[level]->errorCenter, hierarchy->LODs[level]->errorOffsets, hierarchy->errors[level])!=0?hierarchy->errors[level]:0) :
              view.computePixelsOfError(hierarchy->LODs[level]->errorCenter, hierarchy->LODs[level]->errorOffsets, hierarchy->errors[level])) //view.computePixelsOfError(center, hierarchy->errors[level]))
             >  errorTermination) ||
            (hierarchy->LODs[level]->numTris <= triTermination))
            break;
    }
    
    LODNumber = (level>hierarchy->numLODs-1) ? hierarchy->numLODs-1 : level;

    updateStats();
    
    return;
} /** End of DiscreteCut::coarsen() **/


/*****************************************************************************\
 @ DiscreteCut::refine
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
DiscreteCut::refine(ErrorMode mode, int triTermination, float errorTermination)
{
    int level;
    for (level=LODNumber; level>=0; level--)
    {
        if ((((mode == ObjectSpace) ? (view.computePixelsOfError(hierarchy->LODs[level]->errorCenter, hierarchy->LODs[level]->errorOffsets, hierarchy->errors[level])!=0?hierarchy->errors[level]:0) :
              view.computePixelsOfError(hierarchy->LODs[level]->errorCenter, hierarchy->LODs[level]->errorOffsets, hierarchy->errors[level])) //view.computePixelsOfError(center, hierarchy->errors[level]))
             < errorTermination) ||
            (hierarchy->LODs[level]->numTris > triTermination))
            break;
    }
    
    LODNumber = (level<0) ? 0 : level;
    
    updateStats();
    
    return;
} /** End of DiscreteCut::refine() **/


#ifdef GLOD
/*****************************************************************************\
 @ DiscreteCut::getReadbackSizes
 -----------------------------------------------------------------------------
 description :  READ BACK A PATCH!!!
 input       : 
 output      : 
 notes       :  
\*****************************************************************************/
void DiscreteCut::getReadbackSizes(int patch, GLuint* nindices, GLuint* nverts) {
    DiscreteLevel* obj = this->hierarchy->LODs[this->LODNumber];
    DiscretePatch* p = &(obj->patches[patch]);
    
    *nverts = p->getNumUniqueVerts();
    
    // how many indices do we have?
    *nindices = p->numIndices;
}

/*****************************************************************************\
 @ DiscreteCut::readback
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :  *patch.data_flags will be set for what should be produced.
                If we don't have a certain one of these, then don't procduce
                it and unset the flag.
\*****************************************************************************/
void DiscreteCut::readback(int npatch, GLOD_RawPatch* raw) {
    DiscreteLevel* obj = this->hierarchy->LODs[this->LODNumber];
    DiscretePatch* p = &(obj->patches[npatch]);

    AttribSetArray& verts = p->getVerts();
    
    // mask the raw settings against what we have
    if((raw->data_flags & GLOD_HAS_VERTEX_NORMALS) && (!verts.hasAttrib(AS_NORMAL)))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_VERTEX_NORMALS));

    if((raw->data_flags & GLOD_HAS_VERTEX_COLORS_3) && (! verts.hasAttrib(AS_COLOR)))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_VERTEX_COLORS_3));

    if((raw->data_flags & GLOD_HAS_TEXTURE_COORDS_2) && (!verts.hasAttrib(AS_TEXTURE0)))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_TEXTURE_COORDS_2));

    if((raw->data_flags & GLOD_HAS_TEXTURE_COORDS_3))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_TEXTURE_COORDS_3));

    if((raw->data_flags & GLOD_HAS_VERTEX_COLORS_4))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_VERTEX_COLORS_4));

    // verify that the raw is allocated properly
    assert(raw->num_triangles == p->numIndices / 3);
    assert(raw->num_vertices == p->getNumUniqueVerts());
    

    // move the vertices ...
    if(hierarchy->opType == Half_Edge_Collapse) {
        // renumber the cut vertices into this one location
        int *vertGlobalToLocal = new int[verts.getSize()];
        int local_nverts = 0;
        for(int i = 0; i < verts.getSize(); i++)
            vertGlobalToLocal[i] = -1;
        
        // move the indices... numbering the verts as we go....
        for(unsigned i = 0; i < p->numIndices; i++) {
            if(vertGlobalToLocal[p->indices[i]] == -1) {
                // p->indices[i] is unreferenced
                int glob_num = p->indices[i];
                int vert_num = local_nverts++;
                vertGlobalToLocal[glob_num] = vert_num;
                
                verts.getAt(glob_num, raw, vert_num);
            }
            raw->triangles[i] = vertGlobalToLocal[p->indices[i]];
        }
        delete [] vertGlobalToLocal;

    } else {
        int packI = 0;
        unsigned int i;
        for(i = 0; i < verts.getSize(); i++) {
            verts.getAt(i, raw, packI);
            packI++;
        }

        // move the indices
        for(unsigned i = 0; i < p->numIndices; i++)
            raw->triangles[i] = p->indices[i];
    }
    // done
}
#endif

#ifdef GLOD
/*****************************************************************************\
 @DiscreteCut::draw
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void DiscreteCut::draw(int patchnum) {

    if ((patchnum < 0) || (patchnum >= hierarchy->LODs[LODNumber]->numPatches))
    {
        fprintf(stderr, "DiscreteCut::draw(%i): invalid patch number [n=%i]\n", patchnum, hierarchy->LODs[LODNumber]->numPatches);
        return;
    }

  
    DiscretePatch *patch = &(hierarchy->LODs[LODNumber]->patches[patchnum]);
    AttribSetArray& verts = patch->getVerts();
    
    for (int area=0; area<GLOD_NUM_TILES; area++){
#ifdef GLOD_USE_TILES
        if (view.computePixelsOfError(hierarchy->LODs[LODNumber]->errorCenter, 
                                      hierarchy->LODs[LODNumber]->errorOffsets, 
                                      hierarchy->errors[LODNumber],area)==0)
            continue;
#endif
        if (patch->numIndices==0) return;
        
#ifdef GLOD_USE_LISTS
        if (! verts.glInitialized()) {
            int id = glGenLists(1);
            verts.glInit(false); // force use of VA

            glNewList(id, GL_COMPILE);
            verts.glBind(false);
            glDrawElements(GL_TRIANGLES, patch->numIndices, GL_UNSIGNED_INT,
                           patch->indices);
            verts.glUnbind(false);
            glEndList();
            verts.m_VBOid = id;
        }
        glCallList(id);
#else
        if(! verts.glInitialized())
            verts.glInit();
        
        verts.glBind();
        glDrawElements(GL_TRIANGLES, patch->numIndices, GL_UNSIGNED_INT,
                       patch->indices);
        verts.glUnbind();
#endif
    }
}
#endif

/*****************************************************************************\
 @ Discretehierarchy::Optimize
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void DiscreteHierarchy::Optimize() { // only defined for half-edge-collapsed hierarchies...
    if(this->opType != Half_Edge_Collapse) return;
    for(int pnum = 0; pnum < LODs[0]->numPatches; pnum++) {
        //        printf("Optimizing patch %i...\n", pnum); fflush(stdout);
        DiscretePatch* basePatch = &LODs[0]->patches[pnum];

        int *global_vertex_locations = new int[basePatch->getVerts().getSize()];
        for(int i= 0; i < basePatch->getVerts().getSize(); i++)
            global_vertex_locations[i]=i;
        
        int *verts_used_by_L1 = new int[basePatch->getVerts().getSize()];
        int *verts_used_by_L2 = new int[basePatch->getVerts().getSize()];

        int base_vertex_for_L1 = 0;
        for(int level2 = 1; level2 < numLODs; level2++) {
            
            int level1 = level2 - 1;
            DiscretePatch* p1 = &LODs[level1]->patches[pnum];
            DiscretePatch* p2 = &LODs[level2]->patches[pnum];
            for(int i = 0; i< basePatch->getVerts().getSize(); i++) {
                verts_used_by_L1[i] = -1;
                verts_used_by_L2[i] = -1;
            }

            // see which verts are used by each patch
            int verts_in_L1 = 0;
            for(int i= 0; i < p1->numIndices; i++) {
                int p1i = p1->indices[i];
                if(verts_used_by_L1[p1i] == 1) continue;
                verts_used_by_L1[p1i] = 1;
                verts_in_L1++;
            }
            
            // how many verts are unique? 
            int verts_only_in_L1 = 0;
            int verts_only_in_L2 = 0;
            int verts_in_both = 0;
            for(int i= 0; i < p2->numIndices; i++) {
                int p2i = p2->indices[i];
                if(verts_used_by_L2[p2i] == 1) continue;
                verts_used_by_L2[p2i] = 1;

                if(verts_used_by_L1[p2i] == 1) {
                    verts_in_both++;
                } else {
                    verts_only_in_L2++;
                }
            }
            verts_only_in_L1 = verts_in_L1 - verts_in_both;
            
            int* L1_ary = new int[verts_only_in_L1];
            int* both_ary = new int[verts_in_both];
            int* L2_ary = new int[verts_only_in_L2];

            
            // now do the optimization...
            int* L1_cur = L1_ary;
            int* both_cur = both_ary;
            int* L2_cur = L2_ary;
            for(int i= 0; i < p1->numIndices; i++) {
                int p1i = p1->indices[i];
                if(verts_used_by_L1[p1i] == 2) continue; // to prevent double counting
                verts_used_by_L1[p1i] = 2;
                if(verts_used_by_L2[p1i] == -1) { // unique to L1
                    *L1_cur = p1i;
                    L1_cur++;
                }
            }

            for(int i= 0; i < p2->numIndices; i++) {
                int p2i = p2->indices[i];
                if(verts_used_by_L2[p2i] == 2) continue; // to prevent double-counting
                verts_used_by_L2[p2i] = 2;
                
                if(verts_used_by_L1[p2i] == 2) {                    // shared vert... make sure that it is moved into the both group
                    *both_cur = p2i;
                    both_cur++;
                } else {
                    *L2_cur = p2i;
                    L2_cur++;
                }
            }
            
            /*
            printf("Just in L1: ");
            for(int i= 0; i < verts_only_in_L1; i++)
                printf("%i ", L1_ary[i]);
            printf("\n\n");

            printf("In both: ");
            for(int i= 0; i < verts_in_both; i++)
                printf("%i ", both_ary[i]);
            printf("\n\n");

            printf("Just in L2: ");
            for(int i= 0; i < verts_only_in_L2; i++)
                printf("%i ", L2_ary[i]);
                printf("\n\n");*/

            //base_vertex_for_L1 ... +verts_only_in_L1;
            //base_vertex_for_L1 + verts_only_in_L1 .... + verts_in_both
            //base_vertex_for_L1 + verts_in_L1 ..... +verts_only_in_L2;
            for(int i = 0; i < verts_only_in_L1; i++)
                global_vertex_locations[base_vertex_for_L1 + i] = L1_ary[i];
            
            for(int i = 0; i < verts_in_both; i++)
                global_vertex_locations[base_vertex_for_L1 + verts_only_in_L1 + i] = both_ary[i];

            for(int i = 0; i < verts_only_in_L2; i++)
                global_vertex_locations[base_vertex_for_L1 + verts_in_L1 + i] = L2_ary[i];
            
            // update the base_vertex
            base_vertex_for_L1 += verts_only_in_L1;
            delete [] L1_ary; delete [] both_ary; delete [] L2_ary;  
        }

        delete [] verts_used_by_L2;
        delete [] verts_used_by_L1;
        
        basePatch->Shuffle(global_vertex_locations);

        // move around the index arrays
        for(int level = 0; level < numLODs; level++) {
            DiscretePatch* p = &LODs[level]->patches[pnum];
            for(int i = 0; i < p->numIndices; i++) {
                p->indices[i] = global_vertex_locations[p->indices[i]];
            }
        }
        
        delete [] global_vertex_locations;
    }
}

int DiscretePatch::getNumUniqueVerts() {
    if(numUniqueVerts != -1) return numUniqueVerts;

    if(level->hierarchy->opType != Half_Edge_Collapse) {
        numUniqueVerts = verts.getSize();
        return numUniqueVerts;
    }

    DiscretePatch* basep;
    if(level->hierarchy->opType== Half_Edge_Collapse)
        basep = &level->hierarchy->LODs[0]->patches[patchNum];
    else
        basep = this;
    
    // how many vertices do we have?
    int *vertGlobalToLocal = new int[basep->verts.getSize()];
    int local_nverts = 0;
    for(int i = 0; i < basep->verts.getSize(); i++)
        vertGlobalToLocal[i] = -1;
    
    // count the unique verts
    for(unsigned i = 0; i < this->numIndices; i++) {
        if(vertGlobalToLocal[this->indices[i]] == -1) {
            // p->indices[i] is unreferenced
            int glob_num = this->indices[i];
            int vert_num = local_nverts++;
            vertGlobalToLocal[glob_num] = vert_num;
        }
    }
    delete [] vertGlobalToLocal;
    numUniqueVerts = local_nverts;
    return  numUniqueVerts;
}
