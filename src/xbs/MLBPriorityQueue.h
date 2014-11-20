/*****************************************************************************\
  MLBPriorityQueue.h
  --
  Description : The Multi-Level Bucket Priority Queue is relatively
                efficient implementation for a monotone priority
                queue. A monotone priority queue is one for which
                newly inserted items have greater keys than the
                current minimum key. For this implementation, it is
                okay to insert keys in any order until we start
                calling min() or extractMin(). After that point, we
                expect to follow the monotone property. However,
                because we do not always meet this property, the
                implementation provides a linked list called the
                underList to store items with keys that are currently
                too small. This could potentially be sorted or
                searched for the minimum, or even replaced with a
                little heap, but for the current application, we just
                pick them off the underList in any order, so it is not
                exact in these cases.

                This implementation is designed to natively operate on
                32-bit integer keys. The data structure consists of 4
                levels with 256 buckets each. All items are bucketed
                in level 0 according to their highest-order 8
                bits. The smallest non-empty bucket on level 0 is
                emptied and the items stored in level 1 according to
                the next 8 bits. The smallest non-empty bucket on
                level 1 is emptied and the items stored on level 2
                according to their next 8 bits, and similarly for
                level 3. To perform an extractMin(), we look for the
                smallest non-empty bucket on level 3. All items in
                that bucket have exactly the same key, so we remove
                one of them and return it. If level 3 becomes empty,
                we "expand" the next bucket on level 2 up into level
                3. When level 2 becomes empty, we go to level 1, and
                on to level 0 when level 1 is empty.

                To insert an item, we find it's place in level 0. If
                the currently expanded bucket on level 0 is smaller,
                we just insert the item on level 0. If the item's
                bucket is currently expanded, we look to storing it in
                level 1, etc. If the currently expanded bucket is
                larger, then this item violates the monotone
                property. If USE_UNDERLIST is defined at compile time,
                the item is stored in the underList. Otherwise, we can
                actually "collapse" the current buckets back towards
                level 0, insert the item, and expand the appropriate
                bucket upward. As it turns out, if we do this too
                often, it becomes no more efficient than the heap (and
                perhaps it could be worse). Hence the underList.

                This implementation also allows arbitrary items to be
                removed, not just the minimum key, and also allows
                changing the key of an item (which is similar to
                remove followed by insert).

                This implementation also supports positive floating
                point numbers as keys by keeping the same bit
                representation and using as an int. As it turns out,
                this mapping preserves ordering under IEEE floating
                point. Pretty neat, huh?

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/MLBPriorityQueue.h,v $
  $Revision: 1.7 $
  $Date: 2004/11/11 01:05:06 $
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
#ifndef INCLUDED_MLBPRIORITYQUEUE_H
#define INCLUDED_MLBPRIORITYQUEUE_H


/*------------------ Includes Needed for Definitions Below ------------------*/

#ifdef _WIN32
#undef min
#endif

#include <stdlib.h>
#include <stdio.h>

#include <limits.h>

/*-------------------------------- Constants --------------------------------*/

#if 1
#define USE_UNDERLIST
#endif

/*--------------------------------- Macros ----------------------------------*/


/*---------------------------------- Types ----------------------------------*/


/*---------------------------- Function Prototypes --------------------------*/


/*--------------------------------- Classes ---------------------------------*/

class MLBPriorityQueue;
class MLBPriorityQueueElement
{
    private:
        void *_userData;    // The user's data besides the key
        unsigned int _key;  // The key used for prioritization

        // variables typically managed by PQ class
        MLBPriorityQueue *_pq;  // which queue is this element currently in
        MLBPriorityQueueElement *prev, *next; // bucket's linked list

    public:
        friend class MLBPriorityQueue;


        MLBPriorityQueueElement(void *userData, float key)
        {
            _userData = userData;
            _pq = NULL;
            setKey(key);
            prev = next = NULL;
        }
        MLBPriorityQueueElement(void *userData, unsigned int key=UINT_MAX)
        {
            _userData = userData;
            _key = key;
            _pq = NULL;
            prev = next = NULL;
        }
        ~MLBPriorityQueueElement()
        {
            if (_pq != NULL)
            {
                fprintf(stderr, 
                        "MLBPriorityQueueElement free'd while in a MLBPriorityQueue!\n");
                exit(1);
            }
            _userData = NULL;
            _key = UINT_MAX;
            _pq = NULL;
            prev = next = NULL;
        }

        inline MLBPriorityQueue *pq() {return _pq;};
        inline MLBPriorityQueue *heap() {return pq();};
        inline int inPQ() {return (_pq!=NULL);};
        inline int inPQ(MLBPriorityQueue *pq) {return (_pq==pq);};
    
        // The caller is responsible for knowing if it is storing floating
        // point or integer keys in this priority queue and using the
        // appropriate calls

