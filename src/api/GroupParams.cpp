/* GLOD: Raw patch control and vertex-array interfaces
 ***************************************************************************
 * $Id: GroupParams.cpp,v 1.5 2004/07/19 19:18:41 gfx_friends Exp $
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

#include <stdio.h>
#include <math.h>
#if !defined(_WIN32) && !defined(__APPLE__)
#include <values.h>
#endif

#include "hash.h"
#include "glod_core.h"

#include <xbs.h>

/***************************************************************************/
void glodGroupParameteri(GLuint name, GLenum pname, GLint param)
{
    GLOD_Group *group =
	(GLOD_Group *)HashtableSearch(s_APIState.group_hash, name);
    if(group == NULL) {
	GLOD_SetError(GLOD_INVALID_NAME, "Group does not exist", name);
	return;
    }

    switch(pname)
    {
    case GLOD_ADAPT_MODE:
	switch(param)
	{
	case GLOD_ERROR_THRESHOLD:
	    group->setAdaptMode(ErrorThreshold);
	    break;
	case GLOD_TRIANGLE_BUDGET:
	    group->setAdaptMode(TriangleBudget);
	    break;
	default:
	    fprintf(stderr, "glodGroupParameteri():invalid adapt mode.\n");
	    break;
	}
	break;
    case GLOD_ERROR_MODE:
	switch(param)
	{
	case GLOD_OBJECT_SPACE_ERROR:
	    group->setErrorMode(ObjectSpace);
	    break;
	case GLOD_SCREEN_SPACE_ERROR:
	    group->setErrorMode(ScreenSpace);
	    break;
	default:
	    fprintf(stderr, "glodGroupParameteri(): invalid error mode.\n");
	    break;
	}
	break;
    case GLOD_MAX_TRIANGLES:
	group->setTriBudget(param);
	break;
    case GLOD_OBJECT_SPACE_ERROR_THRESHOLD:
	group->setObjectSpaceErrorThreshold((float)param);
	break;
    case GLOD_SCREEN_SPACE_ERROR_THRESHOLD:
	group->setScreenSpaceErrorThreshold((float)param);
	break;


    default:
      GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
      return;
    }

    return;
} /* End of glodGroupParameteri() **/

/* glodGroupParameterf
 ***************************************************************************/
void glodGroupParameterf (GLuint name, GLenum pname, GLfloat param)
{
    GLOD_Group *group =
	(GLOD_Group *)HashtableSearch(s_APIState.group_hash, name);
    if(group == NULL) {
	GLOD_SetError(GLOD_INVALID_NAME, "Group does not exist", name);
	return;
    }

    switch(pname)
    {
    case GLOD_OBJECT_SPACE_ERROR_THRESHOLD:
	group->setObjectSpaceErrorThreshold(param);
	break;
    case GLOD_SCREEN_SPACE_ERROR_THRESHOLD:
	group->setScreenSpaceErrorThreshold(param);
	break;
    default:
      GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
      return;
    }

    return;
    
} /* End of glodGroupParameterf() **/


/* glodGroupParameteriv
 ***************************************************************************/
void glodGetGroupParameteriv (GLuint name, GLenum pname, GLint *param) {
    GLOD_Group *group =
	(GLOD_Group *)HashtableSearch(s_APIState.group_hash, name);
    if(group == NULL) {
	GLOD_SetError(GLOD_INVALID_NAME, "Group does not exist");
	return;
    }
//    switch(pname) {
//    default:
      GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
//      return;
//    }
}

/* glodGroupParameterfv
 ***************************************************************************/
void glodGetGroupParameterfv (GLuint name, GLenum pname, GLfloat *param) {
/*    switch(pname) {
    default:
      GLOD_SetError(GLOD_UNKNOWN_PROPERTY, "Unknown property", pname);
      return;
    }*/
}

/***************************************************************************
 * $Log: GroupParams.cpp,v $
 * Revision 1.5  2004/07/19 19:18:41  gfx_friends
 * Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
 *
 * Revision 1.4  2004/07/12 15:38:40  gfx_friends
 * Converted the GLOD makefiles to the Rich-style makefiles: this means that concurrent Debug and release builds are possible. Also added a post-build step for Win32 to keep some external directory in sync (util/post_build.bat) with GLOD. Many other little tweaks and warning removals.
 *
 * Revision 1.3  2004/06/16 20:30:31  gfx_friends
 * values.h include change for osx
 *
 * Revision 1.2  2004/02/19 15:51:20  gfx_friends
 * Made the system compile in Win32 and patched a bunch of warnings.
 *
 * Revision 1.1  2004/02/04 07:21:02  gfx_friends
 * Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
 *
 ***************************************************************************/
