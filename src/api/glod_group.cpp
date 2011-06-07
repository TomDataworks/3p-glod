/* GLOD: Group management functions
*******************************************************************************
 * $Id $
 * $Revision: 1.50 $
 ******************************************************************************/
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
/*! \file
    GLOD group adaptation routines
 */

/*----------------------------- Local Includes -----------------------------*/


#include <stdio.h>
#if defined(_WIN32) || defined(__APPLE__)
#include <float.h>
#else
#include <values.h>
#endif

#include "xbs.h"
#include "glod_core.h"
#include "hash.h"
#include "Continuous.h"
/*----------------------------- Local Constants -----------------------------*/

//#define GLOD_USE_TILES
/*------------------------------ Local Macros -------------------------------*/


/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/


/*------------------------ Local Function Prototypes ------------------------*/


/*---------------------------------Functions-------------------------------- */


//
//
//     API ENTRIES
//
//

void glodSetLayout(int rows, int cols){
#ifdef GLOD_USE_TILES
    GLOD_TILE_ROWS=rows;
    GLOD_TILE_COLS=cols;
    GLOD_NUM_TILES=rows*cols;
    delete tiles;
    tiles=new GLOD_Tile[GLOD_NUM_TILES];
    float s_x=2.0f/(float)(GLOD_TILE_COLS);
    float s_y=2.0f/(float)(GLOD_TILE_ROWS);
    int k=0;
    for (int i=0; i<GLOD_TILE_COLS; i++)
	for (int j=0; j<GLOD_TILE_ROWS; j++){
	    tiles[k].min_x=-1+i*s_x;
	    tiles[k].max_x=-1+(i+1)*s_x;
	    tiles[k].max_y=1-j*s_y;
	    tiles[k].min_y=1-(j+1)*s_y;
	    k++;
	}
#endif
}

void glodAdjustTiles(float *meanError){
#ifdef GLOD_USE_TILES
    float mean=0;
    float den=0;
    for (int i=0; i<GLOD_NUM_TILES; i++)
	if (meanError[i]!=0) {
	    mean+=meanError[i];
	    den++;
	}
    mean/=den;
    int nv=GLOD_NUM_TILES;
    for (int i=0; i<nv; i++){
	float errorDiff=meanError[i]/mean;
	printf("area %i mean %f global mean %f diff %f\n", i, meanError[i], mean, errorDiff);
	if ((errorDiff>2)){ // || (errorDiff<.5))
	    printf("tiles are unbalanced\n");
	    if ((tiles[i].max_x-tiles[i].min_x)<.2)
		continue;
	    GLOD_Tile *temp_tiles=new GLOD_Tile[GLOD_NUM_TILES+3];
	    for (int j=0; j<GLOD_NUM_TILES; j++)
		temp_tiles[j]=tiles[j];
	    delete tiles;
	    tiles=temp_tiles;
	    float tx=tiles[i].max_x-tiles[i].min_x;
	    float ty=tiles[i].max_y-tiles[i].min_y;
	    tiles[GLOD_NUM_TILES]=tiles[i];
	    tiles[GLOD_NUM_TILES+1]=tiles[i];
	    tiles[GLOD_NUM_TILES+2]=tiles[i];
	    tiles[i].max_x=tiles[i].min_x+tx/2;
	    tiles[i].max_y=tiles[i].min_y+ty/2;
	    tiles[GLOD_NUM_TILES].min_x+=tx/2;
	    tiles[GLOD_NUM_TILES].max_y=tiles[GLOD_NUM_TILES].min_y+ty/2;
	    tiles[GLOD_NUM_TILES+1].min_y+=ty/2;
	    tiles[GLOD_NUM_TILES+1].max_x=tiles[GLOD_NUM_TILES+1].min_x+tx/2;
	    tiles[GLOD_NUM_TILES+2].min_y+=ty/2;
	    tiles[GLOD_NUM_TILES+2].min_x+=tx/2;
	    GLOD_NUM_TILES+=3;
	    }
    }
#endif
}

void glodNewGroup(GLuint name)
{
    GLOD_Group *group =
	(GLOD_Group *)HashtableSearch(s_APIState.group_hash, name);
    if (group != NULL) {
	GLOD_SetError(GLOD_INVALID_NAME, "Group name already exists.", name);
	return;
    }

    group = new GLOD_Group();
    HashtableAdd(s_APIState.group_hash, name, group);
    return;
} /* End of glodNewGroup() **/
  


void glodDeleteGroup(GLuint name)
{
    int i;
    GLOD_Object* obj;
    
    GLOD_Group *group =
	(GLOD_Group *)HashtableSearch(s_APIState.group_hash, name);
    
    if(group == NULL)
    {
	GLOD_SetError(GLOD_INVALID_NAME, "Group does not exist.", name);
	return;
    }

    // iterate through group objects... delete each object
    for(i = 0; i < group->getNumObjects(); i++)
    {
	obj = group->getObject(i);
        
        glodDeleteObject(obj->name);
    }
    
    
    // delete the group
    HashtableDeleteCautious(s_APIState.group_hash, name);
    delete group; // deletes the objects
    return;
    
} /* End of glodDeleteGroup() **/



void glodAdaptGroup(GLuint name) {
    GLOD_Group *group =
	(GLOD_Group *)HashtableSearch(s_APIState.group_hash, name);
    
    if(group == NULL) {
	GLOD_SetError(GLOD_INVALID_NAME, "Group does not exist", name);
	return;
    }
    if ((group->numTiles!=GLOD_NUM_TILES))
	group->changeLayout();
    group->adapt();
} /* End of glodAdaptGroup() **/






//
//
//     GLOD_Group Class Methods
//
//


/*****************************************************************************\
 @ GLOD_Group::addObject
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void GLOD_Group::addObject(GLOD_Object *obj)
{
    if (numObjects == maxObjects)
    {
	if (maxObjects == 0)
	{
	    objects = new (GLOD_Object*)();
	    maxObjects = 1;
	}
	else
	{
	    GLOD_Object **newObjects = new GLOD_Object *[maxObjects*2];
	    for (int i=0; i<numObjects; i++)
		newObjects[i] = objects[i];
	    delete [] objects;
	    objects = newObjects;
	    maxObjects *= 2;
	}
    }

    objects[numObjects++] = obj;
    //printf("adding object %i num %i\n", obj->name, numObjects);
    obj->groupIndex = numObjects-1;
    obj->group = this;
    objectsChanged = 1;

	obj->cut->setGroup(this);

    if(obj->format == GLOD_VDS) {

	switch (errorMode)
	{
	case ObjectSpace:
		if (viewFrustumSimp)
			mpSimplifier->SetErrorFunc(StdErrorObjectSpace);
		else
			mpSimplifier->SetErrorFunc(StdErrorObjectSpaceNoFrustum);
		break;
	case ScreenSpace:
		if (viewFrustumSimp)
			mpSimplifier->SetErrorFunc(StdErrorScreenSpace);
		else
			mpSimplifier->SetErrorFunc(StdErrorScreenSpaceNoFrustum);
		break;
	};
        
//	fprintf(stderr, "mpCut->SetSimplifier()\n");
	((VDSCut*) obj->cut)->mpCut->SetSimplifier(mpSimplifier);
//	fprintf(stderr, "mpCut->SimplifierSet\n");
	
//	fprintf(stderr, "mpSimplifier->AddCut()\n");
	mpSimplifier->AddCut(   ((VDSCut*)obj->cut)->mpCut );
//	fprintf(stderr, "mpSimplifier->CutAdded\n");
    }	
    
#ifndef GLOD_USE_TILES
    currentNumTris += obj->cut->currentNumTris;
#else
    for (int i=0; i<GLOD_NUM_TILES; i++)
	if (obj->cut->currentErrorScreenSpace() > 0.f)
	    currentNumTris[i] += obj->cut->currentNumTris;
#endif
    return;
} /* End of GLOD_Group::addObject() **/



/*****************************************************************************\
 @ GLOD_Group::removeObject
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
GLOD_Group::removeObject(int index)
{
    if ((index < 0) || (index >= numObjects))
    {
	fprintf(stderr, "GLOD_Group::removeObject(): invalid index\n");
	return;
    }
#ifndef GLOD_USE_TILES
    if (objects[index]->cut->currentErrorScreenSpace() > 0.f)
	currentNumTris -= objects[index]->cut->currentNumTris;
#else
    for (int i=0; i<GLOD_NUM_TILES; i++)
	if (objects[index]->cut->currentErrorScreenSpace() > 0.f)
	    currentNumTris[i] -= objects[index]->cut->currentNumTris;
#endif
    //currentNumTris-= objects[index]->cut->currentNumTris;
    
    // remove the object from the coarsen & refine queues
    if(objects[index]->budgetCoarsenHeapData.inHeap())
        coarsenQueue->remove(&objects[index]->budgetCoarsenHeapData);
    if(objects[index]->budgetRefineHeapData.inHeap())
        refineQueue->remove(&objects[index]->budgetRefineHeapData);


    objects[index]->groupIndex = -1;
    objects[index]->group = NULL;
    
    objects[index] = objects[--numObjects];
    objects[index]->groupIndex = index;
    
    objectsChanged = 1;
    return;
} /* End of GLOD_Group::removeObject() **/


/*****************************************************************************\
 @ GLOD_Group::adaptErrorThreshold
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
GLOD_Group::adaptErrorThreshold()
{
	// let vds objects know that they haven't been adapted yet; the first one
	// that is adapted will adapt all the others and set this to true
	vds_objects_adapted = false;

    // Each object can adapt itself independently
#ifndef GLOD_USE_TILES
    currentNumTris=0;
#else
    for (int i=0; i<GLOD_NUM_TILES; i++)
	currentNumTris[i] = 0;
#endif
    switch(errorMode)
    {
    case ScreenSpace:
	for (int i=0; i<numObjects; i++)
	{
	    objects[i]->
		adaptScreenSpaceErrorThreshold(screenSpaceErrorThreshold);
#ifndef GLOD_USE_TILES
	    if (objects[i]->cut->currentErrorScreenSpace() > 0.f)
		currentNumTris += objects[i]->cut->currentNumTris;
#else
	    for (int j=0; j<GLOD_NUM_TILES; j++)
		if (objects[i]->cut->currentErrorScreenSpace() > 0.f)
		    currentNumTris[j] += objects[i]->cut->currentNumTris;
	    //currentNumTris += objects[i]->cut->currentNumTris;
#endif
	}
	break;
    case ObjectSpace:
	for (int i=0; i<numObjects; i++)
	{
	    objects[i]->
		adaptObjectSpaceErrorThreshold(objectSpaceErrorThreshold);
#ifndef GLOD_USE_TILES
	    if (objects[i]->cut->currentErrorScreenSpace() > 0.f)
		currentNumTris += objects[i]->cut->currentNumTris;
#else
	    for (int j=0; j<GLOD_NUM_TILES; j++)
		if (objects[i]->cut->currentErrorScreenSpace() > 0.f)
		    currentNumTris[j]+= objects[i]->cut->currentNumTris;
#endif
	    //currentNumTris += objects[i]->cut->currentNumTris;
	}
	break;
    default:
	fprintf(stderr,
		"GLOD_Group::adaptErrorThreshold(): unknown error mode\n");
	return;
	break;
    }

    return;

} /* End of GLOD_Group::adaptErrorThreshold() **/



