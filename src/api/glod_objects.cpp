/* GLOD: Functions for recieving and processing patches and objects
 *******************************************************************************
 * $Id: glod_objects.cpp,v 1.86 2004/09/21 21:39:15 gfx_friends Exp $
 * $Revision: 1.86 $
 ******************************************************************************/
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

#if 0
#define VERBOSE
#endif

#include "glod_core.h"

#include "hash.h"

#include <xbs.h>
#include "Discrete.h"
#include "DiscretePatch.h"
#include "Continuous.h"
//
//
//     API ENTRIES
//
//


void glodNewObject(GLuint name, GLuint groupname, GLenum format) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    
    if(obj != NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Name already in use:", name);
        return;
    }
    
    // is this format supported?
    if(!(format == GLOD_DISCRETE || format == GLOD_CONTINUOUS || format == GLOD_DISCRETE_MANUAL || format == GLOD_DISCRETE_PATCH)) {
        GLOD_SetError(GLOD_BAD_HIERARCHY, "Invalid hierarchy type:", format);
        return;
    }
    
    // make the object
    obj = new GLOD_Object();
    obj->name = name;
    obj->format = format;
    obj->group_name = groupname;
    HashtableAdd(s_APIState.object_hash, name, obj); // put it in the namespace
    
    // initialize the patch table
    obj->patch_id_map = AllocHashtableBySize(PATCH_HASH_BUCKET_SIZE);
    
    // prepare for inserting arrays
    // this is hierarchy-type specific
    if(format == GLOD_CONTINUOUS || format == GLOD_DISCRETE || format == GLOD_DISCRETE_PATCH) {
        
    } else if(format == GLOD_DISCRETE_MANUAL) {
        //...
    }
    
    // do not make the group yet! An object can only be in a group if it has a hierarchy!!!!
    // we do this in glodBuildObject / glodLoadObject
}


void glodInstanceObject(GLuint name, GLuint instancename, GLuint groupname)
{
    GLOD_Object* obj; GLOD_Object* dst;
    obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL ) {
        GLOD_SetError(GLOD_INVALID_NAME, "Source object doesn't exist:", name);
        return;
    }
    if( HashtableSearch(s_APIState.object_hash, instancename) != NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "An object already exists named ",  name);
        return;
    } 
    if(obj->hierarchy == NULL) {
        GLOD_SetError(GLOD_INVALID_STATE, "The source object has not been simplified yet." );
        return;
    }
    
    // make the object
    dst = new GLOD_Object(*obj);
    
    //fprintf(stderr, "In InstanceObject() - made GLOD_Object %x from GLOD_Object %x\n", dst, obj);
    //fprintf(stderr, "\tGLOD_Object(dst)=%x -> cut: %x -> mpCut: %x\n GLOD_Object(obj)=%x -> cut: %x -> mpCut: %x\n", dst, dst->cut, dst->cut->mpCut, obj, obj->cut, obj->cut->mpCut);
    dst->name = instancename;
    dst->group_name = groupname;
    dst->hierarchy->LockInstance();
    dst->budgetCoarsenHeapData=HeapElement(dst);
    dst->budgetRefineHeapData=HeapElement(dst);
    dst->inArea=new int[GLOD_NUM_TILES];
    
    HashtableAdd(s_APIState.object_hash, instancename, dst);
    
    // duplicate the patch_id_map... we don't have to worry about this changing
    dst->patch_id_map = AllocHashtableBySize(PATCH_HASH_BUCKET_SIZE);
    HASHTABLE_WALK(obj->patch_id_map, node); //hashtable  --> patch name to 0
    HashtableAdd(dst->patch_id_map, node->key, node->data);
    HASHTABLE_WALK_END(obj->patch_id_map);
    
    // make a cut
    dst->cut = obj->hierarchy->makeCut();
    
    //fprintf(stderr, "After dst->cut = obj->hierarchy->makeCut(),\nGLOD_Object(dst)=%x -> cut: %x -> mpCut: %x\n GLOD_Object(obj)=%x -> cut: %x -> mpCut: %x\n", dst, dst->cut, dst->cut->mpCut, obj, obj->cut, obj->cut->mpCut);
    
    // add it to the group...  
    GLOD_Group* group = (GLOD_Group*) HashtableSearch(s_APIState.group_hash, groupname);
    if(group == NULL) {
        group = new GLOD_Group();
        HashtableAdd(s_APIState.group_hash, groupname, group);
    }
    group->addObject(dst);
    
    
} /* End of glodInstanceObject() */


