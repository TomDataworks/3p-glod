/* GLOD: Raw patch control and vertex-array interfaces
 ***************************************************************************
 * $Id: Raw.cpp,v 1.9 2004/07/19 19:18:41 gfx_friends Exp $
 * $Revision: 1.9 $
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

void HandlePatch(GLOD_Object* obj, GLOD_RawPatch* patch, int level, float geometric_error);
GLOD_RawPatch* ProducePatch(GLenum mode, 
			GLenum first, GLenum count, 
			void* indices, GLenum indices_type); // in RawConvert.c


/***************************************************************************/

void glodInsertArrays (GLuint name, GLuint patchname, 
		       GLenum mode,
		       GLint first, GLsizei count,
		       GLuint level, GLfloat geometric_error) {
  GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
  if(obj == NULL) {
    GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist.", name);
    return;
  }
  
  if(obj->hierarchy != NULL) {
    GLOD_SetError(GLOD_INVALID_STATE, "This object has already been built! You cannot add more data to it!", name);
    return;
  }
  
  // get the patch from the vertex array
  GLOD_RawPatch* p = ProducePatch(mode, first, count, NULL, 0);
  if(p == NULL) {
    return; // ProducePatch has already set the error flag
  }

  // convert the any-named patch to a 0...N number
  if(HashtableSearch(obj->patch_id_map, patchname+1) == NULL) { // does this patch already exist
      p->name = HashtableNumElements(obj->patch_id_map);
      HashtableAdd(obj->patch_id_map, patchname+1, (void*) (ptrdiff_t) (p->name+1));
  } else {
    if(obj->format != GLOD_DISCRETE_MANUAL) {
      GLOD_SetError(GLOD_INVALID_NAME, "A patch of this name already exists!\n");
      delete p;
      return;
    }
  }
  
  // handle this patch
  HandlePatch(obj, p, level, geometric_error);
}


void glodInsertElements (GLuint name, GLuint patchname, 
			 GLenum mode, GLuint count, GLenum type, GLvoid* indices,
			 GLuint level, GLfloat geometric_error) {
  GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);

  if(obj == NULL) {
    GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist", name);
    return;
  }

  if(obj->hierarchy != NULL) {
    GLOD_SetError(GLOD_INVALID_STATE, "This object has already been built! You cannot add more data to it!", name);
    return;
  }

  // make the patch
  GLOD_RawPatch* p = ProducePatch(mode, 0, count, indices, type);
  if(p == NULL) {
    return; // ProducePatch has already set the error flag
  }

  // convert the any-named patch to a 0...N number
  if(HashtableSearch(obj->patch_id_map, patchname+1) == NULL) { // does this patch already exist
      p->name = HashtableNumElements(obj->patch_id_map);
      HashtableAdd(obj->patch_id_map, patchname+1, (void*) (ptrdiff_t) (p->name+1));
  } else {
    if(obj->format != GLOD_DISCRETE_MANUAL) {
      GLOD_SetError(GLOD_INVALID_NAME, "A patch of this name already exists!\n");
      delete p;
      return;
    }
  }
  
  // handle this patch
  HandlePatch(obj, p, level, geometric_error);
}

/***************************************************************************
 * CUT READBACK
 ***************************************************************************
 ***************************************************************************/
extern int ProduceVA(GLOD_Cut* c, int patchNum,
                     void* indices, GLenum indices_type);

GLOD_APIENTRY void glodFillArrays( GLuint name, GLuint patch_name ) {
  GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
  int patch_id;
  
  if(obj == NULL) {
    GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist", name);
    return;
  }
  
  if(obj->hierarchy == NULL) {
    GLOD_SetError(GLOD_INVALID_STATE, "This object has not been built!", name);
    return;
  }
  
  // look up the real patch name
  patch_id = HashtableSearchInt(obj->patch_id_map, patch_name+1); // lameness
  if(patch_id == 0) {
    // this patch isn't there
    GLOD_SetError(GLOD_INVALID_PATCH, "Patch of the specified doesn't exist.", patch_name);
    return;
  }
  patch_id --;// now, correct for the lameness of the hashtable, which stores everything +1
  
  // we're good to go... read the object
  if(ProduceVA(obj->cut, patch_id, NULL, 0) == 0) {
    return;
  }
}

GLOD_APIENTRY void glodFillElements( GLuint name, GLuint patch_name, 
                                     GLenum type, GLvoid* out_elements ) {
  // now read the cut
  GLOD_Object* obj = (GLOD_Object*) HashtableSearch(s_APIState.object_hash, name);
  int patch_id;
  
  if(obj == NULL) {
    GLOD_SetError(GLOD_INVALID_NAME, "Object does not exist", name);
    return;
  }
  
  if(obj->hierarchy == NULL) {
    GLOD_SetError(GLOD_INVALID_STATE, "This object has not been built!", name);
    return;
  }

  // look up the real patch name
  patch_id = HashtableSearchInt(obj->patch_id_map, patch_name+1); // lameness
  if(patch_id == 0) {
    // this patch isn't there
    GLOD_SetError(GLOD_INVALID_PATCH, "Patch of the specified doesn't exist.", patch_name);
    return;
  }
  patch_id --;// now, correct for the lameness of the hashtable, which stores everything +1

  // OK. we're good to go... readback this cut
  if(ProduceVA(obj->cut, patch_id, out_elements, type) == 0) {
    return;
  }

}

/***************************************************************************
 ***************************************************************************/
GLOD_RawPatch::~GLOD_RawPatch() {
  if(triangles)
    free(triangles);
  if(vertices)
    free(vertices);
  if(vertex_texture_coords)
    free(vertex_texture_coords);
  if(vertex_normals)
    free(vertex_normals);
  if(vertex_colors)
    free(vertex_colors);
}



/***************************************************************************
 * $Log: Raw.cpp,v $
 * Revision 1.9  2004/07/19 19:18:41  gfx_friends
 * Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
 *
 * Revision 1.8  2004/07/16 16:57:53  gfx_friends
 * When using half-edge collapses, DiscreteHierarchy now stores only one vertex array for eah patch array instead of one for every patch on every per level. --Nat
 *
 * Revision 1.7  2004/07/12 15:38:40  gfx_friends
 * Converted the GLOD makefiles to the Rich-style makefiles: this means that concurrent Debug and release builds are possible. Also added a post-build step for Win32 to keep some external directory in sync (util/post_build.bat) with GLOD. Many other little tweaks and warning removals.
 *
 * Revision 1.6  2004/06/16 20:30:32  gfx_friends
 * values.h include change for osx
 *
 * Revision 1.5  2004/02/19 15:51:21  gfx_friends
 * Made the system compile in Win32 and patched a bunch of warnings.
 *
 * Revision 1.4  2004/02/06 17:34:03  gfx_friends
 * removed some of the printfs from the DM code
 *
 * Revision 1.3  2004/02/06 17:25:09  gfx_friends
 * Discrete_manual mkIII plus one or two small changes to the makefiles
 *
 * Revision 1.2  2004/02/05 17:54:51  gfx_friends
 * Fixed the patch renumbering.
 *
 * Revision 1.1  2004/02/04 07:21:02  gfx_friends
 * Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
 *
 ***************************************************************************/