#ifndef GLOD_USE_TILES

/*****************************************************************************\
 @ GLOD_Group::adaptTriangleBudget
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
GLOD_Group::adaptTriangleBudget()
{
	int TrisAfterRefine;
	float NewError;

	unsigned int LastObjectRefined = UINT_MAX - 1;
	float LastRefineErrorTermination;
	int LastRefineTriTermination;
	unsigned int LastObjectCoarsened = UINT_MAX - 1;
	float LastCoarsenErrorTermination;
	int LastCoarsenTriTermination;
  
    if (!firstBudgetAdapt && !objectsChanged &&	!budgetChanged &&
		(errorMode == ObjectSpace))
	{
		return;
	}

//printf("Beginning adaptTriangleBudget()\n");

	// update currentNumTris
	bool VDScutCounted = false;
	currentNumTris = 0;
    for (int i=0; i<numObjects; i++)
    {
		if (objects[i]->format == GLOD_VDS)
		{
			if (VDScutCounted == true)
				continue;
			else
				VDScutCounted = true;
			currentNumTris += objects[i]->cut->currentNumTris;
		}
		else
		{
			currentNumTris += objects[i]->cut->currentNumTris;
		}
    }

    // initialize (or re-inintialize) the budget algorithm
    refineQueue->clear();
    coarsenQueue->clear();

	bool VDScutAdded = false;
    for (int i=0; i<numObjects; i++)
    {
		GLOD_Object *obj = objects[i];
	//delete obj->budgetCoarsenHeapData;
	//delete obj->budgetRefineHeapData;
	if (obj->format!=GLOD_VDS){
	    obj->budgetCoarsenHeapData=HeapElement(obj);
	    obj->budgetRefineHeapData=HeapElement(obj);
	}
		// all VDS objects in a group share the same VDS::Simplifier, and
		// adapting this simplifier adapts all of the VDS objects at once
		// therefore, we only need a single entry in the queues for all 
		// VDS objects in the group; VDS will take care of distributing the
		// triangles to each object according to its current error.
		if (obj->format == GLOD_VDS)
		{
			if (VDScutAdded == true)
				continue;
			else
				VDScutAdded = true;
		}

		if (errorMode == ObjectSpace)
		{
			obj->budgetCoarsenHeapData.setKey(
				obj->cut->coarsenErrorObjectSpace());
			coarsenQueue->insert(&(objects[i]->budgetCoarsenHeapData));
		
			obj->budgetRefineHeapData.setKey(
				-obj->cut->currentErrorObjectSpace());
			refineQueue->insert(&(objects[i]->budgetRefineHeapData));
		}
		else
		{
			obj->budgetCoarsenHeapData.setKey(
				obj->cut->coarsenErrorScreenSpace());
			coarsenQueue->insert(&(objects[i]->budgetCoarsenHeapData));
	
			obj->budgetRefineHeapData.setKey(
				-obj->cut->currentErrorScreenSpace());
			refineQueue->insert(&(objects[i]->budgetRefineHeapData));
		}
    }

    firstBudgetAdapt = 0;
	objectsChanged = 0;
    
    // adapt all the objects to distribute the budgeted triangles while
    // minimizing the error
    GLOD_Object *coarsenTop, *refineTop;
    char roomToRefine, overBudget, queuesBalanced;

    coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
    refineTop = (GLOD_Object *)refineQueue->min()->userData();

	if (refineTop->cut->refineTris == MAXINT)
	{
		roomToRefine = 0;
	}
	else
	{
		TrisAfterRefine = currentNumTris - refineTop->cut->currentNumTris
			+ refineTop->cut->refineTris;

		roomToRefine = (TrisAfterRefine <= triBudget);
	}

    overBudget = ((currentNumTris > triBudget) &&
		  (coarsenTop->cut->coarsenErrorObjectSpace() != MAXFLOAT));

	if (errorMode == ObjectSpace)
	{
	    queuesBalanced = (coarsenTop->cut->coarsenErrorObjectSpace()
			>= refineTop->cut->currentErrorObjectSpace());
	}
	else
	{
		queuesBalanced = (coarsenTop->cut->coarsenErrorScreenSpace() 
			>= refineTop->cut->currentErrorScreenSpace());
	}
	
	if (errorMode == ScreenSpace)
	{
	  roomToRefine=1;
	  overBudget=1;
	}
#ifdef DEBUG_TRIADAPT
        printf("\n\nbeginning of adapt\n\n");
#endif
    while (roomToRefine || overBudget || !queuesBalanced)
    {
#ifdef DEBUG_TRIADAPT
        printf("\tCurrent Num Tris: %i, TrisAfterRefine: %i, Budget: %i\n",
               currentNumTris, TrisAfterRefine, triBudget);
        printf("\tCoarsen Queue Top Error: %.25f, Refine Queue Top Error: %.25f\n",
               (errorMode == ObjectSpace) ? coarsenTop->cut->coarsenErrorObjectSpace() :
               coarsenTop->cut->coarsenErrorScreenSpace(), (errorMode == ObjectSpace) ? 
               refineTop->cut->currentErrorObjectSpace() : 
               refineTop->cut->currentErrorScreenSpace());
        
        if (roomToRefine)
            printf("\troomToRefine: 1, ");
        else
            printf("\troomToRefine: 0, ");
        if (overBudget)
            printf("overBudget: 1, ");
        else
            printf("overBudget: 0, ");
        if (queuesBalanced)
            printf("queuesBalanced: 1\n");
        else
            printf("queuesBalanced: 0\n");
#endif
	// Refine first, then coarsen, so that coarsening is last (and thus
	// we end up within the budget)

	// refine
	while (roomToRefine)
	{
#ifdef DEBUG_TRIADAPT
	  printf("Beginning refine pass\n");
#endif
	    GLOD_Object *refineObj =
			(GLOD_Object *)refineQueue->extractMin()->userData();

	    GLOD_Object *nextRefine = (refineQueue->size() > 0) ?
			(GLOD_Object *)refineQueue->min()->userData() : NULL;

	    // apply refinement
	    int beforeTris = refineObj->cut->currentNumTris;
	    int triTermination = triBudget - (currentNumTris - beforeTris);

	    if (triTermination < 0)
	    {
	    	triTermination = refineObj->cut->refineTris;
	    }

	    float errorTermination = (nextRefine != NULL) ? 
			nextRefine->cut->currentErrorObjectSpace() : -MAXFLOAT;

	    if ((errorMode == ScreenSpace) && (errorTermination != -MAXFLOAT))
		{
			float nextRefineError = nextRefine->cut->currentErrorScreenSpace();			
			errorTermination =  nextRefineError;
		}
#ifdef DEBUG_TRIADAPT
            printf("refining %u\n", refineObj->name);
            printf("tri term %i errorTerm %f\n", triTermination, errorTermination);
#endif
            refineObj->cut->refine(errorMode, triTermination, errorTermination);

	    int afterTris = refineObj->cut->currentNumTris;
	    currentNumTris = currentNumTris - beforeTris + afterTris;

		if ((refineObj->name == LastObjectRefined) &&
			(triTermination == LastRefineTriTermination) &&
			(errorTermination == LastRefineErrorTermination))
		{
		//	printf("Duplicate refine call - euthanizing budget loop.\n");

			if (coarsenQueue->size() == 1)
				return;

			coarsenQueue->remove(&(refineObj->budgetCoarsenHeapData));
			refineTop= (GLOD_Object *)refineQueue->min()->userData();
		}
		else
		{
			LastObjectRefined = refineObj->name;
			LastRefineTriTermination = triTermination;
			LastRefineErrorTermination = errorTermination;

			// possible reasons that the refine terminated:
			//
			// a) error < errorTermination
			//    -- put back on refine queue (it won't be on top anymore)
			//
			// b) tris > triTermination
			//    -- put on coarsen queue (we will need to coarsen either
			//       this or something else to get within budget)
			//
			// c) no more refinements possible
			//    -- put on coarsen queue
			
			if (errorMode == ObjectSpace)
			{
			    refineObj->budgetRefineHeapData.setKey(
				  -refineObj->cut->currentErrorObjectSpace());
			}
			else
			{
				refineObj->budgetRefineHeapData.setKey(
					-refineObj->cut->currentErrorScreenSpace());
			}

			if (errorMode == ObjectSpace)
			{
				NewError = refineObj->cut->coarsenErrorObjectSpace();
			}
			else
			{
				NewError = refineObj->cut->coarsenErrorScreenSpace();
			}	

			refineQueue->insert(&(refineObj->budgetRefineHeapData));
			coarsenQueue->changeKey(&(refineObj->budgetCoarsenHeapData), NewError);
		}

	    coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
	    refineTop = (GLOD_Object *)refineQueue->min()->userData();

		TrisAfterRefine = (currentNumTris - refineTop->cut->currentNumTris +
			refineTop->cut->refineTris);

	    roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
			(TrisAfterRefine < triBudget);

	    overBudget = ((currentNumTris > triBudget) &&
			  (coarsenTop->cut->coarsenErrorObjectSpace() != MAXFLOAT));

	    //printf("over (refine)? %i %i\n", currentNumTris, triBudget);
	   
	    if (errorMode == ObjectSpace)
	      {
		queuesBalanced = (coarsenTop->cut->coarsenErrorObjectSpace()
				  >= refineTop->cut->currentErrorObjectSpace());
	      }
	    else
	      {
		float coarsenTopError = coarsenTop->cut->coarsenErrorScreenSpace();
		float refineTopError = refineTop->cut->currentErrorScreenSpace();
		queuesBalanced = (coarsenTopError >= refineTopError);
	      }
	    
	    if ((refineTop->cut->currentErrorScreenSpace()<0.000001f)&&(errorMode==ScreenSpace)&&(!overBudget)&&(queuesBalanced))
		return;

#ifdef DEBUG_TRIADAPT
printf("\tCurrent Num Tris: %i, TrisAfterRefine: %i, Budget: %i\n",
	currentNumTris, TrisAfterRefine, triBudget);
printf("before %i after %i rque: %i cque: %i\n", beforeTris, afterTris, refineQueue->size(), coarsenQueue->size());

printf("\tCoarsen Queue Top Error: %f, Refine Queue Top Error: %f\n",
	(errorMode == ObjectSpace) ? coarsenTop->cut->coarsenErrorObjectSpace() :
	coarsenTop->cut->coarsenErrorScreenSpace(), (errorMode == ObjectSpace) ? 
	refineTop->cut->currentErrorObjectSpace() : 
	refineTop->cut->currentErrorScreenSpace());
#endif
#if 1
	    
	    if (queuesBalanced)
	      {
		while ((!overBudget) && (!roomToRefine) && (refineQueue->size() > 1))
		  {

		    //		    printf("Queues Balanced; removing tops of refine and coarsen queues(refine)\n");

		    refineQueue->remove( &(refineTop->budgetRefineHeapData) );
		    coarsenQueue->remove( &(refineTop->budgetCoarsenHeapData) );
		    refineTop = (GLOD_Object *)refineQueue->min()->userData();
		    
		    TrisAfterRefine = (currentNumTris - 
				  refineTop->cut->currentNumTris + refineTop->cut->refineTris);
		    
		    roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
		      (TrisAfterRefine < triBudget);
		  }
	      }
#endif      

	} // end of refinement
#ifdef DEBUG_TRIADAPT
        printf("after refinement\n");
        if (roomToRefine)
            printf("\troomToRefine: 1, ");
        else
            printf("\troomToRefine: 0, ");
        if (overBudget)
            printf("overBudget: 1, ");
        else
            printf("overBudget: 0, ");
        if (queuesBalanced)
            printf("queuesBalanced: 1\n");
        else
            printf("queuesBalanced: 0\n");
#endif
        
	while (overBudget || !queuesBalanced)
	{
#ifdef DEBUG_TRIADAPT
	  printf("Beginning coarsen pass\n");
#endif
	    GLOD_Object *coarsenObj =
			(GLOD_Object *)coarsenQueue->extractMin()->userData();
	  
	    GLOD_Object *nextCoarsen = (coarsenQueue->size() > 0) ?
			(GLOD_Object *)coarsenQueue->min()->userData() : NULL;
		
	    // apply coarsening
	    
	    int beforeTris = coarsenObj->cut->currentNumTris;
	    /*
	    if (beforeTris==0&&(refineQueue->size()>1)){
	      refineQueue->remove( &(coarsenTop->budgetRefineHeapData) );
	      coarsenQueue->remove( &(coarsenTop->budgetCoarsenHeapData) );
	      refineTop = (GLOD_Object *)refineQueue->min()->userData();
	      coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
	      continue;
	      
	    }
	    else if (beforeTris==0&&(refineQueue->size()==1)){
	      */
	      
	    int triTermination = triBudget - (currentNumTris - beforeTris);
	    
	    float errorTermination = (nextCoarsen != NULL) ?
			nextCoarsen->cut->coarsenErrorObjectSpace() : MAXFLOAT;
	    //printf("tri term %i errorTerm(os) %f\n", triTermination, errorTermination);
	    if ((errorMode == ScreenSpace) && (errorTermination != MAXFLOAT))
	      {
		errorTermination = nextCoarsen->cut->coarsenErrorScreenSpace();
	      }
