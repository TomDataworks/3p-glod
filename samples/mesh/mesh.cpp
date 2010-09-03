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
/*
 * mesh.cpp- Modified version of 'simple' app for testing
 * variable-sized mesh datasets.  Currently only creates one patch.  Adjust
 * s_num_cells and s_bump_height to change size and bumpiness of mesh.
 * 
 * History:
 * - Rich Holloway, 3rdTech, 4/04
 * - Original: Simple app by Nat Duca JHU, October, 2002
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <GL/gl.h>

#include <math.h>
#ifndef _WIN32
# include <values.h>
#else
# include <float.h>
#endif
#include <ctype.h>

#include <assert.h>

#include <time.h>
   
#include "glod.h"
#include "nat_math.h"
#include "nat_camera.h"

int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 240;
natCamera s_Camera;

int s_Dragging;
Vec4 eye;

// for debugging
bool s_use_glod = true;

int s_BudgetMode = 0;
int s_ErrorMode = 0;
int s_Triangles = 1000;
float s_Threshold = 1;
int s_PolygonFill = 1;
unsigned int s_LockAdapt = 0;

static const int TEX_WIDTH = 64;
static const int TEX_HEIGHT = 64;

GLuint g_tex_id = 0;
unsigned char g_test_texture[TEX_HEIGHT][TEX_WIDTH][4];

// number of cells in mesh in each direction (x, y)
int s_num_cells = 20;

// grid cells are 1x1 units; this is the scale we apply to the
// zero-mean, -0.5 -> +0.5 random noise function we add to the mesh.
float s_bump_height = 2;

void init_test_texture();

typedef float  Vec3[3];

struct VertexRow {
   float coord[3];
   float tex_coord[2];
};

class Mesh {
public:
   int   width;   // num cells in X
   int   height;  // num cells in Y
   int   num_verts;
   
   int   num_patches;
   
   Vec3     max_vec;
   
   unsigned int texture_id;

   // vertex indices in each patch to be sent to vertex array.
   // one index per triangle vertex, 2 tris per cell => 6 * num_cells
   unsigned int *indices;
   int num_indices; 

   // no normals for now; just specify once as (0,0,1)
   VertexRow *vert_info;

public:
   // mesh goes from 0,0,0 to width, height, 0 and forms 2 * width * height triangles
   Mesh(int _width, int _height) {
      
      // ** note: no GL calls allowed here, since we may not have a
      // ** context yet
      width = _width;
      height = _height;
      num_verts = (width + 1) * (height + 1);
      num_patches = 1;
      VEC3_SET(max_vec, width, height, s_bump_height);
      texture_id = 1;
      num_indices = 0;
      indices = NULL;
      vert_info = NULL;
   }
   
   ~Mesh() {
      delete vert_info;
      delete indices;
   }
   void init() {
      fill();
      printf("num_verts in mesh = %d\n", num_verts);
      if (num_verts < 40) {
         // print();
      }
   }

   // given a _point_ (not cell) i, j in the mesh, return its
   // corresponding vertex index.  i and j run to N+1 in order to
   // address each vertex.  i = x coord; j = y coord.
   int ij_to_vert_index(int i, int j) { return j * (width + 1) + i; }
   
   // fill in geometry for this mesh
   void fill() {
      int j;
      assert(indices == NULL);
      init_test_texture();
      
      // for height value
      srand(0);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      glGenTextures(1, &texture_id);
      glBindTexture(GL_TEXTURE_2D, texture_id);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_WIDTH, TEX_HEIGHT,
                   0, GL_RGBA, GL_UNSIGNED_BYTE, g_test_texture);

      // allocate and fill vert array
      vert_info = new VertexRow[num_verts];
      
      int row_size = width + 1; // one point at end to close last quad
      int num_rows = height + 1; // ditto

      // 6 indices per mesh cell (assumes we're using tris and not tstrips)
      num_indices = 6 * width * height;
      indices = new unsigned int[num_indices];
      
      // this loop defines all of the vertices (not the triangles) in
      // the mesh and their texcoords.  this means that it runs once
      // per point in the mesh-- not once per cell; (ie, (W+1) * (H+1)
      // times).
      int nr_h = num_rows / 2;
      int rs_h = row_size / 2;

      for (j = 0; j < num_rows; j++) {  // Y
         for (int i = 0; i < row_size; i++) {  // X
            // vertices
            int vert_index = ij_to_vert_index(i, j);
            assert(vert_index < num_verts);
            VertexRow *vert = &vert_info[vert_index];
            
            // do mesh in x-z plane so that it shows up in the viewport at startup
            vert->coord[0] = i - rs_h;
            vert->coord[2] = j - nr_h;
            
            // add some random bumpiness to the mesh to make it more
            // interesting for simplification
            vert->coord[1] = s_bump_height * (((float) rand() / (float) RAND_MAX) - 0.5);
            
            // tex coords
            vert->tex_coord[0] = (float) i / (float) width;
            vert->tex_coord[1] = (float) j / (float) height;
         }
      }
      
      // create indices for all the tris in the mesh.  this loop runs
      // N-1 times in each direction (ie, doesn't run on the right
      // edge or top) and makes two tris for each cell.  note that it
      // does reference the edge points via i+1, j+1.
      for (j = 0; j < height; j++) {  // Y
         for (int i = 0; i < width; i++) {  // X
            
            // k is a quad/cell index-- not the same as a point index
            // since the row size is different.
            int k = 6 * (j * width + i);
            
            // lower left, upper left, upper right
            // (assuming Y or j is up)
            indices[k + 0] = ij_to_vert_index(i, j);
            indices[k + 1] = ij_to_vert_index(i, j+1);
            indices[k + 2] = ij_to_vert_index(i+1, j+1);
            
            // low left, up right, low right
            indices[k + 3] = ij_to_vert_index(i, j);
            indices[k + 4] = ij_to_vert_index(i+1, j+1);
            indices[k + 5] = ij_to_vert_index(i+1, j);
         }
      }
      
   } // fill()

   void bindArrays()
   {
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, sizeof(VertexRow), &vert_info[0].coord[0]);
      
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, sizeof(VertexRow), &vert_info[0].tex_coord[0]);
   }

   void print()
   {
      printf("Mesh:\n");
      for (int j = 0; j < height; j++) {  // Y
         for (int i = 0; i < width; i++) {  // X
            printf("  cell (%d, %d):\n", i, j);
            
            int base = 6 * (j * width + i);
            for (int k = 0; k < 6; k++) {
               int index = indices[base + k];
               VertexRow *vert = &vert_info[index];
               printf("   %d: (%.1f, %.1f, %.1f; %.2f, %.2f)\n",
                      index,
                      vert->coord[0], vert->coord[1], vert->coord[2],
                      vert->tex_coord[0], vert->tex_coord[1]);
            }
            printf("\n");
         }
      }
   }
};

Mesh *s_Mesh = NULL;

#ifndef uchar
#define uchar unsigned char
#endif
void init_test_texture()
{
   static const int check_size = 16;
   
   for (int v = 0; v < TEX_HEIGHT; v++) { // row
      for (int u = 0; u < TEX_WIDTH; u++) {  // column
         unsigned char c = (((u & check_size) == 0) ^ 
                           ((v & check_size) == 0)) ? 0 : 1;
         float u_pct = (float) u / TEX_WIDTH;
         float v_pct = (float) v / TEX_HEIGHT;
         g_test_texture[v][u][0] = (uchar)(c * 255);
         g_test_texture[v][u][1] = (uchar)(c * u_pct * 255);
         g_test_texture[v][u][2] = (uchar)(c * v_pct * 255);
         g_test_texture[v][u][3] = (uchar)(255);
      }
   }
}


void SetCamera();

void IdleProc(void) {
   glutPostRedisplay();
}

void KeyProc(unsigned char key, int x, int y) {

   if (key == 'q' || key == 'Q') {
      exit(0);
   } else if (key == 'z') {
      s_Camera.SetMode(CAMERA_ROTATE);
   } else if (key == 'x') {
      s_Camera.SetMode(CAMERA_ZOOM);
   } else if (key == 'c') {
      s_Camera.SetMode(CAMERA_PAN);
   } else if (key == 'v') {
      s_Camera.SetMode(CAMERA_FREE_ROTATE);
   } else if (key == 'l') {
      s_LockAdapt = ! s_LockAdapt;
   } else if (key == ' ') {
      s_BudgetMode = !s_BudgetMode;
      if (!s_BudgetMode) {
         if (s_use_glod) glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_ERROR_THRESHOLD);
         printf("Error threshold mode.\n");
      } else {
         if (s_use_glod) glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_TRIANGLE_BUDGET);
         printf("Triangle budget mode.\n");
      }
   } else if (key == 'e') {
      s_ErrorMode = !s_ErrorMode;
      if (!s_ErrorMode) {
         if (s_use_glod) glodGroupParameteri(0, GLOD_ERROR_MODE, GLOD_OBJECT_SPACE_ERROR);
         printf("Object-space error mode.\n");
      }
      else
      {
         if (s_use_glod) glodGroupParameteri(0, GLOD_ERROR_MODE, GLOD_SCREEN_SPACE_ERROR);
         printf("Screen-space error mode.\n");
      }
   } else if (key == 'w') {
      s_PolygonFill = !s_PolygonFill;
      if (s_PolygonFill)
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      else
         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   } else if (key == '=') {
      s_Triangles /= 2;
      s_Threshold *= 1.5;
   } else if (key == '-') {
      s_Triangles *= 2;
      s_Threshold /= 1.5;
   } else {

#define KP(key,usage) printf(" [%8s] %s\n", (key), (usage));
      
      printf("Keyboard Usage\n");
      KP("q", "Quit");
      KP("l", "Free/unlock adapting of your geometry.");
      KP("space", "Switch from error mode to triangle budget");
      KP("e", "Toggle between object space and world space error modes.");
      KP("w", "Display wireframe or filled.");
      KP("+/-", "Change refinement goal value.");

      printf("Camera Control\n");
      KP("z", "Rotate");
      KP("x", "Zoom");
      KP("c", "Pan");
      KP("v", "Free rotate");
      printf("\n");
   }

   (void)x; (void)y;
}

void PushOrtho(int w, int h) {
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0,w,0,h);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
}

void DrawString(char* str, int x, int y) {
   glRasterPos2i(x, y);
   int l = strlen(str);
   int i = 0;
   while(i < l)
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i++]);
}

void PopOrtho() {
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

void ReshapeProc(int w, int h) {
   glViewport(0, 0, w, h);
   SCREEN_WIDTH = w;
   SCREEN_HEIGHT = h;
   s_Camera.AutoPlaceBall();
   SetCamera();

   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0,
             GL_POSITION,
             eye);
}

void DisplayProc(void) {
   
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
   if (s_BudgetMode != 0) {
      if (s_use_glod) glodGroupParameteri(0, GLOD_MAX_TRIANGLES, s_Triangles);
   } else {
      if (s_ErrorMode == 0) {
         if (s_use_glod) glodGroupParameterf(0, GLOD_OBJECT_SPACE_ERROR_THRESHOLD, s_Threshold);
      } else {
         if (s_use_glod) glodGroupParameterf(0, GLOD_SCREEN_SPACE_ERROR_THRESHOLD, s_Threshold);
      }
   }

   if (s_use_glod) glodBindObjectXform(0, GL_PROJECTION_MATRIX | GL_MODELVIEW_MATRIX);
   
   if (! s_LockAdapt) {
      if (s_use_glod) glodAdaptGroup(0);
   }
   
   int npatches = 1;  // XXXX  hard-wired default case for running w/o GLOD
   if (s_use_glod) glodGetObjectParameteriv(0, GLOD_NUM_PATCHES, &npatches);
   glColor3f(.8,.8,.8);
   glNormal3f(0.0, 1.0, 0.0);
   glEnable(GL_TEXTURE_2D);
   

   glBindTexture(GL_TEXTURE_2D, s_Mesh->texture_id);

   for (int i = 0; i < npatches; i++) {
      if (s_use_glod) {
         glodDrawPatch(0,i);
      }
      else {
         glDrawElements(GL_TRIANGLES, s_Mesh->num_indices, 
                        GL_UNSIGNED_INT, s_Mesh->indices);
      }
   }
  
   PushOrtho(SCREEN_WIDTH, SCREEN_HEIGHT);
   glDisable(GL_LIGHTING);
   glColor3f(1.,1.,1.);
   DrawString("+/- adjust detail, [space] toggle tri/error budget", 2, 3);

   {
      char buf[512];
      sprintf(buf, "%s in %s space: t=%f",
              (s_BudgetMode == 0 ? "error threshold" : "tri budget"),
              (s_ErrorMode == 0 ? "object" : "screen"),
              (s_BudgetMode == 0 ? s_Threshold : s_Triangles));
      DrawString(buf, 2, SCREEN_HEIGHT - 15);
   }

   PopOrtho();
   //glEnable(GL_LIGHTING);

   glutSwapBuffers();
}

void Usage();
void KeyProc(unsigned char key, int x, int y);
void MotionProc(int x, int y);
void MouseProc(int button, int state, int x, int y);

void Usage() {
   printf("Usage:\n");
   printf("   mesh [args]\n");
   printf("     Views a textured mesh using GLOD.\n");
   printf("      -i    Inverts the calculated normals\n");
   printf("      -c    Uses continuous LOD instead of discrete");
   printf("      -n N   Make NxN mesh");
   printf("      -b X   Set bump scale to X");
   printf("\n\n");
   return;
}

void SetCamera() {
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45,((float)SCREEN_WIDTH)/((float)SCREEN_HEIGHT),1,1000);
   glMatrixMode(GL_MODELVIEW);

   glLoadIdentity();
   s_Camera.SetGLState();
}

void MouseProc(int button, int state, int x, int y) {
   float fx = (float)x;
   float fy = (float)SCREEN_HEIGHT- y;

   if (state == GLUT_DOWN) {
      s_Dragging = 1;
      s_Camera.BeginDrag(fx,fy);
   } else if (state == GLUT_UP) {
      s_Dragging = 0;
      s_Camera.EndDrag(fx,fy);
   }
  
   SetCamera();
   glutPostRedisplay();
}

void MotionProc(int x, int y) {
   float fx = (float)x; //(float)window_width - x;
   float fy = (float)SCREEN_HEIGHT - y;

   if (s_Dragging) {
      s_Camera.Drag(fx,fy);
      SetCamera();
      glutPostRedisplay();
   }
}


//////////////////////////////////////////////////////////////////////////////
//
// main 
// 
#ifdef _WIN32
#define strcasecmp(x,y) strcmpi(x,y)
#endif

int main(int argc, char ** argv) {
   char title[] = "GLOD- Mesh Sample App";

   glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
   glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  
   glutCreateWindow((char*)title);
   glutDisplayFunc(DisplayProc);
   glutReshapeFunc(ReshapeProc);
   glutIdleFunc(IdleProc);
   glutKeyboardFunc(KeyProc);
   glutMotionFunc(MotionProc);
   glutMouseFunc(MouseProc);
  
  
   glColor3f(1.0, 1.0, 1.0);
  
   glViewport(0, 0,
              SCREEN_WIDTH, SCREEN_HEIGHT);
   glClearColor(0.2, 0.2, 0.2, 0.0);
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glClearDepth(1.0);

   glLoadIdentity();


   /***************************************************************************
    * Init
    ***************************************************************************/
   int inv = 0;
   int mode = GLOD_DISCRETE;
   bool timing_test = false;
   {
      // get config options
      int i; 
      for (i = 1; i < argc; i++) {
         if (strcasecmp(argv[i], "--help") == 0) {
            Usage();
            return 0;
         }

         if (strcasecmp(argv[i], "-b") == 0) {
            s_bump_height = atof(argv[++i]);
            printf("Set bump scale to %.1f.\n", s_bump_height);
            continue;
         }
      
         if (strcasecmp(argv[i], "-c") == 0) {
            printf("Using continuous simplification.\n");
            mode = GLOD_CONTINUOUS;
            continue;
         }

         if (strcasecmp(argv[i], "-d") == 0) {
            printf("Using discrete simplification.\n");
            mode = GLOD_DISCRETE;
            continue;
         }

         if (strcasecmp(argv[i], "-i") == 0) {
            printf("Inverting normals.\n");
            inv = 1;
            continue;
         }

         if (strcasecmp(argv[i], "-n") == 0) {
            s_num_cells = atoi(argv[++i]);
            printf("Set grid size to %d x %d.\n", s_num_cells, s_num_cells);
            continue;
         }

         if (strcasecmp(argv[i], "-t") == 0) {
            timing_test = true;
            printf("Timing test mode; will exit after simplifying.\n");
            continue;
         }
      }
   }
   
   clock_t start, finish;

   // init mesh now that we have the size
   s_Mesh = new Mesh(s_num_cells, s_num_cells);

   // setup
   if (s_use_glod && glodInit() == 0) {
      printf("Could not load GLOD!\n");
      return 0;
   }
   Vec3 max;
   glEnableClientState(GL_VERTEX_ARRAY);
   {
      // generate mesh object
      printf("generating %d x %d mesh...\n", s_Mesh->width, s_Mesh->height);
    
      start = clock();
   
      // Simplify using GLOD
      if (s_use_glod) glodNewGroup(0);
      if (s_use_glod) glodNewObject(0, 0, mode);

      s_Mesh->init(); 
      s_Mesh->bindArrays();
      
      printf("num_patches = %i\n", s_Mesh->num_patches);
      
      for (int patch_num = 0; patch_num < s_Mesh->num_patches; patch_num++) {
         
         // set up GL client state and array pointers
         if (s_use_glod) {
            glodInsertElements(0, patch_num,
                               GL_TRIANGLES,
                               s_Mesh->num_indices, GL_UNSIGNED_INT,
                               s_Mesh->indices,
                               0, 0.0);
         }
      }
      if (s_use_glod) glodBuildObject(0);

      VEC3_COPY(max, s_Mesh->max_vec); // get ready to position the camera
   }

   {
      VEC4_SET(eye, max[0], max[1], max[2], 0.0);
      VEC3_SCALE(eye,2);
      s_Camera.SetSpeed(eye[0] / 10);
      s_Camera.LookAt(eye[0],eye[1],eye[2],
                      0,0,0,
                      0,1,0);
    
      SetCamera();


      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0,
                GL_POSITION,
                eye);

      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

   }

   finish = clock();

   double delta_t = (double)(finish - start) / CLOCKS_PER_SEC;

   printf("elapsed time = %.1lf seconds\n", delta_t);
   
   if (! timing_test) {
      glutMainLoop();
   }

   return 0;
  
}

