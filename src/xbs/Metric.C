/* GLOD: Simplification error implementation
***************************************************************************
* $Id: Metric.C,v 1.12 2004/12/08 15:21:13 jdt6a Exp $
* $Revision: 1.12 $
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

#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(__APPLE__)
#include <float.h>
#else
#include <values.h>
#endif

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

#include "Metric.h"
#include "PermissionGrid.h"

void SphereErrorData::update(Operation *op){
    xbsVertex *source_vert = op->getSource();
    xbsVertex *destination_vert = op->getDestination();
    
    float length = (source_vert->coord - destination_vert->coord).length();
    radius = MAX(((SphereErrorData *)(destination_vert->errorData))->radius, 
                 ((SphereErrorData *)(source_vert->errorData))->radius + length);
    return;
}

float SphereHalfEdgeError::calculateError(Model *model, Operation *op){
    xbsVertex *source_vert = op->getSource();
    xbsVertex *destination_vert = op->getDestination();
    
    float length = (source_vert->coord - destination_vert->coord).length();
    error = MAX(((SphereErrorData *)(destination_vert->errorData))->radius, 
                ((SphereErrorData *)(source_vert->errorData))->radius + length);
    return error;
}

float SphereEdgeError::calculateError(Model *model, Operation *op){
    EdgeCollapse *edge = (EdgeCollapse *)op;

    xbsVertex *source_vert = edge->getSource();
    xbsVertex *destination_vert = edge->getDestination();

    xbsVertex *genV = genVertex(model, source_vert, destination_vert, edge, 0);

    if (genV == NULL)
        error = MAXFLOAT;
    else
        error = ((SphereErrorData *)(genV->errorData))->radius;

    if (genV != NULL)
        delete genV;

    return error;
}

xbsVertex *SphereEdgeError::genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *op,
                                      int forceGen)
{
    
    float t;
    
    EdgeCollapse *edge = (EdgeCollapse *)op;
    
    EdgeCollapseCase ECcase = edge->computeCase(model);
    
    xbsVertex *source_vert = edge->getSource();
    xbsVertex *destination_vert = edge->getDestination();
    
    int lock = model->borderLock;
    
    if ((lock == 1) && (source_vert->onBorder() == 1) &&
        destination_vert->onBorder() == 1)
    {
        fprintf(stderr, "Have to disable border lock for a vertex. Sorry.\n");
        lock = 0;
    }
    
    float radius1 = ((SphereErrorData *)(source_vert->errorData))->radius;
    float radius2 = ((SphereErrorData *)(destination_vert->errorData))->radius;
    float length = (source_vert->coord - destination_vert->coord).length();
    
    float radius = 0;
    
    switch (ECcase)
    {
        case MoveBoth:
        {
            // general case -- both vertices can move
            
            float half_total = (radius2 +
                                radius1 + length) / 2.0;
            
            if ((half_total > radius2) &&
                (half_total > radius1))
            {
                t = (half_total - radius1) / length;
                radius = half_total;
            }
            else if (radius2 > radius1)
            {
                t = 1.0;
                radius = MAX(radius2, length+radius1);
            }
            else
            {
                t = 0.0;
                radius = MAX(radius1, length+radius2);
            }
            break;
        }
        case MoveSource:
        {
            t = 1.0;
            radius = MAX(radius2, length+radius1);
            break;
        }
        case MoveDestination:
        {
            t = 0.0;
            radius = MAX(radius1, length+radius2);
            break;
        }
        case MoveNeither:
        {
            if (forceGen == false)
                return NULL;
            if (destination_vert->onBorder() == 0)
            {
                t = 0.0;
                radius = MAX(radius1, length+radius2);
            }
            else
            {
                t = 1.0;
                radius = MAX(radius2, length+radius1);
            }
            break;
        }
    }
    
    
    // For now, always generate a new vertex, even if it is identical to
    // one of existing ones. We could probably fix this, but it might
    // require some more changes in updateModel(), etc.
    
    xbsVertex *generated_vert = source_vert->makeNew();

#if 0
    if ((v1 == NULL) && (v2 == NULL))
    {
        fprintf(stderr, "generateVertex(): Invalid attribute vertices!\n");
        exit(1);
    }
#endif
  
    if ((v1 == NULL) || (v2 == NULL))
    {
        if (v1 == NULL)
            v2->copySame(generated_vert);
        else if (v2 == NULL)
            v1->copySame(generated_vert);
        
        xbsVertex *geom_vert = source_vert->makeNew();
        geom_vert->interp(source_vert, destination_vert, t);
        generated_vert->coord = geom_vert->coord;
        delete geom_vert;
    }
    else
        generated_vert->interp(v1, v2, t);
    
    generated_vert->tris = NULL;
    generated_vert->numTris = 0;
    generated_vert->ops = NULL;
    generated_vert->numOps = 0;
    generated_vert->errorData = new SphereErrorData(generated_vert);
    ((SphereErrorData *)(generated_vert->errorData))->radius = radius;
    
    return generated_vert;
    
} /** End of sphereError::generateVertex() **/
    
