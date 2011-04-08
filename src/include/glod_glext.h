/* GLOD: OpenGL interface definitions
 ***************************************************************************
 * $Id: glod_glext.h,v 1.5 2004/07/21 16:38:52 gfx_friends Exp $
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
#ifndef GLOD_GLEXT_H
#define GLOD_GLEXT_H

#define GLOD_BUILD_WITH_VBO

static inline bool glodHasVBO();

/******************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifdef __APPLE__
#define APIENTRY
#endif

/* Constants
 *****************************************************************************/
#ifndef GL_ARB_vertex_buffer_object
/* GL types for handling large vertex buffer objects */
typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;

// constants for vertex buffer objects
#define GL_BUFFER_SIZE_ARB                0x8764
#define GL_BUFFER_USAGE_ARB               0x8765
#define GL_ARRAY_BUFFER_ARB               0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB       0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB       0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB 0x889F
#define GL_READ_ONLY_ARB                  0x88B8
#define GL_WRITE_ONLY_ARB                 0x88B9
#define GL_READ_WRITE_ARB                 0x88BA
#define GL_BUFFER_ACCESS_ARB              0x88BB
#define GL_BUFFER_MAPPED_ARB              0x88BC
#define GL_BUFFER_MAP_POINTER_ARB         0x88BD
#define GL_STREAM_DRAW_ARB                0x88E0
#define GL_STREAM_READ_ARB                0x88E1
#define GL_STREAM_COPY_ARB                0x88E2
#define GL_STATIC_DRAW_ARB                0x88E4
#define GL_STATIC_READ_ARB                0x88E5
#define GL_STATIC_COPY_ARB                0x88E6
#define GL_DYNAMIC_DRAW_ARB               0x88E8
#define GL_DYNAMIC_READ_ARB               0x88E9
#define GL_DYNAMIC_COPY_ARB               0x88EA
#endif

#define GL_CLAMP_TO_EDGE                  0x812F

/* Typedefs
***************************************************************************/
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef GLvoid * (APIENTRY * PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRY * PFNGLUNMAPBUFFERARBPROC) (GLenum target);

/* Functions
 ****************************************************************************/
extern PFNGLGENBUFFERSARBPROC _glGenBuffersARB; // VBO Name Generation Procedure
extern PFNGLBINDBUFFERARBPROC _glBindBufferARB; // VBO Bind Procedure
extern PFNGLBUFFERDATAARBPROC _glBufferDataARB; // VBO Data Loading Procedure
extern PFNGLDELETEBUFFERSARBPROC _glDeleteBuffersARB; // VBO Deletion Procedure

extern PFNGLMAPBUFFERARBPROC _glMapBufferARB; // VBO Map Procedure
extern PFNGLUNMAPBUFFERARBPROC _glUnmapBufferARB; // VBO Unmap Procedure

extern void GLOD_InitGL();
extern void GLOD_CleanupGL();

/* Accessors */
extern bool s_glodHasVBO;
static inline bool glodHasVBO() {return s_glodHasVBO;}

#endif



/***************************************************************************
 * $Log: glod_glext.h,v $
 * Revision 1.5  2004/07/21 16:38:52  gfx_friends
 * A couple of fixes for Windows.
 *
 * Revision 1.4  2004/07/20 21:54:35  gfx_friends
 * Major Discrete and AttribSet rewrite. Discrete now uses attribsets for its vertex data. -Nat
 *
 * Revision 1.3  2004/06/07 16:58:31  gfx_friends
 * Remove our dependancy on glext.h in windows (should compile without it now)
 *
 * Revision 1.2  2004/02/06 17:25:09  gfx_friends
 * Discrete_manual mkIII plus one or two small changes to the makefiles
 *
 * Revision 1.1  2004/02/05 20:04:18  gfx_friends
 * Added glod_glext.h which provides top-level _glBindBufferARB (etc) functions. Note the underscore at the front.... this is to prevent collissions with some systems who'se gl.h defines ARB functions. These are initialized at bootup. A glodHasVBO() function is useful for dynamic use of them. --nat
 *
 ***************************************************************************/
