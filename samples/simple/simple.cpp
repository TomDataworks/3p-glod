
#define DEBUG_ALLOC_BREAKPOINT -1
#define FRAME_COUNTER
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
#include <stdio.h>
# ifdef _DEBUG
#   include <crtdbg.h>
# endif
#else
#include <unistd.h>
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

#include "glod.h"
#include "nat_math.h"
#include "nat_camera.h"
#include "nat_drawstring.h" // for glutTextOut crap
#include "PlyModel.h"

#ifdef FRAME_COUNTER
#include "nat_timer.h" // for frame counter
#define s_FrameCounterBucketSize 10
natFPS s_FPS(s_FrameCounterBucketSize);
#endif

int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 240;
natCamera s_Camera;
PlyModel s_Model;
char*    s_TextureName = NULL;

int s_Dragging;
Vec4 eye;

int s_BudgetMode = 0;
int s_ErrorMode = 0;
int s_Triangles = 1000; float s_Threshold = 1;
int s_PolygonFill = 1;
unsigned int s_LockAdapt = 0;

bool s_SleepAtQuit = false;

void SetCamera();

void IdleProc(void) {
    glutPostRedisplay();
}

void KeyProc(unsigned char key, int x, int y) {
    //  char ukey = toupper(key);
    if (key == 'q' || key=='Q') {
        glodDeleteObject(0);
        glodDeleteGroup(0);
        glodShutdown();
        DeleteModel(&s_Model);
        
        if(s_SleepAtQuit) {
          //while(1)
            //sleep(1);
        }

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
            printf("Error threshold mode.\n");
        } else {
            glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_TRIANGLE_BUDGET);
            printf("Triangle budget mode.\n");
        }
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
    } else if (key == 'w') {
        s_PolygonFill = !s_PolygonFill;
        if (s_PolygonFill)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else if(key == '=') {
        if(s_BudgetMode != 0) 
            s_Triangles *= 2;
        else
            s_Threshold *= 1.5;
    } else if(key == '-') {
        if(s_BudgetMode != 0)
            s_Triangles /= 2;
        else
            s_Threshold /= 1.5;
    } else {
        printf("Keyboard Usage\n");
#define KP(key,usage) printf(" [%8s] %s\n", (key), (usage));
        KP("q", "Quit");
        KP("l", "Free/unlock adapting of your geometry.");
        KP("space", "Switch from error mode to triangle budget");
        KP("e", "Toggle between object space and screen space error modes.");
        KP("w", "Display wireframe or filled.");
        KP("+/-", "Change refinement goal value.");
        
        printf("Camera Control\n");
        KP("z", "Rotate");
        KP("x", "Zoom");
        KP("c", "Pan");
        KP("v", "Free rotate");
        printf("\n");
    }
    
    
    glutPostRedisplay();
    (void)x; (void)y;
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
    
    // DebugCamera(&s_Camera);
    //    SetCamera();
    glodBindObjectXform(0, GL_PROJECTION_MATRIX | GL_MODELVIEW_MATRIX);
    if(! s_LockAdapt)
        glodAdaptGroup(0);
    GLint npatches;
    glodGetObjectParameteriv(0, GLOD_NUM_PATCHES, &npatches);
    glColor3f(.8,.8,.8);
    // set a texture
    if(s_Model.has_texture && s_PolygonFill) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, s_Model.texture_id);
    }
    for(int i=0; i < npatches; i++) {
        glodDrawPatch(0,i);
    }
    if(s_Model.has_texture && s_PolygonFill) {
        glDisable(GL_TEXTURE_2D);
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

#ifdef FRAME_COUNTER
        sprintf(buf, "%.1f fps", s_FPS.GetFPS());
        DrawString(buf, SCREEN_WIDTH - 2 - GetDrawStringWidth(buf), 3);
#endif
    }
    
    PopOrtho();
    glEnable(GL_LIGHTING);
    
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

