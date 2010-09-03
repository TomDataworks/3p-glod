/* GLOD: All of the unused functions that we need to no-op
 * If you decide to use one of these functions, yank it into another file.
 ***************************************************************************
 * $Id: glod_noop_funcs.cpp,v 1.8 2004/07/19 19:18:41 gfx_friends Exp $
 * $Revision: 1.8 $
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
/* $Log: glod_noop_funcs.cpp,v $
/* Revision 1.8  2004/07/19 19:18:41  gfx_friends
/* Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
/*
/* Revision 1.7  2003/07/26 01:17:15  gfx_friends
/* Fixed copyright notice. Added wireframe to sample apps. Minor
/* revisions to documentation.
/*
/* Revision 1.6  2003/07/23 19:55:26  gfx_friends
/* Added copyright notices to GLOD. I'm making a release.
/*
 * Revision 1.5  2003/06/09 19:11:14  gfx_friends
 * More renaming.
 *
 * Revision 1.4  2003/06/05 17:40:09  gfx_friends
 * Patches to build on Win32.
 *
 * Revision 1.3  2003/06/04 16:53:55  gfx_friends
 * Tore out CR.
 *
 * Revision 1.2  2003/01/17 21:33:52  gfx_friends
 * New API support.
 *
 * Revision 1.1  2003/01/14 23:39:32  gfx_friends
 * Major reorganization.
 *
 * Revision 1.2  2003/01/02 22:05:21  duca
 * Moved GLOD management code into glod_object.c
 *
 * Revision 1.1.1.1  2003/01/02 21:21:14  duca
 * First checkin: (1) Chromium SPU, (2) standalone library, (3) build system
 *   -- Nat
 *
 ***************************************************************************/

#include <stdio.h>
