/*****************************************************************************\
  xbs.C
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/xbs.C,v $
  $Revision: 1.31 $
  $Date: 2004/10/20 20:01:40 $
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

#include "xbs.h"
#include "Discrete.h"
#include "Continuous.h"
#include "MTHierarchy.h"

/*----------------------------- Local Constants -----------------------------*/


/*------------------------------ Local Macros -------------------------------*/


/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/

int GLOD_NUM_TILES = 1;
GLOD_Tile *tiles = NULL;

/*------------------------ Local Function Prototypes ------------------------*/


/*---------------------------------Functions-------------------------------- */

/*****************************************************************************\
 @ main
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
int main(int argc, char **argv)
{
    Model model;
    if(argc > 1)
        model.readPly(argv[1]);
    else
        model.readPly();
    model.share(/*0.01*/0.0);
    model.indexVertTris();
    model.removeEmptyVerts();
    model.splitPatchVerts();
    model.verify();
    //model.writePly("shared.ply");
    fprintf(stderr, "Model loaded.\n");

    /*    model.snapMode = PercentReduction;
    model.reductionPercent = 0.75;
    model.snapMode = ManualTriSpec;
    model.numSnapshotSpecs = 4;
    model.snapshotTriSpecs = new unsigned int[4];
    model.snapshotTriSpecs[0] = 70000;
    model.snapshotTriSpecs[1] =  5000;
    model.snapshotTriSpecs[2] =  1000;
    model.snapshotTriSpecs[3] =  750;*/
    
//    model.borderLock = 1;

//    XBSSimplifier simp(&model, Half_Edge_Collapse, Greedy,
//                     /*Discrete_Hierarchy*/ new DiscreteHierarchy());
//    XBSSimplifier simp(&model, Half_Edge_Collapse, Lazy, Discrete_Hierarchy);
//    XBSSimplifier simp(&model, Edge_Collapse, Greedy, Discrete_Hierarchy);
//    XBSSimplifier simp(&model, Edge_Collapse, Lazy, new DiscreteHierarchy());
//    XBSSimplifier simp(&model, Half_Edge_Collapse, Lazy, new DiscreteHierarchy(Edge_Collapse));
//    XBSSimplifier simp(&model, Half_Edge_Collapse, Lazy, new MTHierarchy());
    XBSSimplifier simp(&model, Edge_Collapse, Lazy, new MTHierarchy());
//    XBSSimplifier simp(&model, Half_Edge_Collapse, Greedy, VDS_Hierarchy);
//    XBSSimplifier simp(&model, Half_Edge_Collapse, Lazy, VDS_Hierarchy);
//    XBSSimplifier simp(&model, Edge_Collapse, Greedy, VDS_Hierarchy);
//    XBSSimplifier simp(&model, Edge_Collapse, Lazy, VDS_Hierarchy);
//    Simplifier simp(&model, Half_Edge_Collapse, Independent);
//    Simplifier simp(&model, Half_Edge_Collapse, Greedy);
//    XBSSimplifier simp(&model, Half_Edge_Collapse, Lazy, MT_Hierarchy);
    fprintf(stderr, "Done simplifying.\n");
    simp.writeHierarchy();
    return 0;
    
} /** End of main() **/

/* Utility crap
 *****************************************************************************/
void writeGraph(MTHierarchy* h) {
    MT* mt = h->mt;
    printf("digraph G {\n");
    printf("\tgraph [source=%i,sink=%i];\n", mt->getRoot(), 0);
    for(int i = 0; i < mt->getNumArcs(); i++) {
        mtArc* arc = mt->getArc(i);
        printf("\t%i -> %i [patch=%i]\n", arc->getStart(), arc->getEnd(), arc->getPatchNumber());
    }
    printf("}\n");
}

void patchifyModel(Model& model) {
    xbsVec3 mmin(FLT_MAX, FLT_MAX, FLT_MAX);
    xbsVec3 mmax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for(int i = 0; i < model.getNumVerts(); i++) {
        xbsVertex* vert = model.getVert(i);
        for(int j = 0; j < 3; j++) {
            if(vert->coord[j] < mmin[j])
                mmin[j] = vert->coord[j];
            if(vert->coord[j] > mmax[j])
                mmax[j] = vert->coord[j];
        }
    }

    // now divide...
    xbsVec3 range = mmax - mmin;
    xbsVec3 eps = range * .01;
    mmax += eps;
    mmin -= eps;
    range = mmax - mmin;
    
    float w = 3,h = 1,d = 1;
    for(int i = 0; i < model.getNumTris(); i++) {
        xbsTriangle* tri = model.getTri(i);
        int px,py,pz;
        px = py = pz = 10000;
        for(int k = 0; k < 3; k++) {
            xbsVertex* v = tri->verts[k];
            xbsVec3 relative = v->coord - mmin;
            relative /= range;
            int x = (int)(relative[0] * w);
            int y = (int)(relative[1] * h);
            int z = (int)(relative[2] * d);
            if(x < px) px = x;
            if(y < py) py = y;
            if(z < pz) pz = z;
        }
        int pnum = (int)(w*h*pz + w*py + px);                
        tri->patchNum = pnum;
    }

    // make sure the model knows how many patches its got...
    model.setNumPatches((int)(w*h*d));
    model.writePly("tmp.ply");
}