int main(int argc, char ** argv) {
#if defined(WIN32) && defined(_DEBUG)
    _CrtSetBreakAlloc(DEBUG_ALLOC_BREAKPOINT);
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag( tmpFlag );
#endif
    
    char title[] = "GLOD - Simple Example";
    glutInit(&argc, (char**)argv);
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
    int inv = 0;
    int mode = GLOD_DISCRETE;
    int simp_mode = GLOD_METRIC_SPHERES;
    int edgeLock = GLOD_BORDER_UNLOCK;
    int build_op = GLOD_OPERATOR_HALF_EDGE_COLLAPSE;
    int queue_mode = GLOD_QUEUE_GREEDY;
    float pg_precision = 1.0f;
    {
        // get config options
        int i; 
        for(i = 1; i < argc; i++) {
            if(strcmp(argv[i], "--help") == 0) {
                Usage();
                return 0;
            }

            if(strcmp(argv[i], "-sleep") == 0) {
              s_SleepAtQuit = true;
              continue;
            }

            if(strcmp(argv[i], "-i") == 0) {
                printf("Will invert computed or given normals.\n");
                inv = 1;
                continue;
            }
                            
            if(strcmp(argv[i], "-c") == 0) {
                printf("Using continuous simplification.\n");
                mode = GLOD_CONTINUOUS;
                continue;
            }
            
            if(strcmp(argv[i], "-p") == 0) {
                printf("Using discrete patch simplification.\n");
                mode = GLOD_DISCRETE_PATCH;
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
            
            if(strcmp(argv[i], "-e") == 0) {
                printf("Using full edge collapse.\n");
                build_op = GLOD_OPERATOR_EDGE_COLLAPSE;
                continue;
            }
            
            if(strcmp(argv[i], "-l") == 0) {
                printf("Using border vertex lock.\n");
                edgeLock = GLOD_BORDER_LOCK;
                continue;
            }

            if(strcmp(argv[i], "-t") == 0) {
                printf("Using texture: %s\n", argv[i+1]);
                s_TextureName = argv[i+1];
                i++;
                continue;
            }
            
            if(strcmp(argv[i], "-lq") == 0)
            {
                printf("Using lazy simplification queue.\n");
                queue_mode = GLOD_QUEUE_LAZY;
                continue;
            }

            if(strcmp(argv[i], "-pg") == 0)
            {
                pg_precision = -1.0f;
                printf("Using permission grid.\n");
                simp_mode = GLOD_METRIC_PERMISSION_GRID;
                if (i < argc-1)
                    pg_precision = atof(argv[i+1]);
                if (pg_precision <= 0.0f)
                {
                    pg_precision = 2.0;
                    fprintf (stderr, " Permission grid precision not set.\n");
                }
                else i++; 
                printf (" Setting permission grid precision to %f.\n", pg_precision);
                continue;
            }
            
            // all else failed, it must be the "unflagged one"
            in_file = argv[i];
        }
        
        if(in_file == NULL) {
            printf("No input file specified. Cannot continue.\n\n");
            Usage();
            return 0;
        }
    }
    
    // setup
    if(glodInit() == 0) {
        printf("Error: could not load GLOD!\n");
        return 0;
    }
    
    
    glEnableClientState(GL_VERTEX_ARRAY);
    {
        // load our model file
        LoadModel(in_file,inv);
        
        // Simplify using GLOD
        SetupVertexArray(&s_Model,VERTEX_ARRAY_ELEMENTS);
        
        glodNewGroup(0);
        glodNewObject(0,0,mode);
        printf("This model has %i patches!\n", s_Model.npatches);
        for(int pnum = 0; pnum < s_Model.npatches; pnum++) {
            BindVertexArray(&s_Model, pnum);
            
            glodInsertElements(0, pnum, 
                GL_TRIANGLES, s_Model.plist[pnum].nindices, GL_UNSIGNED_INT, s_Model.plist[pnum].indices,
                0,0.0);
                /*   glodInsertArrays(0, 0, GL_TRIANGLES, 0, s_Model.plist[0].nindices,
            0,0.0);*/
        }
        
        fprintf(stderr, "Building LOD hierarchy...");
        
        glodObjectParameteri(0,GLOD_BUILD_OPERATOR, build_op);
        glodObjectParameteri(0,GLOD_BUILD_ERROR_METRIC,simp_mode);
        glodObjectParameteri(0,GLOD_BUILD_BORDER_MODE, edgeLock);
        glodObjectParameteri(0,GLOD_BUILD_QUEUE_MODE, queue_mode);
        
        //int snapshots[6]={10000,5000,2000,1000,500,10};
        //glodObjectParameteri(0, GLOD_BUILD_SNAPSHOT_MODE, GLOD_SNAPSHOT_TRI_SPEC);
        //glodObjectParameteriv(0, GLOD_BUILD_TRI_SPECS, 6, snapshots);
        /*
        glodObjectParameteri(0, GLOD_BUILD_SNAPSHOT_MODE, GLOD_SNAPSHOT_ERROR_SPEC);
        float errs[1] = {(float)1000};
        glodObjectParameterfv(0, GLOD_BUILD_ERROR_SPECS, 1, errs);
        */
        
        
        glodObjectParameterf(0,GLOD_BUILD_PERMISSION_GRID_PRECISION, pg_precision);
        
        /*
        glodObjectParameteri(0, GLOD_BUILD_SNAPSHOT_MODE, GLOD_SNAPSHOT_ERROR_SPEC);
        const int numerrorspecs = 10;
        float errorspecs[numerrorspecs] = {0.1, 0.5, 1.0, 5.0, 10.0, 50.0, 100.0, 500.0, 1000.0, 5000.0};
        glodObjectParameterfv(0, GLOD_BUILD_ERROR_SPECS, numerrorspecs, errorspecs);
        */
#if 0
        glodObjectParameterf(0, GLOD_BUILD_PERCENT_REDUCTION_FACTOR, 0.75);
#elif 0
        glodObjectParameteri(0, GLOD_BUILD_SNAPSHOT_MODE, GLOD_SNAPSHOT_TRI_SPEC);
        int tris[3] = {10000, 2000, 250};
        glodObjectParameteriv(0, GLOD_BUILD_TRI_SPECS, 3, tris);
#endif
        
        glodBuildObject(0);
        
        fprintf(stderr, "done.\n");
        

    }
    
    { // get ready to position the camera
        Vec3 max; Vec3 min; Vec3 center, tmp;
        VEC3_COPY(max,s_Model.max);
        VEC3_COPY(min,s_Model.min);
        
        VEC3_OP(tmp,max,-,min);
        VEC3_SCALE(tmp,.5);
        VEC3_OP(center,tmp,+,min);
        
        printf("Min: "); VEC3_PRINTLN(min);
        printf("Max: "); VEC3_PRINTLN(max);
        printf("Center: "); VEC3_PRINTLN(center);
        
        VEC3_SCALE(max,2.5);
        s_Camera.LookAt(max[0],max[1],max[2],
                        center[0],center[1],center[2],
                        0,1,0);
        
        /*
        VEC4_SET(eye,max[0],max[1],max[2],0.0);
        VEC3_SCALE(eye,2);

        s_Camera.SetSpeed(eye[0] / 10);
        s_Camera.LookAt(eye[0],eye[1],eye[2],
                        0,0,0,
                        0,1,0);
*/        
        SetCamera();
        
        
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
    
    glutMainLoop();
    
    return 0;
    
}

extern char DEFAULT_TEXTURE;
void Usage() {
    printf("Usage:\n");
    printf("   simple [flags] <filename> [-t texture.ppm]\n");
    printf("       Views a PLY file through GLOD.\n");
    printf("Flags:\n");
    printf("      -i    Inverts the calculated normals\n");
    printf("      -c    Uses continuous LOD instead of Discrete\n");
    printf("      -p    Uses view dependent patch-LOD (experimental)\n");
    printf("      -q    Uses quadric error instead of error spheres\n");
    printf("      -e    Uses full edge collapse instead of half edge\n");
    printf("      -l    Enables border lock (prevents borders from being simplified)\n");
    printf("      -lq   Uses the lazy simplification queue (faster)\n");
    printf("More flags:\n");
    printf("      -t <texture.ppm>   If input filename has texture coordinates, use texture.ppm.\n");
    printf("                         Defaults to %s\n", DEFAULT_TEXTURE_NAME);
    printf("      -pg <p>  Uses permission grid error instead of error spheres, with precision p\n");
    

    printf("\n\n");
    return;
}



void SetCamera() {
    // estimate near and far planes
    Vec3 model_range;
    
    VEC3_OP(model_range, s_Model.max, -, s_Model.min);
    float model_radius = model_range[VEC3_MAJDIM(model_range)];
    float eye_to_center = VEC3_LEN(s_Camera.m_Eye);
    
    float fnear = eye_to_center-model_radius;
    if(fnear < .1) fnear = .1;
    float ffar = eye_to_center+model_radius;
    if(ffar < model_radius) ffar = model_radius;
    
    // apply it 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,((float)SCREEN_WIDTH)/((float)SCREEN_HEIGHT),fnear,ffar);
    
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
    }
    
    if(inv)
        InvertVertexNormals(&s_Model);
    
    if(s_Model.has_texcoords) {
        //printf("Uses texture coordinates! But I don't have a texture file, yet.\n");
        SetupTexture(&s_Model, s_TextureName);
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

/*void DebugCamera() {
Vec3 side; Vec3 up; Vec3 forward;
QUAT_TO_ORIENTATION(&s_Camera.m_Quat,side,up,forward);
printf("Camera debug:\n\tSide:"); VEC3_PRINTLN(side);
printf("\tUp:"); VEC3_PRINTLN(side);
printf("\tForward:"); VEC3_PRINTLN(forward);
printf("\n\n");
  }*/
