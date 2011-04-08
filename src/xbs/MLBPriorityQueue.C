/*****************************************************************************\
  MLBPriorityQueue.C
  --
  Description : Multi-level bucket priority queue
                implementation. Allows insert(), remove(),
                extractMin(), and changeKey(). Should work best for
                monotonic applications (where the minimum key keeps
                decreasing and rarely increases). See
                MLBPriorityQueue.h for a longer description.

  ----------------------------------------------------------------------------
  $Source: /uf6/gfx/glod/cvsroot/glod/src/xbs/MLBPriorityQueue.C,v $
  $Revision: 1.9 $
  $Date: 2004/07/15 12:50:49 $
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

#include <MLBPriorityQueue.h>
#include <math.h>

/*------------------------------ Local Macros -------------------------------*/


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




/*****************************************************************************\
 @ MLBPriorityQueue::insert
 -----------------------------------------------------------------------------
 description : Insert an element into the MLBPriority queue.
 input       : The element is allocated by the caller, who should also
               call setKey() before insert().
 output      :  
 notes       : The actual element (not a copy) is stored in the
               MLBPriorithyQueue.
               The caller is not allowed to delete the element while
               it is in the MLBPriorityQueue!
\*****************************************************************************/
void
MLBPriorityQueue::insert(MLBPriorityQueueElement *element)
{
    if (element->_pq != NULL)
    {
        fprintf(stderr, "MLBPriorityQueue::insert():");
        fprintf(stderr, "element already in an MLBPriorityQueue.\n");
        exit(1);
    }

    // Break the key up into it's 4 bytes (i.e. base 256
    // representation). Each byte corresponds to a bucket number on a
    // particular level

    unsigned char base256[4];
    IntToBase256(element->key(), base256);

    // Find the level at which this item should be created. This may
    // involve collapsing and expanding levels if the key is smaller
    // than the current min and we are not using the underList.

    int level;
    for (level=0; level<=2; level++)
    {
        // Compare this key's bucket number at this level to the bucket
        // number that is currently expanded upward for this level to
        // see if this is the level where we should insert the item

        if (base256[level] < expanded[level])
        {
            // Everything goes on level 0 until the expandLock is
            // turned off
            if (expandLock == 1)
                break;

            // This is the non-monotone insert case. If we're using
            // the underList, put the item on that list. Otherwise, we
            // have to collapse the current bucket structure down to
            // the level where this key was too small, then expand
            // its bucket upward.

#ifdef USE_UNDERLIST
            //fprintf(stderr, "adding to underList.\n");
            
            element->next = underList;
            if (element->next != NULL)
                element->next->prev = element;
            element->prev = NULL;
            underList = element;
            element->_pq = this;
            _size++;
            return;
#else
            // expand this bucket and move up to next level
            collapse(level);
            expanded[level] = base256[level];
#endif
        }
        else if (base256[level] > expanded[level])
        {
            // we're ready to insert here
            break;
        }
        else // base256[level] == expanded[level]
        {
            // move up to next level
        }
    }

    // Now we should be ready to insert at current level
    // (this code is basically as standard doubly-linked list
    // insertion
    element->prev = NULL;
    element->next = levels[level][base256[level]];
    if (element->next != NULL)
        element->next->prev = element;
    levels[level][base256[level]] = element;

    element->_pq = this;

    // Update the masks to indicate that this bucket is non-empty
    if (element->next == NULL)
        setMask(level, base256[level]);
    
    _size++;
} /** End of MLBPriorityQueue::insert() **/


