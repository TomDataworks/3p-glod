/*****************************************************************************\
  Point.C
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/Point.C,v $
  $Revision: 1.5 $
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

#include <Point.h>

/*----------------------------- Local Constants -----------------------------*/


/*------------------------------ Local Macros -------------------------------*/
#ifdef _WIN32
#define for if(0); else for
#endif

/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/


/*------------------------ Local Function Prototypes ------------------------*/


/*---------------------------------Functions-------------------------------- */

/*****************************************************************************\
 @ PointSet::addSampledSubTriangle
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
PointSet::addSampledSubTriangle(mtVec3& vert0, mtVec3& vert1, mtVec3& vert2,
                                char addEdge0, char addEdge1, char addEdge2,
                                float squareMaxSampleRadius)
{
    mtVec3 center = (vert0+vert1+vert2) * (1.0/3.0);
    float squareSampleRadius = (center-vert0).SquaredLength();

    if (squareSampleRadius <= squareMaxSampleRadius)
        return;
    
    mtVec3 subVert0 = (vert0+vert1)*0.5;
    mtVec3 subVert1 = (vert1+vert2)*0.5;
    mtVec3 subVert2 = (vert2+vert0)*0.5;

    if (addEdge0)
        add(&subVert0);
    if (addEdge1)
        add(&subVert1);
    if (addEdge2)
        add(&subVert2);

    addSampledSubTriangle(vert0, subVert0, subVert2, addEdge0, 1, addEdge2,
                          squareMaxSampleRadius);
    addSampledSubTriangle(subVert0, vert1, subVert1, addEdge0, addEdge1, 1,
                          squareMaxSampleRadius);
    addSampledSubTriangle(subVert2, subVert1, vert2, 1, addEdge1, addEdge2,
                          squareMaxSampleRadius);
    addSampledSubTriangle(subVert0, subVert1, subVert2, 0, 0, 0,
                          squareMaxSampleRadius);

    return;    
} /** End of PointSet::addSampledSubTriangle() **/



/*****************************************************************************\
 @ PointSet::addSampledTriangle
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
PointSet::addSampledTriangle(mtVec3& vert0, mtVec3& vert1, mtVec3& vert2,
                             float maxSampleRadius)
{
    float squareMaxSampleRadius = maxSampleRadius*maxSampleRadius;
    mtVec3 center = (vert0+vert1+vert2) * (1.0/3.0);
    float squareSampleRadius = (center-vert0).SquaredLength();

    if (squareSampleRadius <= squareMaxSampleRadius)
    {
        add(&center);
        return;
    }

    mtVec3 subVert0 = (vert0+vert1)*0.5;
    mtVec3 subVert1 = (vert1+vert2)*0.5;
    mtVec3 subVert2 = (vert2+vert0)*0.5;

    addSampledTriangle(vert0, subVert0, subVert2,
                       maxSampleRadius);
    addSampledTriangle(subVert0, vert1, subVert1,
                       maxSampleRadius);
    addSampledTriangle(subVert2, subVert1, vert2,
                       maxSampleRadius);
    addSampledTriangle(subVert0, subVert1, subVert2,
                       maxSampleRadius);

    // temporary for debugging and profiling
    if (numPoints > 1000000)
    {
        fprintf(stderr, "BIG POINT SET.\n");
        exit(1);
    }
    
   
    return;
} /** End of PointSet::addSampledTriangle() **/

