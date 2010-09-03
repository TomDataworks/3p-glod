/* GLOD: Simplification error implementation
***************************************************************************
* $Id: Metric.h,v 1.5 2004/07/22 16:38:01 jdt6a Exp $
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
#ifndef GLOD_METRIC_H
#define GLOD_METRIC_H

#include <stdio.h>
#if defined(_WIN32) || defined(__APPLE__)
#include <float.h>
#else
#include <values.h>
#endif


#include "xbs.h"
#include "glod_core.h"
#include "hash.h"
#include "glod_error.h"

class SphereHalfEdgeError : public GLOD_Error {
    public:
        SphereHalfEdgeError() {error=0;}; //radius=0; center.set(0,0,0);}
        float calculateError(Model *model, Operation *op);
        xbsVertex *genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *operation,
                             int forgeGen){
            return NULL;
        };
};

class SphereEdgeError : public GLOD_Error {
    public:
        SphereEdgeError() {error=0;};
        float calculateError(Model *model, Operation *op);
        xbsVertex *genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *operation,
                             int forceGen);
};

class SphereErrorData : public GLOD_ErrorData {
    public:
        float radius;
//        xbsVec3 center;
        SphereErrorData() {radius=0;};
        SphereErrorData(xbsVertex *vert) {radius=0;};
        void update(Operation *op);
        void init(xbsVertex *vert) {radius=0; };
};

class QuadricHalfEdgeError : public GLOD_Error {
    public:
        QuadricHalfEdgeError() {error=0;};
        float calculateError(Model *model, Operation *op);
        xbsVertex *genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *op,
                             int forceGen){
            return NULL;
        };
};

class QuadricEdgeError : public GLOD_Error {
    public:
        QuadricEdgeError() {error=0;};
        float calculateError(Model *model, Operation *op);
        xbsVertex *genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *op,
                             int forceGen);
};

class QuadricErrorData : public GLOD_ErrorData {
    public:
        QuadricErrorData() {m11=m21=m22=m31=m32=m33=m41=m42=m43=m44=0;};
        QuadricErrorData(xbsVertex *vert) {QuadricErrorData(); init(vert);};
        float m11, m21, m22, m31, m32, m33, m41, m42, m43, m44;
        void update(Operation *op);
        void init(xbsVertex *vert);
};

class PermissionGridHalfEdgeError : public GLOD_Error {
    public:
        PermissionGridHalfEdgeError() {error=0;};
        float calculateError(Model *model, Operation *op);
        xbsVertex *genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *op,
                             int forceGen){
            return NULL;
        };
};


class PermissionGridEdgeError : public GLOD_Error {
    public:
        PermissionGridEdgeError() {error=0;};
        float calculateError(Model *model, Operation *op);
        xbsVertex *genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *op,
                             int forceGen);
};

class PermissionGridErrorData : public GLOD_ErrorData {
    public:
        PermissionGridErrorData() {m11=m21=m22=m31=m32=m33=m41=m42=m43=m44=0;};
        PermissionGridErrorData(xbsVertex *vert) {PermissionGridErrorData(); init(vert);};
        float m11, m21, m22, m31, m32, m33, m41, m42, m43, m44;
        void update(Operation *op);
        void init(xbsVertex *vert);
};

#endif

/***************************************************************************
 * $Log: Metric.h,v $
 * Revision 1.5  2004/07/22 16:38:01  jdt6a
 * prelimary permission grid stuff is set up.  now to integrate this into glod for real.
 *
 * Revision 1.4  2004/07/19 19:18:43  gfx_friends
 * Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
 *
 * Revision 1.3  2004/07/14 14:59:50  gfx_friends
 * Made handling of border heuristics more consistent. Cylinder now
 * simplifies again, and the torus patch borders work pretty well, too.
 * The case where borders are not preserved too well right now is using
 * full edge collapses and error quadrics, because the location of the
 * generated vertex does not in general lie along the collapsed edge
 * (it is chosen by an optimization involving an inversion of the quadric
 * matrix, yada yada yada). We may improve this by adding additional border
 * edge planes into the quadric, as done in some papers by Garland and
 * Lindstrom.
 *
 * Revision 1.2  2004/07/08 16:47:50  gfx_friends
 * Removed tabs and updated indentation for xbs source files
 *
 * Revision 1.1  2004/06/24 21:55:03  gfx_friends
 * new files handling the new and improved error metric stuff
 *
 ***************************************************************************/