void glodBuildObject (GLuint name) { 
    
    GLOD_Object* obj; GLOD_Group* group;
    
    obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL ) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object doesn't exist: ", name);
        return;
    }
    Model *model=NULL;
    // Build the object
    if(obj->format == GLOD_DISCRETE || obj->format == GLOD_CONTINUOUS || obj->format == GLOD_DISCRETE_PATCH) {
        // check our state
        if(obj->prebuild_buffer == NULL) {
            GLOD_SetError(GLOD_INVALID_STATE, "Object has already been built:", name);
            return;
        }
        
        // run it through XBS
        
        XBSSimplifier *simp;
        
#ifdef VERBOSE
        printf("Sharing...\n"); fflush(stdout);
#endif
        
        model = new Model((GLOD_RawObject*)obj->prebuild_buffer);
        delete ((GLOD_RawObject*) obj->prebuild_buffer);
        
        model->share(obj->shareTolerance);
        model->indexVertTris();
        model->removeEmptyVerts();
        model->splitPatchVerts(); // note, if you disable this, undefine glod_core.h:XBS_SPLIT_BORDER_VERTS
        
#ifdef VERBOSE
        printf("Simplifying..."); fflush(stdout);
#endif
             
        model->errorMetric = obj->errorMetric;
        model->borderLock = obj->borderLock;
        model->snapMode = obj->snapMode;
        model->reductionPercent = obj->reductionPercent;
        model->numSnapshotSpecs = obj->numSnapshotSpecs;
        model->snapshotTriSpecs = new unsigned int[obj->numSnapshotSpecs];
        for (int i=0; i<model->numSnapshotSpecs; i++)
            model->snapshotTriSpecs[i] = obj->snapshotTriSpecs[i];
        model->numSnapshotErrorSpecs = obj->numSnapshotErrorSpecs;
        model->snapshotErrorSpecs = new GLfloat[obj->numSnapshotErrorSpecs];
        for (int i=0; i<model->numSnapshotErrorSpecs; i++)
            model->snapshotErrorSpecs[i] = obj->snapshotErrorSpecs[i];
        model->pgPrecision = obj->pgPrecision;

        
        switch(obj->format) {
        case GLOD_DISCRETE:
            obj->hierarchy = new DiscreteHierarchy(obj->opType);
            break;
        case GLOD_VDS:
            obj->hierarchy = new VDSHierarchy();
            break;
        case GLOD_DISCRETE_PATCH:
            obj->hierarchy = new DiscretePatchHierarchy(obj->opType);
            break;
        }
        simp = new XBSSimplifier(model, obj->opType, obj->queueMode, 
            obj->hierarchy);
        delete simp;
        simp = NULL;
        delete model;
        model = NULL;
#ifdef VERBOSE
        printf("done.\n");
#endif
        
    } else if(obj->format == GLOD_DISCRETE_MANUAL) {
        obj->hierarchy = new DiscreteHierarchy(Edge_Collapse);
        ((DiscreteHierarchy*)obj->hierarchy)->initialize((GLOD_RawObject*)obj->prebuild_buffer);
        delete ((GLOD_RawObject*) obj->prebuild_buffer);
        obj->format = GLOD_DISCRETE; // note that from here on, we pretend that we're discrete in all respects... so we change our format.
        
    } else {
        printf("Model is NULL. invalid hierarchy type?\n");
        assert(false);
        exit(0);
    }

    obj->prebuild_buffer = NULL; // each of the above branches must free this themselves.
    
    // put a reference on this hierarchy for later gc
    obj->hierarchy->LockInstance();
    
    // make a cut
    obj->cut = obj->hierarchy->makeCut();
    
    // now add it to the group
    group = (GLOD_Group*) HashtableSearch(s_APIState.group_hash, obj->group_name);
    if(group == NULL) {
        group = new GLOD_Group();
        HashtableAdd(s_APIState.group_hash, obj->group_name, group);
    }
    group->addObject(obj);
} /* End of glodBuildObject() **/


void glodDeleteObject(GLuint name)
{
    GLOD_Object* obj =
        (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    
    if(obj == NULL ) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object doesn't exist:", name);
        return;
    }
    
    FreeHashtableCautious(obj->patch_id_map); // deletes the patch mappings... we're cautious b/c the mappings aren't pointers but actually integers
    
    HashtableDeleteCautious(s_APIState.object_hash, obj->name); // this does not delete the object!!!
    delete obj;                                                 // this does
    
    return;
} /* End of glodDeleteObject() */


