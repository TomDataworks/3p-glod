/******************************************************************************
 * Copyright 2004 David Luebke, Brenden Schubert                              *
 *                University of Virginia                                      *
 ******************************************************************************
 * This file is distributed as part of the VDSlib library, and, as such,      *
 * falls under the terms of the VDSlib public license. VDSlib is distributed  *
 * without any warranty, implied or otherwise. See the VDSlib license for     *
 * more details.                                                              *
 *                                                                            *
 * You should have recieved a copy of the VDSlib Open-Source License with     *
 * this copy of VDSlib; if not, please visit the VDSlib web page,             *
 * http://vdslib.virginia.edu/license for more information.                   *
 ******************************************************************************/
#ifdef _WIN32
//stop compiler from complaining about exception handling being turned off
#pragma warning(disable: 4530)
#endif

#include <cassert>
#include <iostream>
#include "cut.h"
#include "forest.h"

using namespace VDS;
using namespace std;

Cut::Cut()
{
	mpForest = NULL;
	mpRenderer = NULL;
	mpSimplifier = NULL;
	mpNodeRefs = NULL;
    mpTriRefs = NULL;
	mIsValid = false;
	mNumActiveNodes = 0;
	mNumActiveTris = 0;
	mBytesUsed = 0;
	float identitymatrix[16] = {1, 0, 0, 0, 
								0, 1, 0, 0, 
								0, 0, 1, 0,
								0, 0, 0, 1};
	mTransformMatrix.Set(identitymatrix);
	miHighlightedNode = 0;
	miHighlightedTri = 0;
	mpExternalViewClass = NULL;
}

Cut::~Cut()
{
    if (mpNodeRefs != NULL)
    {
		int i;
        unsigned int j;

        // nullify the mpNodeRefs that are in the fold queue ... they're already free'd
        NodeQueue* q = mpSimplifier->mpFoldQueue;
        for(i = 0; i <= q->Size; i++)
            mpNodeRefs[q->GetElement(i)->miNode] = NULL;
        
        q = mpSimplifier->mpUnfoldQueue;
        for(i = 0; i <= q->Size; i++) {
            mpNodeRefs[q->GetElement(i)->miNode] = NULL;
        }

        for(j = 0; j < mpForest->mNumNodes+1; j++)
            if(mpNodeRefs[j] != NULL)
                delete mpNodeRefs[j];
        
        delete[] mpNodeRefs;
    }
    if (mpTriRefs != NULL)
    {
        delete[] mpTriRefs;
    }
}

void Cut::GiveContents(Cut *pCut)
{
//	int i;
	pCut->mpForest = mpForest;
	pCut->mpRenderer = mpRenderer;
	pCut->mpSimplifier = mpSimplifier;
	pCut->mNumActiveTris = mNumActiveTris;
	pCut->mTransformMatrix = mTransformMatrix;
}

void Cut::SetRenderer(Renderer *pRenderer)
{
	if (pRenderer == NULL)
	{
		mpRenderer = NULL;
		mIsValid = false;
	}
	else
	{
		mpRenderer = pRenderer;
		if ((mpSimplifier != NULL) && (mpForest != NULL))
		{
			mIsValid = true;
			mpRenderer->mIsValid = true;
			mpSimplifier->mIsValid = true;
		}
	}
}

void Cut::SetSimplifier(Simplifier *pSimplifier)
{
	if (pSimplifier == NULL)
	{
		mpSimplifier = NULL;
		mIsValid = false;
	}
	else
	{
		mpSimplifier = pSimplifier;
		if ((mpRenderer != NULL) && (mpForest != NULL))
		{
			mIsValid = true;
			mpRenderer->mIsValid = true;
			mpSimplifier->mIsValid = true;
		}
	}
}

void Cut::SetForest(Forest *pForest)
{
	if (pForest == NULL)
	{
		mpForest = NULL;
		mIsValid = false;
	}
	else
	{
		mpForest = pForest;
		if ((mpSimplifier != NULL) && (mpRenderer != NULL))
		{
			mIsValid = true;
			mpRenderer->mIsValid = true;
			mpSimplifier->mIsValid = true;
		}
		mBytesPerTri = sizeof(TriProxy) + sizeof(TriProxyBackRef);
		mBytesPerNode = sizeof(VertexRenderDatum);
		InitializeRefs();
	}
}

void Cut::SetTransformationMatrix(float *ColMajorFloat16Matrix)
{
	mTransformMatrix.Set(ColMajorFloat16Matrix);
}

void Cut::SetTransformationMatrix(Mat4 ColMajorFloat16Matrix)
{
    mTransformMatrix = ColMajorFloat16Matrix;
}

