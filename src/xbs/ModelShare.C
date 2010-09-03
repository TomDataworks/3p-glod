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

/* This is an expansion of code by Greg Turk, with copyright as follows: */

/*

Turn a PLY object with un-shared vertices into one with faces that
share their vertices.

Greg Turk, August 1994

---------------------------------------------------------------

Copyright (c) 1994 The Board of Trustees of The Leland Stanford
Junior University.  All rights reserved.   
  
Permission to use, copy, modify and distribute this software and its   
documentation for any purpose is hereby granted without fee, provided   
that the above copyright notice and this permission notice appear in   
all copies of this software and that you do not sell the software.   
  
THE SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,   
EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY   
WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.   

*/

#include <stdio.h>
#include <math.h>
#include <Model.h>


#if 0
#define VERBOSE
#endif


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
        xbsVertex *vert;
        struct Vertex *shared;
        struct Vertex *next;
} Vertex;


/*** the PLY object ***/

static int nverts;
static Vertex **vlist;

static float tolerance = 0.0;   /* what constitutes "near" */

/* hash table for near neighbor searches */

#define HASH_FILL_RATIO (0.9)
#define PRIME_16BIT 53003
#define PRIME_7BIT  101

typedef struct Hash_Table {     /* uniform spatial subdivision, with hash */
        int npoints;                  /* number of points placed in table */
        Vertex **verts;               /* array of hash cells */
        int num_entries;              /* number of array elements in verts */
        double scale;                 /* 1 / size of cell */
} Hash_Table;


static Hash_Table *init_table(int, float);
static void add_to_hash (Vertex *, Hash_Table *, float);

#ifndef _WIN32
#define TRUE  (1)
#define FALSE (0)
#endif

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

#ifdef _WIN32
#define for if(0); else for
#endif

#define X 0
#define Y 1
#define Z 2

void
Model::share(float coord_tolerance)
{
    tolerance = coord_tolerance;

    nverts = numVerts;
    vlist = new Vertex *[nverts];
    for (int i=0; i<nverts; i++)
    {
        vlist[i] = new Vertex;
        vlist[i]->vert = verts[i];
        vlist[i]->shared = NULL;
        vlist[i]->next = NULL;
        vlist[i]->vert->nextCoincident = vlist[i]->vert;
    }
    
    share_vertices();

    for (int i=0; i<nverts; i++)
    {
        delete vlist[i];
        vlist[i] = NULL;
    }
    delete vlist;
    vlist = NULL;
    
    
    return;
}





void
Model::matchAttributes()
{
    // So far we have used the shared field to mark all geometrically
    // "close" vertices as shared. Now we will unshare the ones with
    // attributes that are too different, but keep them marked as
    // geometrically coincident in the Model.
    
    // first build coincident vert circularly linked lists
    for (int vnum=0; vnum<numVerts; vnum++)
    {
        Vertex *vert = vlist[vnum];
        xbsVertex *xbsVert = vert->vert;

        xbsVertex *next = vert->shared->vert->nextCoincident;
        vert->shared->vert->nextCoincident = xbsVert;
        xbsVert->nextCoincident = next;
    }

    // now adjust sharing of each vertex
    for (int vnum=0; vnum<numVerts; vnum++)
    {
        Vertex *vert = vlist[vnum];
        xbsVertex *xbsVert = vert->vert;
        Vertex *shared = vert->shared;
        
        if (vert == shared)
            continue;

        Vertex *current;
        int firstTime, found;
        for (current = shared, firstTime=1, found=0;
             ((firstTime == 1) || (current != shared));
             current = vlist[current->vert->nextCoincident->index])
        {
            if (current->shared != current)
                continue;

#if 0
            // I think this would work for splitting up multipatch
            // vertices, except the sharing is actually performed before
            // storing the triangle indices with the vertices
            if ((xbsVert->attribsEqual(current->vert)) &&
                ((xbsVert->numTris == 0) || (current->vert->numTris == 0) ||
                 (xbsVert->tris[0]->patchNum == current->vert->tris[0]->patchNum)))
#else
                if (xbsVert->attribsEqual(current->vert))
#endif
                {
                    found = 1;
                    vert->shared = current;
                    break;
                }
            
            if (firstTime == 1)
                firstTime = 0;
        }

        if (found == 0)
            vert->shared = vert; // no other vertex matches, so keep this one
    }

    // Now that we know which vertices will really be removed, remove them
    // from the nextCoincident rings
    for (int vnum=0; vnum<numVerts; vnum++)
    {
        Vertex *vert = vlist[vnum];

        if (vert->shared == vert)
            continue;

        // this vert will be removed, so remove from ring
        xbsVertex *current = vert->vert;
        while (current->nextCoincident != vert->vert)
            current = current->nextCoincident;
        current->nextCoincident = vert->vert->nextCoincident;
    }
}