void glodBindObjectXform(GLuint objectname, GLenum what) {
    float m1[16];
    float m2[16];
    
    const bool h_model = what && GL_MODELVIEW_MATRIX;
    const bool h_proj = what && GL_PROJECTION_MATRIX;
    const bool h_neither = !(h_model || h_proj);
    
    GLOD_Object *obj =
        (GLOD_Object*) HashtableSearch(s_APIState.object_hash, objectname);
    
    if(obj == NULL ) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object doesn't exist:", objectname);
        return;
    }
    
    if(obj->hierarchy == NULL) {
        GLOD_SetError(GLOD_INVALID_STATE, "Object has not been built: ", objectname);
        return;
    }
    
    if(h_neither) {
        memset(m1, 0, sizeof(float)*16);
        m1[0] = 1.0;
        m1[5] = 1.0;
        m1[10] = 1.0;
        m1[15] = 1.0;
    }
    
    if(h_proj) {
        glGetFloatv(GL_PROJECTION_MATRIX, m1);
    }
    
    if(h_model) {
        glGetFloatv(GL_MODELVIEW_MATRIX, m2);
    }
    
    if(h_proj & ! h_model) {
        obj->cut->view.SetFrom(m1,NULL,NULL);
    }else if(h_model & ! h_proj) {
        obj->cut->view.SetFrom(m2, NULL,NULL);    
    } else if(h_neither) {
        obj->cut->view.SetFrom(m1, NULL,NULL);
    } else if(h_proj & h_model) {
        obj->cut->view.SetFrom(m1,m2,NULL);
    }
    
    obj->cut->viewChanged();
}

void glodObjectXform(GLuint objectname, float m1[16], float m2[16], float m3[16])
{
    GLOD_Object *obj =
        (GLOD_Object*) HashtableSearch(s_APIState.object_hash, objectname);
    float mt[16];
    
    if(obj == NULL ) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object doesn't exist:", objectname);
        return;
    }
    
    if(obj->hierarchy != NULL) {
        GLOD_SetError(GLOD_INVALID_STATE, "Object has not been built: ", objectname);
        return;
    }
    
    if( (m1 == NULL && (m2 != NULL || m3 != NULL)) || 
        (m2 == NULL && (m3 != NULL))) {
        GLOD_SetError(GLOD_INVALID_PARAM, "Null matrices must be the latter parameters.\n");
        return;
    }
    
    if( m1 == NULL) {
        m1 = mt;
        memset(m1, 0, sizeof(float)*16);
        m1[0] = 1.0;
        m1[5] = 1.0;
        m1[10] = 1.0;
        m1[15] = 1.0;
    }
    
    // now, bind the xform
    obj->cut->view.SetFrom(m1,m2,m3);
    obj->cut->viewChanged();  
}

/***************************************************************************/

void glodReadbackObject(GLuint name, GLvoid *data) { 
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist:", name);
        return;
    }
    
    int offset = 0;
    
    // prepend the hierarchy type
    memcpy(data, &obj->format, sizeof(int)); offset += sizeof(int);
    
    // now write out the patch->packed_patch hashtable
    memcpy(((char*)data)+offset, &obj->patch_id_map->num_elements, sizeof(unsigned int)); offset += sizeof(unsigned int);
    
    HASHTABLE_WALK(obj->patch_id_map, node);
    memcpy(((char*)data)+offset, (void*)&node->key, sizeof(unsigned int)); offset += sizeof(unsigned int);
    memcpy(((char*)data)+offset, (void*) &node->data, sizeof(unsigned int)); offset += sizeof(unsigned int);
    HASHTABLE_WALK_END(obj->patch_id_map);
    
    // now we have the object
    obj->hierarchy->readback((void*)((char*)data + offset));
}

