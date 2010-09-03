/* GLOD: DiscretePatch output
**************************************************************************/
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
#ifndef _GLOD_XBS_DISCRETEPATCH_H
#define _GLOD_XBS_DISCRETEPATCH_H

#include "glod_glext.h"
#include "Hierarchy.h"
#include "AttribSetArray.h"

//#define GLOD_USE_PATCH_SIMP
//#define GLOD_USE_LISTS

#ifndef XBS_SPLIT_BORDER_VERTS
#error DiscretePatchHierarchy will not work  unless border vert split is turned on in glodBuild/XBS!
#endif

class DiscretePatchLevel;
class DiscretePatchHierarchy;

class DiscretePatchPatch
{
 private:
    DiscretePatchLevel* level;
    
 public:
    int patchNum;
    AttribSetArray verts;

    unsigned int numIndices; // 3*numTris for GL_TRIANGLES
    unsigned int *indices;

    xbsVec3 errorCenter;
    xbsVec3 errorOffsets;

    float Error;

    //HeapElement budgetCoarsenHeapData;
    //HeapElement budgetRefineHeapData;
    
    DiscretePatchPatch() {
        numIndices = 0;
        indices = NULL;

        Error=0;
    };

    void Init(DiscretePatchLevel* l, int patchNum,
              bool hasColor, bool hasNormal, bool hasTexcoord) {
        this->level = l; 
        this->patchNum = patchNum;
        verts.create(hasColor, hasNormal, hasTexcoord);
    }
    
    ~DiscretePatchPatch() {
        if(indices != NULL) delete [] indices; 
    }

    void SetLevel(DiscretePatchLevel* l) { this->level = l; }
    void SetSize(int arraysize) { verts.setSize(arraysize); }
    void SetFrom(unsigned int vnum, xbsVertex* vert) {
        verts.setFrom(vnum, vert);
    }
    int getNumVerts() { return verts.getSize(); }
    int getNumTris() { return numIndices / 3; }

    int getNumUniqueVerts();
};

class DiscretePatchLevel
{
 public:
    DiscretePatchHierarchy* hierarchy;

    int numPatches;
    DiscretePatchPatch *patches;
    
    xbsVec3 errorCenter;
    xbsVec3 errorOffsets;

    int numTris;
    
    DiscretePatchLevel(DiscretePatchHierarchy* h, Model *model);
    DiscretePatchLevel(DiscretePatchHierarchy* h, GLOD_RawObject *raw, unsigned int level);
    DiscretePatchLevel() { }; // used by Hierarchy::load to rebuild us
    ~DiscretePatchLevel() {
        delete [] patches;
    }

 public:
    bool hasColor() {
        return patches[0].verts.hasAttrib(AS_COLOR);
    }
    bool hasNormal() {
        return patches[0].verts.hasAttrib(AS_NORMAL);
    }
    bool hasTexcoord() {
        return patches[0].verts.hasAttrib(AS_TEXTURE0);
    }
};

class DiscretePatchHierarchy : public Hierarchy
{
    public:
        DiscretePatchLevel **LODs;
        xbsReal *errors; // absolute objects space error for each LOD
        xbsReal *originalErrors;
        int numUsedLODs;
        int maxLODs;
        int *numLODs;

        //int *current;
        OperationType opType;
        DiscretePatchHierarchy(OperationType opType): Hierarchy(DiscretePatch_Hierarchy)
        {
            LODs = NULL;
            errors = NULL;
            numLODs = NULL;
            maxLODs = 0;
            numUsedLODs = 0;
            this->opType = opType;
            
            //current = NULL;
        }
        virtual ~DiscretePatchHierarchy()
        {
            for (int i=0; i<numUsedLODs; i++)
            {
                delete LODs[i];
                LODs[i] = NULL;
            }
            if (LODs != NULL)
            {
                delete LODs;
                LODs = NULL;
            }
            if (errors != NULL)
            {
                delete errors;
                errors = NULL;
            }
            numUsedLODs = 0;
            maxLODs = 0;
        }
        virtual void changeQuadricMultiplier(GLfloat multiplier);
        virtual void initialize(Model *model); // used when initializing from xbs
        virtual void initialize(GLOD_RawObject *raw); // used when initializing from DISCRETE_MANUAL
        virtual void manualAddLevel(Model *model, unsigned int level, float geometric_error);
        virtual void finalize(Model *model);
        virtual void update(Model *model, Operation *op);
        virtual void update(Model *model, Operation *op,
                            xbsVertex **sourceMappings,
                            xbsTriangle **changedTris, int numChangedTris,
                            xbsTriangle **destroyedTris, int numDestroyedTris)
        {
            update(model, op);
        };
    
        virtual void update(Model *model, EdgeCollapse *op,
                            xbsVertex **sourceMappings, xbsVertex **destMappings,
                            xbsTriangle **changedTris, int numChangedTris,
                            xbsTriangle **destroyedTris, int numDestroyedTris,
                            xbsVertex *generated_vert)
        {
            update(model, (Operation *)op);
        };

