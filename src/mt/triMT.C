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
#include <stdio.h>
#include "mt.h"
#include "tmesh.h"

/* test standalone
static int tris[12][3]={{0,1,2}, {0,2,3}, {4,5,6}, {4,6,7}, {0,1,5}, {0,5,4},
                        {1,2,6}, {1,6,5}, {2,3,7}, {2,7,6}, {3,0,4}, {3,4,7}};
*/

#define MAXSTRIPS	128
#define MAXVERTS	512
#define MAXARRAY	128

static int  Strip[MAXSTRIPS][MAXVERTS];
static int  Numstrips;
static int  Stripcount[MAXSTRIPS];
#ifdef VERTEXARRAY
static int  Arraycnt[MAXARRAY];
#endif

void CompressStrip(int strip[][MAXVERTS], int numstrips, int stripcount[],
                   int arraycnt[], int *numarraycnt)
{
   (*numarraycnt) = 0;
   for(int sno=0; sno<numstrips; sno++)
   {
      if((*numarraycnt) == MAXARRAY)
      {
         fprintf(stderr, "Need more than %d array elements\n", MAXARRAY);
	 exit(1);
      }
      arraycnt[*numarraycnt] = 2; // The first two are always added
      for(int i=2; i<stripcount[sno]; i++)
      {
	 if(strip[sno][i] == strip[sno][i-2])
         {
	    arraycnt[*numarraycnt] = -arraycnt[*numarraycnt];
            (*numarraycnt)++; arraycnt[*numarraycnt] = 1;
         }
         arraycnt[*numarraycnt] ++;
      }
      (*numarraycnt)++;
   }
}

void out_ambegin(int nverts, int ntris )
{
   // printf("Beginning output\n");
   Numstrips = 0;
}

#ifdef PRINTOUT
void out_amend( void )
{
int i;
int *strip;
   printf("Found %d Strips.\n", Numstrips);
   while(Numstrips--)
   {
      i = Stripcount[Numstrips];
      strip = Strip[Numstrips];
      printf("Strips %d: %d Vertices\n", Numstrips, i);
      while(i --)
         printf("%d ", *strip++);
      printf("\n");
   }
}
#else
void out_amend( void ) { } // silent
#endif

int out_amhashvert( unsigned int v )
{
   // printf("Hash.%d ", v);
   return v;
}

int out_amvertsame( unsigned int v1, unsigned int v2 )
{
   // printf("(%d=?%d)", v1, v2);
   // Could go to Vertex Array and check if they really are ..
   return v1 == v2;
}

void out_amvertdata( unsigned int fptr )
{
    // printf("Found vertex %d\n", fptr);
}

void out_ambgntmesh( void ) 
{
   Stripcount[Numstrips] = 0;
   // printf("\nBegin Strip. ");
}

void out_amendtmesh( void ) 
{
   Numstrips++;
   if(Numstrips == MAXSTRIPS)
      { fprintf(stderr, "Need to generate more than %d strips\n", MAXSTRIPS); exit(1); }
   // printf ("End Mesh\n");
}

void out_amswaptmesh( void ) 
{
   if(Stripcount[Numstrips] == MAXVERTS)
      { fprintf(stderr, "Need to generate more than %d Verts\n", MAXVERTS); exit(1); }
   Strip[Numstrips][Stripcount[Numstrips]] =
		Strip[Numstrips][Stripcount[Numstrips]-2];
   Stripcount[Numstrips]++;
   // printf(" SWAP ");
}

void out_amvert( unsigned int index )
{
   if(Stripcount[Numstrips] == MAXVERTS)
      { fprintf(stderr, "Need to generate more than %d Verts\n", MAXVERTS); exit(1); }
   Strip[Numstrips][Stripcount[Numstrips]] = index;
   Stripcount[Numstrips]++;
   // printf("(Vertex Id[%d] = %d) ", index, tris[index/3][index%3]);
}

#if 0

void makestrips(int ntris, int tris[][3])
{
   Meshobj * mesh =
   newMeshobj(out_ambegin, out_amend, out_amhashvert, out_amvertsame,
                     out_amvertdata, out_ambgntmesh, out_amendtmesh,
                     out_amswaptmesh, out_amvert);
   
   in_ambegin(mesh, ntris*3);
   for(int i=0; i<ntris; i++)
   {  // printf("ADD Tri %d = %d,%d,%d\n", i, tris[i][0],tris[i][1],tris[i][2]);
      in_amnewtri(mesh, i);
      in_amvert(mesh, tris[i][0]);
      in_amvert(mesh, tris[i][1]);
      in_amvert(mesh, tris[i][2]);
   }
   in_amend(mesh);
   freeMeshobj(mesh);
}

