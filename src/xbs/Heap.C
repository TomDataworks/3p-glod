/*****************************************************************************\
  Heap.C
  --
  Description :

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/Heap.C,v $
  $Revision: 1.9 $
  $Date: 2004/07/08 16:44:41 $
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

#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(__APPLE__)
#include <float.h>
#else
#include <values.h>
#endif

#include <Heap.h>
#include <math.h>

/*----------------------------- Local Constants -----------------------------*/

#define TRUE 1
#define FALSE 0

/*------------------------------ Local Macros -------------------------------*/

#define ARRAY(i)  ((i)-1)  /* the pseudo-code in Cormen assumes arrays go
                              from 1 to n, rather than 0 to n-1 */
#define PARENT(i) ((i)/2)
#define LEFT(i)   ((i)*2)
#define RIGHT(i)  ((i)*2 + 1)

/*------------------------------- Local Types -------------------------------*/


/*------------------------ Local Function Prototypes ------------------------*/


/*------------------------------ Local Globals ------------------------------*/


/*---------------------------------Functions-------------------------------- */
#ifdef _WIN32
#define finite _finite
#endif
#ifdef __APPLE__
#define finite isfinite
#endif

void
Heap::insert(HeapElement *element)
{
    int i;

    if (!finite(element->key()))
    {
        fprintf(stderr, "Heap::insert(): key must be finite!\n");
        exit(1);
    }

    if (element->key() == -MAXFLOAT)
    {
        fprintf(stderr, "Heap::insert(): key must be > -MAXFLOAT\n");
        exit(1);
    }
    
    while (_size >= maxSize)
    {
        int j;
        HeapElement **newArray = new HeapElement *[maxSize*2];
        for (j=0; j<_size; j++)
            newArray[j] = array[j];
        delete array;
        array = newArray;
        maxSize = maxSize * 2;
    }
    
    _size++;
    i = _size;
    while ((i>1) && (array[ARRAY(PARENT(i))]->key() > element->key()))
    {
        array[ARRAY(i)] = array[ARRAY(PARENT(i))];
        array[ARRAY(i)]->index = i;
        i = PARENT(i);
    }
    array[ARRAY(i)] = element;
    element->index = i;
    element->_heap = this;
}

void
Heap::remove(HeapElement *element)
{
    HeapElement * min;
    
    changeKey(element, -MAXFLOAT);
    min = extractMin();
    if (min != element)
    {
        fprintf(stderr, "Heap::remove(): removed wrong element!!\n");
        exit(1);
    }
    return;
}

void
Heap::test()
{
    int i;

    for (i=1; i<=_size; i++)
        if (array[ARRAY(i)]->index != i)
        {
            fprintf(stderr, "Heap::test(): Heap element index invalid.\n");
            exit(1);
        }
    fprintf(stderr, "Heap::test(): Heap element indices OK.\n");

    /* test heap property */
    for (i=1; i<=_size; i++)
    {
        if (LEFT(i) <= _size)
            if (array[ARRAY(i)]->key() >
                array[ARRAY(LEFT(i))]->key())
            {
                fprintf(stderr, "Heap::test(): Heap property violated.\n");
                exit(1);
            }
        if (RIGHT(i) <= _size)
            if (array[ARRAY(i)]->key() >
                array[ARRAY(RIGHT(i))]->key())
            {
                fprintf(stderr, "Heap::test(): Heap property violated.\n");
                exit(1);
            }    
    }
    fprintf(stderr, "Heap::test(): Heap property OK.\n");

    fprintf(stderr, "\n");

    return;
}