/************************************************************************************/
/*   Quadric error stuff                                                            */
/************************************************************************************/

float QuadricHalfEdgeError::calculateError(Model *model, Operation *op){
    xbsVertex *source_vert = op->getSource();
    xbsVertex *destination_vert = op->getDestination();
    
    Vec4 dv(destination_vert->coord[0], destination_vert->coord[1],
            destination_vert->coord[2], 1); 
    QuadricErrorData *D = (QuadricErrorData*)destination_vert->errorData;
    QuadricErrorData *S = (QuadricErrorData*)source_vert->errorData;
    float dest_err = 
        (D->m11+S->m11)*dv.X*dv.X + 
        2*(D->m21+S->m21)*dv.X*dv.Y +
        2*(D->m31+S->m31)*dv.X*dv.Z +
        2*(D->m41+S->m41)*dv.X +
        (D->m22+S->m22)*dv.Y*dv.Y +
        2*(D->m32+S->m32)*dv.Y*dv.Z +
        2*(D->m42+S->m42)*dv.Y+
        (D->m33+S->m33)*dv.Z*dv.Z +
        2*(D->m43+S->m43)*dv.Z + (D->m44+S->m44);
    
    
    error=fabs(dest_err);
    error = sqrt(error);
    return error;
}

float QuadricEdgeError::calculateError(Model *model, Operation *op)
{ 
    EdgeCollapse *edge = (EdgeCollapse *)op;

    xbsVertex *source_vert = edge->getSource();
    xbsVertex *destination_vert = edge->getDestination();

    xbsVertex *genV = genVertex(model, source_vert, destination_vert, edge, 0);

    if (genV == NULL)
        error = MAXFLOAT;
    else
    {
        QuadricErrorData *D = (QuadricErrorData*)destination_vert->errorData;
        QuadricErrorData *S = (QuadricErrorData*)source_vert->errorData;
        
        Vec4 dv(genV->coord[0], genV->coord[1], genV->coord[2], 1);
        
        float dest_err = (D->m11+S->m11)*dv.X*dv.X+2*(D->m21+S->m21)*dv.X*dv.Y+2*(D->m31+S->m31)*dv.X*dv.Z+
            2*(D->m41+S->m41)*dv.X+(D->m22+S->m22)*dv.Y*dv.Y+2*(D->m32+S->m32)*dv.Y*dv.Z+2*(D->m42+S->m42)*dv.Y+
            (D->m33+S->m33)*dv.Z*dv.Z+2*(D->m43+S->m43)*dv.Z+(D->m44+S->m44);

        error=fabs(dest_err);
        error=sqrt(error);
        delete genV;
    }
    
    return error;
}

