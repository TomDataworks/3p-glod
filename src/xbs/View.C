/*****************************************************************************\
  View.C
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/View.C,v $
  $Revision: 1.34 $
  $Date: 2004/07/21 18:43:42 $
  $Author: gfx_friends $
  $Locker:  $
\*****************************************************************************/
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


/*----------------------------- Local Includes -----------------------------*/

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <xbs.h>

/*----------------------------- Local Constants -----------------------------*/


/*------------------------------ Local Macros -------------------------------*/


/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/
int view_debug = 0;

/*------------------------ Local Function Prototypes ------------------------*/


/*---------------------------------Functions-------------------------------- */

void GLOD_View::SetFrom(float m1[16], float m2[16], float m3[16]) {
    if(view_debug == 1)
        printf("\n\n\nnew from\n");

    Mat4 M; Mat4 M1(m1);
    Mat4 M2; Mat4 M3;
    if(m2 != NULL)
        M2.Set(m2);

    if(m3 != NULL) {
        M3.Set(m3);
    }
  
    // mult M->M1...M3
    {
        if(m2 != NULL)
            M = M1*M2;
        else
            M = M1;
        if(m3 != NULL)
            M = M*M3;
    }
    matrix = M; // store this

    Mat4 Minverse;
    Minverse = M.Inverse();
}

xbsReal
GLOD_View::computePixelsOfError(xbsVec3 center, xbsVec3 offsets, xbsReal objectSpaceError, int area)

{
    Mat4 mat = this->matrix;
    Point3 points[8];
    int c=0;
    for (int x=0; x<2; x++)
        for (int y=0; y<2; y++)
            for (int z=0; z<2; z++){
                points[c].X=(x==0)?center[0]+offsets[0]:center[0]-offsets[0];
                points[c].Y=(y==0)?center[1]+offsets[1]:center[1]-offsets[1];
                points[c].Z=(z==0)?center[2]+offsets[2]:center[2]-offsets[2];
                c++;
            }
    float xMin=MAXFLOAT, xMax=-MAXFLOAT, yMin=MAXFLOAT, yMax=-MAXFLOAT, zMin=MAXFLOAT, zMax=-MAXFLOAT;

    for (int i=0; i<c; i++){
        points[i]= mat*points[i];
        
        xMin=(points[i].X<xMin)?points[i].X:xMin;
        yMin=(points[i].Y<yMin)?points[i].Y:yMin;
        zMin=(points[i].Z<zMin)?points[i].Z:zMin;
        xMax=(points[i].X>xMax)?points[i].X:xMax;
        yMax=(points[i].Y>yMax)?points[i].Y:yMax;
        zMax=(points[i].Z>zMax)?points[i].Z:zMax;
    }
    
    if(view_debug == 1)
        printf("%f %f\n", zMin, zMax);
    if ((zMax < -1.0f) || (zMin>1.0f)) // node is entirely behind near clipping plane
        return 0;
    if (area==-1){
        if ((xMin > 1.0f) || (xMax < -1.0f) || (yMin > 1.0f) ||(yMax < -1.0f))
            return 0;
    }
    else {
        if (area>=GLOD_NUM_TILES) return 0;
        if ((xMax<tiles[area].min_x) || (xMin>tiles[area].max_x) || (yMax<tiles[area].min_y) || (yMin>tiles[area].max_y)){
            return 0;
        }
    }
    float squareScreenBoxDiag = ((xMax-xMin)*(xMax-xMin)+(yMax-yMin)*(yMax-yMin)) / 8.0;
    float squareObjBoxDiag = offsets.SquaredLength() * 4;
    float error = sqrt(squareScreenBoxDiag/squareObjBoxDiag) * objectSpaceError + 0.0000000001;
    
    return error;
}

xbsReal GLOD_View::checkFrustrum(xbsVec3 center, xbsVec3 offsets, int area){
    if (area!=-1) return 0;
    Vec3 vdsCenter(center[0], center[1], center[2]);
    Vec3 vdsOffsets(offsets[0], offsets[1], offsets[2]);
    Point3 points[8];
    int c=0;
    for (int x=0; x<2; x++)
        for (int y=0; y<2; y++)
            for (int z=0; z<2; z++){
                points[c].X=(x==0)?center[0]+offsets[0]:center[0]-offsets[0];
                points[c].Y=(y==0)?center[1]+offsets[1]:center[1]-offsets[1];
                points[c].Z=(z==0)?center[2]+offsets[2]:center[2]-offsets[2];
                c++;
            }
    float xMin=MAXFLOAT, xMax=-MAXFLOAT, yMin=MAXFLOAT, yMax=-MAXFLOAT, zMin=MAXFLOAT, zMax=-MAXFLOAT;
    
    for (int i=0; i<c; i++){
        points[i]=matrix*points[i];
        xMin=(points[i].X<xMin)?points[i].X:xMin;
        yMin=(points[i].Y<yMin)?points[i].Y:yMin;
        zMin=(points[i].Z<zMin)?points[i].Z:zMin;
        xMax=(points[i].X>xMax)?points[i].X:xMax;
        yMax=(points[i].Y>yMax)?points[i].Y:yMax;
        zMax=(points[i].Z>zMax)?points[i].Z:zMax;
    }
    
    if (zMax < -1.0f || zMin > 1.0f) // node is entirely behind near clipping plane
        return 0.0f;
    
    if ((xMin > 1.0f) || (xMax < -1.0f) || (yMin > 1.0f) ||(yMax < -1.0f))
        return 0.0f;
    return 1.0f;
}


