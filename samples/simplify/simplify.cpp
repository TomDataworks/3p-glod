
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
GLint nLevels = 0;
GLint *levelTris;
GLfloat *levelErrors;
GLfloat reductionFactor=.5;
int modelTris=0;
GLfloat shareTolerance=0;

#define ERROR_SNAPSHOT 1
#define TRI_SNAPSHOT 2
#define PERCENT_SNAPSHOT 3

void DoCutReadback(char *filename);

int snapshotType=0;

int interactive=0;

bool s_SleepAtQuit = false;

void SetCamera();

typedef struct _VA {
    int nindices;
    int nverts;
    VertexArray* va;
    int* indices;
} VA;

typedef struct _Face {
    unsigned char nverts;
    int *idx;
    int patch_num;
    
} vaFace;

static PlyProperty va_vert_props[] = {
    {"x", PLY_FLOAT, PLY_FLOAT, offsetof(VertexArray,coord[0]), 0, 0, 0, 0},
    {"y", PLY_FLOAT, PLY_FLOAT, offsetof(VertexArray,coord[1]), 0, 0, 0, 0},
    {"z", PLY_FLOAT, PLY_FLOAT, offsetof(VertexArray,coord[2]), 0, 0, 0, 0},
    {"nx", PLY_FLOAT, PLY_FLOAT, offsetof(VertexArray,normal[0]), 0, 0, 0, 0},
    {"ny", PLY_FLOAT, PLY_FLOAT, offsetof(VertexArray,normal[1]), 0, 0, 0, 0},
    {"nz", PLY_FLOAT, PLY_FLOAT, offsetof(VertexArray,normal[2]), 0, 0, 0, 0},      
    {"red", PLY_UCHAR, PLY_UCHAR, offsetof(VertexArray,color[0]), 0, 0, 0, 0},
    {"green", PLY_UCHAR, PLY_UCHAR, offsetof(VertexArray,color[1]), 0, 0, 0, 0},
    {"blue", PLY_UCHAR, PLY_UCHAR, offsetof(VertexArray,color[2]), 0, 0, 0, 0},
    {"u", PLY_FLOAT, PLY_FLOAT, offsetof(VertexArray,texcoord[0]), 0, 0, 0, 0},      
    {"v", PLY_FLOAT, PLY_FLOAT, offsetof(VertexArray,texcoord[1]), 0, 0, 0, 0},      
};