xbsVertex *QuadricEdgeError::genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *op,
                                       int forceGen)
{
    Vec4 newCoord;

    float t;
    
    EdgeCollapse *edge = (EdgeCollapse *)op;
    
    EdgeCollapseCase ECcase = edge->computeCase(model);
    
    xbsVertex *source_vert = edge->getSource();
    xbsVertex *destination_vert = edge->getDestination();
    
    int lock = model->borderLock;
    
    if ((lock == 1) && (source_vert->onBorder() == 1) &&
        destination_vert->onBorder() == 1)
    {
        fprintf(stderr, "Have to disable border lock for a vertex. Sorry.\n");
        lock = 0;
    }

    switch (ECcase)
    {
        case MoveBoth:
        {
            // general case -- both vertices can move
            Mat4 m, minv;
            QuadricErrorData *D = (QuadricErrorData*)destination_vert->errorData;
            QuadricErrorData *S = (QuadricErrorData*)source_vert->errorData;
            m.SetRow(0, Vec4(D->m11+S->m11, D->m21+S->m21, D->m31+S->m31, D->m41+S->m41));
            m.SetRow(1, Vec4(D->m21+S->m21, D->m22+S->m22, D->m32+S->m32, D->m42+S->m42));
            m.SetRow(2, Vec4(D->m31+S->m31, D->m32+S->m32, D->m33+S->m33, D->m43+S->m43));
            m=m*.5;
            m.SetRow(3, Vec4(0,0,0,1));
            minv=m.Inverse();
            if ((minv.cells[0][0]==0) && (minv.cells[0][1]==0) && (minv.cells[0][2]==0) && (minv.cells[0][3]==0) &&
                (minv.cells[1][0]==0) && (minv.cells[1][1]==0) && (minv.cells[1][2]==0) && (minv.cells[1][3]==0) &&
                (minv.cells[2][0]==0) && (minv.cells[2][1]==0) && (minv.cells[2][2]==0) && (minv.cells[2][3]==0))
            {
                xbsVec3 sv = source_vert->coord;
                float source_err = (D->m11+S->m11)*sv[0]*sv[0]+2*(D->m21+S->m21)*sv[0]*sv[1]+2*(D->m31+S->m31)*sv[0]*sv[2]+
                    2*(D->m41+S->m41)*sv[0]+(D->m22+S->m22)*sv[1]*sv[1]+2*(D->m32+S->m32)*sv[1]*sv[2]+2*(D->m42+S->m42)*sv[1]+
                    (D->m33+S->m33)*sv[2]*sv[2]+2*(D->m43+S->m43)*sv[2]+(D->m44+S->m44);
                sv = destination_vert->coord;
                float dest_err = (D->m11+S->m11)*sv[0]*sv[0]+2*(D->m21+S->m21)*sv[0]*sv[1]+2*(D->m31+S->m31)*sv[0]*sv[2]+
                    2*(D->m41+S->m41)*sv[0]+(D->m22+S->m22)*sv[1]*sv[1]+2*(D->m32+S->m32)*sv[1]*sv[2]+2*(D->m42+S->m42)*sv[1]+
                    (D->m33+S->m33)*sv[2]*sv[2]+2*(D->m43+S->m43)*sv[2]+(D->m44+S->m44);
                sv = (destination_vert->coord+source_vert->coord)*.5;
                float mid_err = (D->m11+S->m11)*sv[0]*sv[0]+2*(D->m21+S->m21)*sv[0]*sv[1]+2*(D->m31+S->m31)*sv[0]*sv[2]+
                    2*(D->m41+S->m41)*sv[0]+(D->m22+S->m22)*sv[1]*sv[1]+2*(D->m32+S->m32)*sv[1]*sv[2]+2*(D->m42+S->m42)*sv[1]+
                    (D->m33+S->m33)*sv[2]*sv[2]+2*(D->m43+S->m43)*sv[2]+(D->m44+S->m44);
                float terror = MIN(MIN(source_err, dest_err), mid_err);
                if (terror==source_err){
                    t=1.0f;
                }
                else if (terror==dest_err){
                    t=0.0f;
                }
                else /*(terror== mid_err)*/{
                    xbsVec3 temp = (destination_vert->coord+source_vert->coord)*.5;
                    t=0.5f;
                }
            }
            else {
                newCoord = minv*Vec4(0,0,0,1);
                t=-1.0f;
            }
            break;
        }
        case MoveSource:
        {
            t = 1.0;
            break;
        }
        case MoveDestination:
        {
            t = 0.0;
            break;
        }
        case MoveNeither:
        {
            if (forceGen == false)
                return NULL;
            if (destination_vert->onBorder() == 0)
            {
                t = 0.0;
            }
            else
            {
                t = 1.0;
            }
            break;
        }
    }
    

    xbsVertex *generated_vert = source_vert->makeNew();
#if 0
    if ((v1 == NULL) && (v2 == NULL) ) {
        if (t!=-1)
            generated_vert->interp(source_vert, destination_vert, t);
        else
            generated_vert->interp(source_vert, destination_vert, .5);
        return generated_vert;
    }
#endif
#if 1
    if ((v1 == NULL) && (v2 == NULL))
    {
        fprintf(stderr, "generateVertex(): Invalid attribute vertices!\n");
        exit(1);
    }
#endif 
    if ((v1 == NULL) || (v2 == NULL))
    {
        if (v1 == NULL)
            v2->copySame(generated_vert);
        else if (v2 == NULL)
            v1->copySame(generated_vert);
        
        xbsVertex *geom_vert = source_vert->makeNew();
        geom_vert->interp(source_vert, destination_vert, t);
        generated_vert->coord = geom_vert->coord;
        delete geom_vert;
    }
    else {
        if (t!=-1)
            generated_vert->interp(v1, v2, t);
        else
            generated_vert->interp(v1, v2, .5);
    }

    if (t==-1.0)
        generated_vert->coord.set(newCoord.X, newCoord.Y, newCoord.Z);
    generated_vert->tris = NULL;
    generated_vert->numTris = 0;
    generated_vert->ops = NULL;
    generated_vert->numOps = 0;
    generated_vert->errorData = new QuadricErrorData(generated_vert);
    QuadricErrorData *T=(QuadricErrorData*)generated_vert->errorData;
    T->m11=0; 
    T->m21=0; T->m22=0;
    T->m31=0; T->m32=0; T->m33=0;
    T->m41=0; T->m42=0; T->m43=0; T->m44=0;
    QuadricErrorData *S=(QuadricErrorData*)source_vert->errorData;
    T->m11+=S->m11; 
    T->m21+=S->m21; T->m22+=S->m22;
    T->m31+=S->m31; T->m32+=S->m32; T->m33+=S->m33;
    T->m41+=S->m41; T->m42+=S->m42; T->m43+=S->m43; T->m44+=S->m44;
    S=(QuadricErrorData*)destination_vert->errorData;
    T->m11+=S->m11; 
    T->m21+=S->m21; T->m22+=S->m22;
    T->m31+=S->m31; T->m32+=S->m32; T->m33+=S->m33;
    T->m41+=S->m41; T->m42+=S->m42; T->m43+=S->m43; T->m44+=S->m44;
    T->m11*=.5; 
    T->m21*=.5; T->m22*=.5;
    T->m31*=.5; T->m32*=.5; T->m33*=.5;
    T->m41*=.5; T->m42*=.5; T->m43*=.5; T->m44*=.5;
    
//    generated_vert->errorData->setError(error);
    return generated_vert;
    
}

