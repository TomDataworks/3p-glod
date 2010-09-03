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
#include "mt.h"
#ifdef MINIMALSPHERE

#include <minsphr.h>

inline void makeaPT(Point3 *pts, mtReal *vec)
{
   pts->x = vec[0]; pts->y = vec[1]; pts->z = vec[2];
}

#else

inline mtReal dist(mtVec3 v1, mtVec3 v2)
{
   mtVec3 diff = v1-v2;
   return diff.dot(diff);
}

inline void updatebox(mtVec3 min, mtVec3 max, mtVec3 data)
{
   if(min[0] > data[0]) min[0] = data[0];
   if(min[1] > data[1]) min[1] = data[1];
   if(min[2] > data[2]) min[2] = data[2];
   if(max[0] < data[0]) max[0] = data[0];
   if(max[1] < data[1]) max[1] = data[1];
   if(max[2] < data[2]) max[2] = data[2];
}

mtReal mySphere(int npts, mtVec3 *pts, mtVec3 *center)
{
   mtVec3 av;
   mtVec3 min;
   mtVec3 max;
   mtReal r1, r2, r;

   av.set(0, 0, 0);
   max.set(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
   min.set(MAXFLOAT, MAXFLOAT, MAXFLOAT);
   for(int i=0; i<npts; i++)
   {
      av += pts[i];
      updatebox(min, max, pts[i]);
   }

   r1 = 0;
   av *= (float)1.0/npts;

   for(int i=0; i<npts; i++)
   {
      r = dist(pts[i], av);
      if(r > r1) r1 = r;
   }

   r2 = 0;
   min += max;
   min *= 0.5;
   for(int i=0; i<npts; i++)
   {
      r = dist(pts[i], min);
      if(r > r2) r2 = r;
   }

   if(r1 < r2) { *center = av; return r1; }
   else	       { *center = min; return r2;}

}

inline void makeaPT(mtVec3 *pts, mtReal *vec)
{
   pts->set(vec[0], vec[1], vec[2]);
}
#endif /* MINMALSPHERE */

void mtArc::computeSPH(MT *mt)
{
   int lnumTri = numTris;
   int *ltris = tris;
   int *vertIds;
#ifdef MINMALSPHERE
   Point3 *pts = new Point3[3*numTris + numPoints];
   Point3 *lpts = pts;
#else
   mtVec3 *pts = new mtVec3[3*numTris + numPoints];
   mtVec3 *lpts = pts;
#endif

   while(lnumTri--)
   { // We know tris will not change midway somewhere
      vertIds = mt->getTri(*ltris)->verts;
      makeaPT(lpts++, mt->getVert(vertIds[0])->coord.data);
      makeaPT(lpts++, mt->getVert(vertIds[1])->coord.data);
      makeaPT(lpts++, mt->getVert(vertIds[2])->coord.data);
      ltris++;
   }

   for(int ptnum=0; ptnum<numPoints; ptnum++)
   {
       makeaPT(lpts++, mt->getPoint(points[ptnum])->sample->coord.data);
   }
   
#ifndef SGI
#define fsqrt(a) sqrt(a)
#endif
   
#ifdef MINMALSPHERE
   for(int i=0; i<3*numTris+numPoints; i++)
   {
      printf("Point(%d) ", i); fflush(stdout);
      printf("verts: %f %f %f\n", pts[i].x,pts[i].y,pts[i].z);
      fflush(stdout);
   }
   Sphere S = MinimalSphere(numTris*3+numPoints, pts);
   center.set(S.center.x, S.center.y, S.center.z);

   radius = fsqrt(S.rsqr);
#else
   radius = (float) fsqrt(mySphere(numTris*3+numPoints, pts, &center));
#endif /* MINMALSPHERE */

   // printf("Radius = %f, Center = ", radius); center.print(); printf("\n");
   delete pts;
}