static PlyProperty va_face_props[] = { /* list of property information for a vertex */
{"vertex_indices", PLY_INT, PLY_INT, offsetof(vaFace,idx),
    1, PLY_UCHAR, PLY_UCHAR, offsetof(vaFace,nverts)},
{"patch_num", PLY_INT, PLY_INT, offsetof(vaFace,patch_num), 0, 0, 0, 0},
};

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
    glClearColor(0,0,.5,1);
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
            
            if (strcmp(argv[i], "-I") ==0){
                printf("Using interactive mode\n");
                interactive=1;
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
            
            if(strcmp(argv[i], "-share") == 0) {
                shareTolerance = atof(argv[i+1]);
                i++;
                printf("Share tolerance is %f.\n", shareTolerance);
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
            
            if (strcmp(argv[i], "-ntris")==0){
                if (snapshotType){
                    printf("can't have both triangle and error snapshots\n");
                    continue;
                }
                snapshotType = TRI_SNAPSHOT;
                nLevels=1;
                for (int j=0; j<argc; j++){
                    if (strcmp(argv[j], "-nlevels")==0){
                        if (j<argc-1)
                            nLevels = atoi(argv[j+1]);
                    }
                }
                if (i<argc-1) { 
                    levelTris = new GLint[nLevels];
                    printf("creating %i triangle levels\n", nLevels);
                    int l=0;
                    for (int j=i+1; j<i+1+nLevels; j++){
                        levelTris[l] = atoi(argv[j]);
                        printf("\tlevel %i - %i tris\n", l, levelTris[l]);
                        l++;
                    }
                }
                continue;
            }
            
            if (strcmp(argv[i], "-errors")==0){
                if (snapshotType){
                    printf("can't have both triangle and error snapshots\n");
                    continue;
                }
                snapshotType = ERROR_SNAPSHOT;
                nLevels=1;
                for (int j=0; j<argc; j++){
                    if (strcmp(argv[j], "-nlevels")==0){
                        if (j<argc-1)
                            nLevels = atoi(argv[j+1]);
                    }
                }
                if (i<argc-1) { 
                    levelErrors = new GLfloat[nLevels];
                    printf("creating %i error levels\n", nLevels);
                    int l=0;
                    for (int j=i+1; j<i+1+nLevels; j++){
                        levelErrors[l] = atof(argv[j]);
                        printf("\tlevel %i error = %f\n", l, levelErrors[l]);
                        l++;
                    }
                }
                continue;
            }
            
            if (strcmp(argv[i], "-percent")==0){
                if (snapshotType){
                    printf("can't have both triangle and error snapshots\n");
                    continue;
                }
                snapshotType = PERCENT_SNAPSHOT;
                reductionFactor = atof(argv[i+1]);
                if (reductionFactor>1)
                    reductionFactor/=100;
                printf("Each triangle level will have %f%% fewer triangles\n", reductionFactor*100);
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
    
    if (!snapshotType)
        snapshotType = PERCENT_SNAPSHOT;
    
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
        glodObjectParameterf(0, GLOD_BUILD_SHARE_TOLERANCE, shareTolerance);
        
        //int snapshots[6]={10000,5000,2000,1000,500,10};
        //glodObjectParameteri(0, GLOD_BUILD_SNAPSHOT_MODE, GLOD_SNAPSHOT_TRI_SPEC);
        //glodObjectParameteriv(0, GLOD_BUILD_TRI_SPECS, 6, snapshots);
        /*
        glodObjectParameteri(0, GLOD_BUILD_SNAPSHOT_MODE, GLOD_SNAPSHOT_ERROR_SPEC);
        float errs[1] = {(float)1000};
        glodObjectParameterfv(0, GLOD_BUILD_ERROR_SPECS, 1, errs);
        */
        if (mode==GLOD_DISCRETE){
            if (snapshotType==TRI_SNAPSHOT){
                glodObjectParameteri(0, GLOD_BUILD_SNAPSHOT_MODE, GLOD_SNAPSHOT_TRI_SPEC);
                glodObjectParameteriv(0, GLOD_BUILD_TRI_SPECS, nLevels, levelTris);
            } else if (snapshotType==ERROR_SNAPSHOT){
                glodObjectParameteri(0, GLOD_BUILD_SNAPSHOT_MODE, GLOD_SNAPSHOT_ERROR_SPEC);
                glodObjectParameterfv(0, GLOD_BUILD_ERROR_SPECS, nLevels, levelErrors);
            } else if (snapshotType==PERCENT_SNAPSHOT){
                glodObjectParameterf(0, GLOD_BUILD_PERCENT_REDUCTION_FACTOR, reductionFactor);
            }
        }
        
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
        
        if (nLevels==0){ //get number of levels from GLOD
            
        }
        printf("Original model has %i tris\n", modelTris);
        char *newName = new char[strlen(in_file)+20];
        
        if (snapshotType==TRI_SNAPSHOT){
            for (int i=0; i<nLevels; i++){
                glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_TRIANGLE_BUDGET);
                glodGroupParameteri(0, GLOD_MAX_TRIANGLES, levelTris[i]+1);
                
                
                if (mode==GLOD_CONTINUOUS){
                    GLint npatches=0;
                    GLint ntris=0;
                    do {
                        ntris=0;
                        glodAdaptGroup(0);
                        glodGetObjectParameteriv(0, GLOD_NUM_PATCHES, &npatches);
                        GLint *cut_sizes = (GLint*) malloc(sizeof(GLint) * 2);
                        for (int i=0; i<npatches; i++){
                            glodGetObjectParameteriv(0, GLOD_PATCH_SIZES, cut_sizes);
                            ntris+=(cut_sizes[0]/3);
                        }
                    } while ((ntris>(levelTris[i]+1)) || (ntris<(levelTris[i]-1)));
                }
                else 
                    glodAdaptGroup(0);
                sprintf(newName, "out.%i.ply", (int)levelTris[i]);
                DoCutReadback(newName);
            }
        } else if (snapshotType==ERROR_SNAPSHOT){
            for (int i=0; i<nLevels; i++){
                glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_ERROR_THRESHOLD);
                glodGroupParameterf(0, GLOD_OBJECT_SPACE_ERROR_THRESHOLD, levelErrors[i]+.000000001);
                if (mode==GLOD_CONTINUOUS)
                    for (int j=0; j<20; j++)
                        glodAdaptGroup(0);
                else
                    glodAdaptGroup(0);
                sprintf(newName, "out.%.4f.ply", (float)levelErrors[i]);
                DoCutReadback(newName);
            }
        } else if (snapshotType==PERCENT_SNAPSHOT){
            float tris=modelTris;
            while (tris>1){
                glodGroupParameteri(0, GLOD_ADAPT_MODE, GLOD_TRIANGLE_BUDGET);
                glodGroupParameteri(0, GLOD_MAX_TRIANGLES, (int)tris+1);
                if (mode==GLOD_CONTINUOUS){
                    GLint npatches=0;
                    GLint ntris=0;
                    do {
                        ntris=0;
                        glodAdaptGroup(0);
                        glodGetObjectParameteriv(0, GLOD_NUM_PATCHES, &npatches);
                        GLint *cut_sizes = (GLint*) malloc(sizeof(GLint) * 2);
                        for (int i=0; i<npatches; i++){
                            glodGetObjectParameteriv(0, GLOD_PATCH_SIZES, cut_sizes);
                            ntris+=(cut_sizes[0]/3);
                        }
                    } while ((ntris>(tris+1)) || (ntris<(tris-1)));
                }
                else 
                    glodAdaptGroup(0);
                sprintf(newName, "out.%i.ply", (int)tris);
                DoCutReadback(newName);
                tris*=(1-reductionFactor);
            }
        }
        
        fprintf(stderr, "done.\n");
        

    }
    if (!interactive)
        return 0;
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
    printf("   simplify [flags] <filename> [-t texture.ppm]\n");
    printf("       Simplifies a PLY file through GLOD and writes out the results to PLY files.\n");
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
    printf("      -nlevels Number of snapshot levels to use\n");
    printf("      -ntris   Triangle counts for snapshot levels. Must match number specified by nlevels\n");
    printf("      -errors  Error values for snapshot levels. Must match number of levels from nlevels\n");
    printf("      -percent Percent reduction factor for each level\n");
    printf("      -share   Specifies the sharing tolerance when building LOD hierarchy\n");
    printf("      -I       Enters interactive mode, where the created hierarchy can be inspected visually\n");
    
    

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
    modelTris = s_Model.nfaces;
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