#ifdef DEBUG_TRIADAPT
	    printf("coarsening %u\n", coarsenObj->name);
	    printf("tri term %i errorTerm %f\n", triTermination, errorTermination);
#endif

		if (triTermination >= beforeTris)
			triTermination = 0;

	    coarsenObj->cut->coarsen(errorMode, triTermination, errorTermination);
		
	    int afterTris = coarsenObj->cut->currentNumTris;
	    currentNumTris = currentNumTris - beforeTris + afterTris;


		if ((coarsenObj->name == LastObjectCoarsened) &&
			(triTermination == LastCoarsenTriTermination) &&
			(errorTermination == LastCoarsenErrorTermination))
		{
      //              printf("Duplicate coarsen call - euthanizing budget loop.\n");

			if (refineQueue->size() == 1)
				return;

			refineQueue->remove(&(coarsenObj->budgetRefineHeapData));
			refineTop= (GLOD_Object *)refineQueue->min()->userData();
		}
		else
		{
			LastObjectCoarsened = coarsenObj->name;
			LastCoarsenTriTermination = triTermination;
			LastCoarsenErrorTermination = errorTermination;

			// possible reasons that the coarsen terminated:
			//
			// a) error > errorTermination
			//    -- put back on coarsen queue (it won't be on top anymore)
			//
			// b) tris <= triTermination
			//    -- put on refinement queue
			//
 			// c) no more coarsening possible
			//    -- put on refinement queue

			if (errorMode == ObjectSpace)
			{
				coarsenObj->budgetCoarsenHeapData.setKey(
					coarsenObj->cut->coarsenErrorObjectSpace());
			}
			else
			{
				coarsenObj->budgetCoarsenHeapData.setKey(
					coarsenObj->cut->coarsenErrorScreenSpace());
			}
			coarsenQueue->insert(&(coarsenObj->budgetCoarsenHeapData));

			if (errorMode == ObjectSpace)
			{
				NewError = -coarsenObj->cut->currentErrorObjectSpace();
			}
			else
			{
				NewError = -coarsenObj->cut->currentErrorScreenSpace();
			}

			refineQueue->changeKey(&(coarsenObj->budgetRefineHeapData), NewError);
		}
	    
	    coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
	    refineTop = (GLOD_Object *)refineQueue->min()->userData();
	    
		TrisAfterRefine = (currentNumTris - refineTop->cut->currentNumTris +
			  refineTop->cut->refineTris);
	    roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
			(TrisAfterRefine < triBudget);

	    overBudget = ((currentNumTris > triBudget) && 
			(coarsenTop->cut->coarsenErrorObjectSpace() != MAXFLOAT));

	    if (triBudget<0)
	      overBudget=0;

	    if (errorMode == ObjectSpace)
	      {
		
		queuesBalanced = (coarsenTop->cut->coarsenErrorObjectSpace() 
				  >= refineTop->cut->currentErrorObjectSpace());
	      }
	    else
	      {
		queuesBalanced = (coarsenTop->cut->coarsenErrorScreenSpace()
			  >= refineTop->cut->currentErrorScreenSpace());
	      }
#ifdef DEBUG_TRIADAPT    
            printf("\tCurrent Num Tris: %i, TrisAfterRefine: %i, Budget: %i, ref %i, %i\n",
                   currentNumTris, TrisAfterRefine, triBudget, refineTop->cut->refineTris, MAXINT);
            printf("before %i after %i rque: %i cque: %i\n", beforeTris, afterTris, refineQueue->size(), coarsenQueue->size());
            printf("\tCoarsen Queue Top Error: %.50f, Refine Queue Top Error: %.50f\n",
                   (errorMode == ObjectSpace) ? coarsenTop->cut->coarsenErrorObjectSpace() :
                   coarsenTop->cut->coarsenErrorScreenSpace(), (errorMode == ObjectSpace) ? 
                   refineTop->cut->currentErrorObjectSpace() : 
                   refineTop->cut->currentErrorScreenSpace());
#endif
	    if (coarsenQueue->size()==1)
	      queuesBalanced=1;
	    
	   /* 
	    if (refineTop->cut->currentErrorScreenSpace()==0){
		printf("arrrrr %i %f\n", coarsenTop->cut->currentNumTris, coarsenTop->cut->coarsenErrorObjectSpace());
		printf("cq %i rq %i\n", coarsenQueue->size(), refineQueue->size());
		printf("%i %i %i\n", overBudget, roomToRefine, queuesBalanced);
		printf("%i %i\n", currentNumTris, triBudget);
	    }
	    */
	    
	    if ((refineTop->cut->currentErrorScreenSpace()<0.000001f)&&(errorMode==ScreenSpace)&&(!overBudget)&&(queuesBalanced))
		return;
	    
#if 1
	    if (queuesBalanced)
		{
			while (!overBudget&&!roomToRefine&&(refineQueue->size()>1))
			{

			  //	  printf("Queues Balanced; removing tops of refine and coarsen queues(coarsen)\n");

				refineQueue->remove(&(refineTop->budgetRefineHeapData));
				coarsenQueue->remove(&(refineTop->budgetCoarsenHeapData));
				refineTop= (GLOD_Object *)refineQueue->min()->userData();

				TrisAfterRefine = (currentNumTris 
					- refineTop->cut->currentNumTris 
					+ refineTop->cut->refineTris);

				roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
					(TrisAfterRefine < triBudget);
			}
		}
#endif    
	} // end of coarsening
	
//	if (!queuesBalanced&&!roomToRefine&&!overBudget){
//	  roomToRefine=1;
//
//	}
	
    }
    
	

    budgetChanged = 0;

#if 0
    fprintf(stderr, "adaptTriangleBudget(): ");
    fprintf(stderr, "budget: %d, tris: %d\n",
    triBudget, currentNumTris);
#endif
    
    return;
} /* End of GLOD_Group::adaptTriangleBudget() **/

