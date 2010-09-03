/*****************************************************************************\
  testPointSet.C
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/testPointSet.C,v $
  $Revision: 1.4 $
  $Date: 2004/07/08 16:47:49 $
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

#include "Point.h"

/*----------------------------- Local Constants -----------------------------*/

#define MARGIN_THRESHOLD 0.1

/*------------------------------ Local Macros -------------------------------*/


/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/


/*------------------------ Local Function Prototypes ------------------------*/


/*---------------------------------Functions-------------------------------- */


int main(int argc, char **argv)
{
    PointSet p1;
    PointSet p2;
    mtVec3 v0, v1, v2, v3, v4, v5;
    float hausdorff, distanceBound, margin, sampleRadius;
    
#if 0
    p1.randomSet(5000);
    p2.randomSet(5000);
    hausdorff = p1.hausdorff(p2);
    fprintf(stderr, "Hausdorff: %f\n", hausdorff);

    p1.clear();
    p2.clear();
#endif
    
    v0.set(0, 0, 0);
    v1.set(1, 0, 0);
    v2.set(0, 1, 0);

    v3.set(0, 0, 0.01);
    v4.set(1, 0, 0.01);
    v5.set(0, 1, 0.01);

    sampleRadius = 1.0;
    margin = MAXFLOAT;
    while (margin > MARGIN_THRESHOLD)
    {
        p1.clear();     
        p1.addSampledTriangle(v0, v1, v2, sampleRadius);
        p2.clear();
        p2.addSampledTriangle(v3, v4, v5, sampleRadius);

        fprintf(stderr, "Radius: %f, Points: %d %d, ",
                sampleRadius, p1.numPoints, p2.numPoints);

        hausdorff = p1.hausdorff(p2);
        distanceBound = hausdorff + 2.0 * sampleRadius;
        margin = (2.0*sampleRadius) / distanceBound;
        fprintf(stderr, "Hausdorff: %f, Bound: %f, Margin: %f\n",
                hausdorff, distanceBound, margin);
        sampleRadius *= 0.5;
    }
    
    return 0;
}

/*****************************************************************************\
  $Log: testPointSet.C,v $
  Revision 1.4  2004/07/08 16:47:49  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.3  2003/07/26 01:17:45  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.2  2003/07/23 19:55:35  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.1  2003/01/13 20:30:17  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/

