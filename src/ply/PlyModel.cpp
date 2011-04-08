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

// added by Budi
//#define POLYGON_SUPPORT   
#define VERTEX_COLOR         // vertex can read red, green, blue 
// if undef, vertex can read diffuse_red, diffuse_green,
// and diffuse_blue (support for plyshade)

/*----------------------------- Local Includes -----------------------------*/
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <ply.h>

#include "PlyModel.h"

#ifdef WIN32
#include <glod_glext.h>
#else
#include <unistd.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/glx.h>
#endif
#endif

/*----------------------------- Local Constants -----------------------------*/




/*------------------------------ Local Macros -------------------------------*/
#define VEC3_V_OP_V(a,b,op,c)  { (a)[0] = (b)[0] op (c)[0]; \
    (a)[1] = (b)[1] op (c)[1]; \
(a)[2] = (b)[2] op (c)[2]; }
#define CROSSPROD3(a,b,c)       {(a)[0]=(b)[1]*(c)[2]-(b)[2]*(c)[1]; \
    (a)[1]=(b)[2]*(c)[0]-(b)[0]*(c)[2]; \
(a)[2]=(b)[0]*(c)[1]-(b)[1]*(c)[0];}


#define LENGTH3(a)  (sqrt((a)[0]*(a)[0]+(a)[1]*(a)[1]+(a)[2]*(a)[2]))

#define NORMALIZE3(a)                                               \
        {                                                           \
        double LAmag;                                              \
        LAmag = LENGTH3(a);                                        \
        if (LAmag != 0) LAmag = 1.0/LAmag;                         \
        (a)[0] *= LAmag; (a)[1] *= LAmag; (a)[2] *= LAmag;         \
        }

#define SQ_DIST3(a, b)          (((a)[0]-(b)[0])*((a)[0]-(b)[0]) +      \
    ((a)[1]-(b)[1])*((a)[1]-(b)[1]) +      \
((a)[2]-(b)[2])*((a)[2]-(b)[2]))

#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define MIN(x,y) ((x)<(y) ? (x) : (y))

/*------------------------------- Local Types -------------------------------*/


/*------------------------------ Local Globals ------------------------------*/

const Color DEFAULT_COLOR = {200, 200, 200};
static const FloatColor FRONT_COLOR = {0.0, 1.0, 0.0};
static const FloatColor BACK_COLOR = {1.0, 0.0, 0.0};








/*------------------------ Local Function Prototypes ------------------------*/
int LoadPPMTexture(char *);
void InitRegisterCombiners();

/*---------------------------------Functions-------------------------------- */

void RandomColor(Color color)
{
    color[0] = rand()%256;
    color[1] = rand()%256;
    color[2] = rand()%256;
}


/**
* MakePatches:  Given a model with patches (face contains patch_num),
*               build data structure for each patch.
* @author  Jonathan Cohen (modified by Budirijanto Purnomo) (patch 0 by nat)
* @date  08/15/2002
* @param  model     data structure for a model
*/
void MakePatches(PlyModel *model)
{
    Face *face;
    Patch *patch;
    int i, j;
    
    if(! model->has_patches) {
        //      printf("Building fake patch 0.\n");
        model->face_has[1] = TRUE;
        for(i = 0;  i < model->nfaces; i++)
            model->flist[i].patch_num = 0;
    }
    
    model->npatches = 0;
    if (model->face_has[1] == FALSE)
        return;
    
    // ply patches should be numbered from 0 to num_patches - 1
    for (i=0; i<model->nfaces; i++)
    {
        if (model->npatches < (model->flist[i].patch_num + 1))
            model->npatches = model->flist[i].patch_num + 1;
    }
    ALLOCN(model->plist, Patch, model->npatches);
    
    for (i=0; i<model->npatches; i++)
    {
        RandomColor(model->plist[i].color);
        model->plist[i].nindices = 0;
    }
    
    // starting for here is the changes made by Budi
    // collect indices for each patch
    for (i=0; i<model->nfaces; i++)
    {
        face = &(model->flist[i]);
        
        patch = &(model->plist[face->patch_num]);
        
        // nverts * nfaces in a patch is equal to nindices
        for (j=0; j<face->nverts; j++)
            patch->nindices++;
    }
    
    for (i=0; i<model->npatches; i++)
    {
        ALLOCN(model->plist[i].indices, unsigned int, model->plist[i].nindices+1);
        model->plist[i].nindices = 0;   // reinitialize for the for loop below
    }
    
    // start collecting into indices for each patch
    for (i=0; i<model->nfaces; i++)
    {
        face = &(model->flist[i]);
        
        patch = &(model->plist[face->patch_num]);
        
        for (j=0; j<face->nverts; j++)
            patch->indices[ patch->nindices++ ] = face->verts[j];   
    }
}