#else
#if 0
/*****************************************************************************\
 @ GLOD_Group::adaptTriangleBudget
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
GLOD_Group::adaptTriangleBudget()
{
	int TrisAfterRefine;
	float NewError;

	unsigned int LastObjectRefined = UINT_MAX - 1;
	float LastRefineErrorTermination;
	int LastRefineTriTermination;
	unsigned int LastObjectCoarsened = UINT_MAX - 1;
	float LastCoarsenErrorTermination;
	int LastCoarsenTriTermination;
  /*
    if (!firstBudgetAdapt && !objectsChanged &&	!budgetChanged &&
		(errorMode == ObjectSpace))
	{
		return;
	}
*/
//printf("Beginning adaptTriangleBudget()\n");

	// update currentNumTris
	bool VDScutCounted = false;
	//currentNumTris = 0;
	for (int i=0; i<GLOD_NUM_TILES; i++)
	    currentNumTris[i]=0;
    /*
    for (int i=0; i<numObjects; i++)
    {
		if (objects[i]->format == GLOD_VDS)
		{
			if (VDScutCounted == true)
				continue;
			else
				VDScutCounted = true;
			currentNumTris += objects[i]->cut->currentNumTris;
		}
		//else
		//{
		//	currentNumTris += objects[i]->cut->currentNumTris;
		//}
    }
*/
    // initialize (or re-inintialize) the budget algorithm
    refineQueue->clear();
    coarsenQueue->clear();

	bool VDScutAdded = false;
    for (int i=0; i<numObjects; i++)
    {
		GLOD_Object *obj = objects[i];
	for (int j=0; j<GLOD_NUM_TILES; j++)
	    obj->inArea[j]=0;

	//delete obj->budgetCoarsenHeapData;
	//delete obj->budgetRefineHeapData;
	/*
	if (obj->format!=GLOD_VDS){
	    obj->budgetCoarsenHeapData=HeapElement(obj);
	    obj->budgetRefineHeapData=HeapElement(obj);
	    obj->inArea=new int[GLOD_NUM_TILES];
	}
	*/
	if (obj->cut->currentErrorScreenSpace()<0.000001f){
	    obj->cut->coarsen(ObjectSpace, 0.0, MAXFLOAT);
	    for (int j=0; j<GLOD_NUM_TILES; j++)
		obj->inArea[j]=0;
	    ///obj->numAreas=0;
	    continue;
	}
	else {
	    //obj->cut->coarsen(ObjectSpace, 0.0, MAXFLOAT);
	    for (int j=0; j<GLOD_NUM_TILES; j++)
		if (obj->cut->currentErrorScreenSpace(j)>0.f){
		    obj->inArea[j]=1;
		    //obj->numAreas++;
		    currentNumTris[j] += obj->cut->currentNumTris;
		    //printf("%i %i %i %i\n", obj->name, j,currentNumTris[j], objects[i]->inArea[j]);
		}
		else 
		    obj->inArea[j]=0;
	
		// all VDS objects in a group share the same VDS::Simplifier, and
		// adapting this simplifier adapts all of the VDS objects at once
		// therefore, we only need a single entry in the queues for all 
		// VDS objects in the group; VDS will take care of distributing the
		// triangles to each object according to its current error.
		if (obj->format == GLOD_VDS)
		{
			if (VDScutAdded == true)
				continue;
			else
				VDScutAdded = true;
		}

		if (errorMode == ObjectSpace)
		{
			obj->budgetCoarsenHeapData.setKey(
				obj->cut->coarsenErrorObjectSpace());
			coarsenQueue->insert(&(objects[i]->budgetCoarsenHeapData));
		
			obj->budgetRefineHeapData.setKey(
				-obj->cut->currentErrorObjectSpace());
			refineQueue->insert(&(objects[i]->budgetRefineHeapData));
		}
		else
		{
			obj->budgetCoarsenHeapData.setKey(
				obj->cut->coarsenErrorScreenSpace());
			coarsenQueue->insert(&(objects[i]->budgetCoarsenHeapData));
	
			obj->budgetRefineHeapData.setKey(
				-obj->cut->currentErrorScreenSpace());
			refineQueue->insert(&(objects[i]->budgetRefineHeapData));
		}
	}
    }

    firstBudgetAdapt = 0;
	objectsChanged = 0;
    
    // adapt all the objects to distribute the budgeted triangles while
    // minimizing the error
    GLOD_Object *coarsenTop, *refineTop;
    char roomToRefine, overBudget, queuesBalanced;
    //roomToRefine=new char[GLOD_NUM_TILES];
    //overBudget=new char[GLOD_NUM_TILES];
    if ((coarsenQueue->size()==0) || (refineQueue->size()==0))
	return;
    coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
    refineTop = (GLOD_Object *)refineQueue->min()->userData();
    roomToRefine=1;
    overBudget=0;
    for (int i=0; i<GLOD_NUM_TILES; i++){
	if (coarsenTop->inArea[i]==0) continue;
	
	if (!overBudget)
	    overBudget = ((currentNumTris[i] > triBudget[i]) &&
			  (coarsenTop->cut->coarsenErrorObjectSpace() != MAXFLOAT));
	
	//printf("over (refine)? %i %i\n", currentNumTris, triBudget);
	
	if (errorMode == ObjectSpace)
	{
	    queuesBalanced = (coarsenTop->cut->coarsenErrorObjectSpace()
			      >= refineTop->cut->currentErrorObjectSpace());
	    //printf("%f %f\n", coarsenTop->cut->coarsenErrorObjectSpace(), refineTop->cut->currentErrorObjectSpace());
	}
	else
	{
	    float coarsenTopError = coarsenTop->cut->coarsenErrorScreenSpace();
	    float refineTopError = refineTop->cut->currentErrorScreenSpace();
	    queuesBalanced = (coarsenTopError >= refineTopError);
	}
    }
    
	
	//if (errorMode == ScreenSpace)
	//{
	//  roomToRefine=1;
	//  overBudget=1;
	//}
#if 0
	  {
	      float minError[GLOD_NUM_TILES], maxError[GLOD_NUM_TILES], meanError; 
	      int numObj[GLOD_NUM_TILES], curRealTris[GLOD_NUM_TILES];
	      for (int i=0; i<GLOD_NUM_TILES; i++){
		  minError[i]=MAXFLOAT; maxError[i]=0;numObj[i]=0; curRealTris[i]=0;
	      }
	      meanError=0;
	      for (int i=0; i<numObjects; i++){
		  for (int j=0; j<GLOD_NUM_TILES; j++){
		      if (objects[i]->inArea[j]==0) continue;
		      float curError=(errorMode==ObjectSpace)?objects[i]->cut->currentErrorObjectSpace():objects[i]->cut->currentErrorScreenSpace();
		      if (curError<minError[j])
			  minError[j]=curError;
		      if (curError>maxError[j])
			  maxError[j]=curError;
		      curRealTris[j]+=objects[i]->cut->currentNumTris;
		      numObj[j]++;
		  }
	      }
	      for (int i=0; i<GLOD_NUM_TILES; i++){
		  if (minError[i]==MAXFLOAT) minError[i]=0;
		  meanError+=(maxError[i]+minError[i])/2.0f;
	      }
	      meanError/=(float)(GLOD_NUM_TILES);
	      for (int i=0; i<GLOD_NUM_TILES; i++){
		  fprintf(stderr, "initial area %i budget: %d, curTris %i, realTris %i numObjs %i\n	    minError %f, maxError %f, meanError %f\n",
			  i, triBudget[i], currentNumTris[i], curRealTris[i], numObj[i], minError[i], maxError[i], meanError);
	      }
	  }
#endif
	  
	while (overBudget || !queuesBalanced)
	{

	  //printf("Beginning coarsen pass %i %i \n", overBudget, queuesBalanced);
	    if (coarsenQueue->size()==0) break;
	    GLOD_Object *coarsenObj =
			(GLOD_Object *)coarsenQueue->extractMin()->userData();
	  
	    GLOD_Object *nextCoarsen = (coarsenQueue->size() > 0) ?
			(GLOD_Object *)coarsenQueue->min()->userData() : NULL;
		
	    // apply coarsening
	    
	    int beforeTris = coarsenObj->cut->currentNumTris;
	    /*
	    if (beforeTris==0&&(refineQueue->size()>1)){
	      refineQueue->remove( &(coarsenTop->budgetRefineHeapData) );
	      coarsenQueue->remove( &(coarsenTop->budgetCoarsenHeapData) );
	      refineTop = (GLOD_Object *)refineQueue->min()->userData();
	      coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
	      continue;
	      
	    }
	    else if (beforeTris==0&&(refineQueue->size()==1)){
	      */
	    int triTermination=MAXINT;
	    for (int i=0; i<GLOD_NUM_TILES; i++)
		if (coarsenObj->inArea[i])
		    if ((triBudget[i] - (currentNumTris[i] - beforeTris))<triTermination)
			triTermination=triBudget[i] - (currentNumTris[i] - beforeTris);
	    
	    //int triTermination = triBudget - (currentNumTris - beforeTris);
	    
	    float errorTermination = (nextCoarsen != NULL) ?
			nextCoarsen->cut->coarsenErrorObjectSpace() : MAXFLOAT;
	    //printf("tri term %i errorTerm(os) %f\n", triTermination, errorTermination);
	    if ((errorMode == ScreenSpace) && (errorTermination != MAXFLOAT))
	      {
		errorTermination = nextCoarsen->cut->coarsenErrorScreenSpace();
	      }
	    
//	    printf("name %i tri %i  tri term %i errorTerm %f\n", coarsenObj->name, coarsenObj->cut->currentNumTris, triTermination, errorTermination);


		if (triTermination >= beforeTris)
			triTermination = 0;

	    coarsenObj->cut->coarsen(errorMode, triTermination, errorTermination);

//printf("coarsening %u\n", coarsenObj->name);
		
	    int afterTris = coarsenObj->cut->currentNumTris;
	    for (int i=0; i<GLOD_NUM_TILES; i++)
		if (coarsenObj->inArea[i]==1){
		    //printf("area %i cur %i before %i after %i", i, currentNumTris[i], beforeTris, afterTris);
		    currentNumTris[i] = currentNumTris[i] - beforeTris + afterTris;
		    //printf(" after cur %i\n", currentNumTris[i]);
		}

		if (((coarsenObj->name == LastObjectCoarsened) &&
			(triTermination == LastCoarsenTriTermination) &&
			(errorTermination == LastCoarsenErrorTermination)) || (beforeTris==afterTris))
		{
//			printf("Duplicate coarsen call - euthanizing budget loop.\n");

			if (coarsenQueue->size() <= 1)
			    break;
				//return;

			coarsenQueue->remove(&(coarsenObj->budgetCoarsenHeapData));
			coarsenTop= (GLOD_Object *)coarsenQueue->min()->userData();
		}
		else
		{
			LastObjectCoarsened = coarsenObj->name;
			LastCoarsenTriTermination = triTermination;
			LastCoarsenErrorTermination = errorTermination;

			// possible reasons that the coarsen terminated:
			//
			// a) error > errorTermination
			//    -- put back on coarsen queue (it won't be on top anymore)
			//
			// b) tris <= triTermination
			//    -- put on refinement queue
			//
 			// c) no more coarsening possible
			//    -- put on refinement queue

			if (errorMode == ObjectSpace)
			{
				coarsenObj->budgetCoarsenHeapData.setKey(
					coarsenObj->cut->coarsenErrorObjectSpace());
			}
			else
			{
				coarsenObj->budgetCoarsenHeapData.setKey(
					coarsenObj->cut->coarsenErrorScreenSpace());
			}
			coarsenQueue->insert(&(coarsenObj->budgetCoarsenHeapData));

			if (errorMode == ObjectSpace)
			{
				NewError = -coarsenObj->cut->currentErrorObjectSpace();
			}
			else
			{
				NewError = -coarsenObj->cut->currentErrorScreenSpace();
			}

			refineQueue->changeKey(&(coarsenObj->budgetRefineHeapData), NewError);
		}
	    
	    coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
	    refineTop = (GLOD_Object *)refineQueue->min()->userData();
	    roomToRefine=1;
	    overBudget=0;
	    for (int i=0; i<GLOD_NUM_TILES; i++){
		if (coarsenTop->inArea[i]==0) continue;
		
		TrisAfterRefine = (currentNumTris[i] - refineTop->cut->currentNumTris +
				   refineTop->cut->refineTris);
		
		if (roomToRefine)
		    roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
			(TrisAfterRefine < triBudget[i]);
		
		if (!overBudget)
		    overBudget = ((currentNumTris[i] > triBudget[i]) &&
				  (coarsenTop->cut->coarsenErrorObjectSpace() != MAXFLOAT));
		
		//printf("over (refine)? %i %i\n", currentNumTris, triBudget);
		
		if (errorMode == ObjectSpace)
		{
		    queuesBalanced = (coarsenTop->cut->coarsenErrorObjectSpace()
				      >= refineTop->cut->currentErrorObjectSpace());
		    //printf("%f %f\n", coarsenTop->cut->coarsenErrorObjectSpace(), refineTop->cut->currentErrorObjectSpace());
		}
		else
		{
		    float coarsenTopError = coarsenTop->cut->coarsenErrorScreenSpace();
		    float refineTopError = refineTop->cut->currentErrorScreenSpace();
		    queuesBalanced = (coarsenTopError >= refineTopError);
		}
	    }
	    /*
printf("\tCurrent Num Tris: %i, TrisAfterRefine: %i, Budget: %i, ref %i, %i\n",
	currentNumTris, TrisAfterRefine, triBudget, refineTop->cut->refineTris, MAXINT);
 printf("before %i after %i rque: %i cque: %i\n", beforeTris, afterTris, refineQueue->size(), coarsenQueue->size());
printf("\tCoarsen Queue Top Error: %.50f, Refine Queue Top Error: %.50f\n",
	(errorMode == ObjectSpace) ? coarsenTop->cut->coarsenErrorObjectSpace() :
	coarsenTop->cut->coarsenErrorScreenSpace(), (errorMode == ObjectSpace) ? 
	refineTop->cut->currentErrorObjectSpace() : 
	refineTop->cut->currentErrorScreenSpace());
	    */
	    if (coarsenQueue->size()==1)
	      queuesBalanced=1;
	    
	   /* 
	    if (refineTop->cut->currentErrorScreenSpace()==0){
		printf("arrrrr %i %f\n", coarsenTop->cut->currentNumTris, coarsenTop->cut->coarsenErrorObjectSpace());
		printf("cq %i rq %i\n", coarsenQueue->size(), refineQueue->size());
		printf("%i %i %i\n", overBudget, roomToRefine, queuesBalanced);
		printf("%i %i\n", currentNumTris, triBudget);
	    }
	    */
	    if (coarsenTop->cut->currentErrorScreenSpace()<0.000001f)
		break;
	    if ((refineTop->cut->currentErrorScreenSpace()<0.000001f)&&(errorMode==ScreenSpace)&&(!overBudget)&&(queuesBalanced))
		return;
	    
#if 0
	    if (queuesBalanced)
		{
			while (/*!overBudget&&*/!roomToRefine&&(refineQueue->size()>1))
			{

			  //	  printf("Queues Balanced; removing tops of refine and coarsen queues(coarsen)\n");

				refineQueue->remove(&(refineTop->budgetRefineHeapData));
				coarsenQueue->remove(&(refineTop->budgetCoarsenHeapData));
				refineTop= (GLOD_Object *)refineQueue->min()->userData();

				TrisAfterRefine = (currentNumTris 
					- refineTop->cut->currentNumTris 
					+ refineTop->cut->refineTris);

				roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
					(TrisAfterRefine < triBudget);
			}
		}
#endif  
	} // end of coarsening
	
