// at some point, it makes sense for glod to simply link to and use the 
// stderror and stdrender libraries included with the vds distribution,
// replacing these callbacks, as they are just copied from those libraries
// (probably not worth the trouble to do it now, but putting this here as a
// reminder)
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

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glx.h>
#endif
#endif

#include "vds_callbacks.h"
#include "Hierarchy.h"

#define BIGFLOAT 3e38

using namespace VDS;

// RENDERING CALLBACKS ******************************************
void ImmediateModeRenderCallback(VDS::Renderer &rRenderer, VDS::PatchIndex PatchID)
{
    bool HasColors = rRenderer.mpPatchTriData[PatchID].ColorsPresent;
    bool HasNormals = rRenderer.mpPatchTriData[PatchID].NormalsPresent;
    VDS::TriIndex tri;
    VDS::TriIndex NumTris = rRenderer.mpPatchTriData[PatchID].NumTris ? (rRenderer.mpPatchTriData[PatchID].LastActiveTri + 1) : 0;
    const VDS::TriProxy *tri_array = rRenderer.mpPatchTriData[PatchID].TriProxiesArray;
    const VDS::TriProxyBackRef *tri_backref_array = rRenderer.mpPatchTriData[PatchID].TriProxyBackRefs;
    const VDS::VertexRenderDatum *vertex_array = rRenderer.mpVertexRenderData;

    for(tri = 0; tri < NumTris; ++tri)
    {
        glBegin(GL_TRIANGLES);
        if (HasColors)
            glColor4ubv((const GLubyte *) &vertex_array[tri_array[tri][0]].Color);
        if (HasNormals)
            glNormal3fv((const float *) &vertex_array[tri_array[tri][0]].Normal);
        glVertex3fv((const float *) &vertex_array[tri_array[tri][0]].Position);
        if (HasColors)
            glColor4ubv((const GLubyte *) &vertex_array[tri_array[tri][1]].Color);
        if (HasNormals)
            glNormal3fv((const float *) &vertex_array[tri_array[tri][1]].Normal);
        glVertex3fv((const float *) &vertex_array[tri_array[tri][1]].Position);
        if (HasColors)
            glColor4ubv((const GLubyte *) &vertex_array[tri_array[tri][2]].Color);
        if (HasNormals)
            glNormal3fv((const float *) &vertex_array[tri_array[tri][2]].Normal);
        glVertex3fv((const float *) &vertex_array[tri_array[tri][2]].Position);
        glEnd();
    }
}

void FastRenderCallback(Renderer &rRenderer, PatchIndex PatchID)
{
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    glDisable(GL_LIGHTING);
//    glDisable(GL_COLOR_MATERIAL);
//    glShadeModel(GL_FLAT);

    VDS::TriIndex NumTris = rRenderer.mpPatchTriData[PatchID].NumTris ? (rRenderer.mpPatchTriData[PatchID].LastActiveTri + 1) : 0;

// TODO: check to see if vertex data for this renderer is already uploaded (store VertexData pointer)
    VertexRenderDatum *VertexData;
    if (!rRenderer.mUseFastMemory)
    {
        VertexData = rRenderer.mpSystemVertexRenderData;
    }
    else if (rRenderer.mCopyDataToFastMemoryPerFrame)
    {
        rRenderer.CopyVertexDataToFastMemory();
        VertexData = rRenderer.mpFastVertexRenderData;
    }
    else
    {
        VertexData = rRenderer.mpFastVertexRenderData;
    }

    if (rRenderer.mpPatchTriData[PatchID].NormalsPresent)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, rRenderer.mVertexDataStride, &(VertexData[0].Normal));
    }
    else
        glDisableClientState(GL_NORMAL_ARRAY);

    if (rRenderer.mpPatchTriData[PatchID].ColorsPresent)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, rRenderer.mVertexDataStride, &(VertexData[0].Color));
    }
    else
        glDisableClientState(GL_COLOR_ARRAY);

    // textures not implemented yet
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, rRenderer.mVertexDataStride, VertexData);

// this call not needed right now because glVertexArrayRangeNV() is called over entire array when memory is allocated to renderer
//      if (rRenderer.mpMemoryManager->mExtensionsAvailable == 2)
//              rRenderer.mpMemoryManager->glVertexArrayRangeNV(rRenderer.mNumVertices * sizeof(VertexRenderDatum), rRenderer.mpFastVertexRenderData);

    glDrawElements(GL_TRIANGLES, NumTris * 3, GL_UNSIGNED_INT, rRenderer.mpPatchTriData[PatchID].TriProxiesArray);

}

