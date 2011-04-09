/*****************;-*-c++-*-**************************************************\
  Point.h
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/Point.h,v $
  $Revision: 1.6 $
  $Date: 2004/07/08 16:47:50 $
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
#if defined(_WIN32) || defined(__APPLE__)
#include <float.h>
#include <math.h>
#else
#include <values.h>
#endif
#include <math.h>

/*-------------------------------- Constants --------------------------------*/


/*--------------------------------- Macros ----------------------------------*/

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#ifdef _WIN32

#ifndef MAXFLOAT
#define MAXFLOAT              FLT_MAX
#endif

#endif

/*---------------------------------- Types ----------------------------------*/


/*---------------------------- Function Prototypes --------------------------*/


/*--------------------------------- Classes ---------------------------------*/

class PointSet
{
    private:

        void addSampledSubTriangle(mtVec3& vert0, mtVec3& vert1, mtVec3& vert2,
                                   char addEdge0, char addEdge1, char addEdge2,
                                   float squareMaxSampleRadius);
    
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
        ~PointSet()
        {
            delete [] points;
            points = NULL;
            numPoints = maxPoints = 0;
        }
    
        void add(mtVec3 *newPoints, int numNewPoints=1)
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
        float maxMinDistOneSided(PointSet& ps);
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
        void addSampledTriangle(mtVec3& vert0, mtVec3& vert1, mtVec3& vert2,
                                float maxSampleRadius);
        void print()
        {
            for (int i=0; i<numPoints; i++)
                points[i].print();
            fprintf(stderr, "\n");
        }
};

class Grid
{
    private:
        mtVec3 minCorn;
        mtVec3 _cellSize;
        PointSet *cells;
        int    numCells;
    public:
        int    resolution[3];
        Grid(mtVec3 minCorner, mtVec3 maxCorner,
             PointSet& points, mtVec3 cellSize);
        Grid(mtVec3 minCorner, mtVec3 maxCorner,
             PointSet& points, int xyzResolution[3]);
        ~Grid() { delete [] cells; };
        int maxCellPoints()
        {
            int maxPoints = 0;
            for (int i=0; i<numCells; i++)
            {
                maxPoints = MAX(maxPoints, cells[i].numPoints);
            }
            return maxPoints;
        }
        int cellIndex(int xIndex, int yIndex, int zIndex)
        {
            return
                resolution[0]*resolution[1]*zIndex +
                resolution[0]*yIndex + xIndex;
        }
        void xyzIndex(mtVec3 point, int& xIndex, int& yIndex, int& zIndex)
        {
            xIndex =
                (_cellSize[0] == 0.0) ? 0 :
                (int)((point[0]-minCorn[0])/_cellSize[0]);
            yIndex =
                (_cellSize[1] == 0.0) ? 0 :
                (int)((point[1]-minCorn[1])/_cellSize[1]);
            zIndex =
                (_cellSize[2] == 0.0) ? 0 :
                (int)((point[2]-minCorn[2])/_cellSize[2]);
            if ((xIndex < 0) || (xIndex >= resolution[0]))
            {
                fprintf(stderr, "xIndex out of range!\n");
                xIndex = MAX(MIN(xIndex, resolution[0]-1),0);
            }
            if ((yIndex < 0) || (yIndex >= resolution[1]))
            {
                fprintf(stderr, "yIndex out of range!\n");
                yIndex = MAX(MIN(yIndex, resolution[1]-1),0);
            }
            if ((zIndex < 0) || (zIndex >= resolution[2]))
            {
                fprintf(stderr, "zIndex out of range!\n");
                zIndex = MAX(MIN(zIndex, resolution[2]-1),0);
            }
        
            return;
        }
        PointSet& getCell(int cellIndex)
        {
            if ((cellIndex < 0) || (cellIndex >= numCells))
            {
                fprintf(stderr, "Grid::getCell(): cell out of range\n");
                exit(1);
            }
            return cells[cellIndex];
        };
        void getCellandNeighbors(int cellX, int cellY, int cellZ,
                                 PointSet ***sets,
                                 int *numSets);
};

class MultiGrid
{
    private:
        Grid **levels;
        int numLevels;
        int maxLevels;
        mtVec3 minCorn;
        mtVec3 maxCorn;
    public:
        MultiGrid(mtVec3 minCorner, mtVec3 maxCorner,
                  PointSet& points, int maxCellPoints,
                  int maxLevels);
        ~MultiGrid()
        {
            for (int i=0; i<numLevels; i++)
            {
                delete levels[i];
                levels[i] = NULL;
            }
            delete levels;
            levels = NULL;
        }
        PointSet& getCell(mtVec3 point);
        void getCellandNeighbors(mtVec3 point,
                                 PointSet ***sets,
                                 int *numSets);

};

/*---------------------------Globals (externed)------------------------------*/


/* Protection from multiple includes. */
#endif /* INCLUDED_POINT_H */


/*****************************************************************************\
  $Log: Point.h,v $
  Revision 1.6  2004/07/08 16:47:50  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.5  2004/06/16 20:30:36  gfx_friends
  values.h include change for osx

  Revision 1.4  2003/07/26 01:17:44  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.3  2003/07/23 19:55:34  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.2  2003/06/05 17:39:02  gfx_friends
  Patches to build on Win32.

  Revision 1.1  2003/01/13 20:30:15  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/