#if 0// disabled code

xbsReal
GLOD_View::computePixelsOfError(xbsVec3 center, xbsReal objectSpaceError)
{
//    xbsReal range = (center - eye).length() - radius
    if (objectSpaceError == MAXFLOAT)
        return MAXFLOAT/2.0f;

// siggraph paper hack - using the vds error calculation; seems to work
    xbsVec3 range = (center - eye);
    xbsReal range2 = range.dot(range);
    xbsReal radius2 = objectSpaceError * objectSpaceError;
    if (range2 < radius2) // very close to camera
    {
        return MAXFLOAT/2.0f;
    }
    return radius2 / range2;



    xbsReal rangeAlongView = (center-eye).dot(forward);
    /// XXX: we no longer have the viewport transformation! we cannoot compute error
        //       in terms of pixels!
        /*    xbsReal pixels =
              (objectSpaceError * xPixels) / (2.0*rangeAlongView*tanFOVby2);*/

        if (rangeAlongView == 0)
            return MAXFLOAT/2.0f;

        xbsReal pixels =
            (objectSpaceError * 1.0) / (2.0*rangeAlongView*tanFOVby2); /// the 1.0 means that our screen space errors are in terms of percent-of-screen-width*/
            //    xbsReal pixels = (2.0 * rangeAlongView) / cos(xFOV);
            return pixels;
}
#endif