void VBOFastRenderCallback(VDS::Renderer &rRenderer, VDS::PatchIndex PatchID)
{
    VDS::TriIndex NumTris = rRenderer.mpPatchTriData[PatchID].NumTris ? (rRenderer.mpPatchTriData[PatchID].LastActiveTri + 1) : 0;

    if (rRenderer.mpPatchTriData[PatchID].NormalsPresent)
        glNormalPointer(GL_FLOAT, rRenderer.mVertexDataStride, (const GLvoid *)(0 + sizeof(VDS::Point3)));
    if (rRenderer.mpPatchTriData[PatchID].ColorsPresent)
        glColorPointer(4, GL_FLOAT, rRenderer.mVertexDataStride, (const GLvoid *)(0 + sizeof(VDS::Point3) + sizeof(VDS::Vec3)));
    if (rRenderer.mpCut->mpForest->mNumTextures > 0)
        glTexCoordPointer(2, GL_FLOAT, rRenderer.mVertexDataStride, (const GLvoid *)(0 + sizeof(VDS::Point3) + sizeof(VDS::Vec3) + sizeof(VDS::ByteColorA)));

    glVertexPointer(3, GL_FLOAT, rRenderer.mVertexDataStride, (const GLvoid *) NULL);

    glDrawElements(GL_TRIANGLES, (NumTris) * 3, GL_UNSIGNED_INT, rRenderer.mpPatchTriData[PatchID].TriProxiesArray);
//      glDrawRangeElements(GL_TRIANGLES, 0, rRenderer.mNumVertices, (NumTris) * 3, GL_UNSIGNED_INT, rRenderer.mpPatchTriData[PatchID].TriProxiesArray);

#ifdef TIMING_LEVEL_2
    rRenderer.times_30.LowPart = (time_after_copy.LowPart - time_before_copy.LowPart);
#endif
}

// NODE ERROR CALLBACKS ******************************************

VDS::Float StdErrorScreenSpace(VDS::BudgetItem *pItem, const VDS::Cut *pCut)
{
    GLOD_View *view = (GLOD_View *)pCut->mpExternalViewClass;
    xbsVec3 center(pItem->mBBoxCenter.X, pItem->mBBoxCenter.Y, pItem->mBBoxCenter.Z);
    xbsVec3 offsets(pItem->mXBBoxOffset, pItem->mYBBoxOffset, pItem->mZBBoxOffset);
    return view->computePixelsOfError(center, offsets, pCut->mpForest->mpErrorParams[pCut->mpForest->mpNodes[pItem->miNode].miErrorParamIndex]); // 1
}

VDS::Float StdErrorScreenSpaceNoFrustum(VDS::BudgetItem *pItem, const VDS::Cut *pCut)
{
    GLOD_View *view = (GLOD_View *)pCut->mpExternalViewClass;
    xbsVec3 center(pItem->mBBoxCenter.X, pItem->mBBoxCenter.Y, pItem->mBBoxCenter.Z);
    xbsVec3 offsets(pItem->mXBBoxOffset, pItem->mYBBoxOffset, pItem->mZBBoxOffset);
    return view->computePixelsOfError(center, offsets, pCut->mpForest->mpErrorParams[pCut->mpForest->mpNodes[pItem->miNode].miErrorParamIndex]); // 1
}

VDS::Float StdErrorObjectSpace(VDS::BudgetItem *pItem, const VDS::Cut *pCut)
{
    GLOD_View *view = (GLOD_View *)pCut->mpExternalViewClass;
    xbsVec3 center(pItem->mBBoxCenter.X, pItem->mBBoxCenter.Y, pItem->mBBoxCenter.Z);
    xbsVec3 offsets(pItem->mXBBoxOffset, pItem->mYBBoxOffset, pItem->mZBBoxOffset);
    //return view->computePixelsOfError(center, offsets,  pCut->mpForest->mpErrorParams[pCut->mpForest->mpNodes[pItem->miNode].miErrorParamIndex]);
    if (view->checkFrustrum(center,offsets)==1){
        return pCut->mpForest->mpErrorParams[pCut->mpForest->mpNodes[pItem->miNode].miErrorParamIndex];
    }
    else 
        return 0;
}


VDS::Float StdErrorObjectSpaceNoFrustum(VDS::BudgetItem *pItem, const VDS::Cut *pCut)
{
    return pCut->mpForest->mpErrorParams[pCut->mpForest->mpNodes[pItem->miNode].miErrorParamIndex];
}
