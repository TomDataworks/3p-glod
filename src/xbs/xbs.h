/*****************************************************************************\
  xbs.h
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/xbs.h,v $
  $Revision: 1.52 $
  $Date: 2004/10/12 14:57:52 $
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

/* Protection from multiple includes. */
#ifndef INCLUDED_XBS_H
#define INCLUDED_XBS_H


/*------------------ Includes Needed for Definitions Below ------------------*/

#if 1
#define MLBPQ
#endif

#if 0
#define TESTHEAP
#endif

#ifdef MLBPQ
#include <MLBPriorityQueue.h>
#else
#include <Heap.h>
#endif
#include <Model.h>
#include <Hierarchy.h>

#include <vif.h>
#include <vds.h>
#include <forest.h>
#include <simplifier.h>
#include <renderer.h>
#include <cut.h>
#include <glod_core.h>
#include "Metric.h"

#include "XBSEnums.h"
#include "vds_callbacks.h"
#include "Hierarchy.h"

#ifdef _WIN32
#pragma warning ( disable:4355 )
#endif


/*-------------------------------- Constants --------------------------------*/

#define MAX_INSTANCES 100
#ifdef _WIN32
#include <float.h>
#include <limits.h>
#ifndef M_PI
# define M_PI 3.141592653589793238462643383
#endif
#define MAXINT INT_MAX
#define MAXFLT FLT_MAX
#undef min
#endif

/*--------------------------------- Macros ----------------------------------*/
#ifdef _WIN32
#ifndef for
#define for if(0); else for
#endif
#endif

/*---------------------------------- Types ----------------------------------*/




/*---------------------------- Function Prototypes --------------------------*/


/*---------------------------Globals (externed)------------------------------*/


/*--------------------------------- Classes ---------------------------------*/

class Operation;
class EdgeCollapse;
class SimpQueue;
class Hierarchy;

class Operation
{
  protected:
    xbsVertex *source_vert, *destination_vert;
    //float cost;
    char dirty;
    GLOD_Error *error;

  public:
#ifdef MLBPQ
    MLBPriorityQueueElement heapdata;
#else
    HeapElement heapdata;
#endif
    
    Operation()
	: heapdata((void *)(this), MAXFLOAT)
    {
	source_vert = destination_vert = NULL;
	//cost = MAXFLOAT;
	dirty = 1;
	error = NULL;
    };
    virtual ~Operation()
    {
	source_vert = destination_vert = NULL;
	//cost = MAXFLOAT;
	dirty = 1;
	delete error;
    }

    xbsVertex *getSource() const {return source_vert;};
    xbsVertex *getDestination() const {return destination_vert;};
    float getCost() { return error->getError();}; //return cost; };
    char isDirty() {return dirty;};
    void setDirty() {dirty = 1;};
    int duplicatedTriangle(xbsTriangle *tri);
    

#if 0
    float computeSampleCost(Model *model);
#endif
    void getNeighborOpsOld(Model *model,
			Operation ***addOps, int *numAddOps,
			Operation ***removeOps, int *numRemoveOps,
			Operation ***modOps, int *numModOps);
    
    void apply(Model *model, Hierarchy *hierarchy, SimpQueue *queue);

    virtual void initQueue(Model *model, SimpQueue *queue);
    void getNeighborOps(Model *model,
			Operation ***addOps, int *numAddOps,
			Operation ***removeOps, int *numRemoveOps,
			Operation ***modOps, int *numModOps);
    virtual void updateModel(Model *model, Hierarchy *hierarchy,
		     Operation ***addOps, int *numAddOps,
		     Operation ***removeOps, int *numRemoveOps,
		     Operation ***modOps, int *numModOps);
    virtual void computeCost(Model *model);
};

enum EdgeCollapseCase
  { MoveNeither, MoveBoth, MoveSource, MoveDestination };

class EdgeCollapse : public Operation
{
        
            
    public:
        
    EdgeCollapse() : Operation() {};
    virtual void initQueue(Model *model, SimpQueue *queue);
    virtual void updateModel(Model *model, Hierarchy *hierarchy,
		     Operation ***addOps, int *numAddOps,
		     Operation ***removeOps, int *numRemoveOps,
		     Operation ***modOps, int *numModOps);
    virtual void computeCost(Model *model);
    xbsVertex *generateVertex(Model *model, xbsVertex *v1, xbsVertex *v2);
    EdgeCollapseCase computeCase(Model *model);
        
};


class SimpQueue
{
  private:
    Operation *dummy_op;

  protected:
#ifdef MLBPQ
    MLBPriorityQueue heap;
#else
    Heap heap;
#endif
    
