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
#ifndef _GLOD_XBS_CONTINUOUS_H
#define _GLOD_XBS_CONTINUOUS_H

#include "xbs.h"
#include "Hierarchy.h"

class VDSHierarchy : public Hierarchy
{
    private:
        int *danglingVerts;
        int numDanglingVerts;
        int maxDanglingVerts;
    
    public:
            GLfloat quadricMultiplier;    
        
        Vif *vif; // used for building VDS  
    
        Forest *mpForest;  
    
        VDSHierarchy()  : Hierarchy(VDS_Hierarchy)
        {
            vif = NULL;
            mpForest = NULL;
            numDanglingVerts = 0;
            danglingVerts = new int();
            maxDanglingVerts = 1;
        }
        virtual ~VDSHierarchy() {
            if(mpForest != NULL)
                delete mpForest;
            if(danglingVerts != NULL)
                delete [] danglingVerts;
            if(vif != NULL)
                delete vif;
        };
        void InitForLoad() { 
            delete [] danglingVerts;
            danglingVerts = NULL;
            maxDanglingVerts = 0;
            numDanglingVerts = 0;
            mpForest = new Forest; 
        }
        virtual void changeQuadricMultiplier(GLfloat multiplier);
        virtual void initialize(Model *model);
        virtual void finalize(Model *model);
        virtual void update(Model *model, Operation *op,
                            xbsVertex **sourceMappings,
                            xbsTriangle **changedTris, int numChangedTris,
                            xbsTriangle **destroyedTris, int numDestroyedTris);
        virtual void update(Model *model, EdgeCollapse *op,
                            xbsVertex **sourceMappings, xbsVertex **destMappings,
                            xbsTriangle **changedTris, int numChangedTris,
                            xbsTriangle **destroyedTris, int numDestroyedTris,
                            xbsVertex *generated_vert);
        /*virtual void debugWrite(char *filename)
        {
            fprintf(stderr, "VDSHierarchy write not written!\n");
            if (vif == NULL)
                return;
            fprintf(stderr, "Writing Vif file...");
            if (vif->WriteVif2_2("hierarchy.vif") == false)
            {
                fprintf(stderr, "\nerror writing Vif!\n");
                exit(1);
            }
            fprintf(stderr, "done!\n");
        };*/
        
        virtual GLOD_Cut* makeCut();
        virtual int  getReadbackSize();
        virtual void readback(void* dst);
        virtual int load(void* src);

        virtual int GetPatchCount() {
            return mpForest->mNumPatches;
        }
};

class VDSCut : public GLOD_Cut
{    
    public:
        VDSHierarchy *hierarchy;
        VDS::Renderer *mpRenderer;
        VDS::Cut *mpCut;

        VDSCut(VDSHierarchy *hier);

        virtual ~VDSCut()
        {
            if(mpCut != NULL)
                delete mpCut;
            if(mpRenderer != NULL)
                delete mpRenderer;
        };      
                
        virtual void setGroup(GLOD_Group* glodgroup);
        virtual void viewChanged();
        virtual void adaptObjectSpaceErrorThreshold(float threshold);
        virtual void adaptScreenSpaceErrorThreshold(float threshold);
        virtual void draw(int patchnum);
        
        virtual void BindAdaptXform();
        
        virtual void coarsen(ErrorMode mode, int triTermination,
                             float ErrorTermination);
        virtual void refine(ErrorMode mode, int triTermination,
                            float ErrorTermination);
    
        virtual xbsReal coarsenErrorObjectSpace(int area=-1);
        virtual xbsReal currentErrorObjectSpace(int area=-1);
        virtual xbsReal coarsenErrorScreenSpace(int area=-1);
        virtual xbsReal currentErrorScreenSpace(int area=-1);
        
        virtual void updateStats();

        virtual void getReadbackSizes(int patch, GLuint* nindices, GLuint* nverts);
        virtual void readback(int npatch, GLOD_RawPatch* patch);

        void initVBO();
};

#endif //#ifndef _GLOD_XBS_CONTINUOUS_H

/**************************************************************************
  $Log: Continuous.h,v $
  Revision 1.10  2004/10/20 19:33:27  gfx_friends
  Rewrote MTHierarchy driver, made the VDS hack in Operation.C vds-specific

  Revision 1.9  2004/10/12 16:34:54  gfx_friends
  added a multiplier for error quadrics so the error can be more evenly distributed between the different error metrics. It can be changed during runtime

  Revision 1.8  2004/07/09 22:10:04  gfx_friends
  Fixed a ton of VDS-related memory leaks.

  Revision 1.7  2004/07/08 16:47:50  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.6  2004/06/25 18:58:41  gfx_friends
  New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default

  Revision 1.5  2004/06/11 18:30:07  gfx_friends
  Remove all sources of warnings in xbs directory when compiled with -Wall

  Revision 1.4  2004/01/20 07:26:37  gfx_friends
  Fixed cross platform build issues. Apparently noone has been checking to see if we build for Linux of late.

  Revision 1.3  2004/01/12 16:25:45  bms6s
  removed compile-time vbo flags

  moved discrete rendering/initialization from DiscreteObject to DiscreteCut

  added VBO initialization and rendering for continuous cuts (currently crashes due to glMapBuffer() returning null if more than one continuous cut is in use - still debugging this)

  Revision 1.2  2003/12/18 19:48:05  bms6s
  not sure how that got in there...

  Revision 1.1  2003/12/18 19:44:08  bms6s
  separated out VDSHierarchy and VDSCut into continuous.h/c


\*****************************************************************************/
