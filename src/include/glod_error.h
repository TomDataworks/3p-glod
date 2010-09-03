/* GLOD: Simplification error class
***************************************************************************
* $Id: glod_error.h,v 1.4 2004/07/19 19:18:42 gfx_friends Exp $
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

#ifndef GLOD_ERROR_H
#define GLOD_ERROR_H

class Model;
class Operation;
class EdgeCollapse;

class GLOD_Error{
protected:
    float error;
public:
    GLOD_Error() {error=0;};
    virtual float calculateError(Model *model, Operation *op) = 0;
    virtual xbsVertex *genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *op,
                                 int forceGen) = 0;
    void setError(float ierror) {error=ierror;};
    float getError() {return error;};
    
};

class GLOD_ErrorData{
public:
    GLOD_ErrorData() {};
    virtual void update(Operation *op) = 0;
    virtual void init(xbsVertex *vert) = 0;
};

#endif


/***************************************************************************
* $Log: glod_error.h,v $
* Revision 1.4  2004/07/19 19:18:42  gfx_friends
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
* Revision 1.2  2004/07/08 16:16:38  gfx_friends
* many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)
*
* Revision 1.1  2004/06/24 21:55:10  gfx_friends
* new files handling the new and improved error metric stuff
*
***************************************************************************/