/**
* SetupNormalMap:  Load normal map stored in texture for each patch.
* @author  Budirijanto Purnomo 
* @date  08/15/2002
* @param  model   a data structure contains a model
*/
void SetupNormalMap(PlyModel *model)
{
    int i;
    char texturename[80];
    
    if (model->has_texcoords == FALSE || model->has_patches == FALSE)
    {
        model->has_normalmap = FALSE;
        return;
    }
    
    for (i=0; i<model->npatches; i++)
    {
        sprintf(texturename, "%s-%d.ppm", DEFAULT_NORMALMAP_NAME, i);
        
        model->plist[i].texture_id = LoadPPMTexture(texturename);
        
        if (model->plist[i].texture_id == -1)
        {
            model->has_normalmap = FALSE;
            return;
        }
        
        fprintf(stderr, "Finished loading normal map: %s\n", texturename);
    }
    
    model->has_normalmap = TRUE;
    
#ifdef USE_NV
    InitRegisterCombiners();
#endif
}

void ComputeFaceNormals(PlyModel *model)
{
    int i;
    Face *face;
    Vertex *v0, *v1, *v2;
    Vector vec1, vec2;
    
    for (i=0; i<model->nfaces; i++)
    {
        face = &(model->flist[i]);
        v0 = &(model->vlist[face->verts[0]]);
        v1 = &(model->vlist[face->verts[1]]);
        v2 = &(model->vlist[face->verts[2]]);
        
        VEC3_V_OP_V(vec1, v1->coord, -, v0->coord);
        VEC3_V_OP_V(vec2, v2->coord, -, v0->coord);
        CROSSPROD3(face->normal, vec1, vec2);
        NORMALIZE3(face->normal);
    }
    return;
}

void ComputeVertexRadii(PlyModel *model)
{
    int i, j;
    Vertex *v0, *v1;
    Face *face;
    float sq_dist;
    
    for (i=0; i<model->nverts; i++)
        model->vlist[i].radius = MAXFLOAT;
    
    for (i=0; i<model->nfaces; i++)
    {
        face = &model->flist[i];
        for (j=0; j<face->nverts; j++)
        {
            v0 = &model->vlist[face->verts[j]];
            v1 = &model->vlist[face->verts[(j+1)%face->nverts]];
            sq_dist = SQ_DIST3(v0->coord, v1->coord);
            v0->radius = MIN(v0->radius, sq_dist);
            v1->radius = MIN(v0->radius, sq_dist);
        }
    }
    
    for (i=0; i<model->nverts; i++)
        model->vlist[i].radius =
        ( (model->vlist[i].radius >= 0) ?
        sqrt(model->vlist[i].radius) / 2.0 :
    0.00000001 ); // magic constant BAD
    
    return;
}

int LoadPPMTexture( char *filename )
{
    FILE *fp;
    int xsize, ysize, maxval, isize;
    GLvoid *iptr;
    static int id = 0; /* one less than first legal value, which is 1 */
    
    if(( fp = fopen( filename, "rb" )) == NULL )
        return -1;
    
    if(( fgetc( fp ) != 'P' ) || ( fgetc( fp ) != '6' )) {
        printf("Invalid magci in %s\n", filename);
        return -1;
    }
    
    if( fscanf( fp, "%d %d %d", &xsize, &ysize, &maxval ) != 3 ) {
        printf("Unexpected header!\n");
        return -1;
    }
    
    //    printf("[%s]: %ix%i\n", filename, xsize, ysize);
    fgetc(fp);  // advance pointer to after "\n"
    
    isize = xsize * ysize * 3;
    if(( iptr = (GLvoid *) malloc( isize )) == NULL )
        return -1;
    
    fread( iptr, isize, 1, fp );
    fclose( fp );
    
    id++;
    glEnable( GL_TEXTURE_2D );
    glBindTexture(GL_TEXTURE_2D, id );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
#if 1
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#else
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
#endif
    glTexImage2D( GL_TEXTURE_2D, 0, 3, xsize, ysize, 0,
        GL_RGB, GL_UNSIGNED_BYTE, iptr );
    glDisable( GL_TEXTURE_2D );
    free(iptr);
    return id;
}