#endif

mtVec3 *makeIDnorm(MT *mt, mtVec3 *v1, int id1, int id2, int id3)
{
      *v1 = mt->getVert(id2)->coord - mt->getVert(id1)->coord;
      mtVec3 v2 = mt->getVert(id3)->coord - mt->getVert(id2)->coord;
      *v1 = ((*v1)^v2).normalize();
      return v1;
}

void mtArc::makeStrips(MT *mt)
{

   if (numTris == 0)
       return;
   
   Meshobj * mesh =
   newMeshobj(out_ambegin, out_amend, out_amhashvert, out_amvertsame,
                     out_amvertdata, out_ambgntmesh, out_amendtmesh,
                     out_amswaptmesh, out_amvert);
   in_ambegin(mesh, numTris*3);


   int *triIDs = new int [3*numTris];

   for(int i=0,idx=0; i<numTris; i++)
   { 
      in_amnewtri(mesh, i);
      triIDs[idx] = mt->getTri(tris[i])->verts[0];
      in_amvert(mesh, triIDs[idx++]);
      triIDs[idx] = mt->getTri(tris[i])->verts[1]; 
      in_amvert(mesh, triIDs[idx++]);
      triIDs[idx] = mt->getTri(tris[i])->verts[2]; 
      in_amvert(mesh, triIDs[idx++]);
   }

   Numstrips = 0;
   in_amend(mesh);
   freeMeshobj(mesh);

   mtVec3 norm; norm.set(0, 1, 0);
   // We have the strips in global Strips.

#ifdef VERTEXARRAY

   int numv = 0;
   for(int i=0; i<Numstrips; i++)
      numv += Stripcount[i];

   strip = new mtStrip(numv);

   for(int i=0; i<Numstrips; i++)
   {
      int id1 = 0; int id2 = 0; int id = 0;
      // printf("Initialized strip with %d\n", strips[i]->numVerts);
      for(int j=0; j<Stripcount[i]; j++)
      {
         id = triIDs[Strip[i][j]];
	 if(j > 1 && id != id2)
	 {
            if(j&1) // Odd
	       strip->addNormal2(makeIDnorm(mt, &norm, id, id1, id2));
	    else   // Even
	       strip->addNormal2(makeIDnorm(mt, &norm, id2, id1, id));
	 } else
            strip->addNormal2(&norm); // Whatever is in there. We don't care.
         strip->addVert(mt->getVert(id));
	 id2 = id1; id1 = id;
      }
   }
  

   CompressStrip(Strip, Numstrips, Stripcount, Arraycnt, &numStrips);
   stripLen = new (int) [numStrips];
   for(int i=0; i<numStrips; i++)
      {stripLen[i] = Arraycnt[i]; }
#else
   numStrips = Numstrips;
   strips = new mtStrip * [Numstrips];

   for(int i=0; i<Numstrips; i++)
   {
      strips[i] = new mtStrip (Stripcount[i]);
      int id1 = 0; int id2 = 0; int id = 0;
      // printf("Initialized strip with %d\n", strips[i]->numVerts);
      for(int j=0; j<Stripcount[i]; j++)
      {
         id = triIDs[Strip[i][j]];
	 if(j > 1 && id != id2)
	 {
	 /*if(id == id1)
           {
             fprintf(stderr, "Strip has %d: Consecutive ids found=%d @%d[%dth strip]\n",
                              Stripcount[i], id, j,i);
             for(int k=0; k<Stripcount[i]; k++)
		fprintf(stderr, "(%d->%d)", Strip[i][k], triIDs[Strip[i][k]]);
	     fprintf(stderr, "\n");
           }
	 */
            if(j&1) // Odd
	       strips[i]->addNormal2(makeIDnorm(mt, &norm, id, id1, id2));
	    else   // Even
	       strips[i]->addNormal2(makeIDnorm(mt, &norm, id2, id1, id));
	 } else
            strips[i]->addNormal2(&norm); // Whatever is in there. We don't care.
         strips[i]->addVert(mt->getVert(id));
	 id2 = id1; id1 = id;
      }
   }
#endif

  
   delete triIDs;
}
