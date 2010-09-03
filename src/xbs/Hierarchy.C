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
  Hierarchy.C
  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/Hierarchy.C,v $
  $Revision: 1.47 $
  $Date: 2004/09/14 20:10:59 $
  $Author: jdt6a $
  $Locker:  $
\*****************************************************************************/


/*----------------------------- Local Includes -----------------------------*/
#include "xbs.h"
#include "vds_callbacks.h"

/*----------------------------- Local Constants -----------------------------*/


/*------------------------------ Local Macros -------------------------------*/

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/

/*------------------------ Local Function Prototypes ------------------------*/

int
compare_tri_end_nodes(const void *a, const void *b);

/*---------------------------------Functions-------------------------------- */

/*****************************************************************************\
 @ compare_tri_end_nodes
 -----------------------------------------------------------------------------
 description :
 input       :
 output      :
 notes       :
\*****************************************************************************/
int compare_tri_end_nodes(const void *a, const void *b)
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
  $Log: Hierarchy.C,v $
  Revision 1.47  2004/09/14 20:10:59  jdt6a
  added compare_tri_end_nodes back to hierarchy.c to get things to compile again

  Revision 1.46  2004/08/23 16:47:09  gfx_friends
  Pulled MT into a new file so that I can do some experiments with the shortcut algorithm.--nat

  Revision 1.45  2004/07/08 16:47:48  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.44  2004/02/05 20:34:09  gfx_friends
  Modified Discrete and Continuous and Hierarchy to use glod_glext.h instead of their own custom VBO code.

  Revision 1.43  2004/01/13 19:48:36  bms6s
  fixed the problem with discrete instance vbo rendering by making the vbo functions static and changing the condition for initVBO()
  (this fix will make the glGetString call every frame if vbo isn't enabled)

  Revision 1.42  2003/12/18 19:44:07  bms6s
  separated out VDSHierarchy and VDSCut into continuous.h/c

  Revision 1.41  2003/12/14 06:11:05  bms6s
  added variables to adaptTriangleBudget to track the last object coarsened/refined and the parameters it was adapted to; if the same object gets called twice in a row with the same adaptation parameters, then it is removed from the queues.  this seems to get rid of the infinite loops that were occurring due to the vds queues being unbalanced.  it is still possible to have the "oscillating objects" effect due to a repeating sequence of two calls to adaptTriangleBudget such as:
  {
  calling adaptTriangleBudget()
  refining 0
  coarsening 2
  coarsening 1
  coarsening 1 (duplicate - removing 1 from queues)
  coarsening 0
  refining 2

  calling adaptTriangleBudget()
  refining 0
  coarsening 2
  coarsening 1
  coarsening 0
  refining 2
  refining 1
  refining 1 (duplicate - removing 1 from queues)
  refining 2
  coarsening 2
  }
  repeating over and over.  it doesn't happen all that frequently, but still looks unsightly so we should try to figure out how to fix it.

  Revision 1.40  2003/11/13 23:01:50  bms6s
  if no more refinement is possible, VDSCut should set refineTris to MAXINT

  Revision 1.39  2003/10/23 04:38:17  bms6s
  support for proper triangle budget adaptation of a group with multiple continuous cuts
  -all vds cuts within a group use the same simplifier, and calling coarsen or refine on any of them actually results in adaptation of all of them; therefore, we only put the first vds cut encountered into the queues.
  -this also means that the node on top of the fold or unfold queue of a cut's simplifier may not actually belong to that cut, so when calculating screen space coarsenError and currentError it is improper to use the calling cut's view parameters.  instead, we just get the error directly from the budgetitem in the queue, since it will have been set using the correct cut's view parameters in UpdateNodeErrors().
  -encountered situations in which the adaptTriangleBudget() code was calling coarsen with a negative triTermination; since VDS uses an unsigned int as the triTermination, this was obviously causing problems, so if the triTermination is less than zero it now gets set to zero before the SimplifyBudgetAndThreshold() call.

  Revision 1.38  2003/10/23 01:44:24  bms6s
  -removed coarsenError, currentError, coarsenRadius, coarsenCenter, refineRadius, and refineCenter from GLOD_Cut.  replaced them with coarsenErrorObjectSpace(), coarsenErrorScreenSpace(), currentErrorObjectSpace(), and currentErrorScreenSpace().  this makes code much cleaner and ensures consistency in the errors of discrete and continuous cuts (at least inasmuch as the discrete cuts's errors[] array is consistent with VDS' node radii).
  -added call to update VDS errors at the beginning of every continuous cut coarsen() or refine() call.  this is more than necessary; conceivably we could just call it once per glod adaptXXX() call, perhaps in the beginning just before the queues are constructed.
  -if coarsen() is going to get called with triTermination equal to the number of tris already in the cut, then instead we set triTermination to 0.
  -modified vds node error callback StdErrorScreenSpaceNoFrustum to use the VDS::Cut's mpExternalViewClass pointer to calculate node error using computePixelsOfError().  so this means that view frustum simplification is currently disabled.  it shouldn't be too hard to similarly convert StdErrorScreenSpace to use glod's view frustum check and computePixelsOfError as well.
  -modified computePixelsOfError to only take a center and object space error (that's all it was using anyway), since that's all VDS can provide it.

  Revision 1.37  2003/10/21 05:08:12  bms6s
  implemented necessary functions for triangle budget adaptation of groups with a continuous cut (still not functional though due to other issues).

  Revision 1.36  2003/10/20 21:48:39  gfx_friends
  We now have cut readback of VDS objects. Yay.

  Note: if the code for ImmediateModeRenderCallback were to change, then we need to probably update the code that I've written in Hierarchy.C to match. However, this code isn't terribly complicated, so I suspect we won't really get into this situation much.

  Revision 1.35  2003/08/29 21:15:09  gfx_friends
  Trying to fix up the binding from GL matrices to VDS.

  Revision 1.34  2003/08/29 19:30:30  gfx_friends
  Added VDS readback support and fixed up the readback code a bit.

  Revision 1.33  2003/08/28 19:39:11  gfx_friends
  Checkin so I can move computers.

  Revision 1.32  2003/08/14 20:38:50  gfx_friends
  Added the new glodObjectXform parameters and fixed a few related things. However, outstanding issues that exist now are (a) we still compute our errors in pixels, whereas we've decided to switch to angle-of-error, and (b) We can't make VDS work until we either map it to 1 cut/object or change VDS to support transformations per object regardless of cut.

  Revision 1.31  2003/07/26 01:17:43  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.30  2003/07/25 03:00:14  gfx_friends
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

  Revision 1.29  2003/07/23 19:55:34  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.28  2003/07/23 09:53:15  bms6s
  -fixed vdscut::adaptScreenSpaceErrorThreshold to to update node errors before adapting (doh!)
  -fixed vdscut::adaptObjectSpaceErrorThreshold to set the flag not to simplify further vds objects in the same group (doh!)

  Revision 1.27  2003/07/23 06:36:07  bms6s
  vds integrated

  Revision 1.26  2003/07/19 23:50:10  gfx_friends
  Jon Cohen:

  Fixed up output to VDS hierarchy from half edge collapse and edge
  collapse operators. Seems to work, but it's still subject to review by
  Brenden. Also fixed a bug in vertex sharing.

  Revision 1.25  2003/06/30 19:29:58  gfx_friends
  (1) InsertElements now works completely. --nat
  (2) Some XBS classes got moved from xbs.h into discrete.h and Hierarchy.h for
      cleanliness.

  Revision 1.24  2003/06/19 10:05:05  bms6s
  XXX commented out more parts of Hierarchy.C that need to be changed to use VIF2.2

  Revision 1.23  2003/06/16 06:32:01  bms6s
  XXX commented out the parts of xbs that broke due to the new vif format

  Revision 1.22  2003/06/12 01:19:06  bms6s
  fixed linux build

  Revision 1.21  2003/06/11 08:07:24  bms6s
  [vds and glod commit messages are identical]

  temporary memory allocation in place.  everything compiles in windows but i've no doubt completely destroyed the linux build; i will attempt to fix this tomorrow and then ask nat for help.

  beacause of the way vds is set up:
  -calling adapt on a cut actually adapts all cuts in that cut's group
  -calling render on a cut actually renders all cuts in that cut's patch

  api/vds_error,cpp, api/vds_render.cpp, and api/vds_simplify.cpp are not used by anything anymore - they've been replaced by vds_callbacks.h and vds_callbacks.cpp.  vds_callbacks.cpp could be moved to api/ if you think that's more appropriate.

  i replaced gl types in vds with generic equivalents, since i wasn't sure if glodlib will have the gl includes (from the errors i was getting it appeared not).

  gave simple with a load sphere50.ply command line when run in visual studio

  Revision 1.20  2003/06/05 17:38:58  gfx_friends
  Patches to build on Win32.

  Revision 1.19  2003/06/04 16:58:40  gfx_friends
  Phase 1 of XP compile project.

  Revision 1.18  2003/01/21 10:54:36  bms6s
  *** empty log message ***

  Revision 1.16  2003/01/20 22:27:47  bms6s
  *** empty log message ***

  Revision 1.15  2003/01/20 22:19:54  gfx_friends
  Fixed namespace with GDB bug.

  Revision 1.14  2003/01/20 14:50:45  bms6s
  first try at putting some of the vds functions in.  threshold adapting is in place, but
  budget adapting is not.

  Revision 1.13  2003/01/20 07:42:38  gfx_friends
  Added screen-space error mode. It seems to work for threshold mode,
  but still gets stuck in triangle budget mode (object-space seems to
  work okay in budget mode now).

  Revision 1.12  2003/01/19 17:16:17  gfx_friends
  Triangle budget mode works on a single object, but may very well work
  on multiple objects.

  Revision 1.11  2003/01/19 11:19:40  bms6s
  *** empty log message ***

  Revision 1.10  2003/01/19 04:48:58  gfx_friends
  Object-space error threshold mode for a single object in a single
  group seems to be working. Not tested on multiple objects in multiple
  groups, but it is trivial enough that it "should" work.

  Revision 1.9  2003/01/18 23:42:13  gfx_friends
  initial (non-working) version of triangle budget mode, etc.

  Revision 1.8  2003/01/17 18:46:57  gfx_friends
  Moving Group and Object to glod core

  Revision 1.7  2003/01/17 17:02:10  gfx_friends
  Updated ByteColor to ByteColorA to match changes in vds/vif.h

  Revision 1.6  2003/01/17 16:50:21  gfx_friends
  VifWrite()

  Revision 1.5  2003/01/16 16:41:40  gfx_friends
  Added initial support for VDSHierarchy. Conversion to Vif is included,
  but not Vif->VDS

  Revision 1.4  2003/01/16 03:38:57  gfx_friends
  Working on VIF interface

  Revision 1.3  2003/01/15 17:24:10  gfx_friends
  Added DiscreteHierarchy output type to xbs

  Revision 1.2  2003/01/14 23:48:05  gfx_friends
  Part of my grand scheme to take over the world.

  Revision 1.1  2003/01/13 20:30:15  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

\*****************************************************************************/