// must stay in sync with the NewObject code
void glodLoadObject(GLuint name, GLuint group_name, const GLvoid *data) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj != NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "An object of the specified name already exists.", name);
        return;
    }
    
    // make the object
    obj = new GLOD_Object();
    obj->name = name;
    obj->group_name = group_name;
    HashtableAdd(s_APIState.object_hash, name, obj); // put it in the namespace
    
    // load the header... format and patch-indirect table
    int format;
    int offset = 0;
    memcpy(&format, ((char*)data) + offset, sizeof(int)); offset += sizeof(int);
    obj->format = format;
    
    // load the indirect table
    obj->patch_id_map = AllocHashtableBySize(PATCH_HASH_BUCKET_SIZE);
    unsigned int num_indirects;
    memcpy(&num_indirects, ((char*)data) + offset, sizeof(unsigned int)); offset += sizeof(unsigned int);
    for(int i = 0; i < num_indirects; i++) {
        unsigned int k; unsigned int value;
        memcpy(&k, ((char*)data) + offset, sizeof(unsigned int)); offset += sizeof(unsigned int);    
        memcpy(&value, ((char*)data) + offset, sizeof(unsigned int)); offset += sizeof(unsigned int);    
		HashtableAdd(obj->patch_id_map, k, (void*) ((ptrdiff_t) value));
    }
    
    // read the hierarchy
    switch(format) {
    case GLOD_DISCRETE:
        obj->hierarchy = new DiscreteHierarchy(Half_Edge_Collapse); // placeholder: op type gets set in the load()
        break;
    case GLOD_CONTINUOUS:
        obj->hierarchy = new VDSHierarchy();
        ((VDSHierarchy*) obj->hierarchy)->InitForLoad();
        break;
    case GLOD_DISCRETE_PATCH:
        obj->hierarchy = new DiscretePatchHierarchy(Half_Edge_Collapse);
        break;
    default:
        GLOD_SetError(GLOD_BAD_HIERARCHY, "Invalid hierarchy type in source data.", format );
        FreeHashtableCautious(obj->patch_id_map);
        HashtableDeleteCautious(s_APIState.object_hash, obj->name);
        delete obj;
        return;
    }
    if(obj->hierarchy->load((void*)(((char*)data) + offset)) == 0) {
        // we need to reset the object to a base state
        FreeHashtableCautious(obj->patch_id_map);
        HashtableDeleteCautious(s_APIState.object_hash, obj->name);
        delete obj;
        return;
    }
    
    // put this obj into production mode...
    obj->hierarchy->LockInstance();
    obj->cut = obj->hierarchy->makeCut();
    
    // now add it to the group
    GLOD_Group* group = (GLOD_Group*) HashtableSearch(s_APIState.group_hash, obj->group_name);
    if(group == NULL) {
        group = new GLOD_Group();
        HashtableAdd(s_APIState.group_hash, obj->group_name, group);
    }
    group->addObject(obj);
}

/***************************************************************************/

/* called by glodInsertArrays and glodInsertElements which are in Raw.cpp */
void HandlePatch(GLOD_Object* obj, GLOD_RawPatch* patch, int level, float geometric_error) {
    // now store this patch until build time
    if(obj->format == GLOD_DISCRETE || obj->format == GLOD_CONTINUOUS || obj->format == GLOD_DISCRETE_PATCH) {
        // DISCRETE & CONTINUOUS PATCH ACCUMULATION STEP
        if(obj->prebuild_buffer == NULL) obj->prebuild_buffer = (void*) new GLOD_RawObject();
        GLOD_RawObject* raw = (GLOD_RawObject*) obj->prebuild_buffer;
        raw->AddPatch(patch);
    } else {
        // store patches & levels
        patch->level = level;
        patch->geometric_error = geometric_error;
        
        if(obj->prebuild_buffer == NULL) obj->prebuild_buffer = (void*) new GLOD_RawObject();
        GLOD_RawObject* raw = (GLOD_RawObject*) obj->prebuild_buffer;
        raw->AddPatch(patch);
    }
}

/***************************************************************************/
void glodDrawPatch(GLuint name, GLuint patchname) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist", name);
        return;
    }
    
    // look up the real patch name
    int patch_id = HashtableSearchInt(obj->patch_id_map, patchname+1); // lameness
    if(patch_id == 0) {
        // this patch isn't there
        GLOD_SetError(GLOD_INVALID_PATCH, "Patch of the specified doesn't exist.", patchname);
        return;
    }
    patch_id --;// now, correct for the lameness of the hashtable, which stores everything +1
    
    obj->drawPatch(patch_id);
}

void DrawRawGLOD(int name);
void glodDebugDrawObject(GLuint name) {
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    if(obj == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist.\n", name);
        return;
    }
    
    if(obj->hierarchy == NULL) {
        fprintf(stderr, "Warning: drawing of raw objects is not supported by the spec.\n");
        DrawRawGLOD(name);
    } else {
        int p;
        glodGetObjectParameteriv(name, GLOD_NUM_PATCHES, (GLint*)&p);
        for(int i = 0; i < p; i++)
            obj->drawPatch(i);
    }
}




/***************************************************************************/
void DrawRawGLOD(int name) {
    int i; int j; int k;
    // for now, draw the patch wrongly
    GLOD_RawObject* raw;
    GLOD_RawPatch* patch;
    GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
    raw = (GLOD_RawObject*) obj->prebuild_buffer;
    if(raw  == NULL) {
        GLOD_SetError(GLOD_INVALID_NAME, "Raw Object doesn't exist!", name);
        return;
    }
    
    // draw
    for(i = 0; i < (int) raw->num_patches; i++) {
        patch = *raw->patches + i;
        
        glVertexPointer(3,
            GL_FLOAT,
            0,
            patch->vertices);
        glNormalPointer(GL_FLOAT,
            0,
            patch->vertex_normals);
        glDrawElements(GL_TRIANGLES,
            patch->num_triangles*3,
            GL_UNSIGNED_INT,
            patch->triangles);
        
        (void)j; (void)k;
    }
}