  public:
    SimpQueue(Model *model, OperationType opType)
	: heap()
    {
	switch(opType)
	{
	case Half_Edge_Collapse:
	{
	    dummy_op = new Operation;
	    break;
	}
	case Edge_Collapse:
	{
	    dummy_op = new EdgeCollapse;
	    break;
	}
	default:
	{
	    fprintf(stderr, "Operation type not supported yet.\n");
	    exit(1);
	}
	}

	dummy_op->initQueue(model, this);
//	heap.test();
    }
    virtual ~SimpQueue() {delete dummy_op; dummy_op = NULL;};
    void insert(Operation *op)
    {
	if (op->getCost() == MAXFLOAT)
	    return;
	op->heapdata.setKey(op->getCost());
	heap.insert(&(op->heapdata));
#ifdef TESTHEAP
	heap.test();
#endif
    };
    void remove(Operation *op)
    {
	if (op->heapdata.heap() == &heap)
	    heap.remove(&(op->heapdata));
#ifdef TESTHEAP
	heap.test();
#endif
    };
    void modify(Operation *op)
    {
	if (op->heapdata.heap() == &heap)
	{
	    if (op->getCost() == MAXFLOAT)
		remove(op);
	    else
		heap.changeKey(&(op->heapdata), op->getCost());
	}
	else if (op->heapdata.heap() != NULL)
	{
	    fprintf(stderr, "Op is on a different heap?!\n");
	}
	else if (op->getCost() != MAXFLOAT)
	    insert(op);
#ifdef TESTHEAP
	heap.test();
#endif
    };
    virtual Operation *getNextOperation(Model *model)
    {
	if (heap.size() > 0)
	    return (Operation *)(heap.extractMin()->userData());
	else
	    return NULL;
    };
    virtual void update(Model *model,
		Operation **addOps, int numAddOps,
		Operation **removeOps, int numRemoveOps,
		Operation **modOps, int numModOps);
};

class LazySimpQueue : public SimpQueue
{
public:
    LazySimpQueue(Model *model, OperationType opType)
	: SimpQueue(model, opType)
    {
    }
    
    virtual Operation *getNextOperation(Model *model)
    {
	if (heap.size() <= 0)
	    return NULL;
	
	Operation *op =
	    (Operation *)(heap.extractMin()->userData());

	while ((op != NULL) && (op->isDirty() == 1))
	{
	    op->computeCost(model);
	    insert(op);
	    op = (heap.size() > 0) ? (Operation *)(heap.extractMin()->userData())
		: NULL;
	}
	
	return op;
    }
    virtual void update(Model *model,
			Operation **addOps, int numAddOps,
			Operation **removeOps, int numRemoveOps,
			Operation **modOps, int numModOps);
};

class IndependentSimpQueue : public LazySimpQueue
{
private:
#ifdef MLBPQ
    MLBPriorityQueue dependentOps;
#else
    Heap dependentOps;
#endif
    
    void reactivateDependentOps(Model *model);

public:
    IndependentSimpQueue(Model *model, OperationType opType)
	: LazySimpQueue(model, opType), dependentOps()
    {
    };
    virtual void update(Model *model,
			Operation **addOps, int numAddOps,
			Operation **removeOps, int numRemoveOps,
			Operation **modOps, int numModOps);
    virtual Operation *getNextOperation(Model *model);
};

#if 0
static inline Hierarchy* MakeHierarchy(OutputType ot) {
}
#endif

class XBSSimplifier
{
  private:

    Model *model;
    Hierarchy *output;
    SimpQueue *queue;
    int borderLock;
    
  public:
    