/******************************************************************************
Figure out which vertices are close enough to share.
******************************************************************************/

void count_collisions(Hash_Table *table)
{
    const int countSize = 10;
    int numVerts[countSize];

    for (int i=0; i<countSize; i++)
        numVerts[i] = 0;
    
    for (int i=0; i<table->num_entries; i++)
    {
        int count;
        Vertex *vert;
        for (count=0, vert = table->verts[i];
             vert != NULL;
             count++, vert = vert->next);
        count = (count < countSize) ? count : countSize-1;
        numVerts[count]++;
    }

    for (int i=0; i<countSize; i++)
        fprintf(stderr, "Buckets of size %d: %f%%\n", i,
                (float)numVerts[i]/(float)table->num_entries * 100);
}

void
Model::share_vertices()
{
    int i, j;
    Hash_Table *table;
    float squared_dist;

    table = init_table (nverts, tolerance);

    squared_dist = tolerance * tolerance;

    /* place all vertices in the hash table, and in the process */
    /* learn which ones should be shared */

    for (i = 0; i < nverts; i++)
        add_to_hash (vlist[i], table, squared_dist);

#if 0
    count_collisions(table);
#endif

    free(table->verts);
    table->verts = NULL;
    free(table);
    table = NULL;

#ifdef VERBOSE
    int count=0;
    for (i=0; i<nverts; i++)
        if (vlist[i]->shared == vlist[i])
            count++;
    fprintf(stderr, "verts after geom sharing: %d\n", count);
#endif
  
    matchAttributes();

#ifdef VERBOSE
    count=0;
    for (i=0; i<nverts; i++)
        if (vlist[i]->shared == vlist[i])
            count++;
    fprintf(stderr, "verts after attribute splitting: %d\n", count);
#endif
  
    /* fix up the faces to point to the shared vertices */

    for (i = 0; i < numTris; i++)
    {
        xbsTriangle *tri = tris[i];

        /* change vertices to their shared representatives */
        for (j=0; j<3; j++)
            tri->verts[j] =
                vlist[tri->verts[j]->index]->shared->vert;

        // check to make sure no two vertices lie on the same coincident
        // vertices ring
        if ((tri->verts[0]->minCoincident() == tri->verts[1]->minCoincident()) ||
            (tri->verts[0]->minCoincident() == tri->verts[2]->minCoincident()) ||
            (tri->verts[1]->minCoincident() == tri->verts[2]->minCoincident()))
        {
            // removeTri decrements numTris and replace the deleted tri with
            // the last tri in the list!
            removeTri(tri);
            delete tri;
            i--;
        }
    }
  
    // remove the shared vertices
    for (i=0; i<nverts; i++)
    {
        Vertex *vert = vlist[i];

        if (vert->shared != vert)
        {
            // removeVert decrements numVerts and replaces the deleted vertex
            // with the last vertex!
            removeVert(vert->vert);
            delete vert->vert;
            delete vert;
            vlist[i] = vlist[nverts-1];
            nverts--;
            i--;
        }
    }


  
    // This is debatable, but make the geometry of all coincident vertices
    // exactly the same. If we ever decide to allow coincident vertices where
    // the geometry is not equal, we may need to re-think the error metric
    // for GLOD, which just looks at the geometry of the minCoincident vertex
    // at the moment. Changing the minCoincident arbitrarily should not
    // change the error metric.

    for (i=0; i<nverts; i++)
    {
        xbsVertex *vert = getVert(i);
        xbsVertex *min = vert->minCoincident();
        if (vert != min)
            continue;
        vert = vert->nextCoincident;
        for (vert = min->nextCoincident; vert != min; vert = vert->nextCoincident)
            vert->coord = min->coord;
    }
  
#ifdef VERBOSE
    fprintf(stderr, "verts in resulting Model: %d\n", numVerts);

    count=0;
    for (int vnum=0; vnum<numVerts; vnum++)
        if (verts[vnum]->nextCoincident != verts[vnum])
            count++;
    fprintf(stderr, "multi-attribute verts: %d\n", count);

    int numRings = 0;
    for (int vnum=0; vnum<numVerts; vnum++)
    {
        xbsVertex *vert = verts[vnum];
        if (vert->nextCoincident != vert)
        {
            int minIndex = vert->index;
            xbsVertex *next = vert->nextCoincident;
            while (next != vert)
            {
                if (next->index < minIndex)
                    minIndex = next->index;
                next = next->nextCoincident;
            }
            if (minIndex == vert->index)
                numRings++;
        }
    }
    fprintf(stderr, "border vert equivalent: %d\n", numRings);
#endif
  
}


/******************************************************************************
Add a vertex to it's hash table.

Entry:
  vert    - vertex to add
  table   - hash table to add to
  sq_dist - squared value of distance tolerance
******************************************************************************/

