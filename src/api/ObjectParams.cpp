/* GLOD: Object parameter setting and retrieval functions
 ***************************************************************************
 * $Id: ObjectParams.cpp,v 1.12 2004/10/12 16:35:09 gfx_friends Exp $
 * $Revision: 1.12 $
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
#include <stdio.h>
#include <math.h>
#if !defined(_WIN32) && !defined(__APPLE__)
#include <values.h>
#endif

#include "hash.h"
#include "glod_core.h"

#include <xbs.h>

/***************************************************************************/

void glodObjectParameteri (GLuint name, GLenum pname, GLint param) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist.", name);
        return;
    }

    switch(pname) {
        case GLOD_BUILD_OPERATOR:
        {
            switch(param)
            {
                case GLOD_OPERATOR_HALF_EDGE_COLLAPSE:
                    obj->opType = Half_Edge_Collapse;
                    break;
                case GLOD_OPERATOR_EDGE_COLLAPSE:
                    obj->opType = Edge_Collapse;
                    break;
                default:
                    GLOD_SetError(GLOD_UNSUPPORTED_PROPERTY,
                                  "Unsupported simplification operator.", param);
                    return;
                    break;
            }
            break;
        }
        case GLOD_BUILD_QUEUE_MODE:
        {
            switch(param)
            {
                case GLOD_QUEUE_GREEDY:
                    obj->queueMode = Greedy;
                    break;
                case GLOD_QUEUE_LAZY:
                    obj->queueMode = Lazy;
                    break;
                case GLOD_QUEUE_INDEPENDENT:
                    obj->queueMode = Independent;
                    break;
                default:
                    GLOD_SetError(GLOD_UNSUPPORTED_PROPERTY,
                                  "Unsupported simplification queue mode.", param);
                    return;
                    break;
            }
            break;
        }
        case GLOD_BUILD_BORDER_MODE:
        {
            switch(param)
            {
                case GLOD_BORDER_UNLOCK:
                    obj->borderLock = 0;
                    break;
                case GLOD_BORDER_LOCK:
                    obj->borderLock = 1;
                    break;
                default:
                    GLOD_SetError(GLOD_UNSUPPORTED_PROPERTY,
                                  "Unsupported border mode.", param);
                    return;
                    break;
            }
            break;
        }
        case GLOD_BUILD_ERROR_METRIC:
        {
            switch(param)
            {
                case GLOD_METRIC_SPHERES:
                case GLOD_METRIC_QUADRICS:
                case GLOD_METRIC_PERMISSION_GRID:
                    obj->errorMetric = param;
                    break;
                default:
                    GLOD_SetError(GLOD_UNSUPPORTED_PROPERTY,
                                  "Unsupported border mode.", param);
                    return;
                    break;
            }
            break;
        }
        case GLOD_BUILD_SNAPSHOT_MODE:
        {
            switch(param)
            {
                case GLOD_SNAPSHOT_PERCENT_REDUCTION:
                    obj->snapMode = PercentReduction;
                    break;
                case GLOD_SNAPSHOT_TRI_SPEC:
                    obj->snapMode = ManualTriSpec;
                    break;
                case GLOD_SNAPSHOT_ERROR_SPEC:
                    obj->snapMode = ManualErrorSpec;
                    break;
                default:
                    GLOD_SetError(GLOD_UNSUPPORTED_PROPERTY,
                                  "Unsupported snapshot mode.", param);
                    return;
                    break;
            }
      
            break;
        }
        case GLOD_BUILD_SHARE_TOLERANCE:
            if (param < 0.0)
            {
                GLOD_SetError(GLOD_INVALID_PARAM, "Share tolerance out of range");
                return;
            }
            obj->shareTolerance = param;
            break;
  
        default:
            GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
            return;
            break;
    }
    return;
}


void glodObjectParameteriv (GLuint name, GLenum pname, GLint count, GLint *param) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist.", name);
        return;
    }

    switch(pname) {
        case GLOD_BUILD_TRI_SPECS:
        {
            if (count < 1)
            {
                GLOD_SetError(GLOD_INVALID_PARAM, "Invalid triangle specifications.");
                return;
            }
      
            obj->numSnapshotSpecs = count;
            obj->snapshotTriSpecs = new unsigned int[count];
            for (int i=0; i<count; i++)
                obj->snapshotTriSpecs[i] = (unsigned int)param[i];

            unsigned int current = obj->snapshotTriSpecs[0];
            for (int i=1; i<count; i++)
            {
                if (obj->snapshotTriSpecs[i] >= current)
                {
                    GLOD_SetError(GLOD_INVALID_PARAM, "Invalid triangle specifications.");
                    delete obj->snapshotTriSpecs;
                    obj->snapshotTriSpecs = NULL;
                    return;
                }
                current = obj->snapshotTriSpecs[i];
            }
      
            break;
        }
        case GLOD_BUILD_ERROR_SPECS:
        {
            if (count < 1)
            {
                GLOD_SetError(GLOD_INVALID_PARAM, "Invalid error specifications.");
                return;
            }
      
            obj->numSnapshotErrorSpecs = count;
            obj->snapshotErrorSpecs = new float[count];
            for (int i=0; i<count; i++)
                obj->snapshotErrorSpecs[i] = (float)param[i];

            unsigned int current = obj->snapshotTriSpecs[0];
            for (int i=1; i<count; i++)
            {
                if (obj->snapshotTriSpecs[i] >= current)
                {
                    GLOD_SetError(GLOD_INVALID_PARAM, "Invalid triangle specifications.");
                    delete obj->snapshotTriSpecs;
                    obj->snapshotTriSpecs = NULL;
                    return;
                }
                current = obj->snapshotTriSpecs[i];
            }
      
            break;
        }
      
        default:
            GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
            return;
            break;
    }
    return;
}