void DoCutReadback(char *filename) {
    int i;
    GLint npatches;
    GLint* cut_sizes;
    glodGetObjectParameteriv(0, GLOD_NUM_PATCHES, &npatches);
    cut_sizes = (GLint*) malloc(npatches * sizeof(GLint) * 2);
    VA* nVA = NULL;
    int num_nVA = 0;
    nVA = (VA*) malloc(sizeof(VA) * npatches);
    num_nVA = npatches;
    
    // readback all the arrays
    glodGetObjectParameteriv(0, GLOD_PATCH_SIZES, cut_sizes);
    for(i = 0; i < npatches; i++) {
        // Allocate vertex array
        nVA[i].nindices = cut_sizes[2*i]; nVA[i].nverts = cut_sizes[2*i+1];
        nVA[i].va = (VertexArray*) malloc(sizeof(VertexArray) * nVA[i].nverts);
        nVA[i].indices = (int*) malloc(sizeof(unsigned int) * nVA[i].nindices);
        
        // set up the vertex arrays
        glEnable(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(VertexArray), nVA[i].va->coord);
        if(s_Model.has_vertex_normals){
            glEnable(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, sizeof(VertexArray), nVA[i].va->normal);
        }
        if(s_Model.has_vertex_colors){
            glEnable(GL_COLOR_ARRAY);
            glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(VertexArray), nVA[i].va->color);
        }
        if(s_Model.has_texcoords) {
            glEnable(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(VertexArray), nVA[i].va->texcoord);
        }
        
        // reeeadback!
        printf("Reading back; cut has %i tris..., writing to %s\n", nVA[i].nindices / 3, filename);
        glodFillElements(0, i,
                         GL_UNSIGNED_INT, nVA[i].indices);
        printf("done.\n");
    }
    // now write the ply
    if(1) {
        int total_indices = 0; int total_verts = 0;
        for(i = 0; i < npatches; i++) {
            total_indices += nVA[i].nindices;
            total_verts += nVA[i].nverts;
        }
        
        int j;
        PlyFile *ply;
        FILE* f = fopen(filename, "wb");
        if(f == NULL) {
            fprintf(stderr, "Could not open %s\n", filename);
            return;
        }
        
        char *elem_names[] = { "vertex", "face" };
        ply = ply_write( f, 2, elem_names, PLY_ASCII);
        ply_element_count(ply, "vertex", total_verts);
        
        // coord
        ply_describe_property(ply, "vertex", &va_vert_props[0]);
        ply_describe_property(ply, "vertex", &va_vert_props[1]);
        ply_describe_property(ply, "vertex", &va_vert_props[2]);
        
        // normals
        if(s_Model.has_vertex_normals) {
            ply_describe_property(ply, "vertex", &va_vert_props[3]);
            ply_describe_property(ply, "vertex", &va_vert_props[4]);
            ply_describe_property(ply, "vertex", &va_vert_props[5]);
        }
        
        if(s_Model.has_vertex_normals) {
            ply_describe_property(ply, "vertex", &va_vert_props[6]);
            ply_describe_property(ply, "vertex", &va_vert_props[7]);
            ply_describe_property(ply, "vertex", &va_vert_props[8]);
        }
        
        // texture coords
        if(s_Model.has_texcoords) {
            ply_describe_property(ply, "vertex", &va_vert_props[9]);
            ply_describe_property(ply, "vertex", &va_vert_props[10]);
        }
        
        // patches
        ply_element_count(ply, "face", total_indices / 3);
        ply_describe_property(ply, "face", &va_face_props[0]);
        if(npatches > 1)
            ply_describe_property(ply, "face", &va_face_props[1]);
        ply_header_complete(ply);
        
        // write the vertex elements
        ply_put_element_setup(ply, "vertex");
        
        for(i = 0; i < npatches; i++)
            for(j = 0; j < nVA[i].nverts ; j++)
                ply_put_element(ply, (void*) &(nVA[i].va[j]));
        
        // write the triangles
        int ibase = 0; vaFace face;face.idx = new int[3];
        face.nverts = 3;
        
        ply_put_element_setup(ply, "face");
        for(i = 0; i < npatches; i++) {
            for(j = 0; j < nVA[i].nindices / 3; j++) {
                for(int k = 0; k < 3; k++)
                    face.idx[k] = nVA[i].indices[j*3+k]+ibase;
                face.patch_num = i;
                
                // now make a face
                ply_put_element(ply, &face);
            }
            ibase += nVA[i].nverts;
        }
        
        delete [] (face.idx);
        ply_put_other_elements(ply);
        ply_close(ply);
        //fclose(f);
    }
    
    // now free the arrays
    for(i = 0; i < npatches; i++){
        free(nVA[i].indices);
        free(nVA[i].va);
    }
    free(nVA);
    free(cut_sizes);
    nVA = 0;
    num_nVA = 0;
}