void QuadricErrorData::init(xbsVertex *vert) {
    m11=m21=m22=m31=m32=m33=m41=m42=m43=m44=0;
    float numPlanes=0;
    for (int i=0; i<vert->numTris; i++){
        xbsVec3 v=vert->tris[i]->verts[0]->coord;
        xbsVec3 v1=vert->tris[i]->verts[1]->coord-vert->tris[i]->verts[0]->coord;
        xbsVec3 v2=vert->tris[i]->verts[2]->coord-vert->tris[i]->verts[0]->coord;
        xbsVec3 v3=v1^v2;
        float a=v3[0];
        float b=v3[1];
        float c=v3[2];
        float d=-(a*v[0]+b*v[1]+c*v[2]);
        m11+=a*a;
        m21+=a*b; m22+=b*b;
        m31+=a*c; m32+=b*c; m33+=c*c;
        m41+=a*d; m42+=b*d; m43+=c*d; m44+=d*d;
        numPlanes++;
    }
    m11/=numPlanes; 
    m21/=numPlanes; m22/=numPlanes; 
    m31/=numPlanes; m32/=numPlanes; m33/=numPlanes;
    m41/=numPlanes; m42/=numPlanes; m43/=numPlanes; m44/=numPlanes;
//    setError(0.0f);
}

void QuadricErrorData::update(Operation *op){
    xbsVertex *source_vert = op->getSource();
    //Vec4 sv(vert->coord[0], vert->coord[1], vert->coord[2], 1);
    QuadricErrorData *S=(QuadricErrorData*)source_vert->errorData;
    m11+=S->m11; 
    m21+=S->m21; m22+=S->m22;
    m31+=S->m31; m32+=S->m32; m33+=S->m33;
    m41+=S->m41; m42+=S->m42; m43+=S->m43; m44+=S->m44;
    m11*=.5; 
    m21*=.5; m22*=.5; 
    m31*=.5; m32*=.5; m33*=.5;
    m41*=.5; m42*=.5; m43*=.5; m44*=.5;
}