void glodObjectParameterfv(GLuint name, GLenum pname, 
                           GLint count, GLfloat *param) 
{
    GLOD_Object* obj = 
        (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if (obj == NULL) 
    {
        GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist.", name);
        return;
    }

    switch(pname) 
    {
        case GLOD_BUILD_ERROR_SPECS:
        {
            if (count < 1)
            {
                GLOD_SetError(GLOD_INVALID_PARAM, 
                              "Invalid error specifications.");
                return;
            }
            
            obj->numSnapshotErrorSpecs = count;
            obj->snapshotErrorSpecs = new GLfloat[count];
            for (int i=0; i<count; i++)
                obj->snapshotErrorSpecs[i] = param[i];
            
            GLfloat current = obj->snapshotErrorSpecs[0];
            if (current < 0)
            {
                GLOD_SetError(GLOD_INVALID_PARAM, 
                              "Invalid error specifications.");
                delete obj->snapshotErrorSpecs;
                obj->snapshotErrorSpecs = NULL;
                obj->numSnapshotErrorSpecs = 0;
                return;
            }
        
            for (int i=1; i<count; i++)
            {
                if (obj->snapshotErrorSpecs[i] <= current)
                {
                    GLOD_SetError(GLOD_INVALID_PARAM, 
                                  "Invalid error specifications.");
                    delete obj->snapshotErrorSpecs;
                    obj->snapshotErrorSpecs = NULL;
                    return;
                }
                current = obj->snapshotErrorSpecs[i];
            }
            
            break;
        }
        case GLOD_QUADRIC_MULTIPLIER:
            if (count < 1)
                {
                GLOD_SetError(GLOD_INVALID_PARAM, 
                              "Invalid multiplier specifications.");
                return;
                }
            if (param[0]<=0){
                GLOD_SetError(GLOD_INVALID_PARAM, 
                              "Invalid multiplier specifications.");
                return;
            }          
            if (obj->errorMetric!=GLOD_METRIC_QUADRICS){
                GLOD_SetError(GLOD_INVALID_PARAM, 
                              "Quadric multiplier can only be changed when using quadrics");
                return;
            }
            obj->quadricMultiplier = param[0];
            obj->hierarchy->changeQuadricMultiplier(param[0]);
            break;
        default:
        {
            GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
            return;
            break;
        }
    }
    return;
}
    

/* glodObjectParameterf
***************************************************************************/
void glodObjectParameterf (GLuint name, GLenum pname, GLfloat param) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist", name);
        return;
    }

    switch(pname) {
        case GLOD_BUILD_SHARE_TOLERANCE:
            if (param < 0.0)
            {
                GLOD_SetError(GLOD_INVALID_PARAM, "Share tolerance out of range");
                return;
            }
            obj->shareTolerance = param;
            break;
        case GLOD_BUILD_PERCENT_REDUCTION_FACTOR:
            if ((param <= 0.0) || (param >= 1.0))
            {
                GLOD_SetError(GLOD_INVALID_PARAM, "Percent reduction factor out of range");
                return;
            }
            obj->reductionPercent = param;
            break;
        case GLOD_BUILD_PERMISSION_GRID_PRECISION:
            obj->pgPrecision = param;
            break;
        case GLOD_QUADRIC_MULTIPLIER:
            if (param<=0){
                GLOD_SetError(GLOD_INVALID_PARAM, 
                              "Invalid multiplier specifications.");
                return;
            }            
            if (obj->errorMetric!=GLOD_METRIC_QUADRICS){
                GLOD_SetError(GLOD_INVALID_PARAM, 
                              "Quadric multiplier can only be changed when using quadrics");
                return;
            }
            obj->quadricMultiplier = param;
            obj->hierarchy->changeQuadricMultiplier(param);
            break;
        default:
            GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
            return;
    }
}