#if 0
	{
	float minError[GLOD_NUM_TILES], maxError[GLOD_NUM_TILES], meanError; 
	int numObj[GLOD_NUM_TILES], curRealTris[GLOD_NUM_TILES];
	for (int i=0; i<GLOD_NUM_TILES; i++){
	    minError[i]=MAXFLOAT; maxError[i]=0;numObj[i]=0; curRealTris[i]=0;
	}
	meanError=0;
	for (int i=0; i<numObjects; i++){
	    for (int j=0; j<GLOD_NUM_TILES; j++){
		if (objects[i]->inArea[j]==0) continue;
		float curError=(errorMode==ObjectSpace)?objects[i]->cut->currentErrorObjectSpace():objects[i]->cut->currentErrorScreenSpace();
		if (curError<minError[j])
		    minError[j]=curError;
		if (curError>maxError[j])
		    maxError[j]=curError;
		curRealTris[j]+=objects[i]->cut->currentNumTris;
		numObj[j]++;
	    }
	}
	for (int i=0; i<GLOD_NUM_TILES; i++){
	    if (minError[i]==MAXFLOAT) minError[i]=0;
	    meanError+=(maxError[i]+minError[i])/2.0f;
	}
	meanError/=(float)(GLOD_NUM_TILES);
	for (int i=0; i<GLOD_NUM_TILES; i++){
	    fprintf(stderr, "after coarsen area %i budget: %d, curTris %i, realTris %i numObjs %i\n	    minError %f, maxError %f, meanError %f\n",
		    i, triBudget[i], currentNumTris[i], curRealTris[i], numObj[i], minError[i], maxError[i], meanError);
	}
	}
#endif
	
	while (roomToRefine)
	{
	    
	    //printf("Beginning refine pass\n");
	    if (refineQueue->size()==0) break;
	    GLOD_Object *refineObj =
	    (GLOD_Object *)refineQueue->extractMin()->userData();
	    GLOD_Object *nextRefine = (refineQueue->size() > 0) ?
		(GLOD_Object *)refineQueue->min()->userData() : NULL;
	    
	    // apply refinement
	    int beforeTris = refineObj->cut->currentNumTris;
	    int triTermination=MAXINT;
	    for (int i=0; i<GLOD_NUM_TILES; i++)
		if (refineObj->inArea[i])
		    if ((triBudget[i] - (currentNumTris[i] - beforeTris))<triTermination)
			triTermination=triBudget[i] - (currentNumTris[i] - beforeTris);
	    
	    if (triTermination < 0)
	    {
	    	triTermination = refineObj->cut->refineTris;
	    }
	    
	    float errorTermination = (nextRefine != NULL) ? 
		nextRefine->cut->currentErrorObjectSpace() : -MAXFLOAT;
	    
	    if ((errorMode == ScreenSpace) && (errorTermination != -MAXFLOAT))
	    {
		float nextRefineError = nextRefine->cut->currentErrorScreenSpace();			
		errorTermination =  nextRefineError;
	    }
	    
	    refineObj->cut->refine(errorMode, triTermination, errorTermination);
	    
	    //printf("refining %u\n", refineObj->name);
	    
	    int afterTris = refineObj->cut->currentNumTris;
	    for (int i=0; i<GLOD_NUM_TILES; i++)
		if (refineObj->inArea[i]==1){
//		    printf("name %i area %i cur %i before %i after %i", refineObj->name, i, currentNumTris[i], beforeTris, afterTris);
		    currentNumTris[i] = currentNumTris[i] - beforeTris + afterTris;
//		    printf(" after cur %i\n", currentNumTris[i]);
		}
	    if (((refineObj->name == LastObjectRefined) &&
		(triTermination == LastRefineTriTermination) &&
		(errorTermination == LastRefineErrorTermination))) // || (beforeTris==afterTris))
	    {
		//			printf("Duplicate refine call - euthanizing budget loop.\n");
		
		if (refineQueue->size() <= 1)
		    break;
		refineQueue->remove(&(refineObj->budgetRefineHeapData));
		refineTop= (GLOD_Object *)refineQueue->min()->userData();
	    }
	    else
	    {
		LastObjectRefined = refineObj->name;
		LastRefineTriTermination = triTermination;
		LastRefineErrorTermination = errorTermination;
		
		// possible reasons that the refine terminated:
		//
		// a) error < errorTermination
		//    -- put back on refine queue (it won't be on top anymore)
		//
		// b) tris > triTermination
		//    -- put on coarsen queue (we will need to coarsen either
		//       this or something else to get within budget)
		//
		// c) no more refinements possible
		//    -- put on coarsen queue
		
		if (errorMode == ObjectSpace)
		{
		    refineObj->budgetRefineHeapData.setKey(
							   -refineObj->cut->currentErrorObjectSpace());
		}
		else
		{
		    refineObj->budgetRefineHeapData.setKey(
							   -refineObj->cut->currentErrorScreenSpace());
		}
		
		if (errorMode == ObjectSpace)
		{
		    NewError = refineObj->cut->coarsenErrorObjectSpace();
		}
		else
		{
		    NewError = refineObj->cut->coarsenErrorScreenSpace();
		}	
		
		refineQueue->insert(&(refineObj->budgetRefineHeapData));
		//coarsenQueue->changeKey(&(refineObj->budgetCoarsenHeapData), NewError);
		}
		//    printf("coarsen %i refine %i\n", coarsenQueue->size(), refineQueue->size());
	    //coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
	    refineTop = (GLOD_Object *)refineQueue->min()->userData();
	    roomToRefine=1;
	    overBudget=0;
	    for (int i=0; i<GLOD_NUM_TILES; i++){
		if (refineTop->inArea[i]==0) continue;
		TrisAfterRefine = (currentNumTris[i] - refineTop->cut->currentNumTris +
				   refineTop->cut->refineTris);
		if (roomToRefine)
		    roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
			(TrisAfterRefine < triBudget[i]);
		/*
		if (!overBudget)
		    overBudget = ((currentNumTris[i] > triBudget[i]) && (coarsenQueue->size()>0) &&
				  (coarsenTop->cut->coarsenErrorObjectSpace() != MAXFLOAT));
		
		//printf("over (refine)? %i %i\n", currentNumTris, triBudget);
		
		if (errorMode == ObjectSpace)
		{
		    queuesBalanced = (coarsenTop->cut->coarsenErrorObjectSpace()
				      >= refineTop->cut->currentErrorObjectSpace());
		}
		else
		{
		    float coarsenTopError = coarsenTop->cut->coarsenErrorScreenSpace();
		    float refineTopError = refineTop->cut->currentErrorScreenSpace();
		    queuesBalanced = (coarsenTopError >= refineTopError);
		}
		 */
	    }
	    if ((refineTop->cut->currentErrorScreenSpace()<0.000001f)&&(errorMode==ScreenSpace)&&(!overBudget)&&(queuesBalanced))
		return;
	    
	    /*
	     printf("\tCurrent Num Tris: %i, TrisAfterRefine: %i, Budget: %i\n",
		    currentNumTris, TrisAfterRefine, triBudget);
	     
	     
	     printf("\tCoarsen Queue Top Error: %f, Refine Queue Top Error: %f\n",
		    (errorMode == ObjectSpace) ? coarsenTop->cut->coarsenErrorObjectSpace() :
		    coarsenTop->cut->coarsenErrorScreenSpace(), (errorMode == ObjectSpace) ? 
		    refineTop->cut->currentErrorObjectSpace() : 
		    refineTop->cut->currentErrorScreenSpace());
	     */
#if 1
	    
	    ///if (queuesBalanced)
	    //{
		while (/*(!overBudget) && */(!roomToRefine) && (refineQueue->size() > 1))
		{
		    
		    //		    printf("Queues Balanced; removing tops of refine and coarsen queues(refine)\n");
		    
		    refineQueue->remove( &(refineTop->budgetRefineHeapData) );
		    //coarsenQueue->remove( &(refineTop->budgetCoarsenHeapData) );
		    refineTop = (GLOD_Object *)refineQueue->min()->userData();
		    for (int i=0; i<GLOD_NUM_TILES; i++){
			if (refineTop->inArea[i]){
			    TrisAfterRefine = (currentNumTris[i] - 
				       refineTop->cut->currentNumTris + refineTop->cut->refineTris);
			    roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
					(TrisAfterRefine < triBudget[i]);
			    if (roomToRefine==0)
				break;
			}
		    }
		}
		if (refineQueue->size()<1)
		    roomToRefine=0;
	    //}
#endif      
	    
	} // end of refinement
	
	
