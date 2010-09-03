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
/***************************************************************************
 * GLOD-Viewer
 * $Id: GLOD_App.cpp,v 1.33 2004/07/08 16:15:57 gfx_friends Exp $
 ***************************************************************************/

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <ctype.h>
#include <ply.h>

#include <PlyModel.h>

#include "Util.h"


#include "GLOD_App.h"

#include "glod.h"

#include "nat_math.h"
#include "nat_camera.h"


#ifdef _WIN32
#define for if(0); else for
#endif

int s_USE_GLOD = 0;
#define GOAL_ERROR 0
#define GOAL_TRI 1
#define GOAL_MAX 2
#define ERROR_MODE_OBJECT 0
#define ERROR_MODE_SCREEN 1
#define ERROR_MODE_MAX 2


int error_mode = ERROR_MODE_OBJECT;
int goal_mode = GOAL_ERROR;
int glod_lock=0;

float tcount = 1000;
float tincr = 100;
float threshold = 3.0/100.0;
float threshold_incr = 2;//1.0/200;
float s_PolygonFill = 1;

void PrintGoalMode() {
  switch(error_mode) {
  case ERROR_MODE_OBJECT:
    printf("Object space ");
    break;
  case ERROR_MODE_SCREEN:
    printf("Screen space ");
    break;
  }

  switch(goal_mode) {
  case GOAL_ERROR:
    printf("error budget goal.\n");
    break;
  case GOAL_TRI:
    printf("triangle budget goal.\n");
    break;
  }
}

// camera
natCamera s_Camera;
int s_Dragging = 0;
int window_width, window_height;
void SetCamera();


// models in the world
int   nModels;
Model   *models;
Model*  curModel;

// objects in the scene
int nObjects;
Object*  objects;


// GUI Options/Parameters
char  *filename;
char  *texture_filename;
int   textureID = -1;


static const FloatColor FRONT_COLOR = {0.0, 1.0, 0.0};
static const FloatColor BACK_COLOR = {1.0, 0.0, 0.0};

/*****************************************************************************
*****************************************************************************/
void IdleProc()
{
	glutPostRedisplay();
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

void DisplayProc()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  {
    glDisable(GL_LIGHTING); glDisable(GL_COLOR_MATERIAL);
    glLineWidth(3.0);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.0);
    glColor3f(0.0, 0.0, 0.0);
    glEnd();
    glEnable(GL_LIGHTING); glEnable(GL_COLOR_MATERIAL);
  }

  //glvu.UpdateFrame();
  if(s_USE_GLOD) {
    // apply transforms
    for(int i = 0; i < nObjects; i++) {
      Object* obj = &objects[i];

      glPushMatrix();
      glTranslatef(obj->pos[0],obj->pos[1],obj->pos[2]);
      glRotatef(obj->rot[0], obj->rot[1], obj->rot[2], obj->rot[3]);
      //DebugCamera(&s_Camera);
      glodBindObjectXform(obj->glod_name, GL_MODELVIEW_MATRIX | GL_PROJECTION_MATRIX);
      glPopMatrix();
    }
    
    // error mode
    if(error_mode == ERROR_MODE_OBJECT) {
      glodGroupParameteri(0, GLOD_ERROR_MODE, GLOD_OBJECT_SPACE_ERROR);
    } else {
      glodGroupParameteri(0, GLOD_ERROR_MODE, GLOD_SCREEN_SPACE_ERROR);
    }
    
    // goal mode
    if(goal_mode == GOAL_ERROR) {
	glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_ERROR_THRESHOLD);
	if (error_mode == ERROR_MODE_OBJECT)
	    glodGroupParameterf(0, GLOD_OBJECT_SPACE_ERROR_THRESHOLD, threshold);
	else
	    glodGroupParameterf(0, GLOD_SCREEN_SPACE_ERROR_THRESHOLD, threshold);
    } else {
      glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_TRIANGLE_BUDGET);
      glodGroupParameteri(0, GLOD_MAX_TRIANGLES, (int)tcount);
    }
    if (!glod_lock)
	glodAdaptGroup(0);
  }
  
  // now draw
  glEnable(GL_TEXTURE_2D);
  for(int i = 0 ; i < nObjects; i++) {
    Object* obj = &objects[i];
    Model* model = obj->model;

    glShadeModel(GL_SMOOTH);

    glColor3ubv((GLubyte*)&model->plymodel.solid_color);

    glDisable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, FRONT_COLOR);
    glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, BACK_COLOR);

    glPushMatrix();
    glTranslatef(obj->pos[0],obj->pos[1],obj->pos[2]);
    glRotatef(obj->rot[0], obj->rot[1], obj->rot[2], obj->rot[3]);
    glBindTexture(GL_TEXTURE_2D, model->plymodel.texture_id);
    
    if(s_USE_GLOD) {
    for (int i=0; i<obj->model->plymodel.npatches; i++)
      glodDrawPatch(obj->glod_name,i);
    } else {
      BindVertexArray(&model->plymodel, 0);
      DrawModelVA(&model->plymodel, 0);
    }

    glPopMatrix();
  }

  glDisable(GL_TEXTURE_2D);

  PushOrtho(window_width, window_height);
  glDisable(GL_LIGHTING);
  glColor3f(1.,1.,1.);
  DrawString("+/- adjust detail, g= change goal, e= change error mode", 2, 3);
  
  char buf[512];
  sprintf(buf, "%s in %s space: t=%f", 
	  (goal_mode==GOAL_ERROR ? "error threshold" : "tri budget"),
	  (error_mode == ERROR_MODE_OBJECT ? "object" : "screen"),
	  (goal_mode==GOAL_ERROR ? threshold : tcount));
  DrawString(buf, 2, window_height - 15);

  if(!s_USE_GLOD) {
    glColor3f(1.0,0.0,0.0);
    DrawString("GLOD DISABLED!", (4), (window_height - 15) / 2);
  }

  PopOrtho();
  glEnable(GL_LIGHTING);

  

  glutSwapBuffers();
  //glutPostRedisplay();
}