/*****************************************************************************\
  $Log: View.C,v $
  Revision 1.34  2004/07/21 18:43:42  gfx_friends
  Added a flag XBS_SPLIT_BORDER_VERTS that allows us to tell whether a xbsVertex appears in multiple patches or not. If this value is defined, which it should always be for GLOD, the algorithm used to assign a vertex to a patch can be considerably simpler.

  Revision 1.33  2004/07/13 22:02:57  gfx_friends
  Major performance gains by fixing bad mat4 vec4 code.

  Revision 1.32  2004/07/13 19:56:25  gfx_friends
  further optimizations to the primtypes code, as well as to the computePixelsOfError function

  Revision 1.31  2004/07/08 16:47:49  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.30  2004/06/25 18:58:43  gfx_friends
  New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default

  Revision 1.29  2004/06/15 17:58:43  gfx_friends
  Moved the commented frustum code to the bottom of the file for clarity.

  Revision 1.28  2004/06/11 18:25:00  gfx_friends
  vds and discrete are now somewhat consistent in screen-space
  error. Screen-space error should be roughly in the range [0,1] with
  1.0 meaning error is the length of the screen-space diagonal.

  Revision 1.27  2004/06/02 17:14:05  gfx_friends
  Changes to #includes so it works on a stock osx configuration

  Revision 1.26  2004/05/25 19:59:39  gfx_friends
  Another typo fix, this time in checkFrustrum

  Revision 1.25  2004/05/25 06:34:35  bms6s
  fixing typos; are we still meeting today?

  Revision 1.24  2004/05/24 00:53:36  gfx_friends
  Couple minor changes to permit windows compiling once more

  Revision 1.23  2004/05/19 18:30:27  gfx_friends
  Changed the way the bounding box is computed for Discrete, plus added a rather hacky scaling of the VDS object space error so that the object space errors from Discrete and VDS are similiar. The screen space error method gives similiar results without scaling.

  Revision 1.22  2004/03/12 17:00:54  gfx_friends
  First stab at using bounding boxes for error calculations. VDS can finally once again be checked out (i hope...)

  Revision 1.21  2004/01/21 18:00:21  bms6s
  turned view frustum simplification on (maybe we need to make a new group param to be able to control it?)

  changed view parameter extraction so forward vector seems to be captured ok

  Revision 1.20  2004/01/21 11:50:05  bms6s
  dude, what was i thinking!

  Revision 1.19  2004/01/21 11:22:06  bms6s
  put vds error calculation in computePixelsOfError and it seems to do reasonable things

  Revision 1.18  2004/01/21 07:52:40  gfx_friends
  Fixed jitter in the up and forward vectors. Cleaned code and added an ifdef to save my mind when debugging this shizze in the future.

  Revision 1.17  2004/01/20 23:01:09  gfx_friends
  Patch maybe to make gtanks work? -- nat

  Revision 1.16  2004/01/20 20:46:29  bms6s
  putting these lines back in becase screen space error doesn't seem to work without them; however, they cause divide by zero errors in glodTanks, and the matrices look valid when it happens.

  Revision 1.15  2004/01/20 04:48:10  bms6s
  two divide by zero problems:
  -in the function to set the view from the modelview and/or projection matrix, i can't figure out why you were dividing your forward and up vectors by a single entry in the matrix when they were just about to be normalized anyway; this was causing problems when the entry in the matrix was 0, so i just commented it out.
  -in computePixelsOfError, it's possible for rangeAlongView to be 0 - this happens if the forward and (center-eye) vectors are orthogonal, or if the eye is at the center.  this was causing the error to be divided by 0 in these cases, so i added in an explicit check.  dave further asks why we even use the range along the view direction instead of just the length of (center-eye).

  Revision 1.14  2003/10/23 01:44:25  bms6s
  -removed coarsenError, currentError, coarsenRadius, coarsenCenter, refineRadius, and refineCenter from GLOD_Cut.  replaced them with coarsenErrorObjectSpace(), coarsenErrorScreenSpace(), currentErrorObjectSpace(), and currentErrorScreenSpace().  this makes code much cleaner and ensures consistency in the errors of discrete and continuous cuts (at least inasmuch as the discrete cuts's errors[] array is consistent with VDS' node radii).
  -added call to update VDS errors at the beginning of every continuous cut coarsen() or refine() call.  this is more than necessary; conceivably we could just call it once per glod adaptXXX() call, perhaps in the beginning just before the queues are constructed.
  -if coarsen() is going to get called with triTermination equal to the number of tris already in the cut, then instead we set triTermination to 0.
  -modified vds node error callback StdErrorScreenSpaceNoFrustum to use the VDS::Cut's mpExternalViewClass pointer to calculate node error using computePixelsOfError().  so this means that view frustum simplification is currently disabled.  it shouldn't be too hard to similarly convert StdErrorScreenSpace to use glod's view frustum check and computePixelsOfError as well.
  -modified computePixelsOfError to only take a center and object space error (that's all it was using anyway), since that's all VDS can provide it.

  Revision 1.13  2003/08/29 21:15:10  gfx_friends
  Trying to fix up the binding from GL matrices to VDS.

  Revision 1.12  2003/08/27 20:29:11  gfx_friends
  Fixed a bug in screen-space-error computation and improved the functionality of GLOD_App.

  Revision 1.11  2003/08/15 02:57:43  gfx_friends
  Added more bindObjectxform code and some bug fixes and kbd shortcuts to simple as well as locking.

  Revision 1.10  2003/08/15 02:29:43  gfx_friends
  The new bind object xform is working. I'm not sure how well vds integration works because we've never connected up a properly working camera to the system before. But I think we're close.

  Revision 1.9  2003/08/14 22:20:38  gfx_friends
  Trying to make screen space work.

  Revision 1.8  2003/08/14 20:38:50  gfx_friends
  Added the new glodObjectXform parameters and fixed a few related things. However, outstanding issues that exist now are (a) we still compute our errors in pixels, whereas we've decided to switch to angle-of-error, and (b) We can't make VDS work until we either map it to 1 cut/object or change VDS to support transformations per object regardless of cut.

  Revision 1.7  2003/07/26 01:17:45  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.6  2003/07/23 19:55:35  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.5  2003/07/21 21:42:41  gfx_friends
  Jon Cohen -

  Made screen-space error threshold mode work again with discrete
  hierarchies. Screen-space error computation currently assumes
  projection matrix contains a perspective projection with a 45-degree
  field of view.

  Also added an error-mode toggle key to the simple sample.

  Revision 1.4  2003/06/06 16:47:34  gfx_friends
  Win32 build is now functional. -- Nat

  Revision 1.3  2003/06/05 17:39:02  gfx_friends
  Patches to build on Win32.

  Revision 1.2  2003/01/20 22:19:54  gfx_friends
  Fixed namespace with GDB bug.

  Revision 1.1  2003/01/20 04:41:19  gfx_friends
  Fixed GLOD_Group::addObject() and adaptTriangleBudget. Added initial
  class to set view.

\*****************************************************************************/