//	if (!queuesBalanced&&!roomToRefine&&!overBudget){
//	  roomToRefine=1;
//
//	}
	
    //}
    
	

    budgetChanged = 0;
    
	float minError[GLOD_NUM_TILES], maxError[GLOD_NUM_TILES], meanTileError[GLOD_NUM_TILES], meanError; 
	int numObj[GLOD_NUM_TILES], curRealTris[GLOD_NUM_TILES];
	for (int i=0; i<GLOD_NUM_TILES; i++){
	    minError[i]=MAXFLOAT; maxError[i]=0;numObj[i]=0; curRealTris[i]=0;
	}
	meanError=0;
	for (int i=0; i<numObjects; i++){
	    for (int j=0; j<GLOD_NUM_TILES; j++){
		if (objects[i]->inArea[j]==0) continue;
		float curError=(errorMode==ObjectSpace)?objects[i]->cut->currentErrorObjectSpace():objects[i]->cut->currentErrorScreenSpace();
		if (curError<minError[j])
		    minError[j]=curError;
		if (curError>maxError[j])
		    maxError[j]=curError;
		curRealTris[j]+=objects[i]->cut->currentNumTris;
		numObj[j]++;
	    }
	}
	for (int i=0; i<GLOD_NUM_TILES; i++){
	    if (minError[i]==MAXFLOAT) minError[i]=0;
	    meanTileError[i]=(maxError[i]+minError[i])/2.0f;
	    meanError+=(maxError[i]+minError[i])/2.0f;
	}
	meanError/=(float)(GLOD_NUM_TILES);
	glodAdjustTiles(meanTileError);
#if 0
	char fname[20];
	if (errorMode==ScreenSpace)
	    sprintf(fname, "%iv%itriSS.csv", GLOD_NUM_TILES, triBudget[0]);
	else
	    sprintf(fname, "%iv%itriOS.csv", GLOD_NUM_TILES, triBudget[0]);
	//FILE *output = fopen(fname, "a");
	for (int i=0; i<GLOD_NUM_TILES; i++){
	    //fprintf(stderr, "adaptTriangleBudget(): \n");
	    //fprintf(output, "%i,%f,",currentNumTris[i], (minError[i]+maxError[i])/2.0f);
	    if (currentNumTris[i]!=curRealTris[i]) printf("\n\nARG\n\n");
	    fprintf(stderr, "area %i budget: %d, curTris %i, realTris %i numObjs %i\n	    minError %f, maxError %f, meanError %f\n",
		i, triBudget[i], currentNumTris[i], curRealTris[i], numObj[i], minError[i], maxError[i], meanError);
	}
    //fprintf(output, "%f\n", meanError);
    //fclose(output);
#endif
    
    return;
} /* End of GLOD_Group::adaptTriangleBudget() **/
#else
    void