void PrintKeyboardHelp()
{
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Plyview keyboard help:\n");
    fprintf(stderr, "--------------------------\n");
    fprintf(stderr, "  z : Trackball motion\n");
    fprintf(stderr, "  x : Drive motion\n");
    fprintf(stderr, "  c : Translate motion\n");
    fprintf(stderr, "\n\n");
    return;
}


/*****************************************************************************
*****************************************************************************/
void KeyProc(unsigned char key, int x, int y)
{
  switch (key) {
  case '=':
  case '+':
    if(goal_mode == GOAL_ERROR) {
      threshold *= threshold_incr;
      //threshold_incr *= 2.0;
    } else {
      tcount *= 2;
      //tincr *= 2;
    }
    break;
  case '-':
  case '_':
    if(goal_mode == GOAL_ERROR) {
      //      threshold_incr /= 2.0;
      threshold /= threshold_incr;
    } else {
      //      tincr /= 2.0;
      tcount /= 2;
    }
    break;
  case 'e':
    error_mode++;
    if(error_mode == ERROR_MODE_MAX) error_mode = 0;
    PrintGoalMode();
    break;
  case 'w':
      s_PolygonFill = !s_PolygonFill;
      if (s_PolygonFill)
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      else
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
      
  case 'g':
    goal_mode++;
    if(goal_mode == GOAL_MAX) goal_mode = 0;
    PrintGoalMode();
    break;
  case '2':
    glClearColor(0, 0, 0, 1);
    break;
  case '3':
    glClearColor(1, 1, 1, 1);
    break;
  case 'q':
    exit(0);
    break;
  case 'z':
    s_Camera.SetMode(CAMERA_ROTATE);
    break;
  case 'x':
    s_Camera.SetMode(CAMERA_ZOOM);
    break;
  case 'c':
    s_Camera.SetMode(CAMERA_PAN);
    break;
  case 'v':
    s_Camera.SetMode(CAMERA_FREE_ROTATE);
    break;
  case 'l':
    glod_lock=!glod_lock;
    break;
  case 'h':
  case 'H':
  case '?':
  case '/':
    printf("Keyboard Usage\n----------------------------\n");
#define KP(key,usage) printf(" [%8s] %s\n", (key), (usage))
    KP("q", "Quit");
    KP("+/-", "Change refinement goal value");
    KP("w", "toggle wireframe display");
    KP("e", "change error mode to compute over");
    KP("g", "toggle goal mode");
    
    printf("Camera Control\n");
    KP("z", "Rotate");
    KP("x", "Zoom");
    KP("c", "Pan");
    KP("v", "Free rotate");

    printf("\n");
#undef KP
    break;
  };
  
  
  glutPostRedisplay();
}