/************************************************************************************/
/*   PermissionGrid error stuff                                                            */
/************************************************************************************/

float PermissionGridHalfEdgeError::calculateError(Model *model, Operation *op)
{
    int t;
    xbsVertex *vert_iter;
    xbsVertex *source_vert = op->getSource();
    xbsVertex *destination_vert = op->getDestination();
    
    Vec4 dv(destination_vert->coord[0], destination_vert->coord[1],
            destination_vert->coord[2], 1); 
    PermissionGridErrorData *D = (PermissionGridErrorData*)destination_vert->errorData;
    PermissionGridErrorData *S = (PermissionGridErrorData*)source_vert->errorData;
    float dest_err = 
        (D->m11+S->m11)*dv.X*dv.X + 
        2*(D->m21+S->m21)*dv.X*dv.Y +
        2*(D->m31+S->m31)*dv.X*dv.Z +
        2*(D->m41+S->m41)*dv.X +
        (D->m22+S->m22)*dv.Y*dv.Y +
        2*(D->m32+S->m32)*dv.Y*dv.Z +
        2*(D->m42+S->m42)*dv.Y+
        (D->m33+S->m33)*dv.Z*dv.Z +
        2*(D->m43+S->m43)*dv.Z + (D->m44+S->m44);
    
    error=fabs(dest_err);
    error = sqrt(error);

    vert_iter = destination_vert;
    do
    {
        for (t=0; t < vert_iter->numTris; ++t)
        {
            if (!model->permissionGrid->triangleIsValid(vert_iter->tris[t]))
            {
                error = MAXFLOAT;
                return error;
            }
        }
        vert_iter = vert_iter->nextCoincident;
    } while (vert_iter != destination_vert);
    
    return error;
}

float PermissionGridEdgeError::calculateError(Model *model, Operation *op)
{ 
    int t;
    xbsVertex *vert_iter;

    EdgeCollapse *edge = (EdgeCollapse *)op;

    xbsVertex *source_vert = edge->getSource();
    xbsVertex *destination_vert = edge->getDestination();

    xbsVertex *genV = genVertex(model, source_vert, destination_vert, edge, 0);

    if (genV == NULL)
        error = MAXFLOAT;
    else
    {
        PermissionGridErrorData *D = (PermissionGridErrorData*)destination_vert->errorData;
        PermissionGridErrorData *S = (PermissionGridErrorData*)source_vert->errorData;
        
        Vec4 dv(genV->coord[0], genV->coord[1], genV->coord[2], 1);
        
        float dest_err = (D->m11+S->m11)*dv.X*dv.X+2*(D->m21+S->m21)*dv.X*dv.Y+2*(D->m31+S->m31)*dv.X*dv.Z+
            2*(D->m41+S->m41)*dv.X+(D->m22+S->m22)*dv.Y*dv.Y+2*(D->m32+S->m32)*dv.Y*dv.Z+2*(D->m42+S->m42)*dv.Y+
            (D->m33+S->m33)*dv.Z*dv.Z+2*(D->m43+S->m43)*dv.Z+(D->m44+S->m44);

        error=fabs(dest_err);
        error=sqrt(error);

        // need to check against permission grid
        vert_iter = destination_vert;
        do
        {
            for (t=0; t < vert_iter->numTris; ++t)
            {
                if (!model->permissionGrid->triangleIsValid(vert_iter->tris[t]))
                {
                    error = MAXFLOAT;
                    return error;
                }
            }
            vert_iter = vert_iter->nextCoincident;
        } while (vert_iter != destination_vert && error != MAXFLOAT);
    
        delete genV;
    }
    
    return error;
}