GLOD_Group::adaptTriangleBudget()
{
	int TrisAfterRefine;
	float NewError;

	unsigned int LastObjectRefined = UINT_MAX - 1;
	float LastRefineErrorTermination;
	int LastRefineTriTermination;
	unsigned int LastObjectCoarsened = UINT_MAX - 1;
	float LastCoarsenErrorTermination;
	int LastCoarsenTriTermination;
  /*
    if (!firstBudgetAdapt && !objectsChanged &&	!budgetChanged &&
		(errorMode == ObjectSpace))
	{
		return;
	}
*/
//printf("Beginning adaptTriangleBudget()\n");

	// update currentNumTris
	bool VDScutCounted = false;
	//currentNumTris = 0;
	for (int i=0; i<GLOD_NUM_TILES; i++)
	    currentNumTris[i]=0;
    /*
    for (int i=0; i<numObjects; i++)
    {
		if (objects[i]->format == GLOD_VDS)
		{
			if (VDScutCounted == true)
				continue;
			else
				VDScutCounted = true;
			currentNumTris += objects[i]->cut->currentNumTris;
		}
		//else
		//{
		//	currentNumTris += objects[i]->cut->currentNumTris;
		//}
    }
*/
    // initialize (or re-inintialize) the budget algorithm
    refineQueue->clear();
    coarsenQueue->clear();

	bool VDScutAdded = false;
    for (int i=0; i<numObjects; i++)
    {
		GLOD_Object *obj = objects[i];
	for (int j=0; j<GLOD_NUM_TILES; j++)
	    obj->inArea[j]=0;

	//delete obj->budgetCoarsenHeapData;
	//delete obj->budgetRefineHeapData;
	if (obj->format!=GLOD_VDS){
	    obj->budgetCoarsenHeapData=HeapElement(obj);
	    obj->budgetRefineHeapData=HeapElement(obj);
	}
	
	//if (obj->cut->currentErrorScreenSpace()==0){
	    obj->cut->coarsen(ObjectSpace, 0.0, MAXFLOAT);
	//    for (int i=0; i<GLOD_NUM_TILES; i++)
	//	obj->inArea[i]=0;
	    ///obj->numAreas=0;
	  //  continue;
	//}
	//else {
	    for (int j=0; j<GLOD_NUM_TILES; j++)
		if (obj->cut->currentErrorScreenSpace(j) > 0.f){
		    obj->inArea[j]=1;
		    //obj->numAreas++;
		    currentNumTris[j] += obj->cut->currentNumTris;
		    //printf("%i %i %i\n", obj->name, i,currentNumTris[i]);
		}
	//}
		// all VDS objects in a group share the same VDS::Simplifier, and
		// adapting this simplifier adapts all of the VDS objects at once
		// therefore, we only need a single entry in the queues for all 
		// VDS objects in the group; VDS will take care of distributing the
		// triangles to each object according to its current error.
		if (obj->format == GLOD_VDS)
		{
			if (VDScutAdded == true)
				continue;
			else
				VDScutAdded = true;
		}

		if (errorMode == ObjectSpace)
		{
			obj->budgetCoarsenHeapData.setKey(
				obj->cut->coarsenErrorObjectSpace());
			coarsenQueue->insert(&(objects[i]->budgetCoarsenHeapData));
		
			obj->budgetRefineHeapData.setKey(
				-obj->cut->currentErrorObjectSpace());
			refineQueue->insert(&(objects[i]->budgetRefineHeapData));
		}
		else
		{
			obj->budgetCoarsenHeapData.setKey(
				obj->cut->coarsenErrorScreenSpace());
			coarsenQueue->insert(&(objects[i]->budgetCoarsenHeapData));
	
			obj->budgetRefineHeapData.setKey(
				-obj->cut->currentErrorScreenSpace());
			refineQueue->insert(&(objects[i]->budgetRefineHeapData));
		}
    }

    firstBudgetAdapt = 0;
	objectsChanged = 0;
    
    // adapt all the objects to distribute the budgeted triangles while
    // minimizing the error
    GLOD_Object *coarsenTop, *refineTop;
    char roomToRefine, overBudget, queuesBalanced;
    //roomToRefine=new char[GLOD_NUM_TILES];
    //overBudget=new char[GLOD_NUM_TILES];

    coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
    refineTop = (GLOD_Object *)refineQueue->min()->userData();
    /*
	if (refineTop->cut->refineTris == MAXINT)
	{
	    //for (int i=0; i<GLOD_NUM_TILES; i++)
		roomToRefine = 0;
	}
	else
	{
	    for (int i=0; i<GLOD_NUM_TILES; i++){
		if (refineTop->inArea[i]){
		    TrisAfterRefine = currentNumTris[i] - refineTop->cut->currentNumTris
			+ refineTop->cut->refineTris;

		    roomToRefine = (TrisAfterRefine <= triBudget);
		    if (roomToRefine==0)
			break;
		}
	    }
	}

    overBudget = ((currentNumTris > triBudget) &&
		  (coarsenTop->cut->coarsenErrorObjectSpace() != MAXFLOAT));
     */
	if (errorMode == ObjectSpace)
	{
	    queuesBalanced = (coarsenTop->cut->coarsenErrorObjectSpace()
			>= refineTop->cut->currentErrorObjectSpace());
	}
	else
	{
		queuesBalanced = (coarsenTop->cut->coarsenErrorScreenSpace() 
			>= refineTop->cut->currentErrorScreenSpace());
	}
	
	//if (errorMode == ScreenSpace)
	//{
	  roomToRefine=1;
	  overBudget=1;
	//}
    //while (roomToRefine || overBudget || !queuesBalanced)
    //{
        /*    
printf("\tCurrent Num Tris: %i, TrisAfterRefine: %i, Budget: %i\n",
	currentNumTris, TrisAfterRefine, triBudget);
printf("\tCoarsen Queue Top Error: %.25f, Refine Queue Top Error: %.25f\n",
	(errorMode == ObjectSpace) ? coarsenTop->cut->coarsenErrorObjectSpace() :
	coarsenTop->cut->coarsenErrorScreenSpace(), (errorMode == ObjectSpace) ? 
	refineTop->cut->currentErrorObjectSpace() : 
	refineTop->cut->currentErrorScreenSpace());
      
if (roomToRefine)
	printf("\troomToRefine: 1, ");
else
	printf("\troomToRefine: 0, ");
if (overBudget)
	printf("overBudget: 1, ");
else
	printf("overBudget: 0, ");
if (queuesBalanced)
	printf("queuesBalanced: 1\n");
else
	printf("queuesBalanced: 0\n");
      */
	// Refine first, then coarsen, so that coarsening is last (and thus
	// we end up within the budget)

	// refine
	
#if 0
	for (int i=0; i<GLOD_NUM_TILES; i++){
	    fprintf(stderr, "adaptTriangleBudget(): ");
	    fprintf(stderr, "initial budget: %d, tris: %d\n",
		    triBudget[i], currentNumTris[i]);
	}
#endif
		
	while (roomToRefine)
	{
	    
	    //printf("Beginning refine pass\n");
	    
	    GLOD_Object *refineObj =
	    (GLOD_Object *)refineQueue->extractMin()->userData();
	    GLOD_Object *nextRefine = (refineQueue->size() > 0) ?
		(GLOD_Object *)refineQueue->min()->userData() : NULL;
	    
	    // apply refinement
	    int beforeTris = refineObj->cut->currentNumTris;
	    int triTermination=MAXINT;
	    for (int i=0; i<GLOD_NUM_TILES; i++)
		if (refineObj->inArea[i])
		    if ((triBudget[i] - (currentNumTris[i] - beforeTris))<triTermination)
			triTermination=triBudget[i] - (currentNumTris[i] - beforeTris);
	    
	    if (triTermination < 0)
	    {
	    	triTermination = refineObj->cut->refineTris;
	    }
	    
	    float errorTermination = (nextRefine != NULL) ? 
		nextRefine->cut->currentErrorObjectSpace() : 0;//-MAXFLOAT;
	    
	    if ((errorMode == ScreenSpace) && (errorTermination != 0)) //-MAXFLOAT))
	    {
		float nextRefineError = nextRefine->cut->currentErrorScreenSpace();			
		errorTermination =  nextRefineError;
	    }
	    
	    //if ((errorMode == ScreenSpace) && (errorTermination < refineObj->cut->currentErrorScreenSpace()))
		//errorTermination = 0;
	    refineObj->cut->refine(errorMode, triTermination, errorTermination);
	    
	    //printf("refining %u\n", refineObj->name);
	    
	    int afterTris = refineObj->cut->currentNumTris;
	    for (int i=0; i<GLOD_NUM_TILES; i++)
		if (refineObj->inArea[i]==1){
//		    printf("name %i area %i cur %i before %i after %i", refineObj->name, i, currentNumTris[i], beforeTris, afterTris);
		    currentNumTris[i] = currentNumTris[i] - beforeTris + afterTris;
//		    printf(" after cur %i\n", currentNumTris[i]);
		}
	    if (((refineObj->name == LastObjectRefined) &&
		(triTermination == LastRefineTriTermination) &&
		(errorTermination == LastRefineErrorTermination))) // || (beforeTris==afterTris))
	    {
		//			printf("Duplicate refine call - euthanizing budget loop.\n");
		
		if (refineQueue->size() <= 1)
		    break;
//		printf("%i\n", refineQueue->size());
		refineQueue->remove(&(refineObj->budgetRefineHeapData));
		refineTop= (GLOD_Object *)refineQueue->min()->userData();
		//printf("blerg\n");
	    }
	    else
	    {
		LastObjectRefined = refineObj->name;
		LastRefineTriTermination = triTermination;
		LastRefineErrorTermination = errorTermination;
		
		// possible reasons that the refine terminated:
		//
		// a) error < errorTermination
		//    -- put back on refine queue (it won't be on top anymore)
		//
		// b) tris > triTermination
		//    -- put on coarsen queue (we will need to coarsen either
		//       this or something else to get within budget)
		//
		// c) no more refinements possible
		//    -- put on coarsen queue
		
		if (errorMode == ObjectSpace)
		{
		    refineObj->budgetRefineHeapData.setKey(
							   -refineObj->cut->currentErrorObjectSpace());
		}
		else
		{
		    refineObj->budgetRefineHeapData.setKey(
							   -refineObj->cut->currentErrorScreenSpace());
		}
		
		if (errorMode == ObjectSpace)
		{
		    NewError = refineObj->cut->coarsenErrorObjectSpace();
		}
		else
		{
		    NewError = refineObj->cut->coarsenErrorScreenSpace();
		}	
		
		refineQueue->insert(&(refineObj->budgetRefineHeapData));
		coarsenQueue->changeKey(&(refineObj->budgetCoarsenHeapData), NewError);
		}
//		    printf("coarsen %i refine %i\n", coarsenQueue->size(), refineQueue->size());
	    coarsenTop = (GLOD_Object *)coarsenQueue->min()->userData();
	    refineTop = (GLOD_Object *)refineQueue->min()->userData();
	    roomToRefine=1;
	    overBudget=0;
	    for (int i=0; i<GLOD_NUM_TILES; i++){
		if (refineTop->inArea[i]==0) continue;
		TrisAfterRefine = (currentNumTris[i] - refineTop->cut->currentNumTris +
				   refineTop->cut->refineTris);
		if (roomToRefine)
		    roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
			(TrisAfterRefine < triBudget[i]);
		
		if (!overBudget)
		    overBudget = ((currentNumTris[i] > triBudget[i]) &&
				  (coarsenTop->cut->coarsenErrorObjectSpace() != MAXFLOAT));
		
		//printf("obj %i over (refine)? %i %i %i %i\n", refineObj->name, currentNumTris[0], triBudget[0], roomToRefine, overBudget);
		
		if (errorMode == ObjectSpace)
		{
		    queuesBalanced = (coarsenTop->cut->coarsenErrorObjectSpace()
				      >= refineTop->cut->currentErrorObjectSpace());
		}
		else
		{
		    float coarsenTopError = coarsenTop->cut->coarsenErrorScreenSpace();
		    float refineTopError = refineTop->cut->currentErrorScreenSpace();
		    queuesBalanced = (coarsenTopError >= refineTopError);
		}
	    }
	    if ((refineTop->cut->currentErrorScreenSpace() < 0.000001f)&&(errorMode==ScreenSpace)&&(!overBudget)&&(queuesBalanced)){
		return;
	    }
	    
	    /*
	     printf("\tCurrent Num Tris: %i, TrisAfterRefine: %i, Budget: %i\n",
		    currentNumTris, TrisAfterRefine, triBudget);
	     
	     
	     printf("\tCoarsen Queue Top Error: %f, Refine Queue Top Error: %f\n",
		    (errorMode == ObjectSpace) ? coarsenTop->cut->coarsenErrorObjectSpace() :
		    coarsenTop->cut->coarsenErrorScreenSpace(), (errorMode == ObjectSpace) ? 
		    refineTop->cut->currentErrorObjectSpace() : 
		    refineTop->cut->currentErrorScreenSpace());
	     */
#if 1
	    
	    ///if (queuesBalanced)
	    //{
		while (/*(!overBudget) && */(!roomToRefine) && (refineQueue->size() > 1))
		{
		    
		  //  		    printf("Queues Balanced; removing tops of refine and coarsen queues(refine)\n");
		    
		    refineQueue->remove( &(refineTop->budgetRefineHeapData) );
		    coarsenQueue->remove( &(refineTop->budgetCoarsenHeapData) );
		    refineTop = (GLOD_Object *)refineQueue->min()->userData();
		    for (int i=0; i<GLOD_NUM_TILES; i++){
			if (refineTop->inArea[i]){
			    TrisAfterRefine = (currentNumTris[i] - 
				       refineTop->cut->currentNumTris + refineTop->cut->refineTris);
			    roomToRefine = (refineTop->cut->refineTris == MAXINT) ? 0 :
					(TrisAfterRefine < triBudget[i]);
			    if (roomToRefine==0)
				break;
			}
		    }
		}
		if (refineQueue->size()<1)
		    roomToRefine=0;
	    //}
#endif      
	    
	} // end of refinement
	
	
//	if (!queuesBalanced&&!roomToRefine&&!overBudget){
//	  roomToRefine=1;
//
//	}
	
    //}
    
	

    budgetChanged = 0;

#if 0
    for (int i=0; i<GLOD_NUM_TILES; i++){
	fprintf(stderr, "adaptTriangleBudget(): ");
	fprintf(stderr, "after refine budget tile %i: %d, tris: %d\n", i
		triBudget[i], currentNumTris[i]);
    }
#endif
    
    return;
} /* End of GLOD_Group::adaptTriangleBudget() **/
#endif
#endif
/*****************************************************************************\
 @ GLOD_Group::adapt
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
GLOD_Group::adapt()
{
  if (mpSimplifier != NULL)
    {
      switch (errorMode)
	{
	case ObjectSpace:
		if (viewFrustumSimp)
			mpSimplifier->SetErrorFunc(StdErrorObjectSpace);
		else
			mpSimplifier->SetErrorFunc(StdErrorObjectSpaceNoFrustum);
	  break;
	case ScreenSpace:
		if (viewFrustumSimp)
		  mpSimplifier->SetErrorFunc(StdErrorScreenSpace);
		else
		  mpSimplifier->SetErrorFunc(StdErrorScreenSpaceNoFrustum);
	  break;
	};
    }

  for (int i=0; i<numObjects; i++)
    {
		GLOD_Object *obj = objects[i];
		obj->cut->updateStats();
    }
    switch(adaptMode)
    {
      case TriangleBudget:
	adaptTriangleBudget();
	break;
      case ErrorThreshold:
	adaptErrorThreshold();
	break;
    }
    return;
} /* End of GLOD_Group::adapt() **/