/*****************************************************************************\
 @ MLBPriorityQueue::remove
 -----------------------------------------------------------------------------
 description : Remove an element from the MLBPriorityQueue. This
               element does not have to be the minimum key -- it just
               has to be in the queue.
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void
MLBPriorityQueue::remove(MLBPriorityQueueElement *element)
{
    // Make sure the element is in this MLBPriorityQueue
    if (element->_pq != this)
    {
        fprintf(stderr, "MLBPriorityQueue::remove():");
        fprintf(stderr, "element not in this MLBPriorityQueue.\n");
        exit(1);
    }

    // Get the key's 4-byte representation to index buckets
    unsigned char base256[4];
    IntToBase256(element->key(), base256);

    // Find out which level of buckets the element is in (or remove it
    // from the underList if it is in there). Note that the only
    // reason we have to find the element's bucket is in case this is
    // the first item in the bucket and we have to update that pointer
    // (and possibly mark the bucket empty)

    int level;
    for (level=0; level<=2; level++)
    {
        if (base256[level] < expanded[level])
        {
            // If it's bucket is smaller than the currently expanded
            // level, it cannot be in the buckets at all (because the
            // smallest bucket should always be the one that's
            // expanded). So if we are using an underList, it should
            // be in there. Otherwise it is an error.

            // Note that if the expandLock is on, the current expanded
            // bucket is set to -1 for every level, so we should not
            // be able to enter this branch

#ifdef USE_UNDERLIST
            // removing from underList
            //fprintf(stderr, "removing from underList.\n");
            
            if (element->prev == NULL)
                underList = element->next;
            else
                element->prev->next = element->next;
            if (element->next != NULL)
                element->next->prev = element->prev;
            element->prev = element->next = NULL;
            element->_pq = NULL;
            _size--;
            if (_size == 0)
                clear();
            return;
#else
            // error! not found!
            fprintf(stderr, "MLBPriorityQueue::remove(): Element not found!\n");
            exit(1);
#endif
        }
        else if (base256[level] > expanded[level])
        {
            // it's on this level
            // Note: this is the branch we take if the expandLock has
            // not been disabled yet and everything is on level 0
            break;
        }
        else // (base256[level] == expanded[level])
        {
            // move on to next level
        }
    }
    
    // remove from this level
    if (element->prev == NULL)
        levels[level][base256[level]] = element->next;
    else
        element->prev->next = element->next;
    if (element->next != NULL)
        element->next->prev = element->prev;
    element->prev = element->next = NULL;
    element->_pq = NULL;
    _size--;

    // The bucket is now empty...
    if (levels[level][base256[level]] == NULL)
    {
        // Update the bucket mask to indicate this bucket is empty
        clearMask(level, base256[level]);

        // If this element was in the top level, there is a chance
        // there are no more non-empty buckets on this level. If so,
        // we need to expand some other bucket from a lower level.
        if (level == 3)
        {
            int lev;
            for (lev=2; lev>=0; lev--)
            {
                if (lowestNonEmptyBucket(lev+1) == -1)
                    expanded[lev] = -1;
                else
                    break;
            }
            if (lev < 2)
                expand(lev+1);
        }
    }

    if (expanded[0] == -1)
    {
#ifdef USE_UNDERLIST
        MLBPriorityQueueElement *list = underList;
#endif

        clear();
        
#ifdef USE_UNDERLIST
        while (list != NULL)
        {
            MLBPriorityQueueElement *element = list;
            list = element->next;
            element->next = NULL;
            element->_pq = NULL;
            insert(element);
        }
#endif
    }
    
    return;
} /** End of MLBPriorityQueue::remove() **/





/*****************************************************************************\
 @ MLBPriorityQueue::min
 -----------------------------------------------------------------------------
 description : Return the element with the minimum key.
 input       : 
 output      : Pointer to the minimum key element. The element is
               still in the priority queue, so the user may not call
               delete or setKey() (but may call changeKey() as always).
 notes       : The first time this is called, it unlocks the
               expandLock and expands buckets into all levels above
               level 0, making the fast location of minimum key
               elements possible. So after this is called, we are
               expected to mostly follow the monotone property,
               inserting elements with keys greater than the current
               minimum.
\*****************************************************************************/
MLBPriorityQueueElement *
MLBPriorityQueue::min()
{
    if (_size < 1)
        return NULL;

    if (expandLock == 1)
    {
        expandLock = 0;
        expand(0);
    }

#ifdef USE_UNDERLIST
    if (underList != NULL)
        return underList;
#endif
    
    // The lowest non-empty bucket on the top level contains elements
    // with the current minimum key (unless there were elements on the
    // underList)
    int bucket = lowestNonEmptyBucket(3);
    

    if ((bucket==-1) || (levels[3][bucket] == NULL))
    {
        fprintf(stderr, "MLBPriorityQueue::min(): Couldn't find an element!\n");
        exit(1);
    }
    
    return levels[3][bucket];
} /** End of MLBPriorityQueue::min() **/


