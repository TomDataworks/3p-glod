/* GLOD: Any common stuff to GLOD code goes here
 ***************************************************************************
 * $Id: glod_core.h,v 1.61 2004/10/12 16:35:38 gfx_friends Exp $
 * $Revision: 1.61 $
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
#ifndef GLOD_H
#define GLOD_H

#if defined(_WIN32) || defined(__APPLE__)
#include <float.h>
#pragma warning ( disable:4355 )
#else
#include <values.h>
#endif

#include <string.h>

#include "hash.h"

#include <Heap.h>
#include <XBSEnums.h>

/* Local includes
***************************************************************************/
#include "glod_glext.h"

#include "glod.h"
#include <vds.h>
#include <simplifier.h>
#include <cut.h>
#include "vds_callbacks.h"

class xbsVertex;
#include "glod_error.h"

//#define GLOD_USE_TILES

extern int GLOD_TILE_ROWS;
extern int GLOD_TILE_COLS;
extern int GLOD_NUM_TILES;

typedef struct {
    float min_x;
    float max_x;
    float min_y;
    float max_y;
} GLOD_Tile;

extern GLOD_Tile *tiles;

const GLfloat glodcore_IdentityXform[] = {1.0, 0.0, 0.0, 0.0,
0.0, 1.0, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.0, 0.0, 0.0, 1.0};

#define PATCH_HASH_BUCKET_SIZE 32

// in glodBuildObject, if we call model->splitPatchVerts();
// then we should set the folowing flag.
#define XBS_SPLIT_BORDER_VERTS 

enum ErrorMode { ScreenSpace, ObjectSpace };
enum AdaptMode { TriangleBudget, ErrorThreshold };


typedef struct {
    int last_error;

    HashTable* object_hash;
    HashTable* group_hash;
} GLOD_APIState;
extern GLOD_APIState s_APIState;

#include "glod_raw.h" // Get the Raw objects

class Hierarchy;
class GLOD_Group;
class GLOD_Cut;

class GLOD_Object {

public:
    unsigned int name; 
    unsigned int format; // set to GLOD_FORMAT_UNKNOWN 

    GLOD_Group* group;
    unsigned int group_name;
    int groupIndex; // current index of this object in its group

    void* prebuild_buffer; // used to store anything in the pre-build steps...

    Hierarchy* hierarchy;        // tracks reference count with
    // LockInstance/ReleaseInstance

    GLOD_Cut* cut;                 // Not a great word. Any better ideas?
    int *inArea;
    //unsigned int numAreas;
    QueueMode queueMode;
    OperationType opType;
    float shareTolerance;
    int borderLock;
    int errorMetric;
    float importance;
    SnapshotMode snapMode;
    float reductionPercent;
    int numSnapshotSpecs;
    unsigned int *snapshotTriSpecs;
    int numSnapshotErrorSpecs;
    GLfloat *snapshotErrorSpecs;
    float pgPrecision;
    float quadricMultiplier;
    
    HashTable* patch_id_map; // NOTE: the ids in this table are all +1 of their real because HashTable uses 0 as its "empty" value


    HeapElement budgetCoarsenHeapData;
    HeapElement budgetRefineHeapData;


    GLOD_Object() : budgetCoarsenHeapData(this), budgetRefineHeapData(this)
    {
        name = format = group_name = UINT_MAX;
        group = NULL;
        hierarchy = NULL;
        cut = NULL;
        prebuild_buffer = NULL;
        queueMode = Greedy;
        opType = Half_Edge_Collapse;
        shareTolerance = 0.0;
        borderLock = 0;
        errorMetric = GLOD_METRIC_SPHERES;
        importance = 1.0;
        snapMode = PercentReduction;
        reductionPercent = 0.5;
        numSnapshotSpecs = 0;
        snapshotTriSpecs = NULL;
        numSnapshotErrorSpecs = 0;
        snapshotErrorSpecs = NULL;
        inArea=new int[GLOD_NUM_TILES];
        quadricMultiplier=1;
        //numAreas=0;
        //budgetCoarsenHeapData = new HeapElement[GLOD_NUM_TILES](this);
        //budgetRefineHeapData = new HeapElement[GLOD_NUM_TILES](this);
        pgPrecision = 3.0;
    };



