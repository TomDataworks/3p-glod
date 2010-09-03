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
/* Nat Duca
 * JHU, October, 2002
 ****************************************************************************/
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

#include "glod.h"
#include "nat_math.h"
#include "nat_camera.h"
#include "PlyModel.h"

int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 240;
natCamera s_Camera;
PlyModel s_Model;
Vec3 eye;

int s_Dragging;

int s_BudgetMode = 0;
int s_Triangles = 1000; float s_Thresshold = 1;
int s_PolygonFill = 1;

char* out_filename = NULL;
char* GetOutFilename() { return out_filename;}

void SetCamera();

void IdleProc(void) {
    glutPostRedisplay();
}

// readback vertex array
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

VA* nVA = NULL;
int num_nVA = 0;

void DoCutReadback() {
    int i;
    GLint npatches;
    GLint* cut_sizes;
    glodGetObjectParameteriv(0, GLOD_NUM_PATCHES, &npatches);
    cut_sizes = (GLint*) malloc(npatches * sizeof(GLint) * 2);
    
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
        printf("Reading back; cut has %i tris...", nVA[i].nindices / 3);
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
        char *tempname = GetOutFilename();
	char *tmp2 = new char[strlen(tempname)+20];
	sprintf(tmp2, "%s-%i.ply", tempname,  total_indices/ 3);
        FILE* f = fopen(tmp2, "wb");
        if(f == NULL) {
            fprintf(stderr, "AAAhhh, could not open %s\n", GetOutFilename());
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

void KeyProc(unsigned char key, int x, int y) {
    //char ukey = toupper(key);
    if(key == 'q' || key=='Q') {
        glodShutdown();
        DeleteModel(&s_Model);
        exit(0);
        
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
        s_Triangles *= 2;
        s_Thresshold *= 1.5;
        printf("Thresshold: %f\t\tTriangles:%i\n", s_Thresshold, s_Triangles);
    } else if(key == '-') {
        s_Triangles /= 2;
        s_Thresshold /= 1.5;
        printf("Thresshold: %f\t\tTriangles:%i\n", s_Thresshold, s_Triangles);
    } else if(key == '\n' || key=='\r') {
        if(nVA == NULL) {
            DoCutReadback();
        } else {
            printf("Cannot readback twice yet.\n");
        }
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
    glClearColor(0,0,.5,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(nVA == NULL) {
        glodGroupParameteri(0, GLOD_MAX_TRIANGLES, s_Triangles);
        glodGroupParameterf(0, GLOD_OBJECT_SPACE_ERROR_THRESHOLD, s_Thresshold);
        glodBindObjectXform(0, GL_MODELVIEW_MATRIX | GL_PROJECTION_MATRIX);
        glodAdaptGroup(0);
        GLint npatches;
        glodGetObjectParameteriv(0, GLOD_NUM_PATCHES, &npatches);
        GLint *patchnames = (GLint*)malloc(npatches * sizeof(GLint));
        glodGetObjectParameteriv(0, GLOD_PATCH_NAMES, patchnames);
        
        for(int i = 0; i < npatches; i++)
            glodDrawPatch(0, patchnames[i]);
        
        free(patchnames);
    } else { 
        for(int i = 0; i < num_nVA; i++) {
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
            glDrawElements(GL_TRIANGLES, nVA[i].nindices, GL_UNSIGNED_INT, nVA[i].indices);
        }
    }
    
    PushOrtho(SCREEN_WIDTH, SCREEN_HEIGHT);
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    char buf[512];
    sprintf(buf, "[Enter] - Write current model to %s.", GetOutFilename());
    DrawString(buf, 2, 3);
    glPopAttrib();
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

int main(int argc, char ** argv) {
    char title[] = "GLOD Cut Readback Demo";
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
    
    
    printf("Keyboard: space bar === toggle between TRI and Error budget\n          +/- ==== change budget values\n\n\n");
    /***************************************************************************
    * Init
    ***************************************************************************/
    char* in_file = NULL;
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
            
            if(strcmp(argv[i], "-i") == 0) {
                inv = 1;
            }
            
            if(strcmp(argv[i], "-e") == 0) {
                printf("Using full edge collapse.\n");
                build_op = GLOD_OPERATOR_EDGE_COLLAPSE;
                continue;
            }
            
            if(strcmp(argv[i], "-q") == 0) {
                printf("Using quadric error metric.\n");
                simp_mode = GLOD_METRIC_QUADRICS;
                continue;
            }
            
            // all else failed, it must be the "unflagged one"
            if(in_file == NULL)
                in_file = argv[i];
            else
                out_filename = argv[i];
        }
        
        if(in_file == NULL) {
            printf("No input file specified. Cannot continue.\n\n");
            Usage();
            return 0;
        }
        
        if(out_filename == NULL) {
            printf("No output file specified. Cannot continue.\n\n");
            Usage();
            return 0;
        }
    }
    
    // setup
    if(glodInit() == 0) {
        printf("Could not load GLOD!\n");
        return 0;
    }
    
    
    Vec3 max;
    if(1) {
        // load our model file
        LoadModel(in_file,inv);
        
        // Simplify using GLOD
        SetupVertexArray(&s_Model,VERTEX_ARRAY_ELEMENTS);
        
        
        glodNewGroup(0);
        glodNewObject(0,0,mode);
        printf("This model has %i patches\n", s_Model.npatches);
        for(int pnum = 0; pnum < s_Model.npatches; pnum++) {
            BindVertexArray(&s_Model, pnum);
            glodInsertElements(0, pnum, 
                               GL_TRIANGLES, s_Model.plist[pnum].nindices, GL_UNSIGNED_INT, s_Model.plist[pnum].indices,
                               0,0.0);
            /*	glodInsertArrays(0, 0, GL_TRIANGLES, 0, s_Model.plist[0].nindices,
                0,0.0);*/
        }
        glodObjectParameteri(0,GLOD_BUILD_OPERATOR, build_op);
        glodObjectParameteri(0,GLOD_BUILD_ERROR_METRIC,simp_mode);
        glodBuildObject(0);
        
        VEC3_COPY(max,s_Model.max); // get ready to position the camera
    }
    
    Vec3 min; Vec3 center, tmp;
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
    
    if (0){
        VEC3_COPY(eye,max);
        VEC3_SCALE(eye,2);
        s_Camera.LookAt(eye[0],eye[1],eye[2],
            0,0,0,
            0,1,0);
        
        SetCamera();
    }
    
    glutMainLoop();
    
    return 0;
    
}


void Usage() {
    printf("Usage:\n");
    printf("   read_cut [-i] [-c/-d] <input.ply> <output.ply>\n");
    printf("     Writes a ply file of the current simplified cut.\n");
    printf("      -i    Inverts the calculated normals when loading a ply\n");
    printf("      -c    Uses continuous LOD instead of Discrete when loading a ply\n");
    printf("      -e    Uses full-edge collapses instead of half-edge collapses\n");
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
    float tmp[] = {0,0,-1,1};
    glLightfv(GL_LIGHT0,
        GL_POSITION,
        tmp);
    
    //    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
    s_Camera.SetGLState();
}

void LoadModel(char* filename, int inv) {
    read_plyfile(filename, &s_Model);
    float len = CenterOnOrigin(&s_Model);
    s_Thresshold = len * .05;
    if(! s_Model.has_vertex_normals) {
        ComputeVertexNormals(&s_Model,inv);
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