/*****************************************************************************\
 @ MLBPriorityQueue::extractMin
 -----------------------------------------------------------------------------
 description : Remove an element with the minimum key from the
               priority queue and return a pointer to it.
 input       : 
 output      : Pointer to minimum key element. It's no longer in the
               priority queue, so the caller may call delete or do a
               setKey() followed by insert() to put it back in later, etc.
 notes       :
\*****************************************************************************/
MLBPriorityQueueElement *
MLBPriorityQueue::extractMin()
{
    if (_size < 1)
        return NULL;

    MLBPriorityQueueElement *minElement = min();
    remove(minElement);
    
    return minElement;
} /** End of MLBPriorityQueue::extractMin() **/


/*****************************************************************************\
 @ MLBPriorityQueue::changeKey
 -----------------------------------------------------------------------------
 description : Change the key of an element already in the priority queue.
 input       : Pointer to the element, and value of its new key.
 output      : 
 notes       : As a convenience to the current application, the
               element could be absent from the queue and be inserted
               after changing its key. So the caller should not be
               calling insert() after changeKey(). Some of this
               behavior is designed to match that of a particular Heap
               data structure with the same interface.
\*****************************************************************************/
void
MLBPriorityQueue::changeKey(MLBPriorityQueueElement *element, unsigned int key)
{
    if ((element->pq() != NULL) && (element->pq() != this))
    {
        fprintf(stderr, "Trying to change key of element in wrong pq!\n");
        exit(1);
    }
    
    if (element->pq() != NULL)
        remove(element);

    element->setKey(key);

    insert(element);
    
    return;
} /** End of MLBPriorityQueue::changeKey() **/


/*****************************************************************************\
 @ MLBPriorityQueue::IntToBase256
 -----------------------------------------------------------------------------
 description : Convert an integer to base 256. This is just a too-fancy
               way of saying convert a single 32-bit integer into 4
               8-bit integers by extracting the individual bytes.
 input       : The unsigned integer to convert.
 output      : The four 8-bit integers, stored in unsigned chars. The
               first char has the highest-order 8 bits, the last char
               has the lowest-order 8 bits.
 notes       :
\*****************************************************************************/
void
MLBPriorityQueue::IntToBase256(unsigned int I, unsigned char base256[4])
{
    base256[0] = (unsigned char)((I & 0xFF000000) >> 24);
    base256[1] = (unsigned char)((I & 0x00FF0000) >> 16);
    base256[2] = (unsigned char)((I & 0x0000FF00) >> 8);
    base256[3] = (unsigned char)(I & 0x000000FF);
} /** End of MLBPriorityQueue::IntToBase256() **/


/*****************************************************************************\
 @ MLBPriorityQueue::setMask
 -----------------------------------------------------------------------------
 description : Set a bit in the 256-bit bit mask associated with some
               level. This indicates that a particular bucket is non-empty.
 input       : The level number and the bucket number.
 output      : 
 notes       : Each 256-bit mask is stored as 8 32-bit unsigned ints,
               so we have to find the correct bit position in the
               correct unsigned int to set.
\*****************************************************************************/
void
MLBPriorityQueue::setMask(int level, int bucket)
{
    int maskNum = bucket/32;
    int maskBitNum = bucket%32;
    masks[level][maskNum] |= 1 << maskBitNum;
} /** End of MLBPriorityQueue::setMask() **/


/*****************************************************************************\
 @ MLBPriorityQueue::clearMask
 -----------------------------------------------------------------------------
 description : Clear a bit in the 256-bit bit mask associated with some
               level. This indicates that a particular bucket is empty.
 input       : The level number and the bucket number.
 output      : 
 notes       : Each 256-bit mask is stored as 8 32-bit unsigned ints,
               so we have to find the correct bit position in the
               correct unsigned int to clear.
\*****************************************************************************/
void
MLBPriorityQueue::clearMask(int level, int bucket)
{
    int maskNum = bucket/32;
    int maskBitNum = bucket%32;
    masks[level][maskNum] &= ~(1 << maskBitNum);
} /** End of MLBPriorityQueue::clearMask() **/


/*****************************************************************************\
 @ MLBPriorityQueue::getMask
 -----------------------------------------------------------------------------
 description : Get a bit in the 256-bit bit mask associated with some
               level. 
 input       : 
 output      : A boolean value (0 or 1), returned in an integer,
               indicating the current state of the bit in question. 0
               indicates an empty bucket, 1 a non-empty bucket.
 notes       : Each 256-bit mask is stored as 8 32-bit unsigned ints,
               so we have to find the correct bit position in the
               correct unsigned int to clear.
\*****************************************************************************/
int
MLBPriorityQueue::getMask(int level, int bucket)
{
    int maskNum = bucket/32;
    int maskBitNum = bucket%32;
    int set = ((masks[level][maskNum] & (1 << maskBitNum)) == 0) ? 0 : 1;
    return set;
} /** End of MLBPriorityQueue::getMask() **/


