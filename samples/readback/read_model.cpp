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
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
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

#include <math.h>
#if !defined(_WIN32) && !defined(__APPLE__)
# include <values.h>
#else
# include <float.h>
#endif
#include <ctype.h>

#ifndef _WIN32 // for gettimeofday
#include <sys/time.h>
#endif

#include "glod.h"
#include "nat_math.h"
#include "nat_camera.h"
#include "PlyModel.h"

int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 240;
natCamera s_Camera;
PlyModel s_Model;

Vec3 *s_PatchColors;
GLint *s_PatchNames;
int s_LockAdapt = 0;

int s_Dragging;
Vec3 eye;

const int RBK_MAGIC = 1230985712;

int s_BudgetMode = 0;
int s_ErrorMode = 0;
int s_Triangles = 1000; float s_Threshold = 1;
int s_PolygonFill = 1;

void SetCamera();

void IdleProc(void) {
    glutPostRedisplay();
}

void DoModelReadback(char* file) {
    GLint size;
    char* data; 
    int extra = sizeof(Vec3) + 2*sizeof(int);
    
    printf("Beginning Binary Readback...\n");
    glodGetObjectParameteriv(0, GLOD_READBACK_SIZE, &size);
    printf("Object will be %f MB.\n", (float)size / (1024.0*1024.0));
    
    data = (char*) malloc(size + extra);
    
    glodReadbackObject(0, (void*) (data + extra));
    printf("Readback complete.\n");
    printf("Writing to %s... ", file);
    FILE* f = fopen(file, "wb");
    if(f == NULL) {
        printf("File error: could not open %s. Aborting!\n", file);
        free(data);
        return;
    }
    // prepend a camera position to this data
    memcpy(data,&RBK_MAGIC, sizeof(int));
    memcpy(data+sizeof(int),&size, sizeof(int));  
    memcpy(data+2*sizeof(int), s_Model.max, sizeof(Vec3));
    
    fwrite(data, size + extra, 1, f);
    fclose(f);
    free(data);
    printf("done.\n");
}

void DoModelLoad(char* in_file, Vec3 max) {
    int magic;int size; char* data;
    FILE* f = fopen(in_file, "rb");
    fread(&magic, sizeof(int), 1, f);
    if(magic != RBK_MAGIC) {
        printf("Bad magic on %s\n", in_file);
        fclose(f);
        exit(0);
    }
    fread(&size, sizeof(int), 1, f);
    printf("Model needs %i bytes to load.\n",size);
    data = (char*) malloc(size);
    
    fread(max, sizeof(Vec3), 1, f);
    
    fread(data, size, 1, f);
    fclose(f);
    
    // load the data
    printf("Loading object...");
    glodNewGroup(0);
    glodLoadObject(0, 0, data);
    printf("done.\n");
    printf("Max: "); VEC3_PRINT(max);
    free(data);
}

