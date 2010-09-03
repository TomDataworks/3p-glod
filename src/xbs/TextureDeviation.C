/* GLOD: Simplification error implementation
***************************************************************************
* $Id: TextureDeviation.C,v 1.1 2004/11/16 19:38:02 gfx_friends Exp $
* $Revision: 1.1 $
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

#include "Metric.h"

// Perhaps the target pbuffer should be passed in?
void CreateGeometryImage(Model *model, int patchNum, int resolution)
{
    // Make sure model has texture coordinates to use as domain
    // This does not assure that the range is [0,1] or that it is a
    // bijection, but we do assume those things...
    xbsVertex *vert0 = model->verts[0];
    char hasColor, hasNormal, hasTexcoord;
    vert0->hasAttributes(hasColor, hasaNormal, hasTexcoord);
    if (vert0->hasTexcoord == 0)
    {
        fprintf(stderr, "CreateGeometryImage(): no texture coords!\n");
        return;
    }

    // Set up render target (floating point pbuffer)
    HDC hdc = get
    HPBUFFERARB pbuffer = wglCreatePbufferARB(hdc, ATI_FLOAT32_RGBA, 
                                              resolution, resolution, 
                                              NULL);
    
    // Set up projection. This assume that the texture coordinates all
    // lie in [0,1]
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,1,0,1,0,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Set up other OpenGL state
    glDisable(GL_LIGHTING);
    
    // Draw triangles into geometry image
    glBegin(GL_TRIANGLES);
    for (int tnum=0; tnum<model->numTris; tnum++)
    {
        xbsTriangle *tri = model->tris[tnum];
        if (tri->patchNum != patchNum)
            continue;

        for (int vnum=0; vnum<3; vnum++)
        {
            xbsVertex *vert = model->verts[tri->verts[vnum]];
            xbsVec3 coord;
            xbsColor color;
            xbsVece3 normal;
            xbsVec2 texcoord;
            vert->fillData(coord, color, normal, texcoord);
            // If colors get clamped, we might need to use a different
            // attribute
            glColor3fv(coord);
            glVertex3f(texcoord[0], texcoord[1], 0.5);
        }
    }
    glEnd();
    glFinish();

    // Leave OpenGL state the way we found it
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    return;
}