/*****************************************************************************\
 @ PointSet::maxMinDistOneSided
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
float
PointSet::maxMinDistOneSided(PointSet& ps)
{
    float maxMinSqDist = 0;

#if 0 // no grid

    for (int i=0; i<ps.numPoints; i++)
    {
        float minSqDist = minSquareDist(ps.points[i]);
#if 1
        if (minSqDist > maxMinSqDist)
            maxMinSqDist = minSqDist;
#else
        maxMinSqDist = MAX(minSqDist, maxMinSqDist);
#endif
    }

#else // grid

    mtVec3 minCorner = ps.points[0];
    mtVec3 maxCorner = ps.points[0];

    for (int i=1; i<ps.numPoints; i++)
    {
        mtVec3& point = ps.points[i];

        if (point[0] < minCorner[0])
            minCorner[0] = point[0];
        if (point[0] > maxCorner[0])
            maxCorner[0] = point[0];
        
        if (point[1] < minCorner[1])
            minCorner[1] = point[1];
        if (point[1] > maxCorner[1])
            maxCorner[1] = point[1];
        
        if (point[2] < minCorner[2])
            minCorner[2] = point[2];
        if (point[2] > maxCorner[2])
            maxCorner[2] = point[2];
    }
    for (int i=0; i<numPoints; i++)
    {
        mtVec3& point = points[i];

        if (point[0] < minCorner[0])
            minCorner[0] = point[0];
        if (point[0] > maxCorner[0])
            maxCorner[0] = point[0];
        
        if (point[1] < minCorner[1])
            minCorner[1] = point[1];
        if (point[1] > maxCorner[1])
            maxCorner[1] = point[1];
        
        if (point[2] < minCorner[2])
            minCorner[2] = point[2];
        if (point[2] > maxCorner[2])
            maxCorner[2] = point[2];
    }
    
    MultiGrid mgrid(minCorner, maxCorner, *this, 20, 10);
    for (int i=0; i<ps.numPoints; i++)
    {
        mtVec3 point = ps.points[i];
        PointSet **sets;
        int numSets;
        mgrid.getCellandNeighbors(point, &sets, &numSets);

        float minSqDist = MAXFLOAT;
        for (int j=0; j<numSets; j++)
        {
            float setMinSqDist =
                sets[j]->minSquareDist(point);
            if (setMinSqDist < minSqDist)
                minSqDist = setMinSqDist;
        }
        if (minSqDist > maxMinSqDist)
            maxMinSqDist = minSqDist;

        delete sets;
    }
    
#endif

    float maxMinDist = sqrt(maxMinSqDist);
    return maxMinDist;
} /** End of PointSet::maxMinDistOneSided() **/

/*****************************************************************************\
 @ Grid::Grid
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
Grid::Grid(mtVec3 minCorner, mtVec3 maxCorner,
           PointSet& points, mtVec3 cellSize)
{
    _cellSize = cellSize;
    minCorn = minCorner;
    mtVec3 size = maxCorner - minCorner;
    resolution[0] = ((_cellSize[0] == 0) || (size[0] == 0)) ?
        1 : (int)ceil(size[0]/_cellSize[0]);
    resolution[1] = ((_cellSize[1] == 0) || (size[1] == 0)) ?
        1 : (int)ceil(size[1]/_cellSize[1]);
    resolution[2] = ((_cellSize[2] == 0) || (size[2] == 0)) ?
        1 : (int)ceil(size[2]/_cellSize[2]);
    numCells = resolution[0] * resolution[1] * resolution[2];
    cells = new PointSet[numCells];

    for (int i=0; i<points.numPoints; i++)
    {
        mtVec3 point = points.points[i];
        int cellX, cellY, cellZ;
        xyzIndex(point, cellX, cellY, cellZ);
        int cellNum = cellIndex(cellX, cellY, cellZ);
        cells[cellNum].add(&point);
    }
} /** End of Grid::Grid() **/

/*****************************************************************************\
 @ Grid::Grid
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
Grid::Grid(mtVec3 minCorner, mtVec3 maxCorner,
           PointSet &points, int xyzResolution[3])
{
    resolution[0] = xyzResolution[0];
    resolution[1] = xyzResolution[1];
    resolution[2] = xyzResolution[2];
    
    minCorn = minCorner;
    mtVec3 size = maxCorner - minCorner;
    _cellSize[0] = size[0]/(float)resolution[0];
    _cellSize[1] = size[0]/(float)resolution[1];
    _cellSize[2] = size[0]/(float)resolution[2];
    numCells = resolution[0] * resolution[1] * resolution[2];
    cells = new PointSet[numCells];

    for (int i=0; i<points.numPoints; i++)
    {
        mtVec3 point = points.points[i];
        int cellX, cellY, cellZ;
        xyzIndex(point, cellX, cellY, cellZ);
        int cellNum = cellIndex(cellX, cellY, cellZ);
        cells[cellNum].add(&point);
    }
    return;
} /** End of Grid::Grid() **/