/*****************************************************************************\
  $Log: glod_group.cpp,v $
  Revision 1.50  2005/04/26 20:11:35  gfx_friends
  Fixed the readback problem.

  Revision 1.49  2004/08/10 00:41:08  gfx_friends
  fixed up the discrete patch triangle budget management. Should work just fine with multiple objects, as well as different hierarchies

  Revision 1.48  2004/07/20 21:54:34  gfx_friends
  Major Discrete and AttribSet rewrite. Discrete now uses attribsets for its vertex data. -Nat

  Revision 1.47  2004/07/19 17:48:31  gfx_friends
  A glod object should remove itself from the group coarsen/refine queues before it dies.

  Revision 1.46  2004/07/01 20:09:39  gfx_friends
  more fixes to DiscretePatch hierarchy

  Revision 1.45  2004/06/25 18:58:40  gfx_friends
  New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default

/*****************************************************************************\
  $Log: glod_group.cpp,v $
  Revision 1.50  2005/04/26 20:11:35  gfx_friends
  Fixed the readback problem.

  Revision 1.49  2004/08/10 00:41:08  gfx_friends
  fixed up the discrete patch triangle budget management. Should work just fine with multiple objects, as well as different hierarchies

  Revision 1.48  2004/07/20 21:54:34  gfx_friends
  Major Discrete and AttribSet rewrite. Discrete now uses attribsets for its vertex data. -Nat

  Revision 1.47  2004/07/19 17:48:31  gfx_friends
  A glod object should remove itself from the group coarsen/refine queues before it dies.

  Revision 1.46  2004/07/01 20:09:39  gfx_friends
  more fixes to DiscretePatch hierarchy

  Revision 1.45  2004/06/25 18:58:40  gfx_friends
  New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default

  Revision 1.44  2004/06/16 20:30:32  gfx_friends
  values.h include change for osx

  Revision 1.43  2004/05/11 17:12:28  gfx_friends
  Minor change to the instancing fix so it is only applied to discrete

  Revision 1.42  2004/03/29 02:15:44  gfx_friends
  fixed a major bug with triangle budget mode, which would force only the root object from a instance list to be simplified.
  Also a bug fix which would infinately loop if some objects were outside the view frustrum.

  Revision 1.41  2004/02/04 07:21:03  gfx_friends
  Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat

  Revision 1.40  2004/01/21 17:58:26  bms6s
  turned view frustum simplification on (maybe we need to make a new group param to be able to control it?)

  changed view parameter extraction so forward vector seems to be captured ok

  Revision 1.39  2004/01/20 07:43:40  bms6s
  ask and ye shall receive

  Revision 1.38  2004/01/13 15:43:11  bms6s
  fixed vbo rendering of more than one continuous cut
  commented out some debug printfs

  Revision 1.37  2003/12/17 03:37:39  gfx_friends
  Modified the build_object code to allow for DISCRETE_MAUAL objects. Basically DISCRETE_MANUAL objects skip the triangle simplification steps, and assume that all of the necessary LOD's are entered by using insertArrays or insertElements

  Revision 1.36  2003/12/14 06:11:05  bms6s
  added variables to adaptTriangleBudget to track the last object coarsened/refined and the parameters it was adapted to; if the same object gets called twice in a row with the same adaptation parameters, then it is removed from the queues.  this seems to get rid of the infinite loops that were occurring due to the vds queues being unbalanced.  it is still possible to have the "oscillating objects" effect due to a repeating sequence of two calls to adaptTriangleBudget such as:
  {
  calling adaptTriangleBudget()
  refining 0
  coarsening 2
  coarsening 1
  coarsening 1 (duplicate - removing 1 from queues)
  coarsening 0
  refining 2

  calling adaptTriangleBudget()
  refining 0
  coarsening 2
  coarsening 1
  coarsening 0
  refining 2
  refining 1
  refining 1 (duplicate - removing 1 from queues)
  refining 2
  coarsening 2
  }
  repeating over and over.  it doesn't happen all that frequently, but still looks unsightly so we should try to figure out how to fix it.

  Revision 1.35  2003/11/13 19:20:58  bms6s
  yeah my bad (you know me, always overlooking the lowly discrete cuts)

  Revision 1.34  2003/11/07 03:01:18  bms6s
  -added update to currentNumTris at the beginning of adaptTriangleBudget
  [NOTE: VDSCut::currentNumTris currently is the total number of tris for all VDSCuts in that cut's group, not just the number of tris for that specific cut.  this is wildly unintuitive, but needed to work with the fact that one VDSCut represents all of them in that group in the adaptTriangleBudget queues.]
  -added check for if refine triTermination is below zero, and if so set it to the refine cut's refineTris; this is because of the stipulation that in screen space we always start with a refine, which, if we were over budget, would result in the first refine having a negative triTermination.
  -added printfs to help in debugging this code.  i've left them in (commented out) since i get the feeling that they may still be needed in the future...

  this fixes some things, but continuous cuts are still jerky and i don't think boundary conditions work yet.

  Revision 1.33  2003/11/04 18:37:13  gfx_friends
  Several changes to attempt to remove problems with continous objects
  disappearing after a mode change in triangle error mode. Not quite successfull
  as of yet.

  Revision 1.32  2003/10/23 15:56:15  gfx_friends
  Removed the triangle budget printfs.

  Revision 1.31  2003/10/23 05:44:45  gfx_friends
  Minor changes to the triangle budget code. One of the new queueBalanced checks
  was checking the coarsen queue twice, rather then comparing the errors on the
  refine and coarsen queues. The other change is to remove the flickering that
  can occur if the triangle count is spot on in discrete. Changed  roomToRefine
  to be set only if the current triangle count is lower then the budget
  rather then lower then or equal to the budget.

  Revision 1.30  2003/10/23 04:38:17  bms6s
  support for proper triangle budget adaptation of a group with multiple continuous cuts
  -all vds cuts within a group use the same simplifier, and calling coarsen or refine on any of them actually results in adaptation of all of them; therefore, we only put the first vds cut encountered into the queues.
  -this also means that the node on top of the fold or unfold queue of a cut's simplifier may not actually belong to that cut, so when calculating screen space coarsenError and currentError it is improper to use the calling cut's view parameters.  instead, we just get the error directly from the budgetitem in the queue, since it will have been set using the correct cut's view parameters in UpdateNodeErrors().
  -encountered situations in which the adaptTriangleBudget() code was calling coarsen with a negative triTermination; since VDS uses an unsigned int as the triTermination, this was obviously causing problems, so if the triTermination is less than zero it now gets set to zero before the SimplifyBudgetAndThreshold() call.

  Revision 1.29  2003/10/23 01:44:23  bms6s
  -removed coarsenError, currentError, coarsenRadius, coarsenCenter, refineRadius, and refineCenter from GLOD_Cut.  replaced them with coarsenErrorObjectSpace(), coarsenErrorScreenSpace(), currentErrorObjectSpace(), and currentErrorScreenSpace().  this makes code much cleaner and ensures consistency in the errors of discrete and continuous cuts (at least inasmuch as the discrete cuts's errors[] array is consistent with VDS' node radii).
  -added call to update VDS errors at the beginning of every continuous cut coarsen() or refine() call.  this is more than necessary; conceivably we could just call it once per glod adaptXXX() call, perhaps in the beginning just before the queues are constructed.
  -if coarsen() is going to get called with triTermination equal to the number of tris already in the cut, then instead we set triTermination to 0.
  -modified vds node error callback StdErrorScreenSpaceNoFrustum to use the VDS::Cut's mpExternalViewClass pointer to calculate node error using computePixelsOfError().  so this means that view frustum simplification is currently disabled.  it shouldn't be too hard to similarly convert StdErrorScreenSpace to use glod's view frustum check and computePixelsOfError as well.
  -modified computePixelsOfError to only take a center and object space error (that's all it was using anyway), since that's all VDS can provide it.

  Revision 1.28  2003/10/22 04:36:47  bms6s
  adaptTriangleBudget() updated with the things we tried during the conference.
  also (and i guess we can talk about reverting this if it pains you too much, jon) rewrote much of the code to be cosmetically a little more digestible for those of us whose brains don't have hardware support for parsing statements that span 11 lines and use two ? operators.

  Revision 1.27  2003/10/22 00:46:17  bms6s
  added computePixelsOfError function to GLOD_Cut, which by default just calls view.computePixelsOfError.  VDSCut, however, instead calls StdErrorScreenSpace, since that is the function used to calculate errors of the VDS::Cut.

  Revision 1.26  2003/10/17 17:58:29  gfx_friends

  Fixed triangle budget mode, now minimizes screen/object space error while maximizing the triangle count, and keeping it under budget. No more infinite loops

  Revision 1.25  2003/08/29 19:45:53  gfx_friends
  Made screen space for VDS use the frustum check callback.
   CVS: ----------------------------------------------------------------------

  Revision 1.24  2003/08/27 18:36:06  gfx_friends
  Bugfix that fixes glodDeleteGroup and glodDeleteObject.

  Revision 1.23  2003/08/14 22:20:37  gfx_friends
  Trying to make screen space work.

  Revision 1.22  2003/07/26 01:17:15  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.21  2003/07/23 19:55:26  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.20  2003/07/23 06:36:06  bms6s
  vds integrated

  Revision 1.19  2003/07/15 20:18:53  gfx_friends
  Major documentation effort and basic distribution readiness. We now have pod-based documentation for each GLOD function. It will build HTML or Man pages on Linux. To use the man pages, append glod/doc/man to your manpath after running make in doc or doing a top-level make. Also new is a release target... a top level make release builds with -O2 and any flags you also set based on the release target (See glod.conf). Also, #define DEBUG is active when building for debug.

  Revision 1.18  2003/07/09 22:50:03  gfx_friends
  Major documentation effort and minor API changes. On the API change side,
  GLODBuildObject now recieves the format flag for an object being built, while LoadObject now requires NewObject to
  have been called before it can be called. NewObject simply creates an object and group.

  On the documentation side, the sources in ./api now contain a ton of inline comments which document
  the API routines using Doxygen tagging syntax. A top-level makefile target, docs, allows you to build HTML documentation out of these files. When I've finished the documentation, we can also make the same comments go to UNIX Man pages and Windows RTF/HTML help files. I'm still documenting the API. However, if you run make docs on a linux box or some PC with Doxygen installed on it, you'll get the docs and can check them out.

  Cheers,

  -- Nat

  Revision 1.17  2003/07/01 21:49:10  gfx_friends
  Tore out the glodDrawObject in favor of the more complete glodDrawPatch. A bug somewhere between glodBuildObject's call to new Model and the draw, however, is still failing, as all of the patches have identical geometry by the time they hit the drawing point.

  Revision 1.16  2003/07/01 20:49:14  gfx_friends
  Readback for Discrete LOD now works. See samples/readback/read_model for demo and test. This, parenthetically, is also the most fully functional test program that we have. I will bring "simple" up to speed eventually, but in the meantime, you can use read_model similarly (run --help) in much the same way as previous ones to do testing.

  Revision 1.15  2003/06/11 08:07:23  bms6s
  [vds and glod commit messages are identical]

  temporary memory allocation in place.  everything compiles in windows but i've no doubt completely destroyed the linux build; i will attempt to fix this tomorrow and then ask nat for help.

  beacause of the way vds is set up:
  -calling adapt on a cut actually adapts all cuts in that cut's group
  -calling render on a cut actually renders all cuts in that cut's patch

  api/vds_error,cpp, api/vds_render.cpp, and api/vds_simplify.cpp are not used by anything anymore - they've been replaced by vds_callbacks.h and vds_callbacks.cpp.  vds_callbacks.cpp could be moved to api/ if you think that's more appropriate.

  i replaced gl types in vds with generic equivalents, since i wasn't sure if glodlib will have the gl includes (from the errors i was getting it appeared not).

  gave simple with a load sphere50.ply command line when run in visual studio

  Revision 1.14  2003/06/09 19:10:12  gfx_friends
  More renaming. These emails are annoying, yes, but the bright side is that when we're done, this'll look a lot better.

  Revision 1.14  2003/06/05 17:40:09  gfx_friends
  Patches to build on Win32.

  Revision 1.13  2003/06/04 16:53:55  gfx_friends
  Tore out CR.

  Revision 1.12  2003/01/21 09:20:59  gfx_friends
  Intermediate checkin. Things are broken.

  Revision 1.10  2003/01/20 22:28:13  bms6s
  *** empty log message ***

  Revision 1.9  2003/01/20 07:42:38  gfx_friends
  Added screen-space error mode. It seems to work for threshold mode,
  but still gets stuck in triangle budget mode (object-space seems to
  work okay in budget mode now).

  Revision 1.8  2003/01/20 04:41:18  gfx_friends
  Fixed GLOD_Group::addObject() and adaptTriangleBudget. Added initial
  class to set view.

  Revision 1.7  2003/01/19 18:34:38  gfx_friends
  Fixed overflow condition at the high end of triangle budgets.

  Revision 1.6  2003/01/19 17:16:17  gfx_friends
  Triangle budget mode works on a single object, but may very well work
  on multiple objects.

  Revision 1.5  2003/01/19 04:48:57  gfx_friends
  Object-space error threshold mode for a single object in a single
  group seems to be working. Not tested on multiple objects in multiple
  groups, but it is trivial enough that it "should" work.

  Revision 1.4  2003/01/18 23:42:13  gfx_friends
  initial (non-working) version of triangle budget mode, etc.

  Revision 1.3  2003/01/17 21:57:32  gfx_friends
  Patches to make the TestProg work with the API changes. :)

  Revision 1.2  2003/01/17 21:33:52  gfx_friends
  New API support.

  Revision 1.1  2003/01/17 18:46:57  gfx_friends
  Moving Group and Object to glod core

\*****************************************************************************/

