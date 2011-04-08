/* GLOD: Discrete output
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
#ifndef _GLOD_XBS_DISCRETE_H
#define _GLOD_XBS_DISCRETE_H

#include "glod_glext.h"
#include "Hierarchy.h"
#include "AttribSetArray.h"

#ifndef XBS_SPLIT_BORDER_VERTS
typedef struct PatchVertPair {
    unsigned int patch;
    unsigned int vertNum;
    PatchVertPair* pNext;
} PatchVertPair;
#endif

class DiscreteLevel;

//#define GLOD_USE_LISTS
class DiscretePatch
{
 private:
    DiscreteLevel* level;
    int patchNum;
    int numUniqueVerts; // cached!
    AttribSetArray verts;

 public:
    //        unsigned int numVerts;    
    
    unsigned int numIndices; // 3*numTris for GL_TRIANGLES
    unsigned int *indices;
    
    DiscretePatch() {
        numIndices = 0;
        indices = NULL;
        numUniqueVerts = -1;
    };
    void Init(DiscreteLevel* l, int patchNum,
              bool hasColor, bool hasNormal, bool hasTexcoord) {
        this->level = l; 
        this->patchNum = patchNum;
        verts.create(hasColor, hasNormal, hasTexcoord);
    }
    
    ~DiscretePatch() {
        if(indices != NULL) delete [] indices; 
    }
    void SetPatchNum(int pNum) {patchNum=pNum;};
    void SetLevel(DiscreteLevel* l) { this->level = l; }
    void SetSize(int arraysize) { verts.setSize(arraysize); }
    void SetFrom(unsigned int vnum, xbsVertex* vert) {
        verts.setFrom(vnum, vert);
    }
    void Shuffle(int* new_locations) { verts.shuffle(new_locations);}
    AttribSetArray& getVerts();
    
    int getNumVerts() { return getVerts().getSize(); }

    int getNumUniqueVerts();
};

class DiscreteHierarchy;
class DiscreteLevel
{
 public:
    DiscreteHierarchy* hierarchy;
    //        char hasColor, hasNormal, hasTexcoord;
    
    int numPatches;
    DiscretePatch *patches;
    
    xbsVec3 errorCenter;
    xbsVec3 errorOffsets;
    
    int numTris;
    
    DiscreteLevel(DiscreteHierarchy* h, Model *model);
#ifdef GLOD
    DiscreteLevel(DiscreteHierarchy* h, GLOD_RawObject *raw, unsigned int level);
#endif
    DiscreteLevel() { }; // used by Hierarchy::load to rebuild us
    ~DiscreteLevel() {
        delete [] patches;
    }
    
 public:
    bool hasColor() {
        return patches[0].getVerts().hasAttrib(AS_COLOR);
    }
    bool hasNormal() {
        return patches[0].getVerts().hasAttrib(AS_NORMAL);
    }
    bool hasTexcoord() {
        return patches[0].getVerts().hasAttrib(AS_TEXTURE0);
    }

};




class DiscreteHierarchy : public Hierarchy
{
    public:
        DiscreteLevel **LODs;
        xbsReal *errors; // absolute objects space error for each LOD
        xbsReal *originalErrors;
        int numLODs;
        int maxLODs;

#ifndef XBS_SPLIT_BORDER_VERTS
        int nPatchPairs;
        int maxPatchPairs;
        PatchVertPair** patchPairs;
#endif

        int current;
        OperationType opType;

        DiscreteHierarchy(OperationType opType) : Hierarchy(Discrete_Hierarchy) {
            LODs = NULL;
            errors = NULL;
            numLODs = 0;
            maxLODs = 0;
            current = 0;
            this->opType = opType;

#ifndef XBS_SPLIT_BORDER_VERTS
            nPatchPairs = 0;
            patchPairs = NULL;
#endif
        }
        virtual ~DiscreteHierarchy()
        {
            for (int i=0; i<numLODs; i++)
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
            
            numLODs = 0;
            maxLODs = 0;
        }
        virtual void initialize(Model *model); // used when initializing from xbs
#ifdef GLOD
        virtual void initialize(GLOD_RawObject *raw); // used when initializing from DISCRETE_MANUAL
#endif
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

        void Optimize();
        
        int GetVertIdx(int patchNum, xbsVertex* vert);
        void SetVertIdx(int patchNum, xbsVertex* vert, int vertNum);
        
        
        //virtual void debugWrite(char *filename=NULL);
        
        virtual void adapt()
        {
            current = numLODs/2;
        }
        virtual GLOD_Cut *makeCut();
        // Whole hierarchy readback
        virtual int  getReadbackSize();
        virtual void readback(void* dst);
        virtual int load(void* src); // returns 0 on fail
        virtual void changeQuadricMultiplier(GLfloat multiplier);
        virtual int GetPatchCount() {
            return LODs[current]->numPatches;
        }
};

class DiscreteCut : public GLOD_Cut
{
    private:
        void updateStats()
        {
            currentNumTris = hierarchy->LODs[LODNumber]->numTris;
            refineTris = (LODNumber == 0) ? MAXINT :
                hierarchy->LODs[LODNumber-1]->numTris;
            hierarchy->current=LODNumber;
        }
        void computeBoundingSphere();
    
    public:
        DiscreteHierarchy *hierarchy;
        int LODNumber;
    
        // bounding sphere
        xbsVec3 center;
        xbsReal radius;
    
        DiscreteCut(DiscreteHierarchy *hier, int LODNum)
        {
            hierarchy = hier;
            LODNumber = LODNum;
            computeBoundingSphere();
            updateStats();
        }

        virtual ~DiscreteCut() { }

        virtual void viewChanged() { }
        virtual void adaptObjectSpaceErrorThreshold(float threshold);
        virtual void adaptScreenSpaceErrorThreshold(float threshold);
#ifdef GLOD
        virtual void draw(int patchnum);
#endif
        virtual void coarsen(ErrorMode mode, int triTermination,
                             float ErrorTermination);
        virtual void refine(ErrorMode mode, int triTermination,
                            float ErrorTermination);
        virtual xbsReal coarsenErrorObjectSpace(int area=-1) {
            //if (coarsenErrorScreenSpace(area)==0) return 0;
            return (LODNumber>=hierarchy->numLODs-1) ? MAXFLOAT : ((coarsenErrorScreenSpace(area)==0)?0:
                                                                   hierarchy->errors[LODNumber+1]);
        }
        virtual xbsReal currentErrorObjectSpace(int area=-1) {
            if (LODNumber>=hierarchy->numLODs) return MAXFLOAT/2.0f;
            if (currentErrorScreenSpace(area)==0) return 0;
            return hierarchy->errors[LODNumber];
        }
        virtual xbsReal coarsenErrorScreenSpace(int area=-1) {
            return (LODNumber>=hierarchy->numLODs-1)?MAXFLOAT:view.computePixelsOfError(hierarchy->LODs[LODNumber+1]->errorCenter, hierarchy->LODs[LODNumber+1]->errorOffsets, hierarchy->errors[LODNumber+1], area);
            //      return view.computePixelsOfError(center, 
            //              (LODNumber>=hierarchy->numLODs-1) ? 
            //              MAXFLOAT : hierarchy->errors[LODNumber+1]);
        }
        virtual xbsReal currentErrorScreenSpace(int area=-1) {
            if (LODNumber>=hierarchy->numLODs) return MAXFLOAT/2.0f;
            return view.computePixelsOfError(hierarchy->LODs[LODNumber]->errorCenter, hierarchy->LODs[LODNumber]->errorOffsets, hierarchy->errors[LODNumber], area); //view.computePixelsOfError(center, hierarchy->errors[LODNumber]);
        }
    
        /*virtual void debugWrite(char* filename) { 
            fprintf(stderr, "Not written.\n");
            return; 
        }*/

        // readback on a cut on a patch
#ifdef GLOD
        virtual void getReadbackSizes(int patch, GLuint* nindices, GLuint* nverts);
        virtual void readback(int npatch, GLOD_RawPatch* patch);
#endif

        void initVBO();
        void initLists();
};


#endif /* _GLOD_XBS_DISCRETE_H */

/**************************************************************************
 * $Log
 **************************************************************************/
