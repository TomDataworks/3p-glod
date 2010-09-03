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
#include <stdlib.h>
#include "mt.h"
#ifdef _WIN32
#define _WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

// used for special coloring options
#define NEW_FRAME_COUNT    10

// histogram of points of different screen-space sizes
static int pointsPerSize[MAX_POINT_SIZE];

// some variables from the cut that I don't want to bother passing to all
// the low-level rendering routines
static mtCut emptyCut;
static char gDrawMode;
static gStat *gCutStat = &emptyCut.stat;

/*****************************************************************************

  Low-level triangle and point rendering methods

*****************************************************************************/
void mtVertex::draw()
{
    glVertex3fv(this->coord.data);
}

void mtCVertex::draw()
{
    if (!gDrawMode) glColor3ubv(this->color.data);
    glVertex3fv(this->coord.data);
}

void mtNVertex::draw()
{
    glNormal3fv(this->normal.data);
    glVertex3fv(this->coord.data);
}

void mtTVertex::draw()
{
    glTexCoord2fv(this->texcoord.data);
    glVertex3fv(this->coord.data);
}

void mtCNVertex::draw()
{
    if (!gDrawMode) glColor3ubv(this->color.data);
    glNormal3fv(this->normal.data);
}

void mtCTVertex::draw()
{
    if (!gDrawMode) glColor3ubv(this->color.data);
    glTexCoord2fv(this->texcoord.data);
}

void mtNTVertex::draw()
{
    glNormal3fv(this->normal.data);
    glTexCoord2fv(this->texcoord.data);
}

void mtCNTVertex::draw()
{
    if (!gDrawMode) glColor3ubv(this->color.data);
    glNormal3fv(this->normal.data);
    glTexCoord2fv(this->texcoord.data);
}

void mtTriangle::draw(MT *mt)
{
   gCutStat->gNumTris ++;

   glNormal3fv(normal.data);
   mt->getVert(verts[0])->draw();
   mt->getVert(verts[1])->draw();
   mt->getVert(verts[2])->draw();
}

void mtPoint::draw(mtView *view)
{
    mtReal pointSize = 1.0;
    
    gCutStat->gNumPoints ++;

    if (view != NULL)
    {
        // project object-space radius to screen-space radius (size)
        mtVec3 v = sample->coord - view->eye;
        mtReal d = v.dot(view->viewd);
        d = view->zp / d;
        pointSize = d * 2*radius;
        int ipsize = (int)(pointSize+0.5);
        pointsPerSize[ipsize]++;
    }
    glDisable(GL_LIGHTING);
    //glPointSize(pointSize);
    glPointSize((float)(pointSize+0.5));
    glBegin(GL_POINTS);
      sample->draw();
    glEnd();
    glEnable(GL_LIGHTING);
}


/*****************************************************************************

  This is the main rendering routine. It renders all the tris in the arc
  and/or all the points in the arc.

*****************************************************************************/
void mtArc::drawReal(MT *mt, mtView *view)
{
   int *ltris = tris;
   int  lnumTris=numTris;
#ifndef OPTIMIZERENDER
   glBegin(GL_TRIANGLES);
#endif
   while(lnumTris--)
   {
      mt->getTri(*ltris)->draw(mt); ltris++;
   }
#ifndef OPTIMIZERENDER
   glEnd();

   int *lpoints = points;
   int  lnumPoints = numPoints;
   while (lnumPoints--)
   {    
      mt->getPoint(*lpoints)->draw(view); lpoints++;
   }
#endif
}


/*****************************************************************************

  For this arc, setup any special coloring options (gArcRandom) and then
  call either the display lists or drawReal().

*****************************************************************************/
void mtArc::draw(MT *mt, mtCut *cut, float d)
{ 
#ifdef OPTIMIZERENDER

   // for optimized rendering, ignore gArcRandom variable
   glColor3ub(0, 0, 255);

   
   cut->stat.gNewArcStat = -1;
   cut->stat.gArcStat ++;

   if (mt->getRetainedMode())
   {
       cut->stat.gNumTris += numTris;
       glCallList(getdlistID(mt));
   }
   else
   {
       int *ltris = tris;
       int  lnumTris = numTris;
       mtTriangle *tri;
       while(lnumTris--)
       {
	   tri = mt->getTri(*ltris);
	   glNormal3fv(tri->normal.data);
	   mt->getVert(tri->verts[0])->draw();
	   mt->getVert(tri->verts[1])->draw();
	   mt->getVert(tri->verts[2])->draw();
	   
	   ltris++;
	   cut->stat.gNumTris ++;
       }
   }
   
   int *lpoints = points;
   int  lnumPoints = numPoints;
   while (lnumPoints--)
   {    
      mtPoint *pt = mt->getPoint(*lpoints);
      mt->cachePoint(pt, (int)((d*2.0*pt->radius) + 0.5));

      lpoints++;
      cut->stat.gNumPoints ++;
   }

#else

   int thisframeno = cut->getView()->frameno;

   // Assign color. If gArcRandom == 1, then if the arc has
   // been rendered less than NEW_FRAME_COUNT frames, it will be
   // rendered in red, otherwise in green. If gArcRandom == 2,
   // a random color will be assigned. If gArcRandom == 0, blue will
   // be used.
   if(frameno < thisframeno-1)
   {
      // First time for this arc. Reset Keep count.
      keepcolor = 0; 
      cut->stat.gNewArcStat ++;
   }
   cut->stat.gArcStat ++;

   keepcolor ++;
   frameno = thisframeno;

   if (gDrawMode) // If 0 don't bother with colors here.
   {
      if (gDrawMode == 2) {
         glColor3ub(rand()&255, rand()&255, rand()&255);
      } else {
         if (keepcolor < NEW_FRAME_COUNT) 
            glColor3ub(200,0,0); // Use new color
         else
            glColor3ub(0,200,0); // Use regular color
      }
   } else {
      glColor3ub(0, 0, 255);
   }

   if (mt->getRetainedMode())
   {
       cut->stat.gNumPoints += numPoints;
       cut->stat.gNumTris += numTris;
       glCallList(getdlistID(mt));
   }
   
   else
      drawReal(mt, cut->getView());
#endif
}