/*****************************************************************************\
@ read_plyfile()
-----------------------------------------------------------------------------
description : Load a PLY file
input       : Name of file to be loaded
output      : File has been loaded into the PlyModel structure
notes       : 
\*****************************************************************************/
void read_plyfile(char *filename, PlyModel *model)
{
    int i,j,k;
    PlyFile *ply;
    int nprops;
    int num_elems;
    PlyProperty **plist;
    char *elem_name;
    //float version;
    FILE *fp;
    
    /*** Read in the original PLY object ***/
    
    
    if (filename == NULL)
        fp = stdin;
    else
    {
        fp = fopen(filename, "rb");
        if (!fp)
        {
            fprintf(stderr, "read_plyfile(): "
                "Couldn't open file, %s, for reading\n",
                filename);
            exit(1);
        }
    }
    
    ply = ply_read(fp, &model->nelems, &model->elist);  
    
    if (ply == NULL)
    {
        fprintf(stderr, "Couldn't read ply file, %s\n", filename);
        exit(1);
    }
    
    model->has_vertex = model->has_face = model->has_normalmap = FALSE;
    
    for (i = 0; i < model->nelems; i++)
    {
        /* get the description of the first element */
        elem_name = model->elist[i];
        plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);
        
        if (equal_strings ("vertex", elem_name)) {
            
            model->has_vertex = TRUE;
            /* create a vertex list to hold all the vertices */
            ALLOCN(model->vlist, Vertex, num_elems);
            model->nverts = num_elems;
            
            /* set up for getting vertex elements */
            for (k=0; k< (int) MAX_VERT_PROPS; k++)
                model->vert_has[k] = FALSE;
            
            for (j=0; j<nprops; j++)
            {
                for (k=0; k< (int) MAX_VERT_PROPS; k++)
                {
                    if (equal_strings(vert_props[k].name, plist[j]->name))
                    {
                        ply_get_property (ply, elem_name, &vert_props[k]);
                        model->vert_has[k] = TRUE;
                    }
                }
            }
            model->vert_other =
                ply_get_other_properties (ply, elem_name,
                offsetof(Vertex,other_props));
            
            /* test for necessary properties */
            if ((!model->vert_has[0]) ||
                (!model->vert_has[1]) ||
                (!model->vert_has[2]))
            {
                fprintf(stderr, "Vertices don't have x, y, and z\n");
                exit(-1);
            }
            
            if (model->vert_has[3] &&
                model->vert_has[4] &&
                model->vert_has[5])
            {
                model->has_vertex_normals = TRUE;
            }
            else
            {
                model->has_vertex_normals = FALSE;
            }
            
            
            if (model->vert_has[6] && model->vert_has[7]) 
                model->has_texcoords = TRUE;
            else
                model->has_texcoords = FALSE;
            
            if (model->vert_has[9] && model->vert_has[10] && model->vert_has[11])
            {
                model->has_vertex_colors = TRUE;
            }
            else
                model->has_vertex_colors=FALSE;
            
            /* grab all the vertex elements */
            for (j = 0; j < num_elems; j++) {
                ply_get_element (ply, (void *) &(model->vlist[j]));
            }
            
            if (!model->has_vertex_colors)
            {
                for (j=0; j<model->nverts; j++)
                    RandomColor(model->vlist[j].color);
            }
        }
        else if (equal_strings ("face", elem_name)) {
            
            model->has_face = TRUE;
            
            /* create a list to hold all the face elements */
            ALLOCN(model->flist, Face, num_elems);
            model->nfaces = num_elems;
            
            /* set up for getting face elements */
            /* verify which properties these vertices have */
            for (k=0; k< (int) MAX_FACE_PROPS; k++)
                model->face_has[k] = FALSE;
            
            for (j=0; j<nprops; j++)
            {
                for (k=0; k< (int) MAX_FACE_PROPS; k++)
                {
                    if (equal_strings(face_props[k].name, plist[j]->name))
                    {
                        ply_get_property (ply, elem_name, &face_props[k]);
                        model->face_has[k] = TRUE;
                    }
                }
                
            }
            model->face_other =
                ply_get_other_properties (ply, elem_name,
                offsetof(Face,other_props));
            
            /* test for necessary properties */
            if (!model->face_has[0])
            {
                fprintf(stderr, "Faces don't have vertex indices\n");
                exit(-1);
            }
            
            model->has_patches = model->face_has[1];
            
            /* grab all the face elements */
            for (j = 0; j < num_elems; j++) {
                ply_get_element (ply, (void *) &(model->flist[j]));
#ifndef POLYGON_SUPPORT 
                if (model->flist[j].nverts != 3)
                {
                    fprintf(stderr, "Non-triangle faces not supported yet.\n");
                    exit(1);
                }
#endif
            }
            
            ComputeFaceNormals(model);
            
            for (j=0; j<model->nfaces; j++)
                RandomColor(model->flist[j].color);
            
            // produce patch 0
            MakePatches(model);
        }
        else
            model->other_elements = ply_get_other_element (ply, elem_name, num_elems);
        for (int i=0; i<nprops; i++) {
            ply_free_property(plist[i]);
            free(plist[i]);
        }
        free(plist);
        plist = NULL;
  }
  
  model->comments = ply_get_comments (ply, &model->num_comments);
  model->obj_info = ply_get_obj_info (ply, &model->num_obj_info);
  
  ply_close (ply);
  
  //delete ply;
  ply = NULL;
  
  if ((!model->has_vertex) || (!model->has_face))
  {
      fprintf(stderr, "PLY object must have vertex and face elements\n");
      exit(1);
  }
  
  model->solid_color.r = DEFAULT_COLOR.r;
  model->solid_color.g = DEFAULT_COLOR.g;
  model->solid_color.b = DEFAULT_COLOR.b;
  
  model->varray = NULL;
  //  model->has_vertex_colors = 0;
  
  return;
} /** End of read_plyfile() **/


  /**
  * SetupVertexArray: construct array of vertices from vlist using vertex array's
  *                   convention.
  * @author  Budirijanto Purnomo
  * @date  08/15/2002
  * @param  model   a data structure contains a model
  */
  void SetupVertexArray(PlyModel *model, int mode)
  {
      int p,i,j; 
      VertexArray* varray;
      Vertex* vert;
      
      // build the global vertex array if necessary
      if(mode == VERTEX_ARRAY_ELEMENTS) {
          if(model->varray != NULL) { //
              printf("Cannot call SetupVertexArray twice yet!\n");
              exit(0);
          }
          
          // build global varray
          model->vamode = mode;
          fprintf(stderr, "Allocating %6.2fMB system memory for vertex arrays\n",
              (model->nverts*sizeof(VertexArray))/1048576.0);
          ALLOCN(model->varray, VertexArray, model->nverts);
          
          for (i=0; i<model->nverts; i++) {
              vert = &(model->vlist[i]);
              varray = &(model->varray[i]);
              
              for (j=0; j<3; j++) {
                  varray->coord[j] = vert->coord[j];
                  varray->normal[j] = vert->normal[j];
                  varray->color[j] = (float)(vert->color[j])/255;
              }
              
              // have 3 coords for texcoord so we can use it for colors (visualization).
              varray->texcoord[0] = vert->texcoord[0];
              varray->texcoord[1] = vert->texcoord[1];
              //varray->texcoord[2] = 0.0;
          }
          
          // build patch indices :: already built by BuildPatches
          
      } else { // build per-patch arrays
          int bytes =0;
          for(p = 0; p < model->npatches; p++) {
              Patch *patch = &model->plist[p];
              
              // build patch->varray
              bytes += sizeof(VertexArray) * patch->nindices;
              
              ALLOCN(patch->varray,VertexArray,(patch->nindices * 3));
              
              for(i = 0; i < patch->nindices; i++) {
                  varray = &patch->varray[i];
                  vert = &model->vlist[patch->indices[i]];
                  for (j=0; j<3; j++) {
                      varray->coord[j] = vert->coord[j];
                      varray->normal[j] = vert->normal[j];
                      varray->color[j] = (float)(vert->color[j])/255;
                  }
                  // have 3 coords for texcoord so we can use it for colors (visualization).
                  varray->texcoord[0] = vert->texcoord[0];
                  varray->texcoord[1] = vert->texcoord[1];
                  //varray->texcoord[2] = 0.0;
              }
          }
      }
  }
  
  void BindVertexArray(PlyModel* model, int patch) {
      if(model->vamode == VERTEX_ARRAY_ELEMENTS) {
          glEnableClientState(GL_VERTEX_ARRAY);
          glVertexPointer(3, GL_FLOAT, sizeof(VertexArray), model->varray);
          
          if (model->has_vertex_normals) {
              glEnableClientState(GL_NORMAL_ARRAY);
              glNormalPointer(GL_FLOAT, sizeof(VertexArray), model->varray->normal);
          } else {
              glDisableClientState(GL_NORMAL_ARRAY);
          }
          
          if (model->has_texcoords) {
              glEnableClientState(GL_TEXTURE_COORD_ARRAY);
              glTexCoordPointer(2, GL_FLOAT, sizeof(VertexArray), model->varray->texcoord);
          } else {
              glDisableClientState(GL_TEXTURE_COORD_ARRAY);
          }
          if (model->has_vertex_colors) {
              glEnableClientState(GL_COLOR_ARRAY);
              glColorPointer(3, GL_FLOAT, sizeof(VertexArray), model->varray->color);
          } else {
              glDisableClientState(GL_COLOR_ARRAY);
          }
          
      } else { // VERTEX_ARRAY_ARRAYS
          glEnableClientState(GL_VERTEX_ARRAY);
          glVertexPointer(3, GL_FLOAT, sizeof(VertexArray), model->plist[patch].varray);
          
          if (model->has_vertex_normals) {
              glEnableClientState(GL_NORMAL_ARRAY);
              glNormalPointer(GL_FLOAT, sizeof(VertexArray), model->plist[patch].varray->normal);
          } else {
              glDisableClientState(GL_NORMAL_ARRAY);
          }
          
          if (model->has_texcoords) {
              glEnableClientState(GL_TEXTURE_COORD_ARRAY);
              glTexCoordPointer(2, GL_FLOAT, sizeof(VertexArray), model->plist[patch].varray->texcoord);
          } else {
              glDisableClientState(GL_TEXTURE_COORD_ARRAY);
          }
          
          if (model->has_vertex_colors) {
              glEnableClientState(GL_COLOR_ARRAY);
              glColorPointer(3, GL_FLOAT, sizeof(VertexArray), model->plist[patch].varray->color);
          } else {
              glDisableClientState(GL_COLOR_ARRAY);
          }
          
      }
  }
  
  /**
  * DrawModel: Display model using vertex array.
  * @author  Budirijanto Purnomo
  * @date  08/15/2002
  * @param  model  a data structure contains a model
  */
  void DrawModelVA(PlyModel *model,int patch)
  {
#if 0
      
      if(no_colors)
          glColor3ubv(patch->random_color);
      else
          glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(VertexArray), model->varray[0].random_color);
