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
  MTHierarchy.h
  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/MTHierarchy.h,v $
  $Revision: 1.3 $
  $Date: 2004/10/20 19:36:55 $
  $Author: gfx_friends $
\*****************************************************************************/

#ifndef _MTHIERARCHY_H
#define _MTHIERARCHY_H


#include "xbs.h"
#include "mt.h"

class MTHierarchy : public Hierarchy
{
    public:
        MT *mt;

        MTHierarchy() : Hierarchy(MT_Hierarchy) {mt = new MT;};
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

        virtual void debugWrite(char *filename) {mt->writePlyMT(filename);};
        virtual ~MTHierarchy() {delete mt; mt=NULL;};

        virtual int  getReadbackSize() {
            fprintf(stderr, "Hierarchy readback for MT not implemented yet.\n");
            return 0;
        } 
        virtual void readback(void* dst) {
            fprintf(stderr, "Hierarchy readback for MT not implemented yet.\n");
        }
        virtual int load(void* src) {
            fprintf(stderr, "Load (and readback) for MT not implemented.\n");
            return 0;
        }
        virtual int GetPatchCount() {
            exit(0);
            return 0;
        }
        virtual void changeQuadricMultiplier(GLfloat multiplier) { }
};

#endif
/***************************************************************************
 $Log: MTHierarchy.h,v $
 Revision 1.3  2004/10/20 19:36:55  gfx_friends
 Patch to MTHierarchy.h so it compiles. -n

 Revision 1.2  2004/10/20 19:33:32  gfx_friends
 Rewrote MTHierarchy driver, made the VDS hack in Operation.C vds-specific

 Revision 1.1  2004/08/23 16:47:09  gfx_friends
 Pulled MT into a new file so that I can do some experiments with the shortcut algorithm.--nat

 ***************************************************************************/