/* glodObjectParameteriv
***************************************************************************/
void glodGetObjectParameteriv (GLuint name, GLenum pname, GLint *param) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist", name);
        return;
    }
    switch(pname) {
        case GLOD_NUM_PATCHES: /// XXX this may not be terribly safe ... nat asks himself later, why? // nat asks even later, why was he talking to himself? // recurse & panic
            *param = obj->hierarchy->GetPatchCount();
            break;
        case GLOD_PATCH_NAMES: 
        {
            unsigned int k; unsigned int d;
            HASHTABLE_WALK(obj->patch_id_map, node); //hashtable  --> patch name to 0-based-patch-index
            k = node->key - 1; d = (unsigned int) node->data - 1;
            param[d] = k;
            HASHTABLE_WALK_END(obj->patch_id_map);
        }
        break;
        case GLOD_READBACK_SIZE:
        {
            int more = 4 + 8 + 8 * HashtableNumElements(obj->patch_id_map);
            *param = obj->hierarchy->getReadbackSize() + more;
        }
        return;
        case GLOD_PATCH_SIZES:
        {
            if(obj->cut == NULL) {
                GLOD_SetError(GLOD_INVALID_STATE, "Object has not been built or does not have cut yet for some other reason!\n");
                return;
            }

            unsigned int k; unsigned int d;
            GLuint nV, nI;
            HASHTABLE_WALK(obj->patch_id_map, node); //hashtable  --> patch name to 0-based-patch-index
            k = node->key - 1; d = (unsigned int) node->data - 1;
            obj->cut->getReadbackSizes(d, &nI, &nV);
            param[(2*d)] = nI;
            param[(2*d)+1] = nV;
            HASHTABLE_WALK_END(obj->patch_id_map);
            return;
        }
        default:
            GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
            return;
    }
    return;
}

/* glodObjectParameterfv
***************************************************************************/
void glodGetObjectParameterfv (GLuint name, GLenum pname, GLfloat *param) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object doesn't exist.", name);
        return;
    }

    switch(pname) {
        case GLOD_NUM_PATCHES:
            GLOD_SetError(GLOD_UNSUPPORTED_PROPERTY, "GLOD_NUM_PATCHES only supports in integer inputs.");
            return;
        case GLOD_PATCH_NAMES:
            GLOD_SetError(GLOD_UNSUPPORTED_PROPERTY, "GLOD_PATCH_NAMES only supports in integer inputs.");
            return;
        case GLOD_READBACK_SIZE:
            GLOD_SetError(GLOD_UNSUPPORTED_PROPERTY, "GLOD_READBACK_SIZE only supports in integer inputs.");
            return;
        case GLOD_XFORM_MATRIX:
        {
            if(obj->hierarchy == NULL) {
                GLOD_SetError(GLOD_INVALID_STATE, "Object has not been built: ", name);
                return;
            }

            memcpy(param, obj->cut->view.matrix.cells, sizeof(float) * 16);
            break;
        }
        case GLOD_QUADRIC_MULTIPLIER:
            *param = obj->quadricMultiplier;
            break;
        default:
            GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
            return;
    }

}

/***************************************************************************
 * $Log: ObjectParams.cpp,v $
 * Revision 1.12  2004/10/12 16:35:09  gfx_friends
 * added a multiplier for error quadrics so the error can be more evenly distributed between the different error metrics. It can be changed during runtime
 *
 * Revision 1.11  2004/07/28 06:07:10  jdt6a
 * more permission grid work.  most of voxelization code from dachille/kaufman paper in place, but only testing against plane of triangle right now (not the other 6 planes yet).
 *
 * run simple.exe with a "-pg" flag to get the permission grid version (which isn't fully working yet... for some reason the single plane testing which should be very conservative results in too strict of a grid, or i am not testing the grid correctly).  the point sampled version actually results in better, aka more simplified, models, so i think there is a bug somewhere in the voxelization or testing.
 *
 * after a run of simple, a file "pg.dat" will be dumped into the current directory.  the pgvis program lets you visualize this file, which is just the grid.
 *
 * Revision 1.10  2004/07/22 16:38:00  jdt6a
 * prelimary permission grid stuff is set up.  now to integrate this into glod for real.
 *
 * Revision 1.9  2004/07/13 21:55:50  gfx_friends
 * Optimizations!
 *
 * Revision 1.8  2004/07/09 22:05:51  gfx_friends
 * JC:
 *
 * Added new snapshot mode for specifying a list of errors rather than a
 * list of triangles. I haven't tested it yet...
 *
 * Revision 1.7  2004/07/08 16:15:52  gfx_friends
 * many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)
 *
 * Revision 1.6  2004/06/29 14:31:24  gfx_friends
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
 * Revision 1.5  2004/06/24 21:48:59  gfx_friends
 * Added a new metric, quadric errors. Also a major redesign of the error calculation/storage functions, which are now in their own class
 *
 * Revision 1.4  2004/06/16 20:30:32  gfx_friends
 * values.h include change for osx
 *
 * Revision 1.3  2004/06/03 19:04:06  gfx_friends
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
 * Revision 1.2  2004/02/19 15:51:20  gfx_friends
 * Made the system compile in Win32 and patched a bunch of warnings.
 *
 * Revision 1.1  2004/02/04 07:21:02  gfx_friends
 * Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
 *
 ***************************************************************************/