void
Heap::print()
{
    int i, level, levelstart;

    fprintf(stderr, "Heap size: %d\n", _size);
    for (i=1, level=0, levelstart=1; i<=_size; i++)
    {
        if (i == levelstart)
        {
            fprintf(stderr, "-----LEVEL %d-----\n", level);
            levelstart *= 2;
            level++;
        }
        fprintf(stderr, "Node: %g", array[ARRAY(i)]->key());
        if (LEFT(i) <= _size)
            fprintf(stderr,
                    "     Left: %g", array[ARRAY(LEFT(i))]->key());
        if (RIGHT(i) <= _size)
            fprintf(stderr,
                    "     Right: %g", array[ARRAY(RIGHT(i))]->key());
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}


void
Heap::heapify(int index)
{
    int       left, right, smallest;
    HeapElement *temp;
    
    while (1) {
        left = LEFT(index);
        right = RIGHT(index);

        smallest = index;
    
        if ((left <= _size) &&
            (array[ARRAY(left)]->key() <
             array[ARRAY(smallest)]->key()))
            smallest = left;

        if ((right <= _size) &&
            (array[ARRAY(right)]->key() <
             array[ARRAY(smallest)]->key()))
            smallest = right;

        if (smallest != index)
        {
            /* swap smallest and index */
            temp = array[ARRAY(index)];
            array[ARRAY(index)] = array[ARRAY(smallest)];
            array[ARRAY(smallest)] = temp;

            array[ARRAY(index)]->index = index;
            array[ARRAY(smallest)]->index = smallest;

            index = smallest;
        }
        else
            break;
    }
}


HeapElement *
Heap::min()
{
    HeapElement *min;
    
    if (_size < 1)
        return NULL;

    min = array[ARRAY(1)];

    return min;
}

HeapElement *
Heap::extractMin()
{
    HeapElement *min;
    
    if (_size < 1)
        return NULL;
    
    min = array[ARRAY(1)];
    min->index = -1;
    
    array[ARRAY(1)] = array[ARRAY(_size)];
    array[ARRAY(1)]->index = 1;
    
    _size--;
    
    heapify(1);
    min->_heap = NULL;
    return min;
}

void
Heap::changeKey(HeapElement *element, float key)
{
    int i;

    if (!finite(key))
    {
        fprintf(stderr, "Heap::changeKey(): new key must be finite!\n");
        exit(1);
    }

    if ((element->heap() != NULL) && (element->heap() != this))
    {
        fprintf(stderr, "Trying to change key of element in wrong heap!\n");
        exit(1);
    }
    
    if (element->heap() == NULL)
        insert(element);
    
    if (key == element->key())
        return;

    if (key > element->key())
    {
        // special case where we have to change the key of an active
        // HeapElement
        element->_key = key;
        heapify(element->index);
        return;
    }

    /* key < element->key() */
    i = element->index;
    // special case where we have to change the key of an active
    // HeapElement
    element->_key = key;
    while ((i>1) && (array[ARRAY(PARENT(i))]->key() > element->key()))
    {
        array[ARRAY(i)] = array[ARRAY(PARENT(i))];
        array[ARRAY(i)]->index = i;
        i = PARENT(i);
    }
    array[ARRAY(i)] = element;
    element->index = i;
    
    return;
}

#if 0
void test_heap()
{
    int   i;
    Heap *heap;
    Face *face;
    int   testsize;

    testsize = MIN(nfaces, 10);
    
    for (i=0; i<testsize; i++)
    {
        face = &(flist[i]);
        face->heapinfo.key = i;
        face->heapinfo.user_data = (void *)face;
    }

    heap_init(&heap, testsize);

    for (i=testsize-1; i>=0; i--)
        heap_insert(heap, &(flist[i].heapinfo));

    fprintf(stderr, "Heapsize: %d\n", heap->size);
    heap_test(heap);

    heap_print(heap);
    
    fprintf(stderr, "Testing change key\n");
    for (i=0; i<testsize; i++)
    {
        fprintf(stderr, "%s\n",
                (((testsize-i) < flist[i].heapinfo.key) ?
                 "Decreasing key" :
                 (((testsize-i) > flist[i].heapinfo.key) ?
                  "Increasing key" :
                  "Not changing key")));
        
        heap_change_key(heap, &(flist[i].heapinfo), testsize-i);
        heap_print(heap);
        heap_test(heap);
    }
    
    for (i=0; i<testsize; i++)
    {
        heap_extract_min(heap);
        fprintf(stderr, "Heapsize: %d\n", heap->size);
        heap_test(heap);
    }
}
#endif

/*****************************************************************************\
  $Log: Heap.C,v $
  Revision 1.9  2004/07/08 16:44:41  gfx_friends
  Removed tabs and did 4-space indentation on source files in xbs directory.

  Revision 1.8  2004/06/16 20:30:35  gfx_friends
  values.h include change for osx

  Revision 1.7  2004/06/11 18:30:07  gfx_friends
  Remove all sources of warnings in xbs directory when compiled with -Wall

  Revision 1.6  2004/06/10 16:08:50  gfx_friends
  Converted heapify to non-recursive form for a meager speedup.

  Revision 1.5  2004/02/04 17:15:01  gfx_friends
  Adding apple makefiles, code changes, which _hopefully_ won't break anything else...

  Revision 1.4  2003/07/26 01:17:42  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.3  2003/07/23 19:55:33  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.2  2003/06/05 17:38:58  gfx_friends
  Patches to build on Win32.

  Revision 1.1  2003/01/13 20:30:14  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.2  2003/01/05 22:37:57  cohen
  Added heap size parameter to initializer.
  Added convenience functions to determine if element is in a heap.
  Removed unnecessary space allocation macros.

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

  Revision 1.1  2000/01/07 23:09:32  cohen
  Initial revision

\*****************************************************************************/