/*
void Cut::UpdateViewParametersFromMatrix()
{
	Mat4 M(mTransformMatrix);
	Mat4 Minverse;
	Minverse = M.Inverse();

	Point3 viewpoint(Minverse[0][3], Minverse[1][3], Minverse[2][3]);
	Vec3 forwardvec(-Minverse[0][2], -Minverse[1][2], -Minverse[2][2]);
	Vec3 upvec(Minverse[0][1], Minverse[1][1], Minverse[2][1]);

	forwardvec.Normalize();
	upvec.Normalize();

	SetLookVector(forwardvec);
	SetViewPoint(viewpoint);
	SetUpVector(upvec);
	CalcFrustum();
}
*/

/*
// Calculate plane equations for top/bottom/left/right
// planes of frustum based on current view parameters.
void Cut::CalcFrustum()
{
	Vec3 nTop, nBottom, nRight, nLeft;		// normals of the four planes
	float HalfVertFOV = mFov / 2.0f;
	float HalfHorzFOV = HalfVertFOV * mAspectRatio;
	float sinHVert = sin(DEGREES_TO_RADIANS*HalfVertFOV);
	float cosHVert = cos(DEGREES_TO_RADIANS*HalfVertFOV);
	float sinHHorz = sin(DEGREES_TO_RADIANS*HalfHorzFOV);
	float cosHHorz = cos(DEGREES_TO_RADIANS*HalfHorzFOV);

	nTop = (cosHVert * mUpVector) - (sinHVert * mForwardVector);
	nBottom = (-cosHVert * mUpVector) - (sinHVert * mForwardVector);
	nRight = (cosHHorz * mRightVector) - (sinHHorz * mForwardVector);
	nLeft = (-cosHHorz * mRightVector) - (sinHHorz * mForwardVector);
	
	mTopPlane.SetNormal(nTop);
	mBottomPlane.SetNormal(nBottom);
	mRightPlane.SetNormal(nRight);
	mLeftPlane.SetNormal(nLeft);

	mTopPlane.SetPointOnPlane(mViewpoint);
	mBottomPlane.SetPointOnPlane(mViewpoint);
	mRightPlane.SetPointOnPlane(mViewpoint);
	mLeftPlane.SetPointOnPlane(mViewpoint);
}
*/
/*
void Cut::SetLookVector(const Vec3 &rLookVector)
{
    mForwardVector = rLookVector;
    mForwardVector.Normalize();
	mRightVector = mForwardVector % mUpVector;
	mUpVector = mRightVector % mForwardVector;
	CalcFrustum();
}

void Cut::SetUpVector(const Vec3 &rUpVector)
{
	mRightVector = mForwardVector % rUpVector;
	mUpVector = mRightVector % mForwardVector;
	mUpVector.Normalize();
	mRightVector.Normalize();
	CalcFrustum();
}

void Cut::SetFov(Float Fov)
{
    mFov = Fov;
    mInvTanFov = 1 / tan(DEGREES_TO_RADIANS*Fov);
}

void Cut::SetAspectRatio(Float AspectRatio)
{
	mAspectRatio = AspectRatio;
}

void Cut::SetViewPoint(const Point3 &rViewpoint)
{
    mViewpoint = rViewpoint;    
}
*/

void Cut::InitializeRefs()
{
	unsigned int i;
	if (mpForest == NULL)
	{
		cerr << "Error - must set mpForest pointer before initializing refs in cut" << endl;
		return;
	}

	mpNodeRefs = new BudgetItem*[mpForest->mNumNodes+1];

	// refs actuallly are of type pointer to TriProxyBackRef because they point to the triangle's
	// location in the array of backrefs.  this location is needed more often than the triangle's
	// location in the array of proxies, which can be calcuated by finding the index of the location
	// in the array of backrefs and using that to index into the array of proxies
	mpTriRefs = new TriProxyBackRef*[mpForest->mNumTris+1];

	for (i = 0; i <mpForest->mNumNodes+1; ++i)
		mpNodeRefs[i] = NULL;
	for (i = 0; i <mpForest->mNumTris+1; ++i)
		mpTriRefs[i] = NULL;
}

void Cut::CheckForDuplicateNodeRefs()
{
	NodeIndex i,j;
	for (i = 1; i <= mpForest->mNumNodes; ++i)
	{
		if (mpNodeRefs[i] != NULL)
		{
			for (j = 1; j <= mpForest->mNumNodes; ++j)
			{
				if ((mpNodeRefs[i] == mpNodeRefs[j]) && (i != j))
					cout << "error: nodes " << i << " and " << j << " have identical NodeRefs" << endl;
				if (mpNodeRefs[j] != NULL)
				{
					if ((mpNodeRefs[i]->pVertexRenderDatum == mpNodeRefs[j]->pVertexRenderDatum) && (i != j))
						cout << "error: nodes " << i << " and " << j << " have identical pVertexRenderDatums" << endl;
				}
			}
		}
	}
}

#include "HighlightNodeFunctions.cpp"
