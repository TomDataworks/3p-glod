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
/**
 * @author 	Budirijanto Purnomo
 * @version 1.0, 06/03/2002
 * @dependency Defines.h, Util.h
 *
 * This is a collection of utility functions that I use many times.
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <string.h>

#include "Util.h"

/**
 * RenderText:  This function renders one line of text to a window
 * @author  Budirijanto Purnomo
 * @param  text         text to be rendered
 *         x, y   position(x, y) to render the text
 *                x is from left to right, y is from bottom to top
 *         rectangle_width    width of the rectangle (base of the text)
 */
void RenderText(char *text, int x, int y, int rectangle_width)
{
   int WW = glutGet((GLenum)GLUT_WINDOW_WIDTH);
   int WH = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
   int i;

   glPushAttrib(GL_POLYGON_BIT);
   glPolygonMode(GL_FRONT, GL_FILL);

   glPushAttrib(GL_LIGHTING_BIT);
   glDisable(GL_LIGHTING);
   //glDisable(GL_COLOR_MATERIAL);

   glPushAttrib(GL_COLOR_BUFFER_BIT);
   glDisable(GL_DITHER);

   glEnable(GL_BLEND);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glPushAttrib(GL_DEPTH_BUFFER_BIT);
   glDisable(GL_DEPTH_TEST);

   glColor3f(1, 1, 1);
   glPushAttrib(GL_VIEWPORT_BIT);

   glViewport(0, 0, WW, WH);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
      glLoadIdentity();
      gluOrtho2D(0, WW-1, 0, WH-1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
         glLoadIdentity();
  
         if (rectangle_width > 0)
         {
            glColor4f(0, 0, 1.0, 0.4);
            glBegin(GL_POLYGON);
               glVertex2f(x-5, y-4);
               glVertex2f(x+rectangle_width, y-4);
               glVertex2f(x+rectangle_width, y+15);
               glVertex2f(x-5, y+15);
            glEnd();
         }

         glColor3f(1, 1, 1);
         if (x>=0 && y>=0)
            glRasterPos2i(x,y);

         for (i=0; i<(int) strlen(text); i++)
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);
 
        glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glPopAttrib();
   glPopAttrib();
   glPopAttrib();
   glPopAttrib();
   glPopAttrib();
}