void KeyProc(unsigned char key, int x, int y) {
    //  char ukey = toupper(key);
    if(key == 'q' || key=='Q') {
        glodShutdown();  
        DeleteModel(&s_Model);
        exit(0);
    } else if(key == 'z') {
        s_Camera.SetMode(CAMERA_ROTATE);
    } else if(key == 'x') {
        s_Camera.SetMode(CAMERA_ZOOM);
    } else if(key == 'c') {
        s_Camera.SetMode(CAMERA_PAN);
    } else if(key == 'v') {
        s_Camera.SetMode(CAMERA_FREE_ROTATE);
    } else if(key == 'l') {
        s_LockAdapt = ! s_LockAdapt;
    } else if(key == ' ') {
        s_BudgetMode = !s_BudgetMode;
        if (!s_BudgetMode) {
            glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_ERROR_THRESHOLD);
            printf("Error threshhold mode.\n");
        } else {
            glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_TRIANGLE_BUDGET);
            printf("Triangle budget mode.\n");
        }
    } else if (key == 'w') {
        s_PolygonFill = !s_PolygonFill;
        if (s_PolygonFill)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else if(key == '=') {
        s_Triangles /= 2;
        s_Threshold *= 1.5;
        printf("Threshold: %f\t\tTriangles:%i\n", s_Threshold, s_Triangles);
    } else if(key == '-') {
        s_Triangles *= 2;
        s_Threshold /= 1.5;
        printf("Threshold: %f\t\tTriangles:%i\n", s_Threshold, s_Triangles);
    } else if (key == 'e') {
        s_ErrorMode = !s_ErrorMode;
        if (!s_ErrorMode) {
            glodGroupParameteri(0, GLOD_ERROR_MODE, GLOD_OBJECT_SPACE_ERROR);
            printf("Object-space error mode.\n");
        }
        else
        {
            glodGroupParameteri(0, GLOD_ERROR_MODE, GLOD_SCREEN_SPACE_ERROR);
            printf("Screen-space error mode.\n");
        }
    } else {
        printf("Keyboard Usage\n");
#define KP(key,usage) printf(" [%8s] %s\n", (key), (usage));
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
void DisplayProc(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(s_BudgetMode != 0) {
        glodGroupParameteri(0, GLOD_MAX_TRIANGLES, s_Triangles);
    } else {
        if(s_ErrorMode == 0) {
            glodGroupParameterf(0, GLOD_OBJECT_SPACE_ERROR_THRESHOLD, s_Threshold);
        } else {
            glodGroupParameterf(0, GLOD_SCREEN_SPACE_ERROR_THRESHOLD, s_Threshold);
        }
    }
    
    glodBindObjectXform(0, GL_PROJECTION_MATRIX | GL_MODELVIEW_MATRIX);
    if(! s_LockAdapt)
        glodAdaptGroup(0);
    
    GLint npatches;
    glodGetObjectParameteriv(0, GLOD_NUM_PATCHES, &npatches);
    
    /*    if(s_Model.has_vertex_colors)
        glEnableClientState(GL_COLOR_ARRAY);
    if(s_Model.has_vertex_normals)
        glEnableClientState(GL_NORMAL_ARRAY);
    if(s_Model.has_texcoords)
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    */
    for(int i=0; i < npatches; i++) {
        glColor3f(s_PatchColors[i][0],s_PatchColors[i][1],s_PatchColors[i][2]);
        glodDrawPatch(0,s_PatchNames[i]);
    }
    
    
    PushOrtho(SCREEN_WIDTH, SCREEN_HEIGHT);
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1.0);
    //  DrawString("Nat is cooler than GLOD.", 2, 3);
    glEnable(GL_LIGHTING);
    PopOrtho();
    
    glutSwapBuffers();
}

void ReshapeProc(int w, int h) {
    glViewport(0, 0, w, h);
    SCREEN_WIDTH = w;
    SCREEN_HEIGHT = h;
    s_Camera.AutoPlaceBall();
    SetCamera();
}

void Usage();
void KeyProc(unsigned char key, int x, int y);
void MotionProc(int x, int y);
void MouseProc(int button, int state, int x, int y);
void LoadModel(char*,int);

bool IsRBK(char* file) {
    FILE* f = fopen(file, "rb");
    if(f == NULL) {
        printf("Error: Could not open %s\n", file);
        exit(0);
    }
    int mgk;
    fread(&mgk, sizeof(int), 1, f);
    fclose(f);
    if(mgk == RBK_MAGIC)
        return true;
    return false;
}

int main(int argc, char ** argv) {
    char title[] = "GLOD Object Readback Demo";
    glutInit(&argc, (char**)argv); //required for osx apps
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
    glClearColor(0.0,0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);
    
    glLoadIdentity();
    
    
    /***************************************************************************
    * Init
    ***************************************************************************/
    char* in_file = NULL;
    char* out_file = NULL;
    int dump = 1;
    int inv = 0;
    int mode = GLOD_DISCRETE;
    int build_op = GLOD_OPERATOR_HALF_EDGE_COLLAPSE;
    int simp_mode = GLOD_METRIC_SPHERES;

    {
        // get config options
        int i; 
        for(i = 1; i < argc; i++) {
            if(strcmp(argv[i], "--help") == 0) {
                Usage();
                return 0;
            }
            
            if(strcmp(argv[i], "-c") == 0) {
                printf("Using continuous simplification.\n");
                mode = GLOD_CONTINUOUS;
                continue;
            }
            
            if(strcmp(argv[i], "-d") == 0) {
                printf("Using discrete simplification.\n");
                mode = GLOD_DISCRETE;
                continue;
            }
            
            if(strcmp(argv[i], "-q") == 0) {
                printf("Using quadric error.\n");
                simp_mode = GLOD_METRIC_QUADRICS;
                continue;
            }
            
            if(strcmp(argv[i], "-i") == 0) {
                inv = 1;
                continue;
            }
            
            if(strcmp(argv[i], "-e") == 0) {
                printf("Using full edge collapse.\n");
                build_op = GLOD_OPERATOR_EDGE_COLLAPSE;
                continue;
            }

            if(strcmp(argv[i], "-view") == 0) {
                dump = 0;
                continue;
            }
            
            // all else failed, it must be the "unflagged one"
            if(in_file == NULL) 
                in_file = argv[i];
            else
                out_file = argv[i];
        }
        
        if(in_file == NULL) {
            printf("No input file specified. Cannot continue.\n\n");
            Usage();
            return 0;
        }
        
        if(out_file == NULL) {
            if(! IsRBK(in_file)) {
                printf("Input file is not an RBK file. Cannot continue.\n\n");
                Usage();
                return 0;
            }
        }
    }
    
    // setup
    if(glodInit() == 0) {
        printf("Could not load GLOD!\n");
        return 0;
    }
    
    /* init random  */
#ifndef _WIN32
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec % tv.tv_usec);
#endif
    
    // load our model
    Vec3 max;
    glEnableClientState(GL_VERTEX_ARRAY);
    if(out_file != NULL) { // this is the one where we go ply->rbk
        LoadModel(in_file,inv);
        
        // Simplify using GLOD
        SetupVertexArray(&s_Model,VERTEX_ARRAY_ELEMENTS);
        
        glodNewGroup(0);
        glodNewObject(0,0, mode);
        printf("This model has %i patches!\n", s_Model.npatches);
        s_PatchColors = (Vec3*)malloc(sizeof(Vec3) * s_Model.npatches);
        s_PatchNames = (GLint*) malloc(sizeof(GLint) * s_Model.npatches);
 
        for(int pnum = 0; pnum < s_Model.npatches; pnum++) {
            // give it a random color
            for(int k = 0; k < 3; k++)
                s_PatchColors[pnum][k] = ((float)rand() / (float)RAND_MAX);
            
            // foo
            s_PatchNames[pnum] = pnum*2;
            BindVertexArray(&s_Model, pnum);
            
            glodInsertElements(0, s_PatchNames[pnum],
                GL_TRIANGLES, s_Model.plist[pnum].nindices, GL_UNSIGNED_INT, s_Model.plist[pnum].indices,
                0,0.0);
                /*	glodInsertArrays(0, 0, GL_TRIANGLES, 0, s_Model.plist[0].nindices,
            0,0.0);*/
        }
        glodObjectParameteri(0,GLOD_BUILD_OPERATOR, build_op);
        glodObjectParameteri(0,GLOD_BUILD_ERROR_METRIC,simp_mode);
        glodBuildObject(0);
        DoModelReadback(out_file);
        
        if(dump == 1) {
            glodShutdown();
            DeleteModel(&s_Model);
            return 0;
        }
        
        VEC3_COPY(max,s_Model.max); // get ready to position the camera
    } else {
        DoModelLoad(in_file,max);
        
        GLint num_patches;
        glodGetObjectParameteriv(0, GLOD_NUM_PATCHES,&num_patches);
        s_PatchNames = (GLint*) malloc(sizeof(GLint) * num_patches);
        glodGetObjectParameteriv(0, GLOD_PATCH_NAMES,s_PatchNames);
        s_PatchColors = (Vec3*) malloc(sizeof(Vec3) * num_patches);    
        
        for(int pnum = 0; pnum < num_patches; pnum++) {
            // give it a random color
            for(int k = 0; k < 3; k++)
                s_PatchColors[pnum][k] = ((float)rand() / (float)RAND_MAX);
        }
        
        s_Threshold = VEC3_LEN(max) * .05;
    }
    
    {
        
        VEC3_COPY(eye,max);
        VEC3_SCALE(eye,2);
        s_Camera.SetSpeed(eye[0] / 10);
        s_Camera.LookAt(eye[0],eye[1],eye[2],
            0,0,0,
            0,1,0);
        
        SetCamera();
        
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    }
    
    glutMainLoop();
    
    return 0;
    
}