        virtual void debugWrite(char *filename=NULL);
    
        virtual void adapt()
        {
            printf("utter bs?\n");
            //current = numLODs/2;
        }
        virtual GLOD_Cut *makeCut();
        // Whole hierarchy readback
        virtual int  getReadbackSize();
        virtual void readback(void* dst);
        virtual int load(void* src); // returns 0 on fail

        virtual int GetPatchCount() {
            return LODs[0/*current*/]->numPatches;
        }
};

class DiscretePatchCut : public GLOD_Cut
{
    private:
        void updateStats()
        {
            int numPatches=hierarchy->LODs[0]->numPatches;
            currentNumTris=0;
            for (int i=0; i<numPatches; i++){
                DiscretePatchPatch *patch=&hierarchy->LODs[patchLevel[i]]->patches[i];
                currentNumTris+=patch->getNumTris();
            }
            //currentNumTris = hierarchy->LODs[LODNumber]->numTris;
            //refineTris = (LODNumber == 0) ? MAXINT :
            //hierarchy->LODs[LODNumber-1]->numTris;
            DiscretePatchPatch *patch = (DiscretePatchPatch*)refineQueue->min()->userData();
            if (patchLevel[patch->patchNum]==0)
                refineTris = currentNumTris;
            else{
                patch = &hierarchy->LODs[patchLevel[patch->patchNum]-1]->patches[patch->patchNum];
                refineTris = currentNumTris+patch->getNumTris();
            }
            //*(hierarchy->current)=LODNumber;
        }
        void computeBoundingSphere();
    
    public:
        DiscretePatchHierarchy *hierarchy;
        int LODNumber;
    
        Heap *refineQueue;
        Heap *coarsenQueue;
        int *patchLevel;
        
        int modeChange;
        ErrorMode lastMode;
        
    
        // bounding sphere
        xbsVec3 center;
        xbsReal radius;
        
        HeapElement **budgetCoarsenHeapData;
        HeapElement **budgetRefineHeapData;
        
        DiscretePatchPatch *lastCoarsen, *lastRefine;
    
        DiscretePatchCut(DiscretePatchHierarchy *hier, int LODNum)
        {
            hierarchy = hier;
            LODNumber = LODNum;
            computeBoundingSphere();
            patchLevel = new int[hierarchy->LODs[0]->numPatches];
            for (int i=0; i<hierarchy->LODs[0]->numPatches; i++)
                patchLevel[i]=0;
            refineQueue = new Heap;
            coarsenQueue = new Heap;
            int numPatches=hierarchy->LODs[0]->numPatches;
            
            budgetRefineHeapData = new HeapElement*[numPatches];
            budgetCoarsenHeapData = new HeapElement*[numPatches];
            modeChange=1;
            lastMode=ObjectSpace;
            refineQueue->clear();
            coarsenQueue->clear();
            for (int i=0; i<numPatches; i++){
                patchLevel[i]=hierarchy->numLODs[i]-1;
                //while ((hierarchy->LODs[patchLevel[i]]->patches[i].getNumTris()==0) )
                 //   patchLevel[i]--;
                DiscretePatchPatch *patch=&hierarchy->LODs[patchLevel[i]]->patches[i];
                //budgetRefineHeapData[i] = new HeapElement*[patchLevel[i]];
                //budgetCoarsenHeapData[i] = new HeapElement*[patchLevel[i]];
                //for (int j=0; j<hierarchy->numLODs[i]; j++){
                    budgetRefineHeapData[i]=new HeapElement(patch);
                    budgetCoarsenHeapData[i]=new HeapElement(patch);
                //}
                //float error = -view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[patchLevel[i]]);
                //float error = (mode==ScreenSpace) ? 
                //-view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[patchLevel[i]]):
                float error = -hierarchy->errors[patchLevel[i]];
                if (patch->getNumTris()==0) error=-MAXFLOAT/2.0f;
                budgetRefineHeapData[i]->setKey(error);
                budgetCoarsenHeapData[i]->setKey(-error);
                refineQueue->insert(budgetRefineHeapData[i]);
                coarsenQueue->insert(budgetCoarsenHeapData[i]);
            }
            lastCoarsen = NULL;
            lastRefine = NULL;
            updateStats();
        }