#endif      
      
      // element draw mode
      if(model->vamode == VERTEX_ARRAY_ELEMENTS)
          glDrawElements(GL_TRIANGLES, model->plist[patch].nindices, GL_UNSIGNED_INT, model->plist[patch].indices);
      else 
          glDrawArrays(GL_TRIANGLES, 0, model->plist[patch].nindices);
  }
  
  void SetupTexture(PlyModel *model, char* texture_name)
  {
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // default is 4
      glPixelStorei(GL_PACK_ALIGNMENT, 1);    // default is 4
      
      if (model->has_texcoords)
              {
          if(texture_name == NULL) 
              model->texture_name = DEFAULT_TEXTURE_NAME;
          else
              model->texture_name = texture_name;
          model->texture_id = LoadPPMTexture(model->texture_name);
          
          if (model->texture_id == -1)
          {
              model->has_texture = FALSE;
              fprintf(stderr, "Failed to load texture, %s.\n", model->texture_name);
          }
          else {
              printf("Loaded %s as texture file.\n", model->texture_name);
              model->has_texture = TRUE;
          }
      }
      else
      {
          model->texture_name = NULL;
          model->texture_id = -1;
          model->has_texture = FALSE;
      }
      
      return;
  }
  
  /**
  * InitRegisterCombiners:  From texture color, combine with light to reconstruct the
  *                         normal vector for each patch
  * @author  Budirijanto Purnomo
  * @date  08/15/2002
  */
  void InitRegisterCombiners() 
  {	
#ifdef USE_NV
      glEnable(GL_REGISTER_COMBINERS_NV);
      
      glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
      
      //GLfloat light[] = { 1.0/sqrt(3.0), 1.0/sqrt(3), 1.0/sqrt(3.0) };
      GLfloat light[] = {0.0,.5, 0.5 };
      
      glCombinerParameterfvNV(GL_CONSTANT_COLOR1_NV, light);
      glCombinerInputNV(GL_COMBINER0_NV, 
          GL_RGB, 
          GL_VARIABLE_A_NV, 
          GL_TEXTURE0_ARB,
          GL_EXPAND_NORMAL_NV, 
          GL_RGB);
      glCombinerInputNV(GL_COMBINER0_NV,
          GL_RGB,
          GL_VARIABLE_B_NV,
          GL_CONSTANT_COLOR1_NV,
          GL_EXPAND_NORMAL_NV, 
          GL_RGB);
      glCombinerOutputNV(GL_COMBINER0_NV,
          GL_RGB,
          GL_SPARE0_NV,
          GL_DISCARD_NV,
          GL_DISCARD_NV,
          GL_NONE,
          GL_NONE,
          GL_TRUE,
          GL_FALSE,
          GL_FALSE);
      
      GLfloat diffuse[] = { 1.0, 1.0, 1.0 };
      glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, 
          diffuse);
      glFinalCombinerInputNV(GL_VARIABLE_A_NV, 
          GL_SPARE0_NV,
          GL_UNSIGNED_IDENTITY_NV,
          GL_RGB);
      glFinalCombinerInputNV(GL_VARIABLE_B_NV, 
          GL_CONSTANT_COLOR0_NV,
          GL_UNSIGNED_IDENTITY_NV,
          GL_RGB);
      glFinalCombinerInputNV(GL_VARIABLE_C_NV, 
          GL_ZERO, 
          GL_UNSIGNED_IDENTITY_NV, 
          GL_RGB);
      glFinalCombinerInputNV(GL_VARIABLE_D_NV, 
          GL_ZERO, 
          GL_UNSIGNED_IDENTITY_NV, 
          GL_RGB);