static void add_to_hash(Vertex *vert, Hash_Table *table, float sq_dist)
{
    int index;
    unsigned long a,b,c;
    unsigned long aa,bb,cc;
    double scale;
    Vertex *ptr;
    float dx,dy,dz;
    float sq;
    float min_dist;
    Vertex *min_ptr;

    /* determine which cell the position lies within */

    scale = table->scale;

    aa = (((int)(floor(fmod(vert->vert->coord[X] * scale, PRIME_16BIT))))
          + PRIME_16BIT) % PRIME_16BIT;
    bb = (((int)(floor(fmod(vert->vert->coord[Y] * scale, PRIME_16BIT))))
          + PRIME_16BIT) % PRIME_16BIT;
    cc = (((int)(floor(fmod(vert->vert->coord[Z] * scale, PRIME_16BIT))))
          + PRIME_16BIT) % PRIME_16BIT;
  
    /* examine vertices in table to see if we're very close */

    min_dist = 1e20f;
    min_ptr = NULL;

    /* look at 27 cells, centered at cell containing location */

#if 0
    for (a = (aa-1+PRIME_16BIT)%PRIME_16BIT;
         a != (aa+2)%PRIME_16BIT;
         a = (a+1)%PRIME_16BIT)
    {
        for (b = (bb-1+PRIME_16BIT)%PRIME_16BIT;
             b != (bb+2)%PRIME_16BIT;
             b = (b+1)%PRIME_16BIT)
        {
            for (c = (cc-1+PRIME_16BIT)%PRIME_16BIT;
                 c != (cc+2)%PRIME_16BIT;
                 c = (c+1)%PRIME_16BIT)
            {
#else
    unsigned long a_array[3];
    unsigned long b_array[3];
    unsigned long c_array[3];
    a_array[0] = (aa-1+PRIME_16BIT)%PRIME_16BIT;
    a_array[1] = aa;
    a_array[2] = (a_array[1]+1)%PRIME_16BIT;
    b_array[0] = (bb-1+PRIME_16BIT)%PRIME_16BIT;
    b_array[1] = bb;
    b_array[2] = (b_array[1]+1)%PRIME_16BIT;
    c_array[0] = (cc-1+PRIME_16BIT)%PRIME_16BIT;
    c_array[1] = cc;
    c_array[2] = (c_array[1]+1)%PRIME_16BIT;
    int i,j,k;
    for (i=0; i<3; i++)
    {
        a = a_array[i];
        for (j=0; j<3; j++)
        {
            b = b_array[j];
            for (k=0; k<3; k++)
            {
                c = c_array[k];
#endif
              
                /* compute position in hash table */
                index =
                    ((a * PRIME_7BIT) + b) * PRIME_7BIT + c;
                index = index % table->num_entries;
              
              
                /* examine all points hashed to this cell */
                for (ptr = table->verts[index]; ptr != NULL; ptr = ptr->next) {
                  
                    /* distance (squared) to this point */
                    dx = ptr->vert->coord[X] - vert->vert->coord[X];
                    dy = ptr->vert->coord[Y] - vert->vert->coord[Y];
                    dz = ptr->vert->coord[Z] - vert->vert->coord[Z];
                    sq = dx*dx + dy*dy + dz*dz;
                  
                    /* maybe we've found new closest point */
                    if ((sq <= min_dist) && (sq <= sq_dist))
                    {
                        min_dist = sq;
                        min_ptr = ptr;
                    }
                }
            }
        }
    }
  
    /* If we found a match, have new vertex point to the matching vertex. */
    /* If we didn't find a match, add new vertex to the table. */

    if (min_ptr)
        vert->shared = min_ptr;
    else {          /* no match */
        index = ((aa * PRIME_7BIT) + bb) * PRIME_7BIT + cc;
        index = index % table->num_entries;
      
        vert->next = table->verts[index];
        table->verts[index] = vert;
        vert->shared = vert;  /* self-reference as close match */
    }
}


/******************************************************************************
Initialize a uniform spatial subdivision table.  This structure divides
3-space into cubical cells and deposits points into their appropriate
cells.  It uses hashing to make the table a one-dimensional array.

Entry:
  nverts - number of vertices that will be placed in the table
  size   - size of a cell

Exit:
  returns pointer to hash table
******************************************************************************/

static Hash_Table *init_table(int nverts, float size)
{
    int i;
    Hash_Table *table;

    /* allocate new hash table */
    table = (Hash_Table *) malloc (sizeof (Hash_Table));
    table->num_entries = (int)(nverts / HASH_FILL_RATIO);
    table->verts = (Vertex **) malloc (sizeof (Vertex *) * table->num_entries);

    /* set all table elements to NULL */
    for (i = 0; i < table->num_entries; i++)
        table->verts[i] = NULL;

    size *= (float) 1.01; /* expand cell a bit to be conservative */
    size = MAX(size, 1E-10);
    table->scale = 1.0 / size;
    return (table);
}