/*****************************************************************************\
 @ MLBPriorityQueue::lowestNonEmptyBucket
 -----------------------------------------------------------------------------
 description : Find the lowest-numbered, non-empty bucket in a
               particular level. In general, this is the next bucket
               to be expanded upward, and contains the smallest-keyed
               elements in a particular level. For the top level, the
               elements in this bucket have the overall minimum key.
 input       : Level number of buckets (masks) to search.
 output      : Number of smallest non-empty bucket (0-255). If all the
               buckets are empty, it returns -1.
 notes       : This is currently implemented by a brute-force loop
               over the 256 bits. It could be implemented with the
               assistance of an 8-bit lookup table to find the
               smallest non-zero bit in an 8-bit number. However,
               performance profiling does not show this function to be
               critical. In fact, the current implementation is
               probably more friendly to the computer's memory cache
               than one using a lookup table anyway.
\*****************************************************************************/
int
MLBPriorityQueue::lowestNonEmptyBucket(int level)
{
    for (int maskNum=0; maskNum<8; maskNum++)
    {
        int bit = 1;
        for (int bitNum=0; bitNum<32; bitNum++)
        {
            if ((masks[level][maskNum] & bit) != 0)
                return (bitNum + maskNum*32); 
            bit <<= 1;
        }
    }
    return -1;
} /** End of MLBPriorityQueue::lowestNonEmptyBucket() **/


/*****************************************************************************\
 @ MLBPriorityQueue::collapse
 -----------------------------------------------------------------------------
 description : This function collapses all the buckets above a given
               level down into the given level. The assumption is that
               we plan to next expand a different bucket from this
               level. This function is generally only needed if we
               violate the monotone property and are not using the
               underList to handle such cases. That is because in
               normal operation, the buckets are emptied with calls to
               extractMin() and only need to be refilled by calls to
               expand().
 input       : The level number to collapse buckets down into.
 output      : 
 notes       : The priority queue is generally not in a valid state
               after calling collapse. the "expanded" variable for all
               the levels including and above this one are set to -1,
               indicating that they do not have an expanded
               bucket. Again, it is intended to be followed shortly
               after by a call to expand this same level.
\*****************************************************************************/
void
MLBPriorityQueue::collapse(int level)
{
    // collapse all buckets from level+1 (and all levels above) into level
    for (int lev=2; lev>=level; lev--)
    {
        MLBPriorityQueueElement *head = NULL;

        // Create list of all elements from non-empty buckets in level above
        for (int nonEmpty = lowestNonEmptyBucket(lev+1);
             nonEmpty != -1;
             nonEmpty = lowestNonEmptyBucket(lev+1))
        {
            while (levels[lev+1][nonEmpty] != NULL)
            {
                MLBPriorityQueueElement *newhead = levels[lev+1][nonEmpty];
                levels[lev+1][nonEmpty] = newhead->next;
                if (levels[lev+1][nonEmpty] != NULL)
                    levels[lev+1][nonEmpty]->prev = NULL;
                if (head != NULL)
                    head->prev = newhead;
                newhead->next = head;
                head = newhead;
            }

            // Mark the source bucket empty
            clearMask(lev+1, nonEmpty);
        }

        if (head != NULL)
        {
            // Actually store the list of elements removed from level
            // above in bucket on this level
            levels[lev][expanded[lev]] = head;

            // Mark the bucket non-empty
            setMask(lev, expanded[lev]);
        }

        // Indicate that this level is no longer expanded
        expanded[lev] = -1;
    }
} /** End of MLBPriorityQueue::collapse() **/


