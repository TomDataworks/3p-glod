/* GLOD: OpenGL extension interfaces
 ***************************************************************************
 * $Id: glod_glext.cpp,v 1.4 2004/06/10 18:07:26 gfx_friends Exp $
 * $Revision: 1.4 $
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
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "glod_glext.h"

#if ! defined(__APPLE__) && ! defined(_WIN32)
#define GLX_GLXEXT_PROTOTYPES // hack
#include <GL/glx.h>
#endif

PFNGLGENBUFFERSARBPROC _glGenBuffersARB; // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC _glBindBufferARB; // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC _glBufferDataARB; // VBO Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC _glDeleteBuffersARB; // VBO Deletion Procedure
PFNGLMAPBUFFERARBPROC _glMapBufferARB; // VBO Map Procedure
PFNGLUNMAPBUFFERARBPROC _glUnmapBufferARB; // VBO Unmap Procedure

/***************************************************************************/
bool s_glodHasVBO = false;

/***************************************************************************/
void GLOD_InitGL() {
  // query available extensions for VBO
  char *exts=(char *)glGetString(GL_EXTENSIONS);
  if (strstr(exts, "GL_ARB_vertex_buffer_object")!=NULL){
    s_glodHasVBO=true;
  }
  else {
    s_glodHasVBO=false;
  }
  
  if(s_glodHasVBO) {
    //                printf("Vertex buffer objects enabled.\n");
    //    glEnableClientState(GL_VERTEX_ARRAY); /* what? */
    
#ifdef _WIN32
    _glGenBuffersARB =  (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
    _glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
    _glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
    _glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
    _glMapBufferARB = (PFNGLMAPBUFFERARBPROC) wglGetProcAddress("glMapBufferARB");
    _glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC) wglGetProcAddress("glUnmapBufferARB");
#else
#ifdef __APPLE__
    _glGenBuffersARB =  glGenBuffersARB;
    _glBindBufferARB = glBindBufferARB;
    _glBufferDataARB = glBufferDataARB;
    _glDeleteBuffersARB = glDeleteBuffersARB;
    _glMapBufferARB = glMapBufferARB;
    _glUnmapBufferARB = glUnmapBufferARB;
#endif
#if ! defined(__APPLE__) && ! defined(_WIN32)
    _glGenBuffersARB =  (PFNGLGENBUFFERSARBPROC) glXGetProcAddressARB((const GLubyte*)"glGenBuffersARB");
    _glBindBufferARB = (PFNGLBINDBUFFERARBPROC) glXGetProcAddressARB((const GLubyte*)"glBindBufferARB");
    _glBufferDataARB = (PFNGLBUFFERDATAARBPROC) glXGetProcAddressARB((const GLubyte*)"glBufferDataARB");
    _glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) glXGetProcAddressARB((const GLubyte*)"glDeleteBuffersARB");
    _glMapBufferARB = (PFNGLMAPBUFFERARBPROC) glXGetProcAddressARB((const GLubyte*)"glMapBufferARB");
    _glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC) glXGetProcAddressARB((const GLubyte*)"glUnmapBufferARB");
#endif
#endif
  }
}

void GLOD_CleanupGL() {
  
}

/***************************************************************************
 * $Log: glod_glext.cpp,v $
 * Revision 1.4  2004/06/10 18:07:26  gfx_friends
 * Fixed a glx protype.
 *
 * Revision 1.3  2004/06/02 17:13:58  gfx_friends
 * Changes to #includes so it works on a stock osx configuration
 *
 * Revision 1.2  2004/02/19 15:51:21  gfx_friends
 * Made the system compile in Win32 and patched a bunch of warnings.
 *
 * Revision 1.1  2004/02/05 20:04:15  gfx_friends
 * Added glod_glext.h which provides top-level _glBindBufferARB (etc) functions. Note the underscore at the front.... this is to prevent collissions with some systems who'se gl.h defines ARB functions. These are initialized at bootup. A glodHasVBO() function is useful for dynamic use of them. --nat
 *
 ***************************************************************************/