        inline unsigned int key() const {return _key;};
        inline float floatKey() const {return *((float *)(&_key));};
        inline void setKey(float key)
        {
            if (key < 0)
            {
                fprintf(stderr, "MLBPriorityQueueElement::setKey(): ");
                fprintf(stderr, "float keys must be non-negative\n");
                return;
            }
            // IEEE floating point preserves bitwise integer order for
            // non-negative values
            setKey(*((unsigned int *)(&key)));
            return;
        }

        inline void setKey(unsigned int key)
        {
            if (_pq != NULL)
            {
                fprintf(stderr,
                        "MLBPriorityQueueElement::setKey(): ");
                fprintf(stderr,
                        "cannot set key for element already in pq.\n");
                return;
            }
        
            _key = key;
        };
        inline void *userData() {return _userData;};    
};


class MLBPriorityQueue
{
    private:
        int _size; // number of elements in queue
        MLBPriorityQueueElement *levels[4][256]; // actual buckets
        int expanded[3]; // which bucket number on each level is currently
        // expanded  
        unsigned int masks[4][8]; // which buckets on each level are non-empty
        int expandLock; // prevents expanding buckets until extractions begin
#ifdef USE_UNDERLIST
        MLBPriorityQueueElement *underList; // small-key items not in buckets
#endif
#if 0
        // This could be a useful lookup table for determining the smallest
        // non-empty bucket if we're willing to use 32 8-bit masks instead
        // of 8 32-bit masks
        static int lowestOrderOne[256] =
        {
            -1,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            4,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            5,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            4,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            6,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            4,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            5,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            4,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            7,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            4,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            5,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            4,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            6,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            4,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            5,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
            4,  0,  1,  0,  2,  0,  1,  0,
            3,  0,  1,  0,  2,  0,  1,  0,
        };
#endif
        void collapse(int level);
        void expand(int level);
        int lowestNonEmptyBucket(int level);
        void IntToBase256(unsigned int I, unsigned char base256[4]);
        void setMask(int level, int bucket);
        void clearMask(int level, int bucket);
        int  getMask(int level, int bucket);
    
    public:
        MLBPriorityQueue()
        {
            clear();
        };
        ~MLBPriorityQueue()
        {
            _size = 0;
        }
    
        void insert(MLBPriorityQueueElement *element);
        void remove(MLBPriorityQueueElement *element);
        inline void changeKey(MLBPriorityQueueElement *element, float key)
        {
            if (key < 0)
            {
                fprintf(stderr, "MLBPriorityQueue::changeKey(): ");
                fprintf(stderr, "float keys must be non-negative\n");
                return;
            }
            // IEEE floating point preserves bitwise integer order for
            // non-negative values
            changeKey(element, (*((unsigned int *)(&key))));
            return;
        }
        void changeKey(MLBPriorityQueueElement *element, unsigned int key);
    
        MLBPriorityQueueElement *extractMin();
        MLBPriorityQueueElement *min();
        inline void clear()
        {
            _size = 0;
            for (int lev=0; lev<4; lev++)
            {
                for (int bucket=0; bucket<256; bucket++)
                    levels[lev][bucket] = NULL;
                for (int i=0; i<8; i++)
                    masks[lev][i] = 0;
            }
            for (int lev=1; lev<4; lev++)
                expanded[lev-1] = -1;
            expandLock = 1;
#ifdef USE_UNDERLIST
            underList = NULL;
#endif
        };
        inline int size() {return _size;};
        void test();
#if 0
        void print();
#endif
};

/*---------------------------Globals (externed)------------------------------*/





/* Protection from multiple includes. */
#endif /* INCLUDED_MLBPRIORITYQUEUE_H */


/*****************************************************************************\
  $Log: MLBPriorityQueue.h,v $
  Revision 1.7  2004/11/11 01:05:06  gfx_friends
  Fixed a comment typo.

  Revision 1.6  2004/08/23 16:47:09  gfx_friends
  Pulled MT into a new file so that I can do some experiments with the shortcut algorithm.--nat

  Revision 1.5  2004/07/14 12:45:09  gfx_friends
  Fixed a minor bug. When all the buckets become empty, the queue got into
  an invalid state, so further insert() calls resulted in a non-empty
  queue that you could not find items in. Now when the buckets become empty,
  we re-initialize the queue as a quick fix to ensure it's back in a working
  state.

  Revision 1.4  2004/07/08 16:47:49  gfx_friends
  Removed tabs and updated indentation for xbs source files

  Revision 1.3  2004/06/30 13:02:01  gfx_friends
  Added heavy commenting. Observe and imitate.

  Revision 1.2  2004/06/11 19:05:48  gfx_friends
  Got Win32-debug to work after moving the directory structure around.

  Revision 1.1  2004/06/10 16:13:37  gfx_friends
  Added a Multi-level Bucket (MLB) Priority Queue to replace the
  standard binary heap as the main xbs priority queue. It makes use of
  the fact that the xbs priority queue is mostly monotonic, with newly
  inserted items almost always having larger keys than the key of the
  item at the top of the heap. It seems to be about 75% faster than the
  binary heap, so the priority queue is no longer a major bottleneck,
  even for the current fast error metric computation.

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