        virtual void viewChanged() { }
        virtual void adaptObjectSpaceErrorThreshold(float threshold);
        virtual void adaptScreenSpaceErrorThreshold(float threshold);
        virtual void draw(int patchnum);
        virtual void coarsen(ErrorMode mode, int triTermination,
                             float ErrorTermination);
        virtual void refine(ErrorMode mode, int triTermination,
                            float ErrorTermination);
        virtual xbsReal coarsenErrorObjectSpace(int area=-1) {
            //if (coarsenErrorScreenSpace(area)==0) return 0;
            if (lastRefine==NULL){ 
                lastRefine = (DiscretePatchPatch*)coarsenQueue->min()->userData();
            }
            DiscretePatchPatch *patch = lastRefine; //(DiscretePatchPatch*)coarsenQueue->min()->userData();
            patch = &hierarchy->LODs[patchLevel[patch->patchNum]]->patches[patch->patchNum];
            int patchNum = patch->patchNum;
            int level = patchLevel[patchNum];
            float error=1;
            if (level>=(hierarchy->numLODs[patchNum]-1)) 
                error = MAXFLOAT;
            else
                /*
                {
                if (coarsenErrorScreenSpace(area)==0)
                    error = 0;
                else
                    */
                    error = hierarchy->errors[level+1];
		//}
            return error;
            //return (LODNumber>=hierarchy->numLODs-1) ? MAXFLOAT : ((coarsenErrorScreenSpace(area)==0)?0:
            //      hierarchy->errors[LODNumber+1]);
        }
        virtual xbsReal currentErrorObjectSpace(int area=-1) {
            //if (refineQueue->size()==0)
            if (lastRefine==NULL){
                lastRefine = (DiscretePatchPatch*)refineQueue->min()->userData();
            }
            DiscretePatchPatch *patch = lastRefine; //(DiscretePatchPatch*)refineQueue->min()->userData();
            patch = &hierarchy->LODs[patchLevel[patch->patchNum]]->patches[patch->patchNum];
            int patchNum = patch->patchNum;
            int level = patchLevel[patchNum];
            if (level>=hierarchy->numLODs[patchNum]) return MAXFLOAT/2.0f;
            //if (currentErrorScreenSpace(area)==0) return 0;
            return hierarchy->errors[level];
            //if (LODNumber>=hierarchy->numLODs) return MAXFLOAT/2.0f;
            //if (currentErrorScreenSpace(area)==0) return 0;
            //return hierarchy->errors[LODNumber];
        }
        virtual xbsReal coarsenErrorScreenSpace(int area=-1) {
            if (lastRefine==NULL){ 
                lastRefine = (DiscretePatchPatch*)coarsenQueue->min()->userData();
            }
            DiscretePatchPatch *patch = lastRefine; //(DiscretePatchPatch*)coarsenQueue->min()->userData();
            int patchNum = patch->patchNum;
            int level = patchLevel[patchNum];
            if (level>=(hierarchy->numLODs[patchNum]-1))
                return MAXFLOAT;
            else{
                patch = &hierarchy->LODs[level+1]->patches[patch->patchNum];
                return view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[level+1], area);
            }
            //return (level>=(hierarchy->numLODs[patchNum]-1))?MAXFLOAT:
            //  view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[level+1], area);
            //    return (LODNumber>=hierarchy->numLODs-1)?MAXFLOAT:
            //      view.computePixelsOfError(hierarchy->LODs[LODNumber+1]->errorCenter, hierarchy->LODs[LODNumber+1]->errorOffsets, hierarchy->errors[LODNumber+1], area);
            //      return view.computePixelsOfError(center, 
            //              (LODNumber>=hierarchy->numLODs-1) ? 
            //              MAXFLOAT : hierarchy->errors[LODNumber+1]);
        }
        virtual xbsReal currentErrorScreenSpace(int area=-1) {
            if (lastRefine==NULL){
                lastRefine = (DiscretePatchPatch*)refineQueue->min()->userData();
            }
            DiscretePatchPatch *patch = lastRefine; //(DiscretePatchPatch*)refineQueue->min()->userData();
            patch = &hierarchy->LODs[patchLevel[patch->patchNum]]->patches[patch->patchNum];
            int patchNum = patch->patchNum;
            int level = patchLevel[patchNum];
            if (level>=hierarchy->numLODs[patchNum]) return MAXFLOAT/2.0f;
            return view.computePixelsOfError(patch->errorCenter, patch->errorOffsets, hierarchy->errors[level], area);
            //    if (LODNumber>=hierarchy->numLODs) return MAXFLOAT/2.0f;
            //      return view.computePixelsOfError(hierarchy->LODs[LODNumber]->errorCenter, hierarchy->LODs[LODNumber]->errorOffsets, hierarchy->errors[LODNumber], area); //view.computePixelsOfError(center, hierarchy->errors[LODNumber]);
        }
    
        virtual void debugWrite(char* filename) { 
            fprintf(stderr, "Not written.\n");
            return; 
        }

        // readback on a cut on a patch
        virtual void getReadbackSizes(int patch, GLuint* nindices, GLuint* nverts);
        virtual void readback(int npatch, GLOD_RawPatch* patch);

        void initVBO();
        void initLists();
};

#endif /* _GLOD_XBS_DISCRETE_H */

/**************************************************************************
 * $Log
 **************************************************************************/