/*****************************************************************************

  For this cut of the given TM, draw all arcs.

*****************************************************************************/
void mtCut::draw(MT *mt)
{ 
   // We know arcs tris do not get changes from underneath us
   int *larcs = arcs;
   int i = 0, lnumArcs = numArcs;

   stat.start.getTime();

   stat.gNumTris = 0;
   stat.gNumPoints = 0;
   stat.gArcStat = 0;
   stat.gNewArcStat = 0;
   stat.gStripStat = 0;
   stat.gVertStat = 0;
   
   gDrawMode = renderMode;
   gCutStat = &stat;
   
#ifdef OPTIMIZERENDER
   // first render only tris and cache points
   glBegin(GL_TRIANGLES);
#endif
   while(lnumArcs--)
   {
      mt->getArc(*larcs)->draw(mt, this, depths[i++]); larcs++;
   }
#ifdef OPTIMIZERENDER
   glEnd();
   stat.gTimeDrawTris = stat.start.since();
#else
   stat.gTimeDrawTris = stat.start.since();
   stat.gTimeDrawPts   = stat.gTimeDrawTris;
#endif

   if (dumpMode) {
      printf("Drawn a cut with %d triangles and %d points\n", 
             stat.gNumTris, stat.gNumPoints);
      for (i=0; i<MAX_POINT_SIZE; i++) {
         printf("points of size %i = %i\n", i, pointsPerSize[i]);
         pointsPerSize[i] = 0;
      }
   }
}


/*****************************************************************************
*****************************************************************************/
void mtTriangle::print(MT *mt)
{
    printf("Triangle compriding vert nos. %d, %d and %d:\n  ",
           verts[0], verts[1], verts[3]);
    mt->getVert(verts[0])->print();
    mt->getVert(verts[1])->print();
    mt->getVert(verts[2])->print();
    printf("\n");
};


/*****************************************************************************
*****************************************************************************/
void MT::buildDL()
{
   unsigned int list0;
   if (!(list0 = glGenLists(numArcs)))
   {
      printf("Display list creation failed numArcs=%d\n", numArcs);
      exit(1);
   }
   dlistBase = list0;

   // it would be nice to free up the triangles and vertices here as they
   // are retained, but their current allocation would only allow freeing
   // all at once (when it's really too late anyway)
   
   for(int i=0; i<numArcs; i++,list0++)
   {
      glNewList(list0, GL_COMPILE);
      arcs[i].drawReal(this);
      glEndList();
   }
}


/*****************************************************************************
*****************************************************************************/
void MT::flushPointCaches()
{
	memset(numPointsInCache, 0, MAX_POINT_SIZE*sizeof(int));
}


/*****************************************************************************
*****************************************************************************/
void MT::cachePoint(mtPoint *pt, int pointSize)
{
   if (pointSize < 0 || pointSize >= MAX_POINT_SIZE) {
      fprintf(stderr, "Unsupported point size for cache (%i)\n", pointSize);
      return;
   }

   if (numPointsInCache[pointSize] == maxPointsInCache[pointSize]) {
      mtPoint **oldcache = pointCache[pointSize];
      maxPointsInCache[pointSize] *= 2;
      pointCache[pointSize] = new mtPoint *[maxPointsInCache[pointSize]];
      memcpy(pointCache[pointSize], oldcache, 
             sizeof(mtPoint *)*numPointsInCache[pointSize]);
      delete oldcache;
   }

   pointsPerSize[pointSize]++;
   pointCache[pointSize][numPointsInCache[pointSize]] = pt;
   numPointsInCache[pointSize]++;
}


/*****************************************************************************
*****************************************************************************/
void MT::drawPointCaches()
{
   int s, i;

   glDisable(GL_LIGHTING);
   for (s=0; s<MAX_POINT_SIZE; s++) {
      if (numPointsInCache[s] > 0) {
         glPointSize((float)s);
         glBegin(GL_POINTS);
         for (i=0; i<numPointsInCache[s]; i++)
            pointCache[s][i]->sample->draw();
         glEnd();
      }
   }
   glEnable(GL_LIGHTING);
}


