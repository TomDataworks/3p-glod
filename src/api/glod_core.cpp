/* GLOD: Init stuff
 ***************************************************************************
 * $Id: glod_core.cpp,v 1.22 2004/07/19 19:18:41 gfx_friends Exp $
 * $Revision: 1.22 $
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
#include <memory.h>
#include <stdlib.h>

#include "glod_core.h"
#include "glod_glext.h"

#include "manager.h"

GLOD_APIState s_APIState;
int GLOD_NUM_TILES;
int GLOD_TILE_ROWS;
int GLOD_TILE_COLS;
GLOD_Tile *tiles;
VDS::Manager s_VDSMemoryManager;

/*****************************************************************************/

/* glodInit
 ***************************************************************************/
GLOD_APIENTRY GLuint glodInit() {
  // init routine
  memset(&s_APIState, 0, sizeof(s_APIState));
  s_APIState.last_error = GLOD_NO_ERROR;
  s_APIState.object_hash = AllocHashtable();
  s_APIState.group_hash = AllocHashtable();
  GLOD_TILE_ROWS=1;
  GLOD_TILE_COLS=1;
  GLOD_NUM_TILES=GLOD_TILE_ROWS*GLOD_TILE_COLS;

  tiles=new GLOD_Tile();
  tiles[0].min_x=-1;
  tiles[0].max_x=1;
  tiles[0].min_y=-1;
  tiles[0].max_y=1;

  // init opengl extensions
  GLOD_InitGL();

  return 1;
}


/* glodShutdown
 ***************************************************************************/
GLOD_APIENTRY void glodShutdown() {
  // shutdown OpenGL
  GLOD_CleanupGL();

  // cleanup all groups using glodDeleteGroups... this will kill the
  // objects as well
  while(HashtableNumElements(s_APIState.group_hash) > 0) {
	HASHTABLE_FIRST_NODE(s_APIState.group_hash, node);
	glodDeleteGroup(node->key);
  }
  
  assert(HashtableNumElements(s_APIState.group_hash) == 0);
  assert(HashtableNumElements(s_APIState.object_hash) == 0);

  FreeHashtable(s_APIState.group_hash);
  FreeHashtable(s_APIState.object_hash);


  delete [] tiles;
}

/* glodGetError
 ***************************************************************************/
GLOD_APIENTRY GLenum glodGetError ( void ) {
    GLenum error = s_APIState.last_error;
    s_APIState.last_error = GLOD_NO_ERROR;
    return error;
}


/***************************************************************************
 * $Log: glod_core.cpp,v $
 * Revision 1.22  2004/07/19 19:18:41  gfx_friends
 * Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
 *
 * Revision 1.21  2004/07/09 00:47:06  gfx_friends
 * Memory leak fixes a plenty. --nat
 *
 * Revision 1.20  2004/06/25 18:58:40  gfx_friends
 * New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default
 *
 * Revision 1.19  2004/06/04 21:46:40  gfx_friends
 * Modified glodGetError to work like glGetError.
 * Added a glodGetError manual page.
 * Modified some of the existing manual pages.
 *
 * Revision 1.18  2004/02/19 15:51:21  gfx_friends
 * Made the system compile in Win32 and patched a bunch of warnings.
 *
 * Revision 1.17  2004/02/05 20:04:14  gfx_friends
 * Added glod_glext.h which provides top-level _glBindBufferARB (etc) functions. Note the underscore at the front.... this is to prevent collissions with some systems who'se gl.h defines ARB functions. These are initialized at bootup. A glodHasVBO() function is useful for dynamic use of them. --nat
 *
 * Revision 1.16  2004/02/04 07:21:03  gfx_friends
 * Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
 *
 * Revision 1.15  2003/07/26 01:17:15  gfx_friends
 * Fixed copyright notice. Added wireframe to sample apps. Minor
 * revisions to documentation.
 *
 * Revision 1.14  2003/07/23 19:55:26  gfx_friends
 * Added copyright notices to GLOD. I'm making a release.
 *
 * Revision 1.13  2003/07/09 22:50:03  gfx_friends
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
 * Revision 1.12  2003/06/24 03:20:54  bms6s
 * fixed stuff so glod builds; still need to fix xbs to use VIF2.2.
 *
 * right now draw() is in GLOD_Cut.  however, GLOD_Cut is an instance of an
 * object, and if the object consists of more than one patch, we need draw()
 * at the patch level, not the cut level.  how are we going to handle this?
 *
 * Revision 1.11  2003/06/11 08:07:23  bms6s
 * [vds and glod commit messages are identical]
 *
 * temporary memory allocation in place.  everything compiles in windows but i've no doubt completely destroyed the linux build; i will attempt to fix this tomorrow and then ask nat for help.
 *
 * beacause of the way vds is set up:
 * -calling adapt on a cut actually adapts all cuts in that cut's group
 * -calling render on a cut actually renders all cuts in that cut's patch
 *
 * api/vds_error,cpp, api/vds_render.cpp, and api/vds_simplify.cpp are not used by anything anymore - they've been replaced by vds_callbacks.h and vds_callbacks.cpp.  vds_callbacks.cpp could be moved to api/ if you think that's more appropriate.
 *
 * i replaced gl types in vds with generic equivalents, since i wasn't sure if glodlib will have the gl includes (from the errors i was getting it appeared not).
 *
 * gave simple with a load sphere50.ply command line when run in visual studio
 *
 * Revision 1.10  2003/06/10 20:41:10  bms6s
 * added global vds memory manager and stub functions to pass in render memory
 *
 * Revision 1.9  2003/06/09 19:08:27  gfx_friends
 * Renamed glod_core.c to glod_core.cpp -- more cleanup, Nat
 *
 * Revision 1.8  2003/06/09 17:22:18  gfx_friends
 * Minor API patches.
 *
 * Revision 1.7  2003/06/05 17:40:09  gfx_friends
 * Patches to build on Win32.
 *
 * Revision 1.6  2003/06/04 16:53:55  gfx_friends
 * Tore out CR.
 *
 * Revision 1.5  2003/01/17 21:33:52  gfx_friends
 * New API support.
 *
 * Revision 1.4  2003/01/15 20:44:54  bms6s
 * *** empty log message ***
 *
 * Revision 1.3  2003/01/15 18:54:42  gfx_friends
 * New handling semantics for the Object Hashtable.
 *
 * Revision 1.2  2003/01/15 17:24:26  gfx_friends
 * Foo the bar
 *
 * Revision 1.1  2003/01/14 23:39:32  gfx_friends
 * Major reorganization.
 *
 * Revision 1.2  2003/01/10 22:35:38  gfx_friends
 * Beginnings of Masseuse.
 *
 * Revision 1.1  2003/01/10 20:56:43  gfx_friends
 * Added initialization function to libraries, and created raw object structures.
 *
 ***************************************************************************/
