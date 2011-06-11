/* GLOD: Group classes & such
***************************************************************************
* $Id: glod_group.h,v 1.4 2004/07/19 16:43:21 gfx_friends Exp $
* $Revision: 1.4 $
***************************************************************************/
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
#ifndef GLOD_GROUP_H
#define GLOD_GROUP_H

//#define GLOD_USE_TILES

class GLOD_Group
{
private:
    GLOD_Object **objects;
    int numObjects;
    int maxObjects;
    
    // modes
    AdaptMode adaptMode;
    ErrorMode errorMode;
    
    //
    // error threshold mode stuff
    //
    float screenSpaceErrorThreshold;
    float objectSpaceErrorThreshold;
    
    bool viewFrustumSimp;
    
	std::vector<GLOD_Object_Status> RefinedStatusList[2] ;
	std::vector<GLOD_Object_Status> CoarsenedStatusList[2] ;

private:
    void adaptErrorThreshold();
    void adaptTriangleBudget();
    void initQueues();
    void clearQueues();
	bool isEndlessLoop() ;
    
    //
    // triangle budget mode stuff
    //
    
    // priority queues for triangle budget mode
    // Which sounds better, refine/reduce or refine/coarsen?
    // I like the sound of refine/reduce, but reduce is more ambiguous than
    // coarsen (it could mean reduce tris or reduce error)
    char objectsChanged;
    char budgetChanged;
    char firstBudgetAdapt;
    Heap *refineQueue;
    Heap *coarsenQueue;
#ifndef GLOD_USE_TILES
    int triBudget;
    int currentNumTris;
#else
    int *triBudget;
    int *currentNumTris;
#endif
    
public:
    
    VDS::Simplifier* mpSimplifier;
    bool vds_objects_adapted;
    int numTiles;
    
    
    
    GLOD_Group()
    {
        numTiles=GLOD_NUM_TILES;
        objects = NULL;
        numObjects = maxObjects = 0;
        adaptMode = ErrorThreshold;
        errorMode = ObjectSpace;
        //triBudget = 100;
        screenSpaceErrorThreshold = 1.0;
        objectSpaceErrorThreshold = 1.0;
        objectsChanged = budgetChanged = firstBudgetAdapt = 1;
        refineQueue = new Heap;
        coarsenQueue = new Heap;
        //currentNumTris = 0;
        viewFrustumSimp = true;
#ifndef GLOD_USE_TILES
        currentNumTris=0;
        triBudget=1000;
#else
        currentNumTris=new int[GLOD_NUM_TILES];
        triBudget=new int[GLOD_NUM_TILES];
        for (int i=0; i<GLOD_NUM_TILES; i++){
            triBudget[i]=1000;
            currentNumTris[i]=0;
        }
#endif
        mpSimplifier = new VDS::Simplifier;
        //  fprintf(stderr, "new Simplifier\n");
        vds_objects_adapted = false;
        
        mpSimplifier->mSimplificationBreakCount = 100;
    };

    ~GLOD_Group() {
        if (objects != NULL) {
            for (int i=0; i<numObjects; i++) {
                delete objects[i];
                objects[i] = NULL;
            }
            delete [] objects;
            objects = NULL;
        }
        numObjects = maxObjects = 0;
        if (refineQueue != NULL)
        {
            delete refineQueue;
            refineQueue = NULL;
        }
        if (coarsenQueue != NULL)
        {
            delete coarsenQueue;
            coarsenQueue = NULL;
        }

        if(mpSimplifier != NULL)
            delete mpSimplifier;
    }
    
    void changeLayout(){
#ifdef GLOD_USE_TILES
        numTiles=GLOD_NUM_TILES;
        int tri=triBudget[0];
        delete currentNumTris;
        delete triBudget;
        currentNumTris=new int[GLOD_NUM_TILES];
        triBudget=new int[GLOD_NUM_TILES];
        for (int i=0; i<GLOD_NUM_TILES; i++){
            triBudget[i]=tri;
            currentNumTris[i]=0;
        }
        for (int i=0; i<numObjects; i++){
            delete objects[i]->inArea;
            objects[i]->inArea=new int[GLOD_NUM_TILES];
        }
#endif
    }
    
    int getNumObjects() { return numObjects; }
    GLOD_Object* getObject(int index) { return objects[index];}
    void addObject(GLOD_Object*);
    void removeObject(int index);
    
    void setTriBudget(int budget)
    {
#ifndef GLOD_USE_TILES
        triBudget=budget;
#else
        for (int i=0; i<GLOD_NUM_TILES; i++)
            triBudget[i] = budget;
#endif
        budgetChanged = 1;
    }
    void setAdaptMode(AdaptMode mode)
    {
        adaptMode = mode;
        if (adaptMode == TriangleBudget)
            firstBudgetAdapt = 1;
    }
    void setErrorMode(ErrorMode mode)
    {
        errorMode = mode;
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
    }
    void setScreenSpaceErrorThreshold(float threshold)
    {
        screenSpaceErrorThreshold = threshold;
    }
    void setObjectSpaceErrorThreshold(float threshold)
    {
        objectSpaceErrorThreshold = threshold;
    }
    
    void adapt();
    
    AdaptMode GetAdaptMode() { return adaptMode; }
    ErrorMode GetErrorMode() { return errorMode; }
};

#endif
/***************************************************************************
* $Log: glod_group.h,v $
* Revision 1.4  2004/07/19 16:43:21  gfx_friends
* Memory leak patches. -n
*
* Revision 1.3  2004/07/09 00:47:06  gfx_friends
* Memory leak fixes a plenty. --nat
*
* Revision 1.2  2004/06/25 18:58:41  gfx_friends
* New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default
*
* Revision 1.1  2004/02/04 07:21:06  gfx_friends
* Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
*
***************************************************************************/