//
//
//     GLOD_Object Class Methods
//
//



GLOD_Object::~GLOD_Object()
{
    if (group != NULL)
    {
        group->removeObject(groupIndex);
        group = NULL;
    }
    if (cut != NULL)
    {
        delete cut;
        cut = NULL;
    }
    if (hierarchy != NULL)
    {
        hierarchy->ReleaseInstance();
        hierarchy = NULL;
    }
    if (prebuild_buffer != NULL)
    {
        delete (GLOD_RawObject*) prebuild_buffer;
        prebuild_buffer = NULL;
    }
    if (snapshotTriSpecs != NULL)
    {
        delete snapshotTriSpecs;
        snapshotTriSpecs = NULL;
    }
    if (snapshotErrorSpecs != NULL)
    {
	delete snapshotErrorSpecs;
	snapshotErrorSpecs = NULL;
    }

    
    delete [] inArea;
} /* End of GLOD_Object::~GLOD_Object() **/


void GLOD_Object::adaptScreenSpaceErrorThreshold(float threshold)
{
    cut->adaptScreenSpaceErrorThreshold(threshold);
    return;
} /* End of GLOD_Object::adaptScreenSpaceErrorThreshold() **/


void GLOD_Object::adaptObjectSpaceErrorThreshold(float threshold)
{
    cut->adaptObjectSpaceErrorThreshold(threshold);
    return;
} /* End of GLOD_Object::adaptObjectSpaceErrorThreshold() **/

void GLOD_Object::drawPatch(int patchnum)
{
    cut->draw(patchnum);
    return;
} /** End of GLOD_Object::adaptObjectSpaceErrorThreshold() **/