xbsVertex *PermissionGridEdgeError::genVertex(Model *model, xbsVertex *v1, xbsVertex *v2, Operation *op,
                                       int forceGen)
{
    Vec4 newCoord;

    float t;
    
    EdgeCollapse *edge = (EdgeCollapse *)op;
    
    EdgeCollapseCase ECcase = edge->computeCase(model);
    
    xbsVertex *source_vert = edge->getSource();
    xbsVertex *destination_vert = edge->getDestination();
    
    int lock = model->borderLock;
    
    if ((lock == 1) && (source_vert->onBorder() == 1) &&
        destination_vert->onBorder() == 1)
    {
        fprintf(stderr, "Have to disable border lock for a vertex. Sorry.\n");
        lock = 0;
    }

    switch (ECcase)
    {
        case MoveBoth:
        {
            // general case -- both vertices can move
            Mat4 m, minv;
            PermissionGridErrorData *D = (PermissionGridErrorData*)destination_vert->errorData;
            PermissionGridErrorData *S = (PermissionGridErrorData*)source_vert->errorData;
            m.SetRow(0, Vec4(D->m11+S->m11, D->m21+S->m21, D->m31+S->m31, D->m41+S->m41));
            m.SetRow(1, Vec4(D->m21+S->m21, D->m22+S->m22, D->m32+S->m32, D->m42+S->m42));
            m.SetRow(2, Vec4(D->m31+S->m31, D->m32+S->m32, D->m33+S->m33, D->m43+S->m43));
            m=m*.5;
            m.SetRow(3, Vec4(0,0,0,1));
            minv=m.Inverse();
            if ((minv.cells[0][0]==0) && (minv.cells[0][1]==0) && (minv.cells[0][2]==0) && (minv.cells[0][3]==0) &&
                (minv.cells[1][0]==0) && (minv.cells[1][1]==0) && (minv.cells[1][2]==0) && (minv.cells[1][3]==0) &&
                (minv.cells[2][0]==0) && (minv.cells[2][1]==0) && (minv.cells[2][2]==0) && (minv.cells[2][3]==0))
            {
                xbsVec3 sv = source_vert->coord;
                float source_err = (D->m11+S->m11)*sv[0]*sv[0]+2*(D->m21+S->m21)*sv[0]*sv[1]+2*(D->m31+S->m31)*sv[0]*sv[2]+
                    2*(D->m41+S->m41)*sv[0]+(D->m22+S->m22)*sv[1]*sv[1]+2*(D->m32+S->m32)*sv[1]*sv[2]+2*(D->m42+S->m42)*sv[1]+
                    (D->m33+S->m33)*sv[2]*sv[2]+2*(D->m43+S->m43)*sv[2]+(D->m44+S->m44);
                sv = destination_vert->coord;
                float dest_err = (D->m11+S->m11)*sv[0]*sv[0]+2*(D->m21+S->m21)*sv[0]*sv[1]+2*(D->m31+S->m31)*sv[0]*sv[2]+
                    2*(D->m41+S->m41)*sv[0]+(D->m22+S->m22)*sv[1]*sv[1]+2*(D->m32+S->m32)*sv[1]*sv[2]+2*(D->m42+S->m42)*sv[1]+
                    (D->m33+S->m33)*sv[2]*sv[2]+2*(D->m43+S->m43)*sv[2]+(D->m44+S->m44);
                sv = (destination_vert->coord+source_vert->coord)*.5;
                float mid_err = (D->m11+S->m11)*sv[0]*sv[0]+2*(D->m21+S->m21)*sv[0]*sv[1]+2*(D->m31+S->m31)*sv[0]*sv[2]+
                    2*(D->m41+S->m41)*sv[0]+(D->m22+S->m22)*sv[1]*sv[1]+2*(D->m32+S->m32)*sv[1]*sv[2]+2*(D->m42+S->m42)*sv[1]+
                    (D->m33+S->m33)*sv[2]*sv[2]+2*(D->m43+S->m43)*sv[2]+(D->m44+S->m44);
                float terror = MIN(MIN(source_err, dest_err), mid_err);
                if (terror==source_err){
                    t=1.0f;
                }
                else if (terror==dest_err){
                    t=0.0f;
                }
                else /*(terror== mid_err)*/{
                    xbsVec3 temp = (destination_vert->coord+source_vert->coord)*.5;
                    t=0.5f;
                }
            }
            else {
                newCoord = minv*Vec4(0,0,0,1);
                t=-1.0f;
            }
            break;
        }
        case MoveSource:
        {
            t = 1.0;
            break;
        }
        case MoveDestination:
        {
            t = 0.0;
            break;
        }
        case MoveNeither:
        {
            if (forceGen == false)
                return NULL;
            if (destination_vert->onBorder() == 0)
            {
                t = 0.0;
            }
            else
            {
                t = 1.0;
            }
            break;
        }
    }
    

    xbsVertex *generated_vert = source_vert->makeNew();
#if 0
    if ((v1 == NULL) && (v2 == NULL) ) {
        if (t!=-1)
            generated_vert->interp(source_vert, destination_vert, t);
        else
            generated_vert->interp(source_vert, destination_vert, .5);
        return generated_vert;
    }
#endif
#if 1
    if ((v1 == NULL) && (v2 == NULL))
    {
        fprintf(stderr, "generateVertex(): Invalid attribute vertices!\n");
        exit(1);
    }
#endif 
    if ((v1 == NULL) || (v2 == NULL))
    {
        if (v1 == NULL)
            v2->copySame(generated_vert);
        else if (v2 == NULL)
            v1->copySame(generated_vert);
        
        xbsVertex *geom_vert = source_vert->makeNew();
        geom_vert->interp(source_vert, destination_vert, t);
        generated_vert->coord = geom_vert->coord;
        delete geom_vert;
    }
    else {
        if (t!=-1)
            generated_vert->interp(v1, v2, t);
        else
            generated_vert->interp(v1, v2, .5);
    }

    if (t==-1.0)
        generated_vert->coord.set(newCoord.X, newCoord.Y, newCoord.Z);
    generated_vert->tris = NULL;
    generated_vert->numTris = 0;
    generated_vert->ops = NULL;
    generated_vert->numOps = 0;
    generated_vert->errorData = new PermissionGridErrorData(generated_vert);
    PermissionGridErrorData *T=(PermissionGridErrorData*)generated_vert->errorData;
    T->m11=0; 
    T->m21=0; T->m22=0;
    T->m31=0; T->m32=0; T->m33=0;
    T->m41=0; T->m42=0; T->m43=0; T->m44=0;
    PermissionGridErrorData *S=(PermissionGridErrorData*)source_vert->errorData;
    T->m11+=S->m11; 
    T->m21+=S->m21; T->m22+=S->m22;
    T->m31+=S->m31; T->m32+=S->m32; T->m33+=S->m33;
    T->m41+=S->m41; T->m42+=S->m42; T->m43+=S->m43; T->m44+=S->m44;
    S=(PermissionGridErrorData*)destination_vert->errorData;
    T->m11+=S->m11; 
    T->m21+=S->m21; T->m22+=S->m22;
    T->m31+=S->m31; T->m32+=S->m32; T->m33+=S->m33;
    T->m41+=S->m41; T->m42+=S->m42; T->m43+=S->m43; T->m44+=S->m44;
    T->m11*=.5; 
    T->m21*=.5; T->m22*=.5;
    T->m31*=.5; T->m32*=.5; T->m33*=.5;
    T->m41*=.5; T->m42*=.5; T->m43*=.5; T->m44*=.5;
    
//    generated_vert->errorData->setError(error);
    return generated_vert;
    
}