void MouseProc(int button, int state, int x, int y) {
  float fx = (float)x;
  float fy = (float)window_height - y;

  if(state == GLUT_DOWN) {
    s_Dragging = 1;
    s_Camera.BeginDrag(fx,fy);
  } else if(state == GLUT_UP) {
    s_Dragging = 0;
    s_Camera.EndDrag(fx,fy);
  }
  
  SetCamera();
  glutPostRedisplay();
}

void MotionProc(int x, int y) {
  float fx = (float)x; //(float)window_width - x;
  float fy = (float)window_height - y;

  if(s_Dragging) {
    s_Camera.Drag(fx,fy);
    SetCamera();
    glutPostRedisplay();
  }
}

/*****************************************************************************
*****************************************************************************/

void ReshapeProc(int w, int h) {
  glViewport(0, 0, w, h);
  s_Camera.AutoPlaceBall();
  
  window_width = w; window_height = h;
  SetCamera();
}

void SetCamera() {
  // set camera
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  
  
  gluPerspective(45, (float)window_width / (float)window_height, 0.01, 1000.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // set light
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  float tmp[] = {0,0,-1,1};
  glLightfv(GL_LIGHT0,
	    GL_POSITION,
	    tmp);
  
  s_Camera.SetGLState();
}



/*****************************************************************************
*****************************************************************************/
inline void updatebox(GLfloat *gmin, GLfloat *gmax, Point data)
{
   if(gmin[0] > data[0]) gmin[0] = data[0];
   if(gmin[1] > data[1]) gmin[1] = data[1];
   if(gmin[2] > data[2]) gmin[2] = data[2];
   if(gmax[0] < data[0]) gmax[0] = data[0];
   if(gmax[1] < data[1]) gmax[1] = data[1];
   if(gmax[2] < data[2]) gmax[2] = data[2];
}


/*****************************************************************************
*****************************************************************************/
#ifndef INFINITY
#define INFINITY FLT_MAX
#endif
void findModelExtent(GLfloat *gmin, GLfloat *gmax)
{
  int i,j; Object* obj; Model* model;
  Point coord;

  gmin[0] = gmin[1] = gmin[2] = +INFINITY;
  gmax[0] = gmax[1] = gmax[2] = -INFINITY;

  for(i = 0; i < nObjects; i++) {
    obj = &objects[i];
    model = obj->model;
    
    for(j = 0; j < 3; j++)
      coord[j] = obj->pos[j] + model->plymodel.min[j]; 
    updatebox(gmin, gmax, coord);

    for(j = 0; j < 3; j++)
      coord[j] = obj->pos[j] - model->plymodel.min[j]; 
    updatebox(gmin, gmax, coord);

    for(j = 0; j < 3; j++)
      coord[j] = obj->pos[j] + model->plymodel.max[j];
    updatebox(gmin, gmax, coord);

    for(j = 0; j < 3; j++)
      coord[j] = obj->pos[j] - model->plymodel.max[j];
    updatebox(gmin, gmax, coord);
  }
}


/*****************************************************************************
*****************************************************************************/
void Usage()
{
   printf("Usage: filename [parameters], where parameters are:\n");
   printf("\t-help               : display this help message\n");
}


/*****************************************************************************
 *****************************************************************************/
void
ParseArgs(int *argc, char *argv[])
{
  int i;
  filename = NULL;
  for(i = 1; i < *argc; i++) {
    if(strcmp(argv[i], "-glod") == 0) {
      printf("********** ENABLING GLOD *************\n");
      s_USE_GLOD = 1;
    } else if(filename == NULL) {
      filename = argv[i];
    }
  }
    
}


/*****************************************************************************
*****************************************************************************/
void RecomputeBounds(PlyModel* model, float maxEdge) {
  float max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
  float min[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
  float range[3]; float newHalfRange[3];
  float emax; float ratio;

  int i,j;

  for(i = 0; i < model->nverts; i++) {
    Vertex* v = &model->vlist[i];
    for(j = 0; j < 3; j++) {
      if(v->coord[j] > max[j])
	max[j]  = v->coord[j];

      if(v->coord[j] < min[j])
	min[j] = v->coord[j];
    }
  }
  
  memcpy(model->max, max, sizeof(float) * 3);
  memcpy(model->min, min, sizeof(float) * 3);  
  
  if(maxEdge == 0) {
    return;
  }

  // offset it to zero 
  for(i = 0; i < model->nverts; i++) {
    Vertex* v = &model->vlist[i];
    for(j = 0; j < 3; j++)
      v->coord[j] -= min[j];
  }
  
  emax = -FLT_MAX;
  for(i = 0; i < 3; i++) {
    range[i] = max[i] - min[i]; 
    if(range[i] > emax) emax = range[i];
  }
  ratio = maxEdge / emax;
  for(i = 0; i < 3; i++)
    newHalfRange[i] = (range[i] * ratio) / 2;
  
  // now the things range from 0 to range[0..3]
  // scale every vertex by rx and then subtract range which is half
  for(i = 0; i < model->nverts; i++) {
    Vertex* v = &model->vlist[i];
    for(j= 0; j < 3; j++)
      v->coord[j] = (v->coord[j] * ratio) - newHalfRange[j];
  }

  // recompute max & min... they are -newHalfRange to newHalfRange
  memcpy(model->max, newHalfRange, sizeof(float) * 3);
  memcpy(model->min, newHalfRange, sizeof(float) * 3);  
  for(i = 0; i < 3; i++)
    model->min[0] *= -1.0;
  
}

void ComputeNormals(PlyModel* model) {
  if(! model->has_vertex_normals) {
     printf("Computing vertex normals...\n");
     for(int i = 0; i < model->nfaces; i++) {
       Face* face = &model->flist[i];
       Vec3 a,b,c;
       VEC3_COPY(a, (const void*)&model->vlist[face->verts[0]].coord);
       VEC3_COPY(b, (const void*)&model->vlist[face->verts[1]].coord);
       VEC3_COPY(c, (const void*)&model->vlist[face->verts[2]].coord);

       Vec3 fn; Vec3 ta,tb;
       VEC3_OP(ta,c,-,a);
       VEC3_OP(tb,b,-,a);
       VEC3_CROSS(fn, ta,tb); // fn = (c - a) X (b - a);

       for(int j = 0; j < 3; j++)
	 for(int k = 0 ; k < 3; k++)
	   model->vlist[face->verts[j]].normal[k] += fn[k];
     }

     for(int i = 0; i < model->nfaces; i++) {
       Face* face = &model->flist[i];
       Vertex* v;
       float len;
       for(int j = 0; j < 3; j++){
	 v = &model->vlist[face->verts[j]];
	 len = sqrt(pow(v->normal[0], 2.0f) + 
		    pow(v->normal[1], 2.0f) +
		    pow(v->normal[2], 2.0f));
	 for(int k = 0; k < 3; k++)
	   v->normal[k] /= len;
       }
     }

     model->has_vertex_normals = 1;     
   }
}

void LoadScene() {
  FILE* fp;
  char ply_filename[255];
  char tex_filename[255];
  int modelI = 0; int objI = 0;
  
  int i; char c; int temp; char refine_mode[3];
  int nov; char bogus[10];
  
  float edge_size;

  printf("Loading scene file \"%s\"\n",filename);
  
  fp = fopen(filename, "rb");
  if(fp == NULL) {
    printf("Error: could not open %s\n", filename);
    exit(0);
  }
  nModels = -1;

  while(!feof(fp)) {
    fscanf(fp, "%c", &c);
    c = toupper(c);
    if(c == '\n' || c=='\r' || c == ' ') continue;
    
    if(c == 'H') {
      fscanf(fp, "%i %i", &nModels, &nObjects);
      models = (Model*) malloc(nModels * sizeof(Model));
      objects = (Object*) malloc(nObjects * sizeof(Object));
      memset(models, 0, sizeof(Model) * nModels);      
      memset(objects, 0, sizeof(Object) * nObjects);
      
      continue;
    }
    if(nModels <= 0) {
      printf("Error! No header!\n");
      exit(0);
    }
    
    if(! (c == 'N' || c=='I')) {
      printf("Unexpected token '%c'. Aborting.\n", c);
      exit(0);
    }
    
    if(c == 'N') { // [name] [filename] [texture] [edge size]
      Object* obj = &objects[objI++];
      Model* mod = &models[modelI++];
      // instance model
      if(fscanf(fp, "%s %i %i %s %s %f",
		refine_mode, &obj->glod_name, &obj->glod_group,
		ply_filename, tex_filename,
		&edge_size) != 6) {
	printf("Bad record format: N [refine mode C D] [name] [group] [filename] [texture = -] [edge size = 0] <xforms>\n");
	exit(0);
      }
      
      // set up object
      obj->model = mod;

	  // win32 kludge
#ifdef _WIN32
	  int l = strlen(ply_filename);
	  for(int i = 0; i < l; i++) {
		if(ply_filename[i] == '/') ply_filename[i] = '\\';
	  }
#endif

      // do the object & gl setup
      read_plyfile(ply_filename, &mod->plymodel);
      printf("\n\n**************************************************\n");
      printf("Loaded %s: %i triangles\n", ply_filename, mod->plymodel.nverts);
      
      RecomputeBounds(&mod->plymodel, edge_size);
      
      if(! mod->plymodel.has_vertex_normals)
	ComputeNormals(&mod->plymodel);
      
      
      if(strcmp(tex_filename, "-")) {
	SetupTexture(&mod->plymodel, tex_filename);
      } else {
	SetupTexture(&mod->plymodel, NULL);
      }

      // now get the xform
      nov = fscanf(fp, "%s %f %f %f %s %f %f %f %f",
		   bogus,
		   &obj->pos[0], &obj->pos[1], &obj->pos[2],
		   bogus,
		   &obj->rot[0], &obj->rot[1], &obj->rot[2], &obj->rot[3]);
      obj->pos[1] = -obj->pos[1];// invert the y, its wierd
      if(nov != 9) {
	printf("File format error! Not 7 transforms!\n");
	printf("Xform format: / x y z / rotDeg rotX rotY rotZ \n");
	exit(0);
      }
      
      SetupVertexArray(&mod->plymodel, VERTEX_ARRAY_ARRAYS);

      // glod call
      if(s_USE_GLOD) {
	// xxx: Multipatch support!
	//BindVertexArray(&mod->plymodel, 0);

	int mode;
	if(strcmp(refine_mode, "C") == 0) {
	  printf("Using ***continuous [%s]*** refinement\n", refine_mode);
	  mode = GLOD_CONTINUOUS;
	} else {

	  printf("Using ***discrete [%s]*** refinement\n", refine_mode);
	  mode = GLOD_DISCRETE;
	}
	glodNewObject(obj->glod_name, obj->glod_group, mode);
	
	for (int pnum=0; pnum<obj->model->plymodel.npatches; pnum++){
	    BindVertexArray(&mod->plymodel, pnum);
	    glodInsertArrays(obj->glod_name, pnum, GL_TRIANGLES, 0, obj->model->plymodel.plist[pnum].nindices, 0, 0);
	}

	glodBuildObject(obj->glod_name);
      }
      
      // later
    } else if(c == 'I') {
      Object* obj = &objects[objI++];
      Model* mod = NULL;
      if(fscanf(fp, "%i %i %i",
		&temp,
		&obj->glod_name,
		&obj->glod_group) != 3) {
	printf("Record format: I [existing name] [instance name] [group] <xforms>\n");
	exit(0);
      }
      
      for(i = 0; i < objI - 1; i++) {
	if(objects[i].glod_name == temp)
	  mod = objects[i].model;
      }
      
      if(mod == NULL) {
	printf("Could not find object named %i\n", temp);
	exit(0);
      } else {
	printf("Binding object %i to object %i\n", obj->glod_name, temp);
      }
      
      
      obj->model = mod;
      
      // now get the xform
      nov = fscanf(fp, "%s %f %f %f %s %f %f %f %f",
		   bogus,
		   &obj->pos[0], &obj->pos[1], &obj->pos[2],
		   bogus,
		   &obj->rot[0], &obj->rot[1], &obj->rot[2], &obj->rot[3]);
      obj->pos[1] = -obj->pos[1];// invert the y, its wierd

      if(nov != 9) {
	printf("File format error! Not 7 transforms!\n");
	printf("Xform format: / x y z / rotDeg rotX rotY rotZ \n");
	exit(0);
      }

      // instance this object
      if(s_USE_GLOD) {
	glodInstanceObject(temp, obj->glod_name, obj->glod_group);
      }
      
      // later
    }
    

  }
  
  printf("done.\n");

  curModel = &models[0];

  fclose(fp);

}

/*****************************************************************************
*****************************************************************************/
int main(int argc, char **argv)
{
   GLfloat gmin[3], gmax[3];

   // parse command line
   ParseArgs(&argc, argv);
   if(filename == NULL) {
     printf("Error! No filename specified!\n");
     printf("Usage: scene [filename] [-glod]\n\n");
     return 0;
   }


   // Init GUI, OpenGL, etc.
   window_width = 320; window_height = 240;
   char title[] = "GLOD Scene Test";
   glutInit(&argc, (char**)argv);
   glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(window_width, window_height);
   
   glutCreateWindow((char*)title);
   glutDisplayFunc(DisplayProc);
   glutReshapeFunc(ReshapeProc);
   glutIdleFunc(IdleProc);
   glutKeyboardFunc(KeyProc);
   glutMotionFunc(MotionProc);
   glutMouseFunc(MouseProc);
   
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glClearDepth(1.0);

/* glClearColor(0, 0, 0, 1);
   glShadeModel(GL_FLAT);
   glDisable(GL_CULL_FACE);
   glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
   glEnable(GL_COLOR_MATERIAL);
   glLineWidth(2.0);
   
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 100);
   GLfloat lightdiffuse[] = {1.0, 1.0, 1.0, 1.0};
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdiffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, lightdiffuse);   
   glMaterialf(GL_FRONT, GL_SHININESS, 2);
   GLfloat matspecular[] = {1.0,1.0,1.0,1.0};
   glMaterialfv(GL_FRONT, GL_SPECULAR, matspecular);*/


   // load model -- it needs GL to load
   if(s_USE_GLOD){
     glodInit();
   }
   
   LoadScene();

   // compute model bbox and diagonal
   findModelExtent(gmin, gmax);
   //SetupNormalMap(&model);

   Vec3 center; Vec3 tmp;

   VEC3_OP(tmp,gmax,-,gmin);
   VEC3_SCALE(tmp,.5);
   VEC3_OP(center,tmp,+,gmin);
   
   printf("Min: "); VEC3_PRINTLN(gmin);
   printf("Max: "); VEC3_PRINTLN(gmax);
   printf("Center: "); VEC3_PRINTLN(center);

   VEC3_SCALE(gmax,2.5);
   s_Camera.LookAt(gmax[0],gmax[1],gmax[2],
                   center[0],center[1],center[2],
                   0,1,0);
   s_Camera.SetMode(CAMERA_ROTATE);

   glutMainLoop();

   return 0;
}



/*****************************************************************************\
  $Log: GLOD_App.cpp,v $
  Revision 1.33  2004/07/08 16:15:57  gfx_friends
  many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)

  Revision 1.32  2004/07/06 16:25:19  jdt6a
  finally found the bug.  turns out it wasn't a GL problem.  we were uploading the wrong modelview matrix because of an incorrect bounding box for the model because of this typo in PlyModel.cpp.

  works in VS.NET now, with some minor vds changes that i'll commit now.

  Revision 1.31  2004/06/11 19:05:45  gfx_friends
  Got Win32-debug to work after moving the directory structure around.

  Revision 1.30  2004/06/11 06:45:17  gfx_friends
  Wrote makefiles for the samples directory and top level directory.

  Revision 1.29  2004/06/02 17:14:01  gfx_friends
  Changes to #includes so it works on a stock osx configuration

  Revision 1.28  2004/05/11 17:20:34  gfx_friends
  Fixed the problem when loading multipatch objects. patchsphere and torus models can now load and display properly

  Revision 1.27  2004/02/04 18:55:43  gfx_friends
  fixed makefiles to work proper using macs... also small change to the sample programs, which would prevent them from running in osx

  Revision 1.26  2004/02/04 03:47:28  gfx_friends
    - Move format flag from glodBuildObject to glodNewObject... I'm really sorry
      but we discovered in the paper-writing process that this was necessary
    - Refactored the discrete manual handling mode more cleanly. I am going to
      check in a cleaner refactoring soon. Its broken right now. (nat)

  Revision 1.25  2004/01/21 08:03:00  gfx_friends
  Added some debugging routines and made it so that simple & scene don't redraw except when they need to. --nat

  Revision 1.24  2003/10/21 20:43:35  bms6s
  triangle budget with continuous cuts works in object space mode; now working on screen space mode

  Revision 1.23  2003/08/27 20:29:11  gfx_friends
  Fixed a bug in screen-space-error computation and improved the functionality of GLOD_App.

  Revision 1.22  2003/08/14 20:38:49  gfx_friends
  Added the new glodObjectXform parameters and fixed a few related things. However, outstanding issues that exist now are (a) we still compute our errors in pixels, whereas we've decided to switch to angle-of-error, and (b) We can't make VDS work until we either map it to 1 cut/object or change VDS to support transformations per object regardless of cut.

  Revision 1.21  2003/07/26 01:17:37  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.20  2003/07/23 19:55:31  gfx_friends
  Added copyright notices to GLOD. I'm making a release.

  Revision 1.19  2003/07/22 18:32:16  gfx_friends
  Fixed the windows build. We've got a big bad windows bug that keeps the simplifier from working right now, but hopefully that'll be patched soon. -nat

  Revision 1.18  2003/07/22 03:50:37  gfx_friends
  All the people with soul are in this commit.

  Just kidding. BB King has nothing to do with this commit. Instead, I just cleaned up a bit of the scene tool.

  Revision 1.17  2003/07/22 03:28:31  gfx_friends
  Fixed the Scene tool. Mostly. I need to do some more stuff, but its back to comipling. glodAdapt jams! --nat

  Revision 1.16  2003/07/09 22:50:05  gfx_friends
  Major documentation effort and minor API changes. On the API change side,
  GLODBuildObject now recieves the format flag for an object being built, while LoadObject now requires NewObject to
  have been called before it can be called. NewObject simply creates an object and group.

  On the documentation side, the sources in ./api now contain a ton of inline comments which document
  the API routines using Doxygen tagging syntax. A top-level makefile target, docs, allows you to build HTML documentation out of these files. When I've finished the documentation, we can also make the same comments go to UNIX Man pages and Windows RTF/HTML help files. I'm still documenting the API. However, if you run make docs on a linux box or some PC with Doxygen installed on it, you'll get the docs and can check them out.

  Cheers,

  -- Nat

  Revision 1.15  2003/07/01 21:49:11  gfx_friends
  Tore out the glodDrawObject in favor of the more complete glodDrawPatch. A bug somewhere between glodBuildObject's call to new Model and the draw, however, is still failing, as all of the patches have identical geometry by the time they hit the drawing point.

  Revision 1.14  2003/06/16 21:14:39  gfx_friends
  Scene now works in Win32 as well, but is still crashing at various places in the simplifier. -n

  Revision 1.13  2003/06/16 20:51:10  gfx_friends
  Win32 patches to Scene

  Revision 1.12  2003/06/16 19:59:59  gfx_friends
  Removed GLVU from scene test program. -- n

  Revision 1.11  2003/06/09 21:01:02  gfx_friends
  Moved all 3D models to samples/data and got (what was formerly known as GLOD-Test now called Scene) to compile again. It does not build on Win32 yet.

  Revision 1.10  2003/01/21 07:55:48  gfx_friends
  App.

  Revision 1.9  2003/01/21 07:23:45  gfx_friends
  Stuff.

  Revision 1.8  2003/01/20 19:29:48  gfx_friends
  Support for GLOD_CONTINUOUS in scene graphs. Old scene graph files are obsolute.

  Revision 1.7  2003/01/20 07:41:31  gfx_friends
  Modified Makefile to link for me and added fixes for screen-space mode

  Revision 1.6  2003/01/20 06:05:34  gfx_friends
  Fixed up the rendering modes.

  Revision 1.5  2003/01/20 05:24:16  gfx_friends
  Fixed matrix stack bug.

  Revision 1.4  2003/01/20 04:13:27  gfx_friends
  New version of code.

  Revision 1.3  2003/01/19 23:26:05  gfx_friends
  Cleaned up makefile and made link.

  Revision 1.2  2003/01/19 22:30:22  gfx_friends
  Patches.

\*****************************************************************************/