/****************************************************************************
 * $Log: glod_objects.cpp,v $
 * Revision 1.86  2004/09/21 21:39:15  gfx_friends
 * Fixed a problem with setting manual reduction percent and added permission
 * grids to windows dsp file.
 *
 * Revision 1.85  2004/08/04 22:26:27  gfx_friends
 * Miscellaneous patches, including more stuff added to the release notes.
 *
 * Revision 1.84  2004/07/28 06:07:10  jdt6a
 * more permission grid work.  most of voxelization code from dachille/kaufman paper in place, but only testing against plane of triangle right now (not the other 6 planes yet).
 *
 * run simple.exe with a "-pg" flag to get the permission grid version (which isn't fully working yet... for some reason the single plane testing which should be very conservative results in too strict of a grid, or i am not testing the grid correctly).  the point sampled version actually results in better, aka more simplified, models, so i think there is a bug somewhere in the voxelization or testing.
 *
 * after a run of simple, a file "pg.dat" will be dumped into the current directory.  the pgvis program lets you visualize this file, which is just the grid.
 *
 * Revision 1.83  2004/07/22 16:38:00  jdt6a
 * prelimary permission grid stuff is set up.  now to integrate this into glod for real.
 *
 * Revision 1.82  2004/07/21 18:43:41  gfx_friends
 * Added a flag XBS_SPLIT_BORDER_VERTS that allows us to tell whether a xbsVertex appears in multiple patches or not. If this value is defined, which it should always be for GLOD, the algorithm used to assign a vertex to a patch can be considerably simpler.
 *
 * Revision 1.81  2004/07/20 21:54:34  gfx_friends
 * Major Discrete and AttribSet rewrite. Discrete now uses attribsets for its vertex data. -Nat
 *
 * Revision 1.80  2004/07/16 16:57:53  gfx_friends
 * When using half-edge collapses, DiscreteHierarchy now stores only one vertex array for eah patch array instead of one for every patch on every per level. --Nat
 *
 * Revision 1.79  2004/07/12 20:06:12  gfx_friends
 * Fixed a VDScut destructor memory leak (subject to approval by the almighty bms6s) -nat
 *
 * Revision 1.78  2004/07/12 14:18:10  gfx_friends
 * Fixed a humongous mistake which was clearly someone else's fault.
 *
 * Revision 1.77  2004/07/10 03:48:18  gfx_friends
 * Moved the declaration of model a bit, as it wasn't compiling on osx (and i don't see how it could have compiled in windows/linux)
 *
 * Revision 1.76  2004/07/09 22:10:04  gfx_friends
 * Fixed a ton of VDS-related memory leaks.
 *
 * Revision 1.75  2004/07/09 22:05:51  gfx_friends
 * JC:
 *
 * Added new snapshot mode for specifying a list of errors rather than a
 * list of triangles. I haven't tested it yet...
 *
 * Revision 1.74  2004/07/09 00:47:06  gfx_friends
 * Memory leak fixes a plenty. --nat
 *
 * Revision 1.73  2004/07/08 16:15:52  gfx_friends
 * many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)
 *
 * Revision 1.72  2004/07/01 17:47:35  gfx_friends
 * even more leak fixes... looks pretty good now, although there are still small memory leaks in ply_read (around 50bytes)
 *
 * Revision 1.71  2004/06/29 14:31:25  gfx_friends
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
 * Revision 1.70  2004/06/25 19:16:41  gfx_friends
 * defines for the new hierarchy, plus change to the osx project
 *
 * Revision 1.69  2004/06/25 18:58:40  gfx_friends
 * New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default
 *
 * Revision 1.68  2004/06/24 21:49:00  gfx_friends
 * Added a new metric, quadric errors. Also a major redesign of the error calculation/storage functions, which are now in their own class
 *
 * Revision 1.67  2004/06/16 20:30:32  gfx_friends
 * values.h include change for osx
 *
 * Revision 1.66  2004/06/11 19:25:12  gfx_friends
 * Made GLOD keep quiet during execution.
 *
 * Revision 1.65  2004/06/11 19:21:35  gfx_friends
 * Fixed Win32 build.
 *
 * Revision 1.64  2004/06/03 19:04:06  gfx_friends
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
 * Revision 1.63  2004/03/29 02:15:44  gfx_friends
 * fixed a major bug with triangle budget mode, which would force only the root object from a instance list to be simplified.
 * Also a bug fix which would infinately loop if some objects were outside the view frustrum.
 *
 * Revision 1.62  2004/02/19 15:51:21  gfx_friends
 * Made the system compile in Win32 and patched a bunch of warnings.
 *
 * Revision 1.61  2004/02/05 20:33:37  gfx_friends
 * Cleanliness patches to make GLOD less ugly when it boots up on stdio.
 *
 * Revision 1.60  2004/02/04 07:21:03  gfx_friends
 * Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
 *
 * Revision 1.59  2004/02/04 03:47:25  gfx_friends
 *   - Move format flag from glodBuildObject to glodNewObject... I'm really sorry
 *     but we discovered in the paper-writing process that this was necessary
 *   - Refactored the discrete manual handling mode more cleanly. I am going to
 *     check in a cleaner refactoring soon. Its broken right now. (nat)
 *
 * Revision 1.58  2004/02/03 23:32:47  gfx_friends
 * Changes to Discrete Manual to allow multi patch levels, all the other goodies.
 * Still needs a change or two to straighten things out.
 *
 * Revision 1.57  2004/01/20 04:59:38  bms6s
 * changed finite/_finite to how jon made it cross-platform in heap.c
 *
 * Revision 1.56  2004/01/20 04:38:47  bms6s
 * encountered a situation where glGetFloatv was returning a modelview matrix with an infinite value in it, so added these checks in to catch that if it happens
 *
 * Revision 1.55  2003/10/20 21:48:38  gfx_friends
 * We now have cut readback of VDS objects. Yay.
 *
 * Note: if the code for ImmediateModeRenderCallback were to change, then we need to probably update the code that I've written in Hierarchy.C to match. However, this code isn't terribly complicated, so I suspect we won't really get into this situation much.
 *
 * Revision 1.54  2003/10/14 19:12:27  gfx_friends
 * Patched a bug in the instancing code that prevented the program "scene" from running. Bug was in glodInstanceObject. Now scenes should work again.
 *
 * Revision 1.53  2003/08/29 19:30:28  gfx_friends
 * Added VDS readback support and fixed up the readback code a bit.
 *
 * Revision 1.52  2003/08/27 18:36:06  gfx_friends
 * Bugfix that fixes glodDeleteGroup and glodDeleteObject.
 *
 * Revision 1.51  2003/08/15 02:57:43  gfx_friends
 * Added more bindObjectxform code and some bug fixes and kbd shortcuts to simple as well as locking.
 *
 * Revision 1.50  2003/08/14 20:38:47  gfx_friends
 * Added the new glodObjectXform parameters and fixed a few related things. However, outstanding issues that exist now are (a) we still compute our errors in pixels, whereas we've decided to switch to angle-of-error, and (b) We can't make VDS work until we either map it to 1 cut/object or change VDS to support transformations per object regardless of cut.
 *
 * Revision 1.49  2003/07/26 02:42:20  gfx_friends
 * Bugfix
 *
 * Revision 1.48  2003/07/26 01:17:15  gfx_friends
 * Fixed copyright notice. Added wireframe to sample apps. Minor
 * revisions to documentation.
 *
 * Revision 1.47  2003/07/25 03:00:13  gfx_friends
 * Jon Cohen -
 *
 * Cleaned up sharing a bit more -- now remove vertices which no longer
 * have any triangles.
 *
 * Removed all #define VDS_CONTEXT nonsense. VDS no longer #includes
 * anything from GLOD, so it is not necessary.
 *
 * Fixed some subtle problems with simplifying complicated
 * multi-attribute models, like brain-gear.normals.ply (which has normals
 * computed with sharp angle splitting). This model still trips some
 * warning messages (oops4 and oops2) when I simplify to VDS_Hierarchy
 * with Half_Edge_Collapse, but for now the program seems to recover well
 * enough to generate a working VDS. (the warnings basically indicate
 * that some vertices were not removed from the Model even after they
 * have been merged in the VIF). But it's a sign that something still
 * goes wrong in xbs occasionally.
 *
 * Shut up warning messages about things that are now considered okay.
 *
 * Revision 1.46  2003/07/23 19:55:26  gfx_friends
 * Added copyright notices to GLOD. I'm making a release.
 *
 * Revision 1.45  2003/07/23 06:36:06  bms6s
 * vds integrated
 *
 * Revision 1.44  2003/07/23 03:34:37  bms6s
 * set object format before generating hierarchy
 *
 * Revision 1.43  2003/07/22 02:57:17  gfx_friends
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
 * Revision 1.42  2003/07/21 23:10:51  gfx_friends
 * Added cut readback support. I'm still debugging, but need to move computers to my home. --n
 *
 * Revision 1.41  2003/07/21 21:42:39  gfx_friends
 * Jon Cohen -
 *
 * Made screen-space error threshold mode work again with discrete
 * hierarchies. Screen-space error computation currently assumes
 * projection matrix contains a perspective projection with a 45-degree
 * field of view.
 *
 * Also added an error-mode toggle key to the simple sample.
 *
 * Revision 1.40  2003/07/18 23:28:03  gfx_friends
 * New release codes.
 *
 * Revision 1.39  2003/07/18 23:02:46  gfx_friends
 * Another patch.
 *
 * Revision 1.38  2003/07/18 23:00:14  gfx_friends
 * The readback program works now in Windows. Also, I fixed a memory leak.
 *
 * Revision 1.37  2003/07/18 22:19:35  gfx_friends
 * Fixed most of the build problems. The lights have mysteriously gone off in the simple test program... I'm trying to figure out why. But the rest works, I think
 *
 * Revision 1.36  2003/07/16 16:12:29  gfx_friends
 * Added splitting of multi-patch vertices. Thus if a vertex touches
 * triangles from multiple patches, it is split into a multi-attribute
 * vertex (even if they have the same attributes). This is useful because
 * each of these vertices can store information like its index in any
 * hierarchy-related data structures.
 *
 * Revision 1.35  2003/07/15 20:18:53  gfx_friends
 * Major documentation effort and basic distribution readiness. We now have pod-based documentation for each GLOD function. It will build HTML or Man pages on Linux. To use the man pages, append glod/doc/man to your manpath after running make in doc or doing a top-level make. Also new is a release target... a top level make release builds with -O2 and any flags you also set based on the release target (See glod.conf). Also, #define DEBUG is active when building for debug.
 *
 * Revision 1.34  2003/07/09 22:50:03  gfx_friends
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
 * Revision 1.33  2003/07/02 21:50:29  gfx_friends
 * The patch numbering bug is now fixed. This bug was that all of our code assumes that patch numbers are tightly packed in the range 0..num_patches, whereas the GLOD api does not inforce such restrictions. The solution to this is a layer of indrection at the API level, and two glodGetObjectParameteriv calls, GLOD_PATCH_NAMES and GLOD_NUM_PATCHES. --n
 *
 * Revision 1.32  2003/07/01 21:49:10  gfx_friends
 * Tore out the glodDrawObject in favor of the more complete
 * glodDrawPatch. A bug somewhere between glodBuildObject's call to new
 * Model and the draw, however, is still failing, as all of the patches
 * have identical geometry by the time they hit the drawing point.
 *
 * Revision 1.31  2003/07/01 20:49:14  gfx_friends
 *  Readback for Discrete LOD now works. See samples/readback/read_model
 *  for demo and test. This, parenthetically, is also the most fully
 *  functional test program that we have. I will bring "simple" up to speed
 *  eventually, but in the meantime, you can use read_model similarly (run
 *  --help) in much the same way as previous ones to do testing.
 *
 * Revision 1.30  2003/06/30 19:29:56  gfx_friends
 * (1) InsertElements now works completely. --nat
 * (2) Some XBS classes got moved from xbs.h into discrete.h and Hierarchy.h for
 *     cleanliness.
 *
 * Revision 1.29  2003/06/27 04:33:47  gfx_friends
 * We now have a functioning glodInsertElements. There is a bug in
 * ModelShare.c that infiniteloops. I'm chasing that. -- n
 *
 * Revision 1.28  2003/06/26 18:52:57  gfx_friends
 * Major rewrite of GLOD-side Vertex Array handling routines (the so-called
 * masseusse) to allow more robust inputs. Let me tell you, the VA
 * interface is really pretty when you're using it, but using the data in a
 * coherent way is a nightmare because of all of the different options you
 * have as a user. This will allow me to implement the readback interface
 * faster... in theory, although that is going to be an equal nightmare. --
 * nat
 *
 * Revision 1.27  2003/06/09 19:15:02  gfx_friends
 * glod_objects.c -> glod_objects.cpp
 *
 * Revision 1.26  2003/06/06 16:47:34  gfx_friends
 * Win32 build is now functional. -- Nat
 *
 * Revision 1.25  2003/06/05 17:40:09  gfx_friends
 * Patches to build on Win32.
 *
 * Revision 1.24  2003/06/04 16:53:56  gfx_friends
 * Tore out CR.
 *
 * Revision 1.23  2003/01/22 19:12:05  gfx_friends
 * Fixed a few null pointer printfs.
 *
 * Revision 1.22  2003/01/22 17:39:57  gfx_friends
 * There is no spoon. Just like there are no bugs. But aside from that, fixed a BUildObject instantiation of Discrete bug.
 *
 * Revision 1.21  2003/01/21 10:54:31  bms6s
 * it now builds and works
 *
 * Revision 1.18  2003/01/20 22:19:53  gfx_friends
 * Fixed namespace with GDB bug.
 *
 * Revision 1.17  2003/01/20 19:23:42  gfx_friends
 * Binding code for GLOD_CONTINUOUS
 *
 * Revision 1.16  2003/01/20 07:42:38  gfx_friends
 * Added screen-space error mode. It seems to work for threshold mode,
 * but still gets stuck in triangle budget mode (object-space seems to
 * work okay in budget mode now).
 *
 * Revision 1.15  2003/01/20 04:41:19  gfx_friends
 * Fixed GLOD_Group::addObject() and adaptTriangleBudget. Added initial
 * class to set view.
 *
 * Revision 1.14  2003/01/20 04:14:40  gfx_friends
 * Fixed texturing bugs.
 *
 * Revision 1.13  2003/01/19 04:48:57  gfx_friends
 * Object-space error threshold mode for a single object in a single
 * group seems to be working. Not tested on multiple objects in multiple
 * groups, but it is trivial enough that it "should" work.
 *
 * Revision 1.12  2003/01/19 01:11:24  gfx_friends
 * *** empty log message ***
 *
 * Revision 1.11  2003/01/18 23:42:13  gfx_friends
 * initial (non-working) version of triangle budget mode, etc.
 *
 * Revision 1.10  2003/01/18 00:36:35  gfx_friends
 * Patch to the drawing structure & some misc stuff.
 *
 * Revision 1.9  2003/01/17 21:33:52  gfx_friends
 * New API support.
 *
 * Revision 1.8  2003/01/16 03:38:57  gfx_friends
 * Working on VIF interface
 *
 * Revision 1.7  2003/01/16 02:40:58  gfx_friends
 * Ported VDS callbacks and include support into GLOD.
 *
 * Revision 1.6  2003/01/15 20:28:23  gfx_friends
 * Fixed problem with HalfEdgeCollapse (turned back on something that
 * had been commented out)
 *
 * Revision 1.5  2003/01/15 20:12:41  gfx_friends
 * Basic functionality of GLOD with DiscreteHierarchy and EdgeCollapse.
 *
 * Revision 1.4  2003/01/15 18:54:43  gfx_friends
 * New handling semantics for the Object Hashtable.
 *
 * Revision 1.3  2003/01/15 17:27:08  gfx_friends
 * Fixed the foo the bar checkin.
 *
 * Revision 1.2  2003/01/15 17:24:26  gfx_friends
 * Foo the bar
 *
 * Revision 1.1  2003/01/14 23:39:32  gfx_friends
 * Major reorganization.
 *
 * Revision 1.4  2003/01/13 23:50:23  gfx_friends
 * Added the first pass at the Raw conversion components.
 *
 * Revision 1.3  2003/01/10 22:35:38  gfx_friends
 * Beginnings of Masseuse.
 *
 * Revision 1.2  2003/01/10 20:56:43  gfx_friends
 * Added initialization function to libraries, and created raw object structures.
 *
 * Revision 1.1  2003/01/02 22:05:21  duca
 * Moved GLOD management code into glod_object.c
 *
 * 
 ***************************************************************************/
