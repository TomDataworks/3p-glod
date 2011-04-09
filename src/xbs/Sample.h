/******************;-*-c++-*-**************************************************\
  Point.h
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/Sample.h,v $
  $Revision: 1.4 $
  $Date: 2004/07/08 16:47:51 $
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


/* Protection from multiple includes. */
#ifndef INCLUDED_POINT_H
#define INCLUDED_POINT_H


/*------------------ Includes Needed for Definitions Below ------------------*/

#include <mt.h>
#include <values.h>
#include <math.h>

/*-------------------------------- Constants --------------------------------*/


/*--------------------------------- Macros ----------------------------------*/

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

/*---------------------------------- Types ----------------------------------*/


/*---------------------------- Function Prototypes --------------------------*/


/*--------------------------------- Classes ---------------------------------*/

class PointSet
{
    public:
        mtVec3 *points;
        int numPoints;
        int maxPoints;

        PointSet()
        {
            points = new mtVec3();
            numPoints = 0;
            maxPoints = 1;
        }
        void add(mtVec3 *newPoints, int numNewPoints)
        {
            while ((numPoints + numNewPoints) >= maxPoints)
            {
                mtVec3 *nPoints = new mtVec3[maxPoints*2];
                for (int i=0; i<numPoints; i++)
                    nPoints[i] = points[i];
                delete [] points;
                points = nPoints;
                maxPoints *= 2;
            }
            for (int i=0; i<numNewPoints; i++)
                points[numPoints++] = newPoints[i];
            return;
        }
        void clear() { numPoints = 0; }
        float minSquareDist(mtVec3& point)
        {
            float minSqDist = MAXFLOAT;
            float sqDist;
            for (int i=0; i<numPoints; i++)
            {
                sqDist = (point-points[i]).SquaredLength();
#if 1 /* much faster on my PC -- fewer memory writes */
                if (sqDist < minSqDist)
                    minSqDist = sqDist;
#else
                minSqDist = MIN(minSqDist, sqDist);
#endif
            }
            return minSqDist;
        }
        float maxMinDistOneSided(PointSet& ps)
        {
            float maxMinSqDist = 0;
            for (int i=0; i<numPoints; i++)
            {
                float minSqDist = minSquareDist(ps.points[i]);
#if 1
                if (minSqDist > maxMinSqDist)
                    maxMinSqDist = minSqDist;
#else
                maxMinSqDist = MAX(minSqDist, maxMinSqDist);
#endif
            }
            float maxMinDist = sqrt(maxMinSqDist);
            return maxMinDist;
        }
        float hausdorff(PointSet& ps)
        {
            float maxMinDist1 = maxMinDistOneSided(ps);
            float maxMinDist2 = ps.maxMinDistOneSided(*this);
            float hausdorffDist = MAX(maxMinDist1, maxMinDist2);
            return hausdorffDist;
        }
        void randomSet(int numPoints)
        {
            mtVec3 *randPoints = new mtVec3[numPoints];
            for (int i=0; i<numPoints; i++)
            {
                float randX = (double)(rand()) / (double)(RAND_MAX);
                float randY = (double)(rand()) / (double)(RAND_MAX);
                float randZ = (double)(rand()) / (double)(RAND_MAX);
                randPoints[i].set(randX, randY, randZ);
            }
            add(randPoints, numPoints);
            delete [] randPoints;
        }
        void addSampledTriangle(mtVec3& vert1, mtVec3& vert2, mtVec3& vert3,
                                float maxSampleDist);
};

/*---------------------------Globals (externed)------------------------------*/


/* Protection from multiple includes. */
#endif /* INCLUDED_POINT_H */


/*****************************************************************************\
  $Log: Sample.h,v $
  Revision 1.4  2004/07/08 16:47:51  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.3  2003/07/26 01:17:44  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.2  2003/07/23 19:55:35  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.1  2003/01/13 20:30:16  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/