void PermissionGridErrorData::init(xbsVertex *vert) {
    m11=m21=m22=m31=m32=m33=m41=m42=m43=m44=0;
    float numPlanes=0;
    for (int i=0; i<vert->numTris; i++){
        xbsVec3 v=vert->tris[i]->verts[0]->coord;
        xbsVec3 v1=vert->tris[i]->verts[1]->coord-vert->tris[i]->verts[0]->coord;
        xbsVec3 v2=vert->tris[i]->verts[2]->coord-vert->tris[i]->verts[0]->coord;
        xbsVec3 v3=v1^v2;
        float a=v3[0];
        float b=v3[1];
        float c=v3[2];
        float d=-(a*v[0]+b*v[1]+c*v[2]);
        m11+=a*a;
        m21+=a*b; m22+=b*b;
        m31+=a*c; m32+=b*c; m33+=c*c;
        m41+=a*d; m42+=b*d; m43+=c*d; m44+=d*d;
        numPlanes++;
    }
    m11/=numPlanes; 
    m21/=numPlanes; m22/=numPlanes; 
    m31/=numPlanes; m32/=numPlanes; m33/=numPlanes;
    m41/=numPlanes; m42/=numPlanes; m43/=numPlanes; m44/=numPlanes;
//    setError(0.0f);
}