/*****************************************************************************\
 @ MLBPriorityQueue::expand
 -----------------------------------------------------------------------------
 description : Expand this level by removing elements from the
               smallest-valued non-empty bucket on this level and
               pushing them up into the levels above. It assumes that
               this level is not currently expanded and all the levels
               above are non-empty. This function is necessary, for
               example, when calls to extractMin() have emptied the
               entire top level.
 input       : The bucket level to expand.
 output      : 
 notes       :
\*****************************************************************************/
void
MLBPriorityQueue::expand(int level)
{
    for (int lev=level; lev<=2; lev++)
    {
        // Make sure the level is not already expanded
        if (expanded[lev] != -1)
        {
            fprintf(stderr, "MLBPriorityQueue::expand(): trying to expand an expanded level!\n");
            exit(1);
        }
        
        // Find the bucket to expand
        int nonEmpty = lowestNonEmptyBucket(lev);

        // If all buckets are empty, there is nothing for us to expand
        if (nonEmpty == -1)
            break;

        // Move each of the elements from the smallest non-empty
        // bucket up to the next level
        while (levels[lev][nonEmpty] != NULL)
        {
            // remove from bucket
            MLBPriorityQueueElement *elem = levels[lev][nonEmpty];
            levels[lev][nonEmpty] = elem->next;
            if (levels[lev][nonEmpty] != NULL)
                levels[lev][nonEmpty]->prev = NULL;

            // compute destination bucket in level above
            unsigned char base256[4];
            IntToBase256(elem->key(), base256);

            // insert into destination bucket
            elem->next = levels[lev+1][base256[lev+1]];
            if (levels[lev+1][base256[lev+1]] != NULL)
                levels[lev+1][base256[lev+1]]->prev = elem;
            levels[lev+1][base256[lev+1]] = elem;

            // Mark the destination bucket non-empty
            if (elem->next == NULL)
                setMask(lev+1, base256[lev+1]);
        }

        // Indicate which bucket is now expanded at this level
        expanded[lev] = nonEmpty;

        // Mark the bucket we just expanded empty (because we've moved
        // all the elements to higher levels)
        clearMask(lev, nonEmpty);
    }
} /** End of MLBPriorityQueue::expand() **/


/*****************************************************************************\
 @ MLBPriorityQueue::test
 -----------------------------------------------------------------------------
 description : This is a little test routine to verify that the
               priority queue is in a valid state. Naturally, this is
               a relatively slow and verbose routine and should
               typically only be used for debugging.
 input       : 
 output      : 
 notes       : There are probably a few other tests one could add that
               I didn't happen to think of or bother coding.
\*****************************************************************************/
void
MLBPriorityQueue::test()
{
    // empty pq
    if (size() == 0)
    {
        // all masks should be zero
        for (int lev=0; lev<4; lev++)
            for (int maskNum=0; maskNum<8; maskNum++)
                if (masks[lev][maskNum] != 0)
                {
                    fprintf(stderr,
                            "Non-zero mask in empty pq.\n");
                }

        // nothing should be expanded
        for (int lev=0; lev<3; lev++)
            if (expanded[lev] != -1)
            {
                fprintf(stderr, "Expanded level in empty pq.\n");
            }

        // all levels should have NULL pointers
        for (int lev=0; lev<4; lev++)
            for (int bucket=0; bucket<256; bucket++)
                if (levels[lev][bucket] != NULL)
                {
                    fprintf(stderr, "Non-NULL bucket in empty pq.\n");
                }
        return;
    }

    //
    // non-empty pq
    //

    // First three levels should be expanded
    if (expandLock == 0)
    {
        for (int lev=0; lev<3; lev++)
        {
            if ((expanded[lev] < 0) || (expanded[lev] > 255))
                fprintf(stderr, "Level not properly expanded.\n");
            if (levels[lev][expanded[lev]] != NULL)
                fprintf(stderr, "Expanded bucket not NULL.\n");
        }
    }
     
     
    // Compare masks to buckets
    for (int lev=0; lev < 4; lev++)
        for (int bucket=0; bucket < 256; bucket++)
        {
            int mask = getMask(lev, bucket);
            if (((mask != 0) && (levels[lev][bucket] == NULL)) ||
                ((mask == 0) && (levels[lev][bucket] != NULL)))
            {
                fprintf(stderr, "Mask and buckets disagree.\n");
            }
        }
     
    // Number of elements should agree with size() and the elements should
    // be in the correct buckets.
    int count = 0;
    for (int lev=0; lev<4; lev++)
        for (int bucket=0; bucket<256; bucket++)
        {
            for (MLBPriorityQueueElement *elem = levels[lev][bucket];
                 elem != NULL; elem = elem->next, count++)
            {
                unsigned char base256[4];
                IntToBase256(elem->key(), base256);
                for (int le=0; le<lev; le++)
                    if (base256[le] != expanded[le])
                    {
                        fprintf(stderr, "Element on wrong level\n");
                    }
                if (base256[lev] != bucket)
                {
                    fprintf(stderr, "Element in wrong bucket.\n");
                }
                if (elem->key() == UINT_MAX)
                {
                    fprintf(stderr, "Element key is UINT_MAX\n");
                }
                if (elem->pq() != this)
                {
                    fprintf(stderr, "Element pq point is wrong.\n");
                }
            }
        }
    if (count != size())
        fprintf(stderr, "Elements found != pq size.\n");

    return;
} /** End of MLBPriorityQueue::test() **/




