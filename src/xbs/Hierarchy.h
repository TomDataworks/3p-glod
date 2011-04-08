/* GLOD: Hierarchy output types
***************************************************************************/
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
#ifndef HIERARCHY_H
#define HIERARCHY_H
#include "Model.h"
#include "vds_callbacks.h"
#include "manager.h"

extern VDS::Manager s_VDSMemoryManager;

// predefs
class EdgeCollapse;
class Model;
class Operation;

#include "View.h"

class GLOD_Cut
{
    public:
        int currentNumTris;  // current tris             
        int refineTris;      // tris after next refine

        // let cuts have a pointer to their group
        GLOD_Group* group;

        GLOD_View view;
    
        // need some kind of camera
    
        GLOD_Cut(){
            currentNumTris = 0;
        };
        virtual ~GLOD_Cut(){
        }
        virtual void setGroup(GLOD_Group* glodgroup) { group = glodgroup;};
        virtual void adaptObjectSpaceErrorThreshold(float threshold) {};
        virtual void adaptScreenSpaceErrorThreshold(float threshold) {};
#ifdef GLOD
        virtual void draw(int patchnum) {};
#endif
        virtual void viewChanged() = 0;
        virtual void coarsen(ErrorMode error,
                             int triTermination,
                             float errorTermination) {};
        virtual void refine(ErrorMode error,
                            int triTermination,
                            float errorTermination) {};
        virtual xbsReal coarsenErrorObjectSpace(int area=-1) = 0;
        virtual xbsReal currentErrorObjectSpace(int area=-1) = 0;
        virtual xbsReal coarsenErrorScreenSpace(int area=-1) = 0;
        virtual xbsReal currentErrorScreenSpace(int area=-1) = 0;
        virtual void updateStats()=0;

        // readback of a patch on a cut
#ifdef GLOD
        virtual void getReadbackSizes(int patch, GLuint* nindices, GLuint* nverts) = 0;
        virtual void readback(int npatch, GLOD_RawPatch* patch) = 0;
#endif

    
        // VBO rendering stuff
        GLuint VBO_id;
}; // GLOD_Cut

class Hierarchy
{
    private:
        int ref_count;
        OutputType hier_type;
    public:
        
        Hierarchy(OutputType ht) {ref_count = 0; hier_type = ht;};
        virtual void initialize(Model *model) {};
        virtual void finalize(Model *model) {};
        virtual void update(Model *model, Operation *op,
                            xbsVertex **sourceMappings,
                            xbsTriangle **changedTris, int numChangedTris,
                            xbsTriangle **destroyedTris, int numDestroyedTris) {};
        virtual void update(Model *model, EdgeCollapse *op,
                            xbsVertex **sourceMappings, xbsVertex **destMappings,
                            xbsTriangle **changedTris, int numChangedTris,
                            xbsTriangle **destroyedTris, int numDestroyedTris,
                            xbsVertex *generated_vert) {};
        virtual ~Hierarchy() {};

        //virtual void debugWrite(char *filename) = 0;
    
        // readback of a full hierarchy
        virtual int load(void* src) = 0; // returns 0 on fail
        virtual int  getReadbackSize() = 0;
        virtual void readback(void* dst) = 0;

        virtual void changeQuadricMultiplier(GLfloat multiplier) = 0;
        
        virtual int GetPatchCount() = 0;

        OutputType getHierarchyType() { return hier_type; }
        
        void LockInstance() {
            ref_count++;
        }
     
        void ReleaseInstance() {
            ref_count--;
            if(ref_count == 0) {
                delete this;
            }
        }

        virtual GLOD_Cut *makeCut() {return NULL;};
};