/*****************************************************************************\
  $Log: xbs.C,v $
  Revision 1.31  2004/10/20 20:01:40  gfx_friends
  Full edge collapses on MT work, too.

  Revision 1.30  2004/10/20 19:43:15  gfx_friends
  MT patch is complete. XBS standalone works. -nat

  Revision 1.29  2004/10/20 19:33:35  gfx_friends
  Rewrote MTHierarchy driver, made the VDS hack in Operation.C vds-specific

  Revision 1.28  2004/08/23 16:47:10  gfx_friends
  Pulled MT into a new file so that I can do some experiments with the shortcut algorithm.--nat

  Revision 1.27  2004/07/30 21:00:01  gfx_friends
  Made xbs app compile again

  Revision 1.26  2004/07/08 16:44:41  gfx_friends
  Removed tabs and did 4-space indentation on source files in xbs directory.

  Revision 1.25  2004/06/29 12:57:14  gfx_friends
  JC: Made things compile under Linux with Chris Niski's error metric stuff.

  Revision 1.24  2004/06/11 18:30:08  gfx_friends
  Remove all sources of warnings in xbs directory when compiled with -Wall

  Revision 1.23  2004/06/03 19:04:09  gfx_friends
  Added a "border lock" mode to prevent xbs from moving/removing any
  vertices on a geometric border. The determination of whether or not
  something is on a geometric border is somewhat heuristic. It is not
  clear what we want to call a border in the presence of various sorts
  of non-manifold vertices (which may be created by xbs, even if the
  orinal model is manifold).

  To use border lock mode, set the object's GLOD_BUILD_BORDER_MODE to
  GLOD_BORDER_LOCK before building.

  Revision 1.22  2004/06/03 14:33:51  gfx_friends
  Jon: removed '#if 0' around main()

  Revision 1.21  2004/05/26 20:08:44  gfx_friends
  changed the object space error method for vds, instead of a scalar it square roots the error, which is the proper way of doing it

  Revision 1.20  2004/02/04 07:21:08  gfx_friends
  Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat

  Revision 1.19  2003/08/13 23:59:26  gfx_friends
  Jon Cohen:

  1) Fixed bug in color interpolations
  2) Removed Mesa dependencies on my laptop, then removed related stuff
     from the Makefiles. Hopefully I was not overzealous. If someone
     actually wants to compile/link using Mesa, they may need back the
     additional paths and may need to link with pthread (but otherwise,
     just linking with pthread slows down execution).
  3) Perl paths in documentation making scripts -- we may need to change
     these back, depending on what machine we make the documentation on.

  Revision 1.18  2003/07/26 01:17:46  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.17  2003/07/25 03:00:15  gfx_friends
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

  Revision 1.16  2003/07/23 19:55:35  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.15  2003/07/23 07:00:14  gfx_friends
  Commented out compiling xbs executable, because it now depends again
  on GLOD being compiled first.

  Revision 1.14  2003/07/22 02:57:19  gfx_friends
  Added code to support three object parameters: BUILD_OPERATOR,
  BUILD_QUEUE, and SHARE_TOLERANCE. Operator is things like
  EDGE_COLLAPSE and HALF_EDGE_COLLAPSE. QUEUE is GREEDY, LAZY, or
  INDEPENDENT, and SHARE_TOLERANCE is merges vertices up to some
  geometry tolerance before simplification. If attributes differ, they
  become multi-attribute verts.

  Also added a new little cylinder model to test multi-attribute
  vertices within a single patch.

  Revision 1.13  2003/07/19 23:50:11  gfx_friends
  Jon Cohen:

  Fixed up output to VDS hierarchy from half edge collapse and edge
  collapse operators. Seems to work, but it's still subject to review by
  Brenden. Also fixed a bug in vertex sharing.

  Revision 1.12  2003/07/16 16:12:30  gfx_friends
  Added splitting of multi-patch vertices. Thus if a vertex touches
  triangles from multiple patches, it is split into a multi-attribute
  vertex (even if they have the same attributes). This is useful because
  each of these vertices can store information like its index in any
  hierarchy-related data structures.

  Revision 1.11  2003/07/16 03:12:29  gfx_friends
  Added xbs support for "multi-attribute vertices". These are
  geometrically coincident vertices that may have different
  attributes. Geometric coincidence is maintained throughout the
  simplification process and attributes are correctly propagated along.

  For the full edge collapse, the heuristics for preventing attribute
  seams along patch boundaries could still use a little work.

  Things seem to work for the DiscreteHierarchy output. VDS hierarchy
  has not been integrated yet.

  Revision 1.10  2003/01/20 22:19:54  gfx_friends
  Fixed namespace with GDB bug.

  Revision 1.9  2003/01/18 23:42:13  gfx_friends
  initial (non-working) version of triangle budget mode, etc.

  Revision 1.8  2003/01/17 18:46:57  gfx_friends
  Moving Group and Object to glod core

  Revision 1.7  2003/01/17 16:50:21  gfx_friends
  VifWrite()

  Revision 1.6  2003/01/16 16:41:41  gfx_friends
  Added initial support for VDSHierarchy. Conversion to Vif is included,
  but not Vif->VDS

  Revision 1.5  2003/01/16 03:38:58  gfx_friends
  Working on VIF interface

  Revision 1.4  2003/01/15 20:12:41  gfx_friends
  Basic functionality of GLOD with DiscreteHierarchy and EdgeCollapse.

  Revision 1.3  2003/01/15 17:24:11  gfx_friends
  Added DiscreteHierarchy output type to xbs

  Revision 1.2  2003/01/14 23:48:06  gfx_friends
  Part of my grand scheme to take over the world.

  Revision 1.1  2003/01/13 20:30:17  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.2  2003/01/08 05:19:14  cohen
  Added first version of full edge collapse.

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/