    XBSSimplifier(Model *mdl, OperationType opType = Half_Edge_Collapse,
	       QueueMode qm = Lazy, Hierarchy* h = NULL, int bordLck=0)
    {
	model = mdl;
	output = h;
	borderLock = bordLck;
	
	output->initialize(model);

    if (model->errorMetric == GLOD_METRIC_PERMISSION_GRID)
        model->initPermissionGrid();

	switch(qm)
	{
	case Greedy:
	{
	    queue = new SimpQueue(model, opType);
	    break;
	}
	case Lazy:
	{
	    queue = new LazySimpQueue(model, opType);
	    break;
	}
	case Independent:
	{
	    queue = new IndependentSimpQueue(model, opType);
	    break;
	}
	case Randomized:
	{
	    fprintf(stderr, "Randomized queue mode not supported yet.\n");
	    exit(1);
	    break;
	}
	default:
	{
	    fprintf(stderr, "Unknown Queue Type\n");
	    exit(1);
	    break;
	}
	
	}
	
	for (Operation *op = queue->getNextOperation(model); op != NULL;
	     op = queue->getNextOperation(model))
	{
	    // debug
            model->testVertOps();

#if 1
            // If everything is REALLY kept up to date and we keep
            // around all the information necessary for propagating
            // the error, we may not need to do this cost computation
            // here.
            
            float oldcost = op->getCost(); //cost;
            
            op->computeCost(model);
            
            if (op->getCost() != oldcost)
            {
#ifdef XBSDEBUG
                fprintf(stderr, "Cost not kept up to date! (%g != %g)\n",
                        oldcost, op->getCost());
#endif
                queue->insert(op);
                continue;
            }
#endif
       
	    op->apply(model, output, queue);

	    // hack for now -- when an operation is aborted, it is not
	    // removed from the source vertex, so do not delete
            if (op->getSource() == op->getDestination())
		delete op;
            
            // debug
            model->testVertOps();
	}
	
	output->finalize(model);
    };
    ~XBSSimplifier()
    {
	delete queue;
	queue = NULL;
	model = NULL;
	output = NULL;
    }
    Hierarchy *getHierarchy() {return output;};
    void writeHierarchy(char *filename = NULL) {output->debugWrite(filename);};
};


#if 0
class Sample
{
  public:
    mtVec3 position;
    mtTriangle *tri;
    Sample *closest;
};
#endif



#if 0  // some old notes
enum QueueMode = { Greedy, Lazy, Independent };
enum OperationType = { Vertex_Cluster, Vertex_Pair, Edge_Collapse,
		       Half_Edge_Collapse };

class SimpQueue
{
    QueueMode mode;
    Heap heap;

    init(Model *model);
    // mode mostly determines if changedOps are cleaned, flagged dirty, or
    // deleted
    updateNeighborCosts(deletedOps, changedOps, newOps);
    Operation getNextOperation();
}

class TriangleData
{
  public:
    int mtTriIndex;
    int mtEndNodeIndex;
    ppError error;
}

class Model
{
    MT *mt;
    VertexData *vdata;
    TriData    *tdata;
    
    init(char *filename);
    
    load();
}

class XBSSimplifier
{
    Model *model;
    SimpQueue *queue;
    
    /*
      Load model
      Choose simplification modes
      Init Queue
      While !(Queue->empty())
        Get next operation
	Perform next operation
	Update Queue
      Finalize model
      Save model
    */
    Simplify(Model, MT, modes, functions, etc.);
}

class Grid
{
    insert();
    delete();
    getNeighborhoodCells(element);
}

class MultiGrid
{
    insert();
    delete();
    element findClosest(element);
}

class Operation
{
    /* Neighbors are other operations that would be affected by performing
       this operation. Some neighbors will be deleted, some will have a
       different cost, and some may be created. */
    getNeighbors(Model);
    getCost(Model);
    apply(Model);
}

class VertexPair : Operation
{
}

#endif // 0



/* Protection from multiple includes. */
#endif // INCLUDED_XBS_H