#endif
  }
  
  
  
  /**
  * DrawModelImmediate: Display model using immediate mode (non-vertex array).
  *                     This is a fallback for some of the rendering modes that haven't
  *                     been implemented with vertex array.  
  *                     In future, we don't need this.
  * @author  Jonathan Cohen (modified by Budirijanto Purnomo)
  * @param  model   the data structure contains a model
  */
  void DrawModelImmediate(PlyModel *model)
  {
      int i;
      Face *face;
      
      
      glBegin(GL_TRIANGLES);
      for (i=0; i<model->nfaces; i++)
      {
          face = &(model->flist[i]);
          
          //glNormal3fv(face->normal);
          
          //glColor3ubv(model->plist[face->patch_num].random_color);
          
          if (model->has_vertex_normals)
              glNormal3fv((GLfloat*)&model->vlist[face->verts[0]].normal);
          if (model->has_vertex_colors)
              glColor3ubv((GLubyte*)&model->vlist[face->verts[0]].color);
          if (model->has_texcoords)
              glTexCoord2fv((GLfloat*)&model->vlist[face->verts[0]].texcoord);
          
          glVertex3fv((GLfloat*)&model->vlist[face->verts[0]].coord);
          
          if (model->has_vertex_normals)
              glNormal3fv((GLfloat*)&model->vlist[face->verts[0]].normal);
          if (model->has_vertex_colors)
              glColor3ubv((GLubyte*)&model->vlist[face->verts[0]].color);
          if (model->has_texcoords)
              glTexCoord2fv((GLfloat*)&model->vlist[face->verts[0]].texcoord);
          
          glVertex3fv((GLfloat*)&model->vlist[face->verts[1]].coord);
          
          if (model->has_vertex_normals)
              glNormal3fv((GLfloat*)&model->vlist[face->verts[0]].normal);
          if (model->has_vertex_colors)
              glColor3ubv((GLubyte*)&model->vlist[face->verts[0]].color);
          if (model->has_texcoords)
              glTexCoord2fv((GLfloat*)&model->vlist[face->verts[0]].texcoord);
          
          glVertex3fv((GLfloat*)&model->vlist[face->verts[2]].coord);
      }
      glEnd();
      
      return;
  }
  
  
  void DrawSphere(Point center, double radius)
  {
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glTranslated(center[X], center[Y], center[Z]);
      glScaled(radius, radius, radius);
      glEnable(GL_NORMALIZE);
      glCallList(SPHERE_DLIST_ID);
      glDisable(GL_NORMALIZE);
      glPopMatrix();
      return;
  }
  
  void ComputeVertexNormals(PlyModel *model, int inv) {
      int i, j, k; float fi;
      Vertex* v[3]; Vertex* t;
      Face* f;
      float mag;
      
      float vecA[3];
      float vecB[3];
      float normal[3];
      
      if(inv)
          fi = -1.0;
      else
          fi = 1.0;
      
      
      
      // set the normals to be big and bad
      for(i = 0; i < model->nfaces; i++) {
          f = model->flist + i;
          for(j = 0; j < 3; j++)
              v[j] = model->vlist + f->verts[j];
          
          for(j = 0; j < 3; j++)    // v1 - v0
              vecA[j] = v[1]->coord[j] - v[0]->coord[j];
          for(j = 0; j < 3; j++)    // v2 - v0
              vecB[j] = v[2]->coord[j] - v[0]->coord[j];
          
          //printf("A: %f %f %f\n", vecA[0], vecA[1], vecA[2]);
          //printf("B: %f %f %f\n", vecB[0], vecB[1], vecB[2]);
          
          // vecA X vecB
          normal[0] = (vecA[1] * vecB[2]) - (vecB[1] * vecA[2]);
          normal[1] = (vecA[0] * vecB[2]) - (vecB[0] * vecA[2]);
          normal[2] = (vecA[0] * vecB[1]) - (vecB[0] * vecA[1]);
          //    printf("Normal: %f %f %f: \n", normal[0], normal[1], normal[2]);
          
          //printf(">  ");
          for(j = 0; j < 3; j++) { // for each vertex
              for(k = 0; k < 3; k++) { // for each coord
                  v[j]->normal[k] += normal[k];
                  //printf("%.2f, ", v[j].normal[k]);
              }
          }
          //printf("\n\n");
      }
      
      // normalize coords
      for(i = 0; i < model->nverts; i++) {
          t = model->vlist+i;
          
          mag = t->normal[0]*t->normal[0] + t->normal[1]*t->normal[1] + t->normal[2]*t->normal[2];
          mag = sqrt(mag);
          
          for(j = 0; j < 3; j++)
              t->normal[j] /= mag * fi;
          
          //printf("%i %f %f %f\n", i, t->normal[0], t->normal[1], t->normal[2]);
          //printf("\n");
      }
      
      model->has_vertex_normals = 1;
  }
  
  void InvertVertexNormals(PlyModel* model) {
      for(int i = 0; i < model->nverts; i++) {
          for(int j = 0; j < 3; j++)
              model->vlist[i].normal[j] *= -1;
      }
  }
  
  float CenterOnOrigin(PlyModel *model) { // returns diagonal
      Vertex* t;
      float vecMax[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
      float vecMin[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
      float vecOff[3];
      int i,j;
      // compute bounds
      for(i = 0; i < model->nverts; i++) {
          t = model->vlist+i;
          for(j = 0; j < 3; j++) {
              if(t->coord[j] > vecMax[j]) vecMax[j] = t->coord[j];
              if(t->coord[j] < vecMin[j]) vecMin[j] = t->coord[j];	
          }
      }
      
      // compute the offset
      for(i = 0; i < 3; i++)
          vecOff[i] = .5 * (vecMax[i] - vecMin[i]) + vecMin[i];
      
      for(i = 0; i < model->nverts; i++) {
          t = model->vlist+i;
          for(j = 0; j < 3; j++)
              t->coord[j] -= vecOff[j];
      }
      
      float bbox_diagonal =
          (vecMax[0]-vecMin[0])*(vecMax[0]-vecMin[0]) +
          (vecMax[1]-vecMin[1])*(vecMax[1]-vecMin[1]) +
          (vecMax[2]-vecMin[2])*(vecMax[2]-vecMin[2]);
      bbox_diagonal = sqrt(bbox_diagonal);
      
      // recompute bbox
      for(i = 0; i < 3; i++) {
          vecMax[i] -= vecOff[i];
          vecMin[i] -= vecOff[i];
      }
      
      // store bbox
      memcpy(model->max, vecMax, sizeof(vecMax));
      memcpy(model->min, vecMin, sizeof(vecMin));
      return bbox_diagonal;
  }
  
  
  void WriteVAElements(char* filename, int nverts, VertexArray* va, int nelems, int* elems) {
      
  }
  
  void DeleteModel(PlyModel *model) {
      int i;
      // free face list
      Face* flist  = model->flist;
      for(i = 0; i < model->nfaces; i++) {
          free(flist[i].verts);
          if(flist[i].other_props != NULL)
              free(flist[i].other_props);
      }
      free(model->flist);
      
      // free vert list
      Vertex* vlist = model->vlist;
      for(i = 0; i < model->nverts; i++) {
          if(vlist[i].other_props != NULL)
              free(vlist[i].other_props);
      }
      free(model->vlist);
      
      // free patch list
      if(model->npatches != 0) {
          for(i = 0; i < model->npatches; i++) {
              Patch* p = &model->plist[i];
              free(p->indices);
              if(p->varray != NULL)
                  free(p->varray);
          }
          free(model->plist);
      }
      
      // free global vertex array.... 
      if(model->varray != NULL)
          free(model->varray);
      
      // now free ply attributes...
      if(model->face_other != NULL) {
          ply_free_other_property(model->face_other);
          free(model->face_other);
      }
      
      if(model->vert_other != NULL) {
          ply_free_other_property(model->vert_other);
          free(model->vert_other);
      }
      
      
      if(model->elist != NULL) {
          for(int i = 0; i < model->nelems; i++)
              free(model->elist[i]);
          free(model->elist);
      }
  }
/*****************************************************************************\
  PlyModel.cpp
  --
  Description : Code to display a ply object. Allows basic controls such
  as changing color and normal rendering modes, manipulation of the camera,
  etc.

  $Log: PlyModel.cpp,v $
  Revision 1.22  2004/07/19 19:27:48  gfx_friends
  Modified error handling for texturing in simple app. Now warns when it can't
  load a texture and does not try to bind non-existant textures (which could
  cause a crash from NVIDIA driver on exit() )

  Revision 1.21  2004/07/16 16:57:54  gfx_friends
  When using half-edge collapses, DiscreteHierarchy now stores only one vertex array for eah patch array instead of one for every patch on every per level. --Nat

  Revision 1.20  2004/07/14 20:20:02  gfx_friends
  Made simple load a texture and use it if the input has uv coordinates.

  Revision 1.19  2004/07/09 20:51:52  gfx_friends
  Tabification changes.

  Revision 1.18  2004/07/09 20:48:06  gfx_friends
  Forgot to checkin the top-level include and sample changes.

  Revision 1.17  2004/07/09 00:47:07  gfx_friends
  Memory leak fixes a plenty. --nat

  Revision 1.16  2004/07/08 16:16:02  gfx_friends
  many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)

  Revision 1.15  2004/07/06 17:57:15  gfx_friends
  Deleteting a malloc'd pointer changed to a free call. --n

  Revision 1.14  2004/07/06 16:25:20  jdt6a
  finally found the bug.  turns out it wasn't a GL problem.  we were uploading the wrong modelview matrix because of an incorrect bounding box for the model because of this typo in PlyModel.cpp.

  works in VS.NET now, with some minor vds changes that i'll commit now.

  Revision 1.13  2004/07/02 20:08:46  gfx_friends
  removed the delete ply

  Revision 1.12  2004/07/01 17:47:27  gfx_friends
  even more leak fixes... looks pretty good now, although there are still small memory leaks in ply_read (around 50bytes)

  Revision 1.11  2004/07/01 03:57:21  gfx_friends
  memory leak fix when loading textures

  Revision 1.10  2004/06/11 19:05:46  gfx_friends
  Got Win32-debug to work after moving the directory structure around.

  Revision 1.9  2004/06/07 16:58:07  gfx_friends
  Remove our dependancy on glext.h in windows (should compile without it now)

  Revision 1.8  2004/06/02 17:14:00  gfx_friends
  Changes to #includes so it works on a stock osx configuration

  Revision 1.7  2004/02/05 20:33:27  gfx_friends
  Cleanliness and order patches.

  Revision 1.6  2004/01/17 16:53:25  gfx_friends
  vertex color values from ply files are now properly read into the vertex arrays
  and are therefore rendered when the sample apps are run

  Revision 1.5  2003/07/26 01:17:34  gfx_friends
  Fixed copyright notice. Added wireframe to sample apps. Minor
  revisions to documentation.

  Revision 1.4  2003/07/23 00:24:46  gfx_friends
  A bunch of usability patches.

  Revision 1.3  2003/07/22 21:35:21  gfx_friends
  Removed outdated debugging call that was now causing crashes under Windows.

  Revision 1.2  2003/07/21 23:10:52  gfx_friends
  Added cut readback support. I'm still debugging, but need to move computers to my home. --n

  Revision 1.1  2003/07/21 03:08:00  gfx_friends
  Trying to break the dependencies between the samples directory and the top level directory so that I can distribute /samples without the whole source tree. -n

  Revision 1.1  2003/07/02 16:47:41  gfx_friends
  Moved samples/readback/PlyModel.cpp and nat_* to the ply lib and include directories to clean up the directory structures.

  Revision 1.4  2003/07/01 20:49:15  gfx_friends
  Readback for Discrete LOD now works. See samples/readback/read_model for demo and test. This, parenthetically, is also the most fully functional test program that we have. I will bring "simple" up to speed eventually, but in the meantime, you can use read_model similarly (run --help) in much the same way as previous ones to do testing.

  Revision 1.3  2003/06/30 20:37:06  gfx_friends
  Beginnings of a readback system. -- n

  Revision 1.2  2003/06/30 19:27:38  gfx_friends
  Minor bugs in DrawElements -nat

  Revision 1.1  2003/06/26 18:41:48  gfx_friends
  Major rewrite of vertex-array interfaces on the applicationi side to allow us to more thoroughly test the GLOD interface. PlyModel.cpp and PlyModel.h appear now in both scene and readback; however, the copy in scene is now obsolete. -- nat

  Revision 1.6  2003/06/16 20:51:10  gfx_friends
  Win32 patches to Scene

  Revision 1.5  2003/06/16 19:59:59  gfx_friends
  Removed GLVU from scene test program. -- n

  Revision 1.4  2003/01/20 06:05:34  gfx_friends
  Fixed up the rendering modes.

  Revision 1.3  2003/01/20 04:13:27  gfx_friends
  New version of code.

  Revision 1.2  2003/01/19 22:30:22  gfx_friends
  Patches.


  Modified by Budirijanto Purnomo, June 6th 2002 (writing to qslim format)
  June 27th, add polygon support (instead of just triangles)
  August 15th, Budi:  added support for normal map.
  August 17th, Budi:  added vertex arrays (and vertex array range for
                       windows)
\*****************************************************************************/