void PermissionGridErrorData::update(Operation *op){
    xbsVertex *source_vert = op->getSource();
    //Vec4 sv(vert->coord[0], vert->coord[1], vert->coord[2], 1);
    PermissionGridErrorData *S=(PermissionGridErrorData*)source_vert->errorData;
    m11+=S->m11; 
    m21+=S->m21; m22+=S->m22;
    m31+=S->m31; m32+=S->m32; m33+=S->m33;
    m41+=S->m41; m42+=S->m42; m43+=S->m43; m44+=S->m44;
    m11*=.5; 
    m21*=.5; m22*=.5; 
    m31*=.5; m32*=.5; m33*=.5;
    m41*=.5; m42*=.5; m43*=.5; m44*=.5;
}


/***************************************************************************
 * $Log: Metric.C,v $
 * Revision 1.12  2004/12/08 15:21:13  jdt6a
 * Fixed bugs in voxelizer.  Implemented easy triangle test method.  See glod email I'm about to send for more details.
 *
 * Revision 1.11  2004/09/14 20:14:01  jdt6a
 * added voxelization against all 7 planes now.  no significant visual difference.
 *
 * still need to create the triangle to test correctly within the error metric class (which is completely wrong now, and may require some assistance from someone who knows more about glod than i) and rewrite the triangle validation method.
 *
 * -j
 *
 * Revision 1.10  2004/08/04 20:02:21  gfx_friends
 * G++ doesn't let you delete an undefined class (e.g. you say class Foo; then you delete it in some other class's destructor). This came up in the way that Model.h and PermissionGrid.h depended on each other. I moved ~Model to the .C file and modified includes accordingly. --nat
 *
 * Revision 1.9  2004/07/28 06:07:10  jdt6a
 * more permission grid work.  most of voxelization code from dachille/kaufman paper in place, but only testing against plane of triangle right now (not the other 6 planes yet).
 *
 * run simple.exe with a "-pg" flag to get the permission grid version (which isn't fully working yet... for some reason the single plane testing which should be very conservative results in too strict of a grid, or i am not testing the grid correctly).  the point sampled version actually results in better, aka more simplified, models, so i think there is a bug somewhere in the voxelization or testing.
 *
 * after a run of simple, a file "pg.dat" will be dumped into the current directory.  the pgvis program lets you visualize this file, which is just the grid.
 *
 * Revision 1.8  2004/07/22 16:38:01  jdt6a
 * prelimary permission grid stuff is set up.  now to integrate this into glod for real.
 *
 * Revision 1.7  2004/07/19 19:18:43  gfx_friends
 * Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
 *
 * Revision 1.6  2004/07/14 16:20:44  gfx_friends
 * normalized the quadric error calculations, so that the error values are not increasing exponentially
 *
 * Revision 1.5  2004/07/14 14:59:50  gfx_friends
 * Made handling of border heuristics more consistent. Cylinder now
 * simplifies again, and the torus patch borders work pretty well, too.
 * The case where borders are not preserved too well right now is using
 * full edge collapses and error quadrics, because the location of the
 * generated vertex does not in general lie along the collapsed edge
 * (it is chosen by an optimization involving an inversion of the quadric
 * matrix, yada yada yada). We may improve this by adding additional border
 * edge planes into the quadric, as done in some papers by Garland and
 * Lindstrom.
 *
 * Revision 1.4  2004/07/13 22:02:57  gfx_friends
 * Major performance gains by fixing bad mat4 vec4 code.
 *
 * Revision 1.3  2004/07/08 16:47:49  gfx_friends
 * Removed tabs and updated indentation for xbs source files
 *
 * Revision 1.2  2004/06/25 16:49:50  gfx_friends
 * changed the calculate error functions for quadric errors a bit
 *
 * Revision 1.1  2004/06/24 22:16:38  gfx_friends
 * change of location for Metric.C, was accidentaly in api
 *
 * Revision 1.1  2004/06/24 21:54:56  gfx_friends
 * new files handling the new and improved error metric stuff
 *
 ***************************************************************************/
