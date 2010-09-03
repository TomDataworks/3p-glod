/*****************************************************************************\
  Heap.h
  --
  Description : 

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/Heap.h,v $
  $Revision: 1.13 $
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
#include <math.h>
#if defined(_WIN32) || defined(__APPLE__)
#include <float.h>
#define MAXFLOAT FLT_MAX
#undef min
#else 
#include <values.h>
#endif

/* Protection from multiple includes. */
#ifndef INCLUDED_HEAP_H
#define INCLUDED_HEAP_H


/*------------------ Includes Needed for Definitions Below ------------------*/


/*-------------------------------- Constants --------------------------------*/


/*--------------------------------- Macros ----------------------------------*/


/*---------------------------------- Types ----------------------------------*/


/*---------------------------- Function Prototypes --------------------------*/


/*--------------------------------- Classes ---------------------------------*/

class Heap;
class HeapElement
{
    private:
        void *_userData;
        float _key;

        // variables typically managed by Heap class
        Heap *_heap;
        int index;

    public:
        friend class Heap;
    
        HeapElement(void *userData, float key=MAXFLOAT)
        {
            _userData = userData;
            _key = key;
            _heap = NULL;
            index = -1;
        }
        ~HeapElement()
        {
            _userData = NULL;
            _key = -1;
            _heap = NULL;
            index = -1;
        }
    
        inline Heap *heap() {return _heap;};
        inline int inHeap() {return (_heap!=NULL);};
        inline int inHeap(Heap *heap) {return (_heap==heap);};
    
        inline float key() const {return _key;};
        inline void setKey(float key)
        {
            if (_heap != NULL)
            {
                fprintf(stderr,
                        "HeapElement::setKey(): ");
                fprintf(stderr,
                        "cannot set key for element already in heap.\n");
                return;
            }

            _key = key;
        };
    
        inline void *userData() {return _userData;};    
};


class Heap
{
    private:
        int _size;
        int maxSize;
        HeapElement **array;
        void heapify(int index);
    
    public:
        Heap(int initialMaxSize=1)
        {
            _size = 0;
            maxSize = initialMaxSize;
            array = new HeapElement *[maxSize];
        };
        ~Heap()
        {
            for (int i=0; i<_size; i++)
            {
                array[i]->_heap = NULL;
                array[i]->index = -1;
            }
            delete array;
            maxSize = 0;
            _size = 0;
        }
    
        void insert(HeapElement *element);
        void remove(HeapElement *element);
        void changeKey(HeapElement *element, float key);
        HeapElement *extractMin();
        HeapElement *min();
        inline void clear()
        {
            for (int i=0; i<_size; i++)
            {
                array[i]->_heap = NULL;
                array[i]->index = -1;
            }
            _size = 0;
        };
        inline int size() {return _size;};
        void test();
        void print();
};

/*---------------------------Globals (externed)------------------------------*/





/* Protection from multiple includes. */
#endif /* INCLUDED_HEAP_H */


/*****************************************************************************\
  $Log: Heap.h,v $
  Revision 1.13  2004/07/08 16:47:50  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.12  2004/07/08 16:15:30  gfx_friends
  many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)

  Revision 1.11  2004/06/16 20:30:35  gfx_friends
  values.h include change for osx

  Revision 1.10  2004/06/11 18:30:08  gfx_friends
  Remove all sources of warnings in xbs directory when compiled with -Wall

  Revision 1.9  2004/06/10 16:08:50  gfx_friends
  Converted heapify to non-recursive form for a meager speedup.

  Revision 1.8  2003/07/26 01:17:43  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.7  2003/07/23 19:55:33  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.6  2003/07/16 03:12:28  gfx_friends
  Added xbs support for "multi-attribute vertices". These are
  geometrically coincident vertices that may have different
  attributes. Geometric coincidence is maintained throughout the
  simplification process and attributes are correctly propagated along.

  For the full edge collapse, the heuristics for preventing attribute
  seams along patch boundaries could still use a little work.

  Things seem to work for the DiscreteHierarchy output. VDS hierarchy
  has not been integrated yet.

  Revision 1.5  2003/06/05 17:38:58  gfx_friends
  Patches to build on Win32.

  Revision 1.4  2003/01/19 01:11:24  gfx_friends
  *** empty log message ***

  Revision 1.3  2003/01/18 23:42:13  gfx_friends
  initial (non-working) version of triangle budget mode, etc.

  Revision 1.2  2003/01/14 00:06:19  gfx_friends
  Added destructors.

  Revision 1.1  2003/01/13 20:30:14  gfx_friends
  Added builder library, xbs (cross-bar simplifier)

  Revision 1.2  2003/01/05 22:37:57  cohen
  Added heap size parameter to initializer.
  Added convenience functions to determine if element is in a heap.
  Removed unnecessary space allocation macros.

  Revision 1.1  2002/10/17 21:05:11  cohen
  Initial revision

\*****************************************************************************/