/*****************************************************************************\
 @ MLBPriorityQueue::test_pq
 -----------------------------------------------------------------------------
 description : Build and test a priority queue with a few simple scenarios.
               I guess this routine also serves as an example of the
               calling semantics for using this data structure.
 input       : Number of elements to use in test scenarios.
 output      : 
 notes       : Because these scenarios use random keys, there will
               probably not be duplicate keys, and the buckets will
               probably be pretty sparse at all but the lowest
               level(s). You may want to mod the random keys by some
               number to concentrate the values into a smaller range.
\*****************************************************************************/
void test_pq(int testsize)
{
    int   i;
    MLBPriorityQueue *pq;
    MLBPriorityQueueElement **elements;
    
    elements = new MLBPriorityQueueElement *[testsize];
    
    for (i=0; i<testsize; i++)
    {
        elements[i] = new MLBPriorityQueueElement(NULL, (unsigned int)rand());
    }
    
    
    pq = new MLBPriorityQueue();
    pq->test();
    
    for (i=testsize-1; i>=0; i--)
        pq->insert(elements[i]);

    pq->test();
    fprintf(stderr, "Pqsize: %d\n", pq->size());
    
    
    fprintf(stderr, "Testing change key\n");
    for (i=0; i<testsize; i++)
    {
        pq->changeKey(elements[i], (unsigned int)rand());
        pq->test();
    }

    fprintf(stderr, "Pqsize: %d\n", pq->size());
    pq->test();
    
    for (i=0; i<testsize; i++)
    {
        pq->extractMin();
//      fprintf(stderr, "Pqsize: %d\n", pq->size);
    }

    pq->test();
} /** End of MLBPriorityQueue::test_pq() **/


#if 0
/*****************************************************************************\
 @ main
 -----------------------------------------------------------------------------
 description : A main() routine for testing the MLBPriorityQueue data
               structure. 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
int main(int argc, char **argv)
{
    test_pq(1000);
    return 0;
} /** End of main() **/
#endif

/*****************************************************************************\
  $Log: MLBPriorityQueue.C,v $
  Revision 1.9  2004/07/15 12:50:49  gfx_friends
  Fixed the previous bug fix.

  Revision 1.8  2004/07/14 12:45:08  gfx_friends
  Fixed a minor bug. When all the buckets become empty, the queue got into
  an invalid state, so further insert() calls resulted in a non-empty
  queue that you could not find items in. Now when the buckets become empty,
  we re-initialize the queue as a quick fix to ensure it's back in a working
  state.

  Revision 1.7  2004/07/08 16:44:41  gfx_friends
  Removed tabs and did 4-space indentation on source files in xbs directory.

  Revision 1.6  2004/06/30 13:02:01  gfx_friends
  Added heavy commenting. Observe and imitate.

  Revision 1.5  2004/06/16 20:30:35  gfx_friends
  values.h include change for osx

  Revision 1.4  2004/06/11 21:15:05  gfx_friends
  fixed a makefile to work with osx

  Revision 1.3  2004/06/11 19:05:47  gfx_friends
  Got Win32-debug to work after moving the directory structure around.

  Revision 1.2  2004/06/11 18:30:08  gfx_friends
  Remove all sources of warnings in xbs directory when compiled with -Wall

  Revision 1.1  2004/06/10 16:13:37  gfx_friends
  Added a Multi-level Bucket (MLB) Priority Queue to replace the
  standard binary heap as the main xbs priority queue. It makes use of
  the fact that the xbs priority queue is mostly monotonic, with newly
  inserted items almost always having larger keys than the key of the
  item at the top of the heap. It seems to be about 75% faster than the
  binary heap, so the priority queue is no longer a major bottleneck,
  even for the current fast error metric computation.

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