    /*    GLOD_Object(const GLOD_Object& obj) :
    budgetCoarsenHeapData(this), budgetRefineHeapData(this)
    {
    name = group_name = UINT_MAX;
    format = obj.format;
    hierarchy = obj.hierarchy;
    importance = obj.importance;
    cut = NULL;
    queueMode = obj.queueMode;
    opType = obj.opType;
    shareTolerance = obj.shareTolerance;
    }*/

    ~GLOD_Object();

    void drawPatch(int pnum);
    void adaptScreenSpaceErrorThreshold(float threshold);
    void adaptObjectSpaceErrorThreshold(float threshold);    
};

#include "glod_group.h"

static void inline GLOD_SetError(int num, const char* message) {
#ifdef DEBUG
    fprintf(stderr, "GLOD: %s.\n", message);
#endif
#ifdef GLOD
    if (s_APIState.last_error == GLOD_NO_ERROR)
        s_APIState.last_error = num;
#endif
}

static void inline GLOD_SetError(int num, const char* message, int code) {
#ifdef DEBUG
    fprintf(stderr, "GLOD: %s (0x%08x).\n", message, code);
#endif
#ifdef GLOD
    if (s_APIState.last_error == GLOD_NO_ERROR)
        s_APIState.last_error = num;
#endif
}