/**************************************************************************
  $Log: Hierarchy.h,v $
  Revision 1.44  2004/10/20 19:33:30  gfx_friends
  Rewrote MTHierarchy driver, made the VDS hack in Operation.C vds-specific

  Revision 1.43  2004/10/12 16:34:54  gfx_friends
  added a multiplier for error quadrics so the error can be more evenly distributed between the different error metrics. It can be changed during runtime

  Revision 1.42  2004/08/23 16:47:09  gfx_friends
  Pulled MT into a new file so that I can do some experiments with the shortcut algorithm.--nat

  Revision 1.41  2004/07/21 18:43:42  gfx_friends
  Added a flag XBS_SPLIT_BORDER_VERTS that allows us to tell whether a xbsVertex appears in multiple patches or not. If this value is defined, which it should always be for GLOD, the algorithm used to assign a vertex to a patch can be considerably simpler.

  Revision 1.40  2004/07/21 18:37:48  gfx_friends
  ND:
   Fixes for separate compilation of xbs without glod, etc.

  Revision 1.39  2004/07/20 21:54:36  gfx_friends
  Major Discrete and AttribSet rewrite. Discrete now uses attribsets for its vertex data. -Nat

  Revision 1.38  2004/07/09 22:10:04  gfx_friends
  Fixed a ton of VDS-related memory leaks.

  Revision 1.37  2004/07/08 20:18:45  gfx_friends
  couple small changes to the error metric code potentially fixing up problems with errors not getting assigned to new vertices properly in special cases

  Revision 1.36  2004/07/08 16:47:50  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.35  2004/07/08 16:15:30  gfx_friends
  many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)

  Revision 1.34  2004/06/25 18:58:42  gfx_friends
  New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default

  Revision 1.33  2004/03/12 17:00:53  gfx_friends
  First stab at using bounding boxes for error calculations. VDS can finally once again be checked out (i hope...)

  Revision 1.32  2004/02/19 15:51:27  gfx_friends
  Made the system compile in Win32 and patched a bunch of warnings.

  Revision 1.31  2004/02/05 20:34:10  gfx_friends
  Modified Discrete and Continuous and Hierarchy to use glod_glext.h instead of their own custom VBO code.

  Revision 1.30  2004/02/04 17:15:02  gfx_friends
  Adding apple makefiles, code changes, which _hopefully_ won't break anything else...

  Revision 1.29  2004/01/13 19:48:37  bms6s
  fixed the problem with discrete instance vbo rendering by making the vbo functions static and changing the condition for initVBO()
  (this fix will make the glGetString call every frame if vbo isn't enabled)

  Revision 1.28  2004/01/13 16:44:48  bms6s
  fixed bug in vbo rendering of discrete object instances
  removed vbo debug printfs

  Revision 1.27  2004/01/13 15:43:14  bms6s
  fixed vbo rendering of more than one continuous cut
  commented out some debug printfs

  Revision 1.26  2004/01/12 20:03:54  gfx_friends
  Small addition, automatically switching the VBO on/off depending on the extensions available.

  Revision 1.25  2004/01/12 16:17:47  bms6s
  moved VBO stuff into GLOD_Cut
  added an initExt() function to GLOD_Cut which initializes the vbo functions

  Revision 1.24  2003/12/18 19:44:07  bms6s
  separated out VDSHierarchy and VDSCut into continuous.h/c

  Revision 1.23  2003/11/04 18:39:44  gfx_friends
  Small change to GLOD Cut, added a updateStats function to make it more
  compatible with discrete calls

  Revision 1.22  2003/10/23 01:44:25  bms6s
  -removed coarsenError, currentError, coarsenRadius, coarsenCenter, refineRadius, and refineCenter from GLOD_Cut.  replaced them with coarsenErrorObjectSpace(), coarsenErrorScreenSpace(), currentErrorObjectSpace(), and currentErrorScreenSpace().  this makes code much cleaner and ensures consistency in the errors of discrete and continuous cuts (at least inasmuch as the discrete cuts's errors[] array is consistent with VDS' node radii).
  -added call to update VDS errors at the beginning of every continuous cut coarsen() or refine() call.  this is more than necessary; conceivably we could just call it once per glod adaptXXX() call, perhaps in the beginning just before the queues are constructed.
  -if coarsen() is going to get called with triTermination equal to the number of tris already in the cut, then instead we set triTermination to 0.
  -modified vds node error callback StdErrorScreenSpaceNoFrustum to use the VDS::Cut's mpExternalViewClass pointer to calculate node error using computePixelsOfError().  so this means that view frustum simplification is currently disabled.  it shouldn't be too hard to similarly convert StdErrorScreenSpace to use glod's view frustum check and computePixelsOfError as well.
  -modified computePixelsOfError to only take a center and object space error (that's all it was using anyway), since that's all VDS can provide it.

  Revision 1.21  2003/10/22 00:46:17  bms6s
  added computePixelsOfError function to GLOD_Cut, which by default just calls view.computePixelsOfError.  VDSCut, however, instead calls StdErrorScreenSpace, since that is the function used to calculate errors of the VDS::Cut.

  Revision 1.20  2003/10/21 20:43:36  bms6s
  triangle budget with continuous cuts works in object space mode; now working on screen space mode

  Revision 1.19  2003/10/21 19:15:45  bms6s
  i think this needs to be initialized here..

  Revision 1.18  2003/10/21 05:08:12  bms6s
  implemented necessary functions for triangle budget adaptation of groups with a continuous cut (still not functional though due to other issues).

  Revision 1.17  2003/10/20 21:48:39  gfx_friends
  We now have cut readback of VDS objects. Yay.

  Note: if the code for ImmediateModeRenderCallback were to change, then we need to probably update the code that I've written in Hierarchy.C to match. However, this code isn't terribly complicated, so I suspect we won't really get into this situation much.

  Revision 1.16  2003/08/29 21:15:10  gfx_friends
  Trying to fix up the binding from GL matrices to VDS.

  Revision 1.15  2003/08/29 19:30:30  gfx_friends
  Added VDS readback support and fixed up the readback code a bit.

  Revision 1.14  2003/08/14 20:38:50  gfx_friends
  Added the new glodObjectXform parameters and fixed a few related things. However, outstanding issues that exist now are (a) we still compute our errors in pixels, whereas we've decided to switch to angle-of-error, and (b) We can't make VDS work until we either map it to 1 cut/object or change VDS to support transformations per object regardless of cut.

  Revision 1.13  2003/07/26 01:17:43  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.12  2003/07/23 19:55:34  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.11  2003/07/23 06:36:08  bms6s
  vds integrated

  Revision 1.10  2003/07/22 02:57:19  gfx_friends
  Added code to support three object parameters: BUILD_OPERATOR,
  BUILD_QUEUE, and SHARE_TOLERANCE. Operator is things like
  EDGE_COLLAPSE and HALF_EDGE_COLLAPSE. QUEUE is GREEDY, LAZY, or
  INDEPENDENT, and SHARE_TOLERANCE is merges vertices up to some
  geometry tolerance before simplification. If attributes differ, they
  become multi-attribute verts.

  Also added a new little cylinder model to test multi-attribute
  vertices within a single patch.

  Revision 1.9  2003/07/21 23:10:53  gfx_friends
  Added cut readback support. I'm still debugging, but need to move computers to my home. --n

  Revision 1.8  2003/07/21 21:42:41  gfx_friends
  Jon Cohen -

  Made screen-space error threshold mode work again with discrete
  hierarchies. Screen-space error computation currently assumes
  projection matrix contains a perspective projection with a 45-degree
  field of view.

  Also added an error-mode toggle key to the simple sample.

  Revision 1.7  2003/07/19 23:50:10  gfx_friends
  Jon Cohen:

  Fixed up output to VDS hierarchy from half edge collapse and edge
  collapse operators. Seems to work, but it's still subject to review by
  Brenden. Also fixed a bug in vertex sharing.

  Revision 1.6  2003/07/18 22:19:43  gfx_friends
  Fixed most of the build problems. The lights have mysteriously gone off in the simple test program... I'm trying to figure out why. But the rest works, I think

  Revision 1.5  2003/07/16 03:12:28  gfx_friends
  Added xbs support for "multi-attribute vertices". These are
  geometrically coincident vertices that may have different
  attributes. Geometric coincidence is maintained throughout the
  simplification process and attributes are correctly propagated along.

  For the full edge collapse, the heuristics for preventing attribute
  seams along patch boundaries could still use a little work.

  Things seem to work for the DiscreteHierarchy output. VDS hierarchy
  has not been integrated yet.

  Revision 1.4  2003/07/01 21:49:12  gfx_friends
  Tore out the glodDrawObject in favor of the more complete glodDrawPatch. A bug somewhere between glodBuildObject's call to new Model and the draw, however, is still failing, as all of the patches have identical geometry by the time they hit the drawing point.

  Revision 1.3  2003/07/01 20:49:17  gfx_friends
  Readback for Discrete LOD now works. See samples/readback/read_model for demo and test. This, parenthetically, is also the most fully functional test program that we have. I will bring "simple" up to speed eventually, but in the meantime, you can use read_model similarly (run --help) in much the same way as previous ones to do testing.

  Revision 1.2  2003/06/30 20:37:06  gfx_friends
  Beginnings of a readback system. -- n

  Revision 1.25  2003/06/30 19:29:58  gfx_friends
  (1) InsertElements now works completely. --nat
  (2) Some XBS classes got moved from xbs.h into discrete.h and Hierarchy.h for
      cleanliness.
**************************************************************************/

#endif // HIERARCHY_H