/*****************************************************************************\
  $Log: xbs.h,v $
  Revision 1.52  2004/10/12 14:57:52  gfx_friends
  Compilation fixes.

  Revision 1.51  2004/08/23 16:47:10  gfx_friends
  Pulled MT into a new file so that I can do some experiments with the shortcut algorithm.--nat

  Revision 1.50  2004/07/28 06:07:11  jdt6a
  more permission grid work.  most of voxelization code from dachille/kaufman paper in place, but only testing against plane of triangle right now (not the other 6 planes yet).

  run simple.exe with a "-pg" flag to get the permission grid version (which isn't fully working yet... for some reason the single plane testing which should be very conservative results in too strict of a grid, or i am not testing the grid correctly).  the point sampled version actually results in better, aka more simplified, models, so i think there is a bug somewhere in the voxelization or testing.

  after a run of simple, a file "pg.dat" will be dumped into the current directory.  the pgvis program lets you visualize this file, which is just the grid.

  Revision 1.49  2004/07/20 21:54:36  gfx_friends
  Major Discrete and AttribSet rewrite. Discrete now uses attribsets for its vertex data. -Nat

  Revision 1.48  2004/07/15 12:52:29  gfx_friends
  Moved check for cost out of date up the call stack so we can avoid
  performing operations out of order. Also, shut up the warning about out of
  date costs so everyone can go back to thinking that xbs just works. :-P

  Revision 1.47  2004/07/14 18:43:33  gfx_friends
  Nothing to see here...

  Revision 1.46  2004/07/14 14:59:51  gfx_friends
  Made handling of border heuristics more consistent. Cylinder now
  simplifies again, and the torus patch borders work pretty well, too.
  The case where borders are not preserved too well right now is using
  full edge collapses and error quadrics, because the location of the
  generated vertex does not in general lie along the collapsed edge
  (it is chosen by an optimization involving an inversion of the quadric
  matrix, yada yada yada). We may improve this by adding additional border
  edge planes into the quadric, as done in some papers by Garland and
  Lindstrom.

  Revision 1.45  2004/06/25 18:58:43  gfx_friends
  New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default

  Revision 1.44  2004/06/24 21:49:24  gfx_friends
  Added a new metric, quadric errors. Also a major redesign of the error calculation/storage functions, which are now in their own class

  Revision 1.43  2004/06/11 18:30:08  gfx_friends
  Remove all sources of warnings in xbs directory when compiled with -Wall

  Revision 1.42  2004/06/10 16:18:02  gfx_friends
  Added a Multi-level Bucket (MLB) Priority Queue to replace the
  standard binary heap as the main xbs priority queue. It makes use of
  the fact that the xbs priority queue is mostly monotonic, with newly
  inserted items almost always having larger keys than the key of the
  item at the top of the heap. It seems to be about 75% faster than the
  binary heap, so the priority queue is no longer a major bottleneck,
  even for the current fast error metric computation.

  Revision 1.41  2004/06/03 19:04:09  gfx_friends
  Added a "border lock" mode to prevent xbs from moving/removing any
  vertices on a geometric border. The determination of whether or not
  something is on a geometric border is somewhat heuristic. It is not
  clear what we want to call a border in the presence of various sorts
  of non-manifold vertices (which may be created by xbs, even if the
  orinal model is manifold).

  To use border lock mode, set the object's GLOD_BUILD_BORDER_MODE to
  GLOD_BORDER_LOCK before building.

  Revision 1.40  2004/02/04 07:21:09  gfx_friends
  Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat

  Revision 1.39  2003/12/18 19:44:07  bms6s
  separated out VDSHierarchy and VDSCut into continuous.h/c

  Revision 1.38  2003/07/26 01:17:46  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.37  2003/07/25 03:00:15  gfx_friends
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

  Revision 1.36  2003/07/23 19:55:35  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.35  2003/07/22 02:57:20  gfx_friends
  Added code to support three object parameters: BUILD_OPERATOR,
  BUILD_QUEUE, and SHARE_TOLERANCE. Operator is things like
  EDGE_COLLAPSE and HALF_EDGE_COLLAPSE. QUEUE is GREEDY, LAZY, or
  INDEPENDENT, and SHARE_TOLERANCE is merges vertices up to some
  geometry tolerance before simplification. If attributes differ, they
  become multi-attribute verts.

  Also added a new little cylinder model to test multi-attribute
  vertices within a single patch.

  Revision 1.34  2003/07/19 23:50:11  gfx_friends
  Jon Cohen:

  Fixed up output to VDS hierarchy from half edge collapse and edge
  collapse operators. Seems to work, but it's still subject to review by
  Brenden. Also fixed a bug in vertex sharing.

  Revision 1.33  2003/07/18 22:19:44  gfx_friends
  Fixed most of the build problems. The lights have mysteriously gone off in the simple test program... I'm trying to figure out why. But the rest works, I think

  Revision 1.32  2003/07/16 03:12:30  gfx_friends
  Added xbs support for "multi-attribute vertices". These are
  geometrically coincident vertices that may have different
  attributes. Geometric coincidence is maintained throughout the
  simplification process and attributes are correctly propagated along.

  For the full edge collapse, the heuristics for preventing attribute
  seams along patch boundaries could still use a little work.

  Things seem to work for the DiscreteHierarchy output. VDS hierarchy
  has not been integrated yet.

  Revision 1.31  2003/07/01 20:49:17  gfx_friends
  Readback for Discrete LOD now works. See samples/readback/read_model for demo and test. This, parenthetically, is also the most fully functional test program that we have. I will bring "simple" up to speed eventually, but in the meantime, you can use read_model similarly (run --help) in much the same way as previous ones to do testing.

  Revision 1.30  2003/06/30 20:37:07  gfx_friends
  Beginnings of a readback system. -- n

  Revision 1.29  2003/06/30 19:29:58  gfx_friends
  (1) InsertElements now works completely. --nat
  (2) Some XBS classes got moved from xbs.h into discrete.h and Hierarchy.h for
      cleanliness.

  Revision 1.28  2003/06/24 03:20:55  bms6s
  fixed stuff so glod builds; still need to fix xbs to use VIF2.2.

  right now draw() is in GLOD_Cut.  however, GLOD_Cut is an instance of an
  object, and if the object consists of more than one patch, we need draw()
  at the patch level, not the cut level.  how are we going to handle this?

  Revision 1.27  2003/06/16 06:32:01  bms6s
  XXX commented out the parts of xbs that broke due to the new vif format

  Revision 1.26  2003/06/12 16:55:56  gfx_friends
  Fixed minor bug with xbs that delete memory that still had pointers to it floating around.

  Revision 1.25  2003/06/11 08:07:24  bms6s
  [vds and glod commit messages are identical]

  temporary memory allocation in place.  everything compiles in windows but i've no doubt completely destroyed the linux build; i will attempt to fix this tomorrow and then ask nat for help.

  beacause of the way vds is set up:
  -calling adapt on a cut actually adapts all cuts in that cut's group
  -calling render on a cut actually renders all cuts in that cut's patch

  api/vds_error,cpp, api/vds_render.cpp, and api/vds_simplify.cpp are not used by anything anymore - they've been replaced by vds_callbacks.h and vds_callbacks.cpp.  vds_callbacks.cpp could be moved to api/ if you think that's more appropriate.

  i replaced gl types in vds with generic equivalents, since i wasn't sure if glodlib will have the gl includes (from the errors i was getting it appeared not).

  gave simple with a load sphere50.ply command line when run in visual studio

  Revision 1.24  2003/06/05 17:39:03  gfx_friends
  Patches to build on Win32.

  Revision 1.23  2003/06/04 16:58:41  gfx_friends
  Phase 1 of XP compile project.

  Revision 1.22  2003/01/22 01:12:43  gfx_friends
  Fixed include file nightmares. -- nat

  Revision 1.21  2003/01/22 00:01:38  bms6s
  Added #define guards so that vds can include the xbs View class without including all of xbs.
  not fully working yet, vds complains of forward declaration errors.

  Revision 1.20  2003/01/21 10:54:37  bms6s
  *** empty log message ***

  Revision 1.19  2003/01/21 09:21:03  gfx_friends
  Intermediate checkin. Things are broken.

  Revision 1.18  2003/01/20 22:33:39  bms6s
  *** empty log message ***

  Revision 1.17  2003/01/20 22:27:48  bms6s
  *** empty log message ***

  Revision 1.16  2003/01/20 22:19:54  gfx_friends
  Fixed namespace with GDB bug.

  Revision 1.15  2003/01/20 14:50:45  bms6s
  first try at putting some of the vds functions in.  threshold adapting is in place, but
  budget adapting is not.

  Revision 1.14  2003/01/20 07:42:38  gfx_friends
  Added screen-space error mode. It seems to work for threshold mode,
  but still gets stuck in triangle budget mode (object-space seems to
  work okay in budget mode now).

  Revision 1.13  2003/01/20 04:41:19  gfx_friends
  Fixed GLOD_Group::addObject() and adaptTriangleBudget. Added initial
  class to set view.

  Revision 1.12  2003/01/19 17:16:18  gfx_friends
  Triangle budget mode works on a single object, but may very well work
  on multiple objects.

  Revision 1.11  2003/01/19 01:11:25  gfx_friends
  *** empty log message ***

  Revision 1.10  2003/01/18 23:42:14  gfx_friends
  initial (non-working) version of triangle budget mode, etc.

  Revision 1.9  2003/01/17 21:33:52  gfx_friends
  New API support.

  Revision 1.8  2003/01/17 16:50:21  gfx_friends
  VifWrite()

  Revision 1.7  2003/01/16 16:41:41  gfx_friends
  Added initial support for VDSHierarchy. Conversion to Vif is included,
  but not Vif->VDS

  Revision 1.6  2003/01/16 03:38:58  gfx_friends
  Working on VIF interface

  Revision 1.5  2003/01/15 20:12:41  gfx_friends
  Basic functionality of GLOD with DiscreteHierarchy and EdgeCollapse.

  Revision 1.4  2003/01/15 17:24:11  gfx_friends
  Added DiscreteHierarchy output type to xbs

  Revision 1.3  2003/01/14 23:48:06  gfx_friends
  Part of my grand scheme to take over the world.

  Revision 1.2  2003/01/14 00:06:20  gfx_friends
  Added destructors.

  Revision 1.1  2003/01/13 20:30:18  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.3  2003/01/08 05:19:14  cohen
  Added first version of full edge collapse.

  Revision 1.2  2003/01/05 22:40:04  cohen
  Added initialization of model from an MT (with vertices and triangles
  only -- no nodes or edges).

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/