void Usage() {
    printf("Usage:\n");
    printf("   read_model [-i] [-c/-d] [-view] filename.ply output.rbk\n");
    printf("     Writes the simplified model to an intermediate file.\n");
    printf("      -i    Inverts the calculated normals\n");
    printf("      -c    Uses continuous LOD instead of Discrete");
    printf("      -view Present a GUI after writing the RBK file.\n");
    printf("      -e    Use full-edge collapses during building.\n");
    printf("      -q    Use Quadric error metric.\n");
    printf("\n");  
    printf("   read_model input.rbk\n");
    printf("     Displays a simplified model stored in the intermediate format.\n");
    printf("\n\n");
    return;
}



void SetCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,((float)SCREEN_WIDTH)/((float)SCREEN_HEIGHT),1,1000);
    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity();
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float tmp[] = {1,1,0,1};
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLightfv(GL_LIGHT0,
              GL_POSITION,
              tmp);
    s_Camera.SetGLState();
}

void LoadModel(char* filename, int inv) {
    read_plyfile(filename, &s_Model);
    float len = CenterOnOrigin(&s_Model);
    s_Threshold = len * .05;
    if(! s_Model.has_vertex_normals) {
        ComputeVertexNormals(&s_Model,inv);
    } else {
        if(inv)
            InvertVertexNormals(&s_Model);
    }
    
    if(s_Model.has_texcoords) {
        printf("Uses texture coordinates! But I don't have a texture file, yet.\n");
    }
}

void MouseProc(int button, int state, int x, int y) {
    float fx = (float)x;
    float fy = (float)SCREEN_HEIGHT- y;
    
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
    float fy = (float)SCREEN_HEIGHT - y;
    
    if(s_Dragging) {
        s_Camera.Drag(fx,fy);
        SetCamera();
        glutPostRedisplay();
    }
}

