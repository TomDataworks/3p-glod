/*****************************************************************************\
  Sample.C
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/Sample.C,v $
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

#include "xbs.h"

/*----------------------------- Local Constants -----------------------------*/

#define TRUE  1
#define FALSE 0

/*------------------------------ Local Macros -------------------------------*/

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/


/*------------------------ Local Function Prototypes ------------------------*/

int closestSample(mtVec3 point, mtVec3 **samples, int numSamples);
int PointInTriangle(mtVec2 vert1, mtVec2 vert2, mtVec2 vert3, mtVec2 point);
void generateSamples(mtVec3 &vert1, mtVec3 &vert2, mtVec3 &vert3,
                     mtReal sampleSpacing,
                     mtVec3 ***samples, int *numSamples);

/*---------------------------------Functions-------------------------------- */


/*****************************************************************************\
 @ PointInTriangle()
 -----------------------------------------------------------------------------
 description : Returns TRUE if the 2D point is in the 2D triangle,
               FALSE otherwise.
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
int PointInTriangle(mtVec2 vert1, mtVec2 vert2, mtVec2 vert3, mtVec2 point)
{
    int c = FALSE;

    if ((((vert1[MT_Y]<=point[MT_Y]) && (point[MT_Y]<vert3[MT_Y])) ||
         ((vert3[MT_Y]<=point[MT_Y]) && (point[MT_Y]<vert1[MT_Y]))) &&
        (point[MT_X] < ((vert3[MT_X] - vert1[MT_X]) *
                        (point[MT_Y] - vert1[MT_Y]) /
                        (vert3[MT_Y] - vert1[MT_Y]) +
                        vert1[MT_X])))
        c = !c;
    if ((((vert2[MT_Y]<=point[MT_Y]) && (point[MT_Y]<vert1[MT_Y])) ||
         ((vert1[MT_Y]<=point[MT_Y]) && (point[MT_Y]<vert2[MT_Y]))) &&
        (point[MT_X] < ((vert1[MT_X] - vert2[MT_X]) *
                        (point[MT_Y] - vert2[MT_Y]) /
                        (vert1[MT_Y] - vert2[MT_Y]) +
                        vert2[MT_X])))
        c = !c;
    if ((((vert3[MT_Y]<=point[MT_Y]) && (point[MT_Y]<vert2[MT_Y])) ||
         ((vert2[MT_Y]<=point[MT_Y]) && (point[MT_Y]<vert3[MT_Y]))) &&
        (point[MT_X] < ((vert2[MT_X] - vert3[MT_X]) *
                        (point[MT_Y] - vert3[MT_Y]) /
                        (vert2[MT_Y] - vert3[MT_Y]) +
                        vert3[MT_X])))
        c = !c;

    return (c ? TRUE : FALSE);
} /** End of PointInTriangle() **/



/*****************************************************************************\
 @ generateSamples
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void generateSamples(mtVec3 &vert1, mtVec3 &vert2, mtVec3 &vert3,
                     mtReal sampleSpacing,
                     mtVec3 ***samples, int *numSamples)
{
    mtVec3 vec1 = vert2 - vert1;
    vec1.normalize();
    mtVec3 vec2 = vert3 - vert1;
    vec2.normalize();
    mtVec3 normal = vec1 ^ vec2;
    normal.normalize();
    
    // coordinate system for sample grid is (v1, v2, v3) with the grid
    // in the v1-v2 plane
    mtVec3 v1 = vec1;
    mtVec3 v3 = normal;
    mtVec3 v2 = normal ^ vec1;

    fprintf(stderr, "v1: ");
    v1.print();
    fprintf(stderr, "\n");
    fprintf(stderr, "v2: ");
    v2.print();
    fprintf(stderr, "\n");
    fprintf(stderr, "v3: ");
    v3.print();
    fprintf(stderr, "\n");
    
    // convert verts to (v1,v2) coordinates
    mtVec2 vert1Grid = mtVec2(0,0);
    mtVec2 vert2Grid = mtVec2((vert2 - vert1).length(), 0);
    mtVec3 edge3 = vert3 - vert1;
    mtVec2 vert3Grid = mtVec2(edge3.dot(v1), edge3.dot(v2));

    mtVec2 minCorner = mtVec2(MIN(vert1Grid[MT_X], vert3Grid[MT_X]),
                              MIN(vert1Grid[MT_Y], vert3Grid[MT_Y]));
    mtVec2 maxCorner = mtVec2(MAX(vert2Grid[MT_X], vert3Grid[MT_X]),
                              MAX(vert2Grid[MT_Y], vert3Grid[MT_Y]));

    mtVec2 gridLengths = maxCorner - minCorner;
    int maxSamples =
        (int)((gridLengths[MT_X]/sampleSpacing) + 1) *
        (int)((gridLengths[MT_Y]/sampleSpacing) + 1);
    
    *samples = new mtVec3* [maxSamples];
    *numSamples = 0;
    for (mtReal xcoord = minCorner[MT_X] ;//+ sampleSpacing*0.5;
         xcoord <= maxCorner[MT_X]; xcoord += sampleSpacing)
    {
        for (mtReal ycoord = minCorner[MT_Y] ;// + sampleSpacing*0.5;
             ycoord <= maxCorner[MT_Y]; ycoord += sampleSpacing)
        {
            mtVec2 gridSample = mtVec2(xcoord, ycoord);
            if (PointInTriangle(vert1Grid, vert2Grid, vert3Grid,
                                gridSample))
            {
                mtVec3 sample3D =
                    vert1 + v1 * gridSample[MT_X] + v2 * gridSample[MT_Y];
                (*samples)[(*numSamples)++] = new mtVec3(sample3D);
            };
        }
    }
    return;
} /** End of generateSamples() **/

/*****************************************************************************\
 @ closestSample
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
int closestSample(mtVec3 point, Sample *samples, int numSamples)
{
    int closest = -1;
    mtReal min_sq_dist = MAXFLOAT;
    for (int i=0; i<numSamples; i++)
    {
        mtVec3 vec = point - samples[i].position);
    mtReal sq_dist = vec.dot(vec);
    if (sq_dist < min_sq_dist)
    {
        closest = i;
        min_sq_dist = sq_dist;
    }
}

if (closest == -1)
{
    fprintf(stderr, "Couldn't find closest sample.\n");
    exit(1);
}
    
return closest;
} /** End of closestSample() **/

/*****************************************************************************\
 @ 
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
()
{
} /** End of () **/

#ifdef TEST_SAMPLE
int main()
{
    mtVec3 vert1(0,0,0);
    mtVec3 vert2(1,0,0);
    mtVec3 vert3(1,1,0);

    mtVec3 **samples;
    int numSamples;

    generateSamples(vert1, vert2, vert3, 0.1, &samples, &numSamples);
    for (int i=0; i<numSamples; i++)
    {
        samples[i]->print();
        fprintf(stderr, "\n");
    }
    return 0;
}
#endif

/*****************************************************************************\
  $Log: Sample.C,v $
  Revision 1.5  2004/07/08 16:47:49  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.4  2003/07/26 01:17:44  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.3  2003/07/23 19:55:35  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.2  2003/06/05 17:39:02  gfx_friends
  Patches to build on Win32.

  Revision 1.1  2003/01/13 20:30:16  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/