#endif /* GLOD_H */
/***************************************************************************
 * $Log: glod_core.h,v $
 * Revision 1.61  2004/10/12 16:35:38  gfx_friends
 * added a multiplier for error quadrics so the error can be more evenly distributed between the different error metrics. It can be changed during runtime
 *
 * Revision 1.60  2004/08/05 16:11:08  gfx_friends
 * added a commented out define which enables the tile support
 *
 * Revision 1.59  2004/07/28 06:07:10  jdt6a
 * more permission grid work.  most of voxelization code from dachille/kaufman paper in place, but only testing against plane of triangle right now (not the other 6 planes yet).
 *
 * run simple.exe with a "-pg" flag to get the permission grid version (which isn't fully working yet... for some reason the single plane testing which should be very conservative results in too strict of a grid, or i am not testing the grid correctly).  the point sampled version actually results in better, aka more simplified, models, so i think there is a bug somewhere in the voxelization or testing.
 *
 * after a run of simple, a file "pg.dat" will be dumped into the current directory.  the pgvis program lets you visualize this file, which is just the grid.
 *
 * Revision 1.58  2004/07/21 18:43:42  gfx_friends
 * Added a flag XBS_SPLIT_BORDER_VERTS that allows us to tell whether a xbsVertex appears in multiple patches or not. If this value is defined, which it should always be for GLOD, the algorithm used to assign a vertex to a patch can be considerably simpler.
 *
 * Revision 1.57  2004/07/21 18:37:47  gfx_friends
 * ND:
 *  Fixes for separate compilation of xbs without glod, etc.
 *
 * Revision 1.56  2004/07/20 21:54:35  gfx_friends
 * Major Discrete and AttribSet rewrite. Discrete now uses attribsets for its vertex data. -Nat
 *
 * Revision 1.55  2004/07/19 19:18:42  gfx_friends
 * Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
 *
 * Revision 1.54  2004/07/09 22:05:53  gfx_friends
 * JC:
 *
 * Added new snapshot mode for specifying a list of errors rather than a
 * list of triangles. I haven't tested it yet...
 *
 * Revision 1.53  2004/06/29 14:31:25  gfx_friends
 * JC:
 *
 * Added GLOD_BUILD_SNAPSHOT_MODE parameter, which defaults to
 * GLOD_SNAPSHOT_PERCENT_REDUCTION and may alternatively be set to
 * GLOD_SNAPSHOT_TRI_SPEC, which allows the app to explicitly list the
 * triangle counts for discrete levels. Percent reduction may be set
 * anywhere in (0,1) and defaults to 0.5. I had to add a new
 * glodObjectParameteriv() call to set the list of triangle counts, and
 * unlike most OpenGL vector calls, it needs to specify how many elements
 * are in the vector (OpenGL typically says [1234] in the name of the
 * call).
 *
 * Revision 1.52  2004/06/25 18:58:41  gfx_friends
 * New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default
 *
 * Revision 1.51  2004/06/24 21:49:08  gfx_friends
 * Added a new metric, quadric errors. Also a major redesign of the error calculation/storage functions, which are now in their own class
 *
 * Revision 1.50  2004/06/16 20:30:34  gfx_friends
 * values.h include change for osx
 *
 * Revision 1.49  2004/06/04 21:46:47  gfx_friends
 * Modified glodGetError to work like glGetError.
 * Added a glodGetError manual page.
 * Modified some of the existing manual pages.
 *
 * Revision 1.48  2004/06/03 19:04:07  gfx_friends
 * Added a "border lock" mode to prevent xbs from moving/removing any
 * vertices on a geometric border. The determination of whether or not
 * something is on a geometric border is somewhat heuristic. It is not
 * clear what we want to call a border in the presence of various sorts
 * of non-manifold vertices (which may be created by xbs, even if the
 * orinal model is manifold).
 *
 * To use border lock mode, set the object's GLOD_BUILD_BORDER_MODE to
 * GLOD_BORDER_LOCK before building.
 *
 * Revision 1.47  2004/06/02 17:13:59  gfx_friends
 * Changes to #includes so it works on a stock osx configuration
 *
 * Revision 1.46  2004/02/04 07:21:05  gfx_friends
 * Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
 *
 * Revision 1.45  2004/02/04 03:47:27  gfx_friends
 *   - Move format flag from glodBuildObject to glodNewObject... I'm really sorry
 *     but we discovered in the paper-writing process that this was necessary
 *   - Refactored the discrete manual handling mode more cleanly. I am going to
 *     check in a cleaner refactoring soon. Its broken right now. (nat)
 *
 * Revision 1.44  2004/01/21 17:58:27  bms6s
 * turned view frustum simplification on (maybe we need to make a new group param to be able to control it?)
 *
 * changed view parameter extraction so forward vector seems to be captured ok
 *
 * Revision 1.43  2004/01/13 15:43:12  bms6s
 * fixed vbo rendering of more than one continuous cut
 * commented out some debug printfs
 *
 * Revision 1.42  2003/12/18 00:46:28  gfx_friends
 * Added two fields to RawPatch to allow manual discrete patches
 *
 * Revision 1.41  2003/12/12 16:14:46  bms6s
 * the most primitive of interruptable simplificaton schemes - just setting the simp break count to a reasonable constant (100).  this seems to work fine; the only problems i encountered were with the vds queues being unbalanced causing the coarsen/refine queues to become unbalanceable, which happens regardless of simp break count.
 *
 * Revision 1.40  2003/10/23 01:44:24  bms6s
 * -removed coarsenError, currentError, coarsenRadius, coarsenCenter, refineRadius, and refineCenter from GLOD_Cut.  replaced them with coarsenErrorObjectSpace(), coarsenErrorScreenSpace(), currentErrorObjectSpace(), and currentErrorScreenSpace().  this makes code much cleaner and ensures consistency in the errors of discrete and continuous cuts (at least inasmuch as the discrete cuts's errors[] array is consistent with VDS' node radii).
 * -added call to update VDS errors at the beginning of every continuous cut coarsen() or refine() call.  this is more than necessary; conceivably we could just call it once per glod adaptXXX() call, perhaps in the beginning just before the queues are constructed.
 * -if coarsen() is going to get called with triTermination equal to the number of tris already in the cut, then instead we set triTermination to 0.
 * -modified vds node error callback StdErrorScreenSpaceNoFrustum to use the VDS::Cut's mpExternalViewClass pointer to calculate node error using computePixelsOfError().  so this means that view frustum simplification is currently disabled.  it shouldn't be too hard to similarly convert StdErrorScreenSpace to use glod's view frustum check and computePixelsOfError as well.
 * -modified computePixelsOfError to only take a center and object space error (that's all it was using anyway), since that's all VDS can provide it.
 *
 * Revision 1.39  2003/10/21 05:08:11  bms6s
 * implemented necessary functions for triangle budget adaptation of groups with a continuous cut (still not functional though due to other issues).
 *
 * Revision 1.38  2003/08/29 19:45:54  gfx_friends
 * Made screen space for VDS use the frustum check callback.
 *  CVS: ----------------------------------------------------------------------
 *
 * Revision 1.37  2003/08/29 19:30:29  gfx_friends
 * Added VDS readback support and fixed up the readback code a bit.
 *
 * Revision 1.36  2003/08/14 20:38:48  gfx_friends
 * Added the new glodObjectXform parameters and fixed a few related things. However, outstanding issues that exist now are (a) we still compute our errors in pixels, whereas we've decided to switch to angle-of-error, and (b) We can't make VDS work until we either map it to 1 cut/object or change VDS to support transformations per object regardless of cut.
 *
 * Revision 1.35  2003/07/26 01:17:25  gfx_friends
 * Fixed copyright notice. Added wireframe to sample apps. Minor
 * revisions to documentation.
 *
 * Revision 1.34  2003/07/23 19:55:28  gfx_friends
 * Added copyright notices to GLOD. I'm making a release.
 *
 * Revision 1.33  2003/07/23 06:36:07  bms6s
 * vds integrated
 *
 * Revision 1.32  2003/07/22 03:28:29  gfx_friends
 * Fixed the Scene tool. Mostly. I need to do some more stuff, but its back to comipling. glodAdapt jams! --nat
 *
 * Revision 1.31  2003/07/22 02:57:17  gfx_friends
 * Added code to support three object parameters: BUILD_OPERATOR,
 * BUILD_QUEUE, and SHARE_TOLERANCE. Operator is things like
 * EDGE_COLLAPSE and HALF_EDGE_COLLAPSE. QUEUE is GREEDY, LAZY, or
 * INDEPENDENT, and SHARE_TOLERANCE is merges vertices up to some
 * geometry tolerance before simplification. If attributes differ, they
 * become multi-attribute verts.
 *
 * Also added a new little cylinder model to test multi-attribute
 * vertices within a single patch.
 *
 * Revision 1.30  2003/07/21 23:10:51  gfx_friends
 * Added cut readback support. I'm still debugging, but need to move computers to my home. --n
 *
 * Revision 1.29  2003/07/15 20:19:04  gfx_friends
 * Major documentation effort and basic distribution readiness. We now have pod-based documentation for each GLOD function. It will build HTML or Man pages on Linux. To use the man pages, append glod/doc/man to your manpath after running make in doc or doing a top-level make. Also new is a release target... a top level make release builds with -O2 and any flags you also set based on the release target (See glod.conf). Also, #define DEBUG is active when building for debug.
 *
 * Revision 1.28  2003/07/09 22:50:04  gfx_friends
 * Major documentation effort and minor API changes. On the API change side,
 * GLODBuildObject now recieves the format flag for an object being built, while LoadObject now requires NewObject to
 * have been called before it can be called. NewObject simply creates an object and group.
 *
 * On the documentation side, the sources in ./api now contain a ton of inline comments which document
 * the API routines using Doxygen tagging syntax. A top-level makefile target, docs, allows you to build HTML documentation out of these files. When I've finished the documentation, we can also make the same comments go to UNIX Man pages and Windows RTF/HTML help files. I'm still documenting the API. However, if you run make docs on a linux box or some PC with Doxygen installed on it, you'll get the docs and can check them out.
 *
 * Cheers,
 *
 * -- Nat
 *
 * Revision 1.27  2003/07/02 21:50:29  gfx_friends
 * The patch numbering bug is now fixed. This bug was that all of our code assumes that patch numbers are tightly packed in the range 0..num_patches, whereas the GLOD api does not inforce such restrictions. The solution to this is a layer of indrection at the API level, and two glodGetObjectParameteriv calls, GLOD_PATCH_NAMES and GLOD_NUM_PATCHES. --n
 *
 * Revision 1.26  2003/07/01 21:49:11  gfx_friends
 * Tore out the glodDrawObject in favor of the more complete glodDrawPatch. A bug somewhere between glodBuildObject's call to new Model and the draw, however, is still failing, as all of the patches have identical geometry by the time they hit the drawing point.
 *
 * Revision 1.25  2003/07/01 20:49:15  gfx_friends
 * Readback for Discrete LOD now works. See samples/readback/read_model for demo and test. This, parenthetically, is also the most fully functional test program that we have. I will bring "simple" up to speed eventually, but in the meantime, you can use read_model similarly (run --help) in much the same way as previous ones to do testing.
 *
 * Revision 1.24  2003/06/30 19:29:57  gfx_friends
 * (1) InsertElements now works completely. --nat
 * (2) Some XBS classes got moved from xbs.h into discrete.h and Hierarchy.h for
 *     cleanliness.
 *
 * Revision 1.23  2003/06/27 04:33:48  gfx_friends
 * We now have a functioning glodInsertElements. There is a bug in ModelShare.c that infiniteloops. I'm chasing that. -- n
 *
 * Revision 1.22  2003/06/26 18:52:59  gfx_friends
 * Major rewrite of GLOD-side Vertex Array handling routines (the so-called masseusse) to allow more robust inputs. Let me tell you, the VA interface is really pretty when you're using it, but using the data in a coherent way is a nightmare because of all of the different options you have as a user. This will allow me to implement the readback interface faster... in theory, although that is going to be an equal nightmare. -- nat
 *
 * Revision 1.21  2003/06/05 17:40:10  gfx_friends
 * Patches to build on Win32.
 *
 * Revision 1.20  2003/06/04 16:57:16  gfx_friends
 * Tore out chromium.
 *
 * Revision 1.19  2003/01/22 01:12:42  gfx_friends
 * Fixed include file nightmares. -- nat
 *
 * Revision 1.18  2003/01/21 09:21:01  gfx_friends
 * Intermediate checkin. Things are broken.
 *
 * Revision 1.17  2003/01/20 22:33:51  bms6s
 * *** empty log message ***
 *
 * Revision 1.16  2003/01/20 22:28:28  bms6s
 * *** empty log message ***
 *
 * Revision 1.15  2003/01/20 04:41:19  gfx_friends
 * Fixed GLOD_Group::addObject() and adaptTriangleBudget. Added initial
 * class to set view.
 *
 * Revision 1.14  2003/01/19 17:16:17  gfx_friends
 * Triangle budget mode works on a single object, but may very well work
 * on multiple objects.
 *
 * Revision 1.13  2003/01/19 04:48:58  gfx_friends
 * Object-space error threshold mode for a single object in a single
 * group seems to be working. Not tested on multiple objects in multiple
 * groups, but it is trivial enough that it "should" work.
 *
 * Revision 1.12  2003/01/18 23:42:13  gfx_friends
 * initial (non-working) version of triangle budget mode, etc.
 *
 * Revision 1.11  2003/01/18 00:36:35  gfx_friends
 * Patch to the drawing structure & some misc stuff.
 *
 * Revision 1.10  2003/01/17 21:33:52  gfx_friends
 * New API support.
 *
 * Revision 1.9  2003/01/17 19:53:58  gfx_friends
 * GLOD_Object and GLOD_Group hierarchy is solidifying.
 *
 * Revision 1.8  2003/01/17 19:23:42  gfx_friends
 * Ultimate frisbee!
 *
 * Revision 1.7  2003/01/17 19:12:33  gfx_friends
 * added GLOD_Group::addObject
 *
 * Revision 1.6  2003/01/17 19:07:18  gfx_friends
 * Converted GLOD_Object to a class.
 *
 * Revision 1.5  2003/01/17 18:46:57  gfx_friends
 * Moving Group and Object to glod core
 *
 * Revision 1.4  2003/01/17 18:37:16  gfx_friends
 * Updated object.
 *
 * Revision 1.3  2003/01/15 18:54:43  gfx_friends
 * New handling semantics for the Object Hashtable.
 *
 * Revision 1.2  2003/01/15 17:24:26  gfx_friends
 * Foo the bar
 *
 * Revision 1.1  2003/01/14 23:37:57  gfx_friends
 * Reorg.
 *
 * Revision 1.5  2003/01/13 21:56:20  gfx_friends
 * Added vertex count.
 *
 * Revision 1.4  2003/01/13 20:10:37  gfx_friends
 * Updated interface file for GLOD to simplifier (nat)
 *
 * Revision 1.3  2003/01/10 22:35:38  gfx_friends
 * Beginnings of Masseuse.
 *
 * Revision 1.2  2003/01/10 20:56:43  gfx_friends
 * Added initialization function to libraries, and created raw object structures.
 *
 * Revision 1.1.1.1  2003/01/02 21:21:14  duca
 * First checkin: (1) Chromium SPU, (2) standalone library, (3) build system
 *   -- Nat
 *
 ***************************************************************************/
