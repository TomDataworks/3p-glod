/* GLOD: Code for dealing with user-inserted vertex arrays
 ***************************************************************************
 * $Id: glod_raw.h,v 1.5 2004/07/19 19:18:42 gfx_friends Exp $
 * $Revision: 1.5 $
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
#ifndef GLOD_RAW_H
#define GLOD_RAW_H

#define GLOD_HAS_VERTEX_COLORS_3  0x0001
#define GLOD_HAS_VERTEX_COLORS_4  0x0002
#define GLOD_HAS_VERTEX_NORMALS   0x0004
#define GLOD_HAS_TEXTURE_COORDS_2 0x0008
#define GLOD_HAS_TEXTURE_COORDS_3 0x0010

class GLOD_RawPatch {
 public:
  int name;
  GLuint level;
  GLfloat geometric_error;
  unsigned int data_flags; /* one of the above constants */
  
  unsigned int num_triangles;
  unsigned int num_vertices;
  
  GLint* triangles; /* not necessarily 3 indices */
  
  GLfloat* vertices;  /* vertices 3f */
  GLfloat* vertex_texture_coords; /* texture_coords 2f */ /*XXX must change to 3f*/
  GLfloat* vertex_normals; /* 3f */
  GLfloat* vertex_colors;  /* 3f */

 public:
  GLOD_RawPatch() { 
      num_triangles = num_vertices = 0;
      triangles = NULL;
      vertices = vertex_texture_coords = vertex_normals = vertex_colors = NULL;
  };
  ~GLOD_RawPatch();
};

class GLOD_RawObject {
 public:
  GLOD_RawPatch** patches;
  unsigned int num_patches;
 public:
  GLOD_RawObject() {
    bufsize = 2; num_patches = 0;
    patches = new GLOD_RawPatch*[bufsize];
  }
  ~GLOD_RawObject() {
    for(unsigned int i = 0; i < num_patches; i++)
      delete patches[i];
    delete[] patches;
  }
  
  void AddPatch(GLOD_RawPatch* p) {
    if(bufsize == num_patches) {
      bufsize *= 2;
      GLOD_RawPatch** tbuf = new GLOD_RawPatch*[bufsize];
      for(unsigned int i = 0; i < num_patches; i++)
	tbuf[i] = patches[i];
      delete[] patches;
      patches = tbuf;
    }
    patches[num_patches] = p;
    num_patches++;
  }
 private:
  unsigned int bufsize;
};

#endif
/***************************************************************************
 * $Log: glod_raw.h,v $
 * Revision 1.5  2004/07/19 19:18:42  gfx_friends
 * Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
 *
 * Revision 1.4  2004/06/11 18:47:41  gfx_friends
 * Fix sources of g++ warnings
 *
 * Revision 1.3  2004/06/01 19:02:03  gfx_friends
 * Rich Holloways fix for multiple deletes
 *
 * Revision 1.2  2004/02/06 17:25:09  gfx_friends
 * Discrete_manual mkIII plus one or two small changes to the makefiles
 *
 * Revision 1.1  2004/02/04 07:21:06  gfx_friends
 * Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
 *
 ***************************************************************************/