/*****************************************************************************\
 @ Grid::getCellandNeighbors
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
Grid::getCellandNeighbors(int cellX, int cellY, int cellZ,
                          PointSet ***sets,
                          int *numSets)
{
    int cIndex = cellIndex(cellX, cellY, cellZ);
    if (cIndex >= numCells)
    {
        fprintf(stderr, "Grid::getCellandNeighbors(): cellIndex out of range.\n");
        exit(1);
    }

    int minCellX = MAX(cellX-1, 0);
    int maxCellX = MIN(cellX+1, resolution[0]-1);
    int minCellY = MAX(cellY-1, 0);
    int maxCellY = MIN(cellY+1, resolution[1]-1);
    int minCellZ = MAX(cellZ-1, 0);
    int maxCellZ = MIN(cellZ+1, resolution[2]-1);

    *numSets =
        (maxCellX-minCellX+1) *
        (maxCellY-minCellY+1) *
        (maxCellZ-minCellZ+1);
    *sets = new PointSet *[*numSets];
    *numSets = 0;
    for (int i=minCellX; i<=maxCellX; i++)
        for (int j=minCellY; j<=maxCellY; j++)
            for (int k=minCellZ; k<=maxCellZ; k++)
            {
                cIndex = cellIndex(i,j,k);
                if (cIndex >= numCells)
                {
                    fprintf(stderr,
                            "Grid::getCellandNeighbors(): cellIndex out of range.\n");
                    exit(1);
                }
                (*sets)[(*numSets)++] = &(cells[cIndex]);
            }
    return;
} /** End of Grid::getCellandNeighbors() **/


/*****************************************************************************\
 @ MultiGrid::MultiGrid
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
MultiGrid::MultiGrid(mtVec3 minCorner, mtVec3 maxCorner,
                     PointSet& points, int maxCellPoints,
                     int maxLevs)
{
    mtVec3 size = (maxCorner - minCorner)*1.1; // extra buffer space
    mtVec3 center = (minCorner + maxCorner)*0.5;
    minCorn = center - (size*0.5);
    maxCorn = center + (size*0.5);
    maxLevels = maxLevs;
    levels = new Grid *[maxLevels];
    numLevels = 0;
    float currentSize = MAX(MAX(size[0], size[1]), size[2]);
    levels[0] = new Grid(minCorn, maxCorn, points,
                         mtVec3(currentSize,currentSize,currentSize));
    numLevels = 1;
    for (int i=1;
         ((i<maxLevels) && (levels[i-1]->maxCellPoints() > maxCellPoints));
         i++)
    {
        currentSize *= 0.5;
        levels[i] = new Grid(minCorn, maxCorn, points, currentSize);
        numLevels++;
    }
    return;
} /** End of MultiGrid::MultiGrid() **/

/*****************************************************************************\
 @ MultiGrid::getCell
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
PointSet&
MultiGrid::getCell(mtVec3 point)
{
    for (int level = numLevels-1; level >= 0; level--)
    {
        Grid *grid = levels[level];
        int cellX, cellY, cellZ, cellIndex;
        grid->xyzIndex(point, cellX, cellY, cellZ);
        cellIndex = grid->cellIndex(cellX, cellY, cellZ);
        PointSet &points = grid->getCell(cellIndex);
        if (points.numPoints > 0)
            return points;
    }

    fprintf(stderr, "MultiGrid::getCell(): no points found!\n");
    return *new PointSet;
    
} /** End of MultiGrid::getCell() **/

/*****************************************************************************\
 @ MultiGrid::getCellandNeighbors
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
MultiGrid::getCellandNeighbors(mtVec3 point,
                               PointSet ***sets,
                               int *numSets)
{
    int level;
    int cellX, cellY, cellZ;
    for (level = numLevels-1; level >= 0; level--)
    {
        Grid *grid = levels[level];
        int cellIndex;
        grid->xyzIndex(point, cellX, cellY, cellZ);
        cellIndex = grid->cellIndex(cellX, cellY, cellZ);
        PointSet &points = grid->getCell(cellIndex);
        if (points.numPoints > 0)
            break;
    }

    if (level < 0)
    {
        fprintf(stderr, "MultiGrid::getCell(): no points found!\n");
        exit(1);
    }

    Grid *grid = levels[level];
    grid->getCellandNeighbors(cellX, cellY, cellZ, sets, numSets);
    return;
    
} /** End of MultiGrid::getCell() **/


/*****************************************************************************\
  $Log: Point.C,v $
  Revision 1.5  2004/07/08 16:47:49  gfx_friends
  Removed tabs and updated indentation for xbs source files

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

