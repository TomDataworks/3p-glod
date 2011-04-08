/* GLOD: Convert from Any Vertex Array into One Indexed Array
 ***************************************************************************
 * $Id: RawConvert.cpp,v 1.32 2005/03/12 19:35:16 gfx_friends Exp $
 * $Revision: 1.32 $
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
#include <stdio.h>
#include <stdlib.h>

#include "glod_core.h"
#include "Hierarchy.h"
#include "hash.h"

#include <memory.h>
#include <assert.h>

typedef struct VaState {
  void* va; GLint va_size; GLenum va_type; GLsizei va_stride;
  void* na; GLenum na_type; GLsizei na_stride;
  void* ta; GLint ta_size; GLenum ta_type; GLsizei ta_stride;
  void* ca; GLint ca_size; GLenum ca_type; GLsizei ca_stride;
  void* ia; GLenum ia_type;
  int first;
} VaState;

typedef struct _VaPack {
  GLOD_RawPatch* p;
  float* va;
  float* na;
  float* ta;
  float* ca;
  GLint* ia;
} VaPack;



void GetTriangle(VaState* vas, int mode, int tri, int* dst);
void SetTriangle(VaState* vas, int mode, int tri, int* src);

int GetV(VaState* vas, int mode, float* dst, int vert);
int GetN(VaState* vas, int mode, float* dst, int vert);
int GetT(VaState* vas, int mode, float* dst, int vert);
int GetC(VaState* vas, int mode, float* dst, int vert);

int SetV(VaState* vas, int mode, float* src, int vert);
int SetN(VaState* vas, int mode, float* src, int vert);
int SetT(VaState* vas, int mode, float* src, int vert);
int SetC(VaState* vas, int mode, float* src, int vert);

GLfloat fixType(GLfloat input, int type);
GLfloat fixSetType(GLfloat input, int type);
GLfloat GetFloatAtOffset(char* base, int i, int type);
GLint GetIntAtOffset(char* base, int i, int type);
void SetValAtOffsetf(char* base, int i, int type, float f);
void SetValAtOffseti(char* base, int i, int type, int val);

static int TypeSize(int type);
void GetVAState(VaState* vas);

int GetStrideSize(int numElements, int type);

int PredictSizes(VaState* va, GLuint mode, int count, GLvoid* indices, int *num_triangles, int *num_vertices, int *num_elems);

/***** PRODUCE PATCH ---> goes from current VA state to a GLOD_RawPatch ******/
GLOD_RawPatch* ProducePatch(GLenum mode, 
			GLenum first, GLenum count, 
			void* indices, GLenum indices_type) {
  GLOD_RawPatch* p = new GLOD_RawPatch();

  VaState vas;
  int num_triangles; int num_vertices;
  int num_elems;
  
  GetVAState(&vas);
  vas.ia = indices;
  vas.ia_type = indices_type;
  vas.first = first;
  
  if(!(mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP ||
       mode == GL_QUADS || mode == GL_QUAD_STRIP)) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Unsupported data format for Vertex Array input", mode);
    return NULL;
  }
  
  
  // init
  memset(p, 0, sizeof(GLOD_RawPatch));
  if(vas.va == NULL) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "glGetPointerv(vp) == NULL. Cannot continue!");
    return NULL;
  }
  // verify
  if(vas.va_size != 3) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Not a 3-coord vertex array!");
    return NULL;
  }
  
  if(vas.ca && vas.ca_size == 4) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Only tri-color RGB is supported.");
    return NULL;
  }

  if(vas.ta && vas.ta_size != 2) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Only 2-D texcoords are supported.");
    return NULL;
  }

  // predict sizes
  if(PredictSizes(&vas, mode, count, indices, &num_triangles, &num_vertices, &num_elems) == 0)
    return NULL;

  // alloc & set flags
  {
    if(vas.na != NULL) {
      p->data_flags |= GLOD_HAS_VERTEX_NORMALS;
      p->vertex_normals = (GLfloat*) malloc(sizeof(GLfloat) * num_vertices * 3);
    }
    
    if(vas.ta != NULL) {
      p->data_flags |= GLOD_HAS_TEXTURE_COORDS_2;
      p->vertex_texture_coords = (GLfloat*) malloc(sizeof(GLfloat) * num_vertices * vas.ta_size);
    }
    
    if(vas.ca != NULL) {
      p->vertex_colors = (GLfloat*) malloc(sizeof(GLfloat) * num_vertices * vas.ca_size);
      p->data_flags |= GLOD_HAS_VERTEX_COLORS_3;
    }
  }

  // alloc tri & vertices
  p->vertices = (GLfloat*) malloc(sizeof(GLfloat) * num_vertices * 3);
  p->triangles = (GLint*) malloc(sizeof(GLint) * num_triangles * 3);
  
  p->num_vertices = num_vertices;
  p->num_triangles = num_triangles;

  // convert coords
  int vcount = 0;
  VaPack vap;
  vap.ia = p->triangles;
  vap.va = p->vertices; vap.na = p->vertex_normals; vap.ca = p->vertex_colors; vap.ta = p->vertex_texture_coords;
  int i=0; float buf[4]; int ibuf[3]; int j = 0;
  
  {
    // what we have to do is figure out whether 
    HashTable* index_hash = AllocHashtable();
    int cur_idx = 0; int a;
    // warning: keys in hashtable are +1 of their true index
    for(i = 0; i < num_triangles; i++) { // foreach triangle
      GetTriangle(&vas,mode,i,ibuf);
      
      for(j = 0; j < 3; j++) { // foreach vertex in triangle
	
	int tmp = HashtableSearchInt(index_hash, ibuf[j]+1);
	int index = tmp - 1; // we store things in the hashtable +1
	
	if(tmp == 0) { // this vertex hasn't been seen before
	  
	  // copy this vertex over
	  a = GetV(&vas, mode, buf, ibuf[j]);
	  memcpy(vap.va, buf, sizeof(float) * 3); vap.va += a;
	  
	  if(vas.na) {
	    a = GetN(&vas, mode, buf, ibuf[j]);
	    memcpy(vap.na, buf, sizeof(float) * 3); vap.na += a;
	  }
	    
	  if(vas.ta) {
	    a = GetT(&vas, mode, buf, ibuf[j]);
	    memcpy(vap.ta, buf, sizeof(float) * 2); vap.ta += a;
	  }
	  
	  if(vas.ca) {
	    a = GetC(&vas, mode, buf, ibuf[j]);
	    memcpy(vap.ca, buf, sizeof(float) * 3); vap.ca += a;
	  }
	  
	  // put it into the hash --- asign a new index
	  index = cur_idx; cur_idx++;
	  HashtableAdd(index_hash, ibuf[j]+1,(void*) (ptrdiff_t) (index+1));
          // we have another vertex
          vcount++;
	}

	// point iA at  index
	*vap.ia = index; vap.ia ++;
      }
    }
    FreeHashtableCautious(index_hash);
    p->num_vertices = vcount;
  }

  return p;
}

/***** PRODUCE VA ---> goes from a GLOD_Cut to GLOD_RawPatch to the current VA state ******/
/**** WE ONLY PRODUCE GL_TRIANGLE ARRAYS */
int ProduceVA(GLOD_Cut* c, int patch, 
              void* indices, GLenum indices_type) {
  VaState vas;

  GetVAState(&vas);
  vas.ia = indices;
  vas.ia_type = indices_type;
  vas.first = 0;
  
  // verify the state of these vertex arrays...
  // we can only read at the moment:
  //    tri-color
  //    2d texture coords
  //    normals
  //    vertices
  if(vas.va == NULL) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "glGetPointerv(vp) == NULL. Cannot continue!");
    return 0;
  }
  if(vas.va_size != 3) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Not a 3-coord vertex array!");
    return 0;
  }
  
  if(vas.ca && vas.ca_size == 4) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Only tri-color RGB is supported.");
    return 0;
  }

  if(vas.ta && vas.ta_size != 2) {
    GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Only 2-D texcoords are supported.");
    return 0;
  }
  
  // what are the maximum sizes for the readback?
  GLuint cut_nindices, cut_nverts;
  c->getReadbackSizes(patch, &cut_nindices, &cut_nverts);
  
  /// create the temporary RawPatch
  GLOD_RawPatch* p = new GLOD_RawPatch(); p->name = patch;
  p->vertices = (GLfloat*)malloc(3 * sizeof(float) * cut_nverts);
  if(vas.ca) {
    p->data_flags |= GLOD_HAS_VERTEX_COLORS_3;
    p->vertex_colors = (GLfloat*)malloc(3 * sizeof(float) * cut_nverts);
  }
  if(vas.na) {
    p->data_flags |= GLOD_HAS_VERTEX_NORMALS;
    p->vertex_normals = (GLfloat*)malloc(3 * sizeof(float) * cut_nverts);
  }
  if(vas.ta) {
    p->data_flags |= GLOD_HAS_TEXTURE_COORDS_2;
    p->vertex_texture_coords = (GLfloat*)malloc(2 * sizeof(float) * cut_nverts);
  }
  p->triangles = (GLint*) malloc(sizeof(int) * cut_nindices);
  p->num_triangles = cut_nindices / 3;
  p->num_vertices = cut_nverts;
  
  // convert cut.patch[patch] into this rawobject
  c->readback(patch, p);
  
  // convert the RawObject into the user's specified vertex array format...
  int i;
  
  if(indices) {
    // vertices first
    for(i = 0; i < cut_nverts; i++) {
      SetV(&vas, GL_TRIANGLES, p->vertices + 3*i, i);
      if(vas.na)
        SetN(&vas, GL_TRIANGLES, p->vertex_normals + 3*i, i);    
      if(vas.ca)
        SetC(&vas, GL_TRIANGLES, p->vertex_colors + 3*i, i);
      if(vas.ta)
        SetT(&vas, GL_TRIANGLES, p->vertex_texture_coords + 2 * i, i);
    }
    
    // now the index array if present 
    for(i = 0; i < cut_nindices / 3; i++)
        SetTriangle(&vas, GL_TRIANGLES, i, (int*)(p->triangles + 3*i));
  } else {
    int real_index;
    for(i = 0; i < cut_nindices; i++) {
      real_index = p->triangles[i];

      SetV(&vas, GL_TRIANGLES, p->vertices + 3*real_index, i);
      if(vas.na)
        SetN(&vas, GL_TRIANGLES, p->vertex_normals + 3*real_index, i);    
      if(vas.ca)
        SetC(&vas, GL_TRIANGLES, p->vertex_colors + 3*real_index, i);
      if(vas.ta)
        SetT(&vas, GL_TRIANGLES, p->vertex_texture_coords + 2 * real_index, i);
    }
  }

  // free the raw object
//  free(p->vertices);
//  if(vas.ca)
//    free(p->vertex_colors);
//  if(vas.na)
//    free(p->vertex_normals);
//  if(vas.ta)
//    free(p->vertex_texture_coords);
//  free(p->triangles);
  delete(p);
  return 1;
}


// to get a tri in a array format, verts should be
//  {tri*3, tri*3+1, tri*3+2}
int GetV(VaState* vas, int mode, float* dst, int vert) {
  vert += vas->first;
  dst[0] = GetFloatAtOffset((char*)vas->va + vert*vas->va_stride, 0, vas->va_type);
  dst[1] = GetFloatAtOffset((char*)vas->va + vert*vas->va_stride, 1, vas->va_type);
  dst[2] = GetFloatAtOffset((char*)vas->va + vert*vas->va_stride, 2, vas->va_type);
  return 3;
}



int GetN(VaState* vas, int mode, float* dst, int vert) {
  vert += vas->first;
  dst[0] = GetFloatAtOffset((char*)vas->na + vert*vas->na_stride, 0, vas->va_type);
  dst[1] = GetFloatAtOffset((char*)vas->na + vert*vas->na_stride, 1, vas->va_type);
  dst[2] = GetFloatAtOffset((char*)vas->na + vert*vas->na_stride, 2, vas->va_type);
  dst[0] = fixType(dst[0], vas->na_type);
  dst[1] = fixType(dst[1], vas->na_type);
  dst[2] = fixType(dst[2], vas->na_type);
  return 3;
}



int GetT(VaState* vas, int mode, float* dst, int vert) {
  vert += vas->first;
  dst[0] = GetFloatAtOffset((char*)vas->ta + vert*vas->ta_stride, 0, vas->ta_type);
  dst[1] = GetFloatAtOffset((char*)vas->ta + vert*vas->ta_stride, 1, vas->ta_type);
  dst[0] = fixType(dst[0], vas->ta_type);
  dst[1] = fixType(dst[1], vas->ta_type);
  return 2;
}

int GetC(VaState* vas, int mode, float* dst, int vert) {
  vert += vas->first;
  dst[0] = GetFloatAtOffset((char*)vas->ca + vert*vas->ca_stride, 0, vas->ca_type);
  dst[1] = GetFloatAtOffset((char*)vas->ca + vert*vas->ca_stride, 1, vas->ca_type);
  dst[2] = GetFloatAtOffset((char*)vas->ca + vert*vas->ca_stride, 2, vas->ca_type);
  //printf("getC %f %f %f %x %x\n", dst[0], dst[1], dst[2], vas->ca_type, GL_FLOAT);
  dst[0] = fixType(dst[0], vas->ca_type);
  dst[1] = fixType(dst[1], vas->ca_type);
  dst[2] = fixType(dst[2], vas->ca_type);
  return 3;
}

void GetTriangle(VaState* vas, int mode, int tri, int* dst) {
  switch(mode) {
  case GL_TRIANGLES:
    if(vas->ia) {
      dst[0] = GetIntAtOffset((char*)vas->ia, vas->first+tri*3, vas->ia_type);
      dst[1] = GetIntAtOffset((char*)vas->ia, vas->first+tri*3+1, vas->ia_type);
      dst[2] = GetIntAtOffset((char*)vas->ia, vas->first+tri*3+2, vas->ia_type);
    } else {
      dst[0] = vas->first + tri*3;
      dst[1] = vas->first + tri*3+1;
      dst[2] = vas->first + tri*3+2;
    }
    return;
	
  default:
    assert(false);
  }
}

/****************************************************************************/
int SetV(VaState* vas, int mode, float* src, int vert) {
  vert += vas->first;
  SetValAtOffsetf((char*)vas->va + vert*vas->va_stride, 0, vas->va_type, src[0]);
  SetValAtOffsetf((char*)vas->va + 4 + vert*vas->va_stride, 1, vas->va_type, src[1]);
  SetValAtOffsetf((char*)vas->va + 8 + vert*vas->va_stride, 2, vas->va_type, src[2]);
  return 3;
}



int SetN(VaState* vas, int mode, float* src, int vert) {
  vert += vas->first;
    src[0] = fixSetType(src[0], vas->na_type);
    src[1] = fixSetType(src[1], vas->na_type);
    src[2] = fixSetType(src[2], vas->na_type);
  SetValAtOffsetf((char*)vas->na + vert*vas->na_stride, 0, vas->na_type, src[0]);
  SetValAtOffsetf((char*)vas->na + 4 + vert*vas->na_stride, 1, vas->na_type, src[1]);
  SetValAtOffsetf((char*)vas->na + 8 + vert*vas->na_stride, 2, vas->na_type, src[2]);
  return 3;
}



int SetT(VaState* vas, int mode, float* src, int vert) {
  vert += vas->first;
    src[0] = fixSetType(src[0], vas->ta_type);
    src[1] = fixSetType(src[1], vas->ta_type);
  SetValAtOffsetf((char*)vas->ta + vert*vas->ta_stride, 0, vas->ta_type, src[0]);
  SetValAtOffsetf((char*)vas->ta + 4 + vert*vas->ta_stride, 1, vas->ta_type, src[1]);
  return 2;
}

int SetC(VaState* vas, int mode, float* src, int vert) {
  vert += vas->first;
    src[0] = fixSetType(src[0], vas->ca_type);
    src[1] = fixSetType(src[1], vas->ca_type);
    src[2] = fixSetType(src[2], vas->ca_type);
    SetValAtOffsetf((char*)vas->ca + vert*vas->ca_stride, 0, vas->ca_type, src[0]);
    SetValAtOffsetf((char*)vas->ca + 4 + vert*vas->ca_stride, 1, vas->ca_type, src[1]);
    SetValAtOffsetf((char*)vas->ca + 8 + vert*vas->ca_stride, 2, vas->ca_type, src[2]);
  return 3;
}

void SetTriangle(VaState* vas, int mode, int tri, int* src) {
  switch(mode) {
  case GL_TRIANGLES:
    if(vas->ia) {
      SetValAtOffseti((char*)vas->ia, vas->first+tri*3,   vas->ia_type, src[0]);
      SetValAtOffseti((char*)vas->ia, vas->first+tri*3+1, vas->ia_type, src[1]);
      SetValAtOffseti((char*)vas->ia, vas->first+tri*3+2, vas->ia_type, src[2]);
    } else {
    }
    return;
	
  default:
    assert(false);
  }
}

GLfloat fixSetType(GLfloat input, int type){
    switch(type){
        case GL_BYTE:
            return input*127;
        case GL_UNSIGNED_BYTE:
            return input*255;
        case GL_SHORT:
            return input*32767;
        case GL_UNSIGNED_SHORT:
            return input*65535;
        case GL_INT:
            return input*2147483647;
        case GL_UNSIGNED_INT:
            return input*4294967295;
        case GL_FLOAT:
            return input;
        default:
            assert(false);
    }
    return 0;
}

GLfloat fixType(GLfloat input, int type){
    switch(type){
        case GL_BYTE:
            return input/127.0;
        case GL_UNSIGNED_BYTE:
            return input/255.0;
        case GL_SHORT:
            return input/32767.0;
        case GL_UNSIGNED_SHORT:
            return input/65535.0;
        case GL_INT:
            return input/2147483647.0;
        case GL_UNSIGNED_INT:
            return input/4294967295.0;
        case GL_FLOAT:
            return input;
        default:
            assert(false);
    }
    return 0;
}

/****************************************************************************/

GLfloat GetFloatAtOffset(char* base, int i, int type) {
  switch(type) {
  case GL_BYTE:
    return (GLfloat)*((GLbyte*)(((char*) base) + sizeof(GLbyte) * i));
  case GL_SHORT:
    return (GLfloat)*((GLshort*)(((char*) base) + sizeof(GLshort) * i));
  case GL_INT:
    return (GLfloat)*((GLint*)(((char*) base) + sizeof(GLint) * i));
  case GL_FLOAT:
    return (GLfloat)*((GLfloat*)(((char*) base) + sizeof(GLfloat) * i));
  case GL_DOUBLE:
    return (GLfloat)*((GLdouble*)(((char*) base) + sizeof(GLdouble) * i));
  case GL_UNSIGNED_BYTE:
    return (GLfloat)*((GLubyte*)(((char*) base) + sizeof(GLubyte) * i));
  case GL_UNSIGNED_SHORT:
    return (GLfloat)*((GLushort*)(((char*) base) + sizeof(GLushort) * i));
  case GL_UNSIGNED_INT:
    return (GLfloat)*((GLuint*)(((char*) base) + sizeof(GLuint) * i));
  default:
    assert(false);
  }
  return 0;
}

GLint GetIntAtOffset(char* base, int i, int type) {
  switch(type) {
  case GL_BYTE:
    return (GLint)*((GLbyte*)(((char*) base) + sizeof(GLbyte) * i));
  case GL_SHORT:
    return (GLint)*((GLshort*)(((char*) base) + sizeof(GLshort) * i));
  case GL_INT:
    return (GLint)*((GLint*)(((char*) base) + sizeof(GLint) * i));
  case GL_FLOAT:
    return (GLint)*((GLfloat*)(((char*) base) + sizeof(GLfloat) * i));
  case GL_DOUBLE:
    return (GLint)*((GLdouble*)(((char*) base) + sizeof(GLdouble) * i));
  case GL_UNSIGNED_BYTE:
    return (GLint)*((GLubyte*)(((char*) base) + sizeof(GLubyte) * i));
  case GL_UNSIGNED_SHORT:
    return (GLint)*((GLushort*)(((char*) base) + sizeof(GLushort) * i));
  case GL_UNSIGNED_INT:
    return (GLint)*((GLuint*)(((char*) base) + sizeof(GLuint) * i));
  default:
    assert(false);
  }
  return 0;
}

void SetValAtOffsetf(char* base, int i, int type, float f) {
  switch(type) {
  case GL_BYTE:
    *((GLbyte*)(((char*) base) /*+ sizeof(GLbyte) * i*/)) = (GLbyte) f; break;
  case GL_SHORT:
    *((GLshort*)(((char*) base) /*+ sizeof(GLshort) * i*/)) = (GLshort) f; break;
  case GL_INT:
    *((GLint*)(((char*) base) /*+ sizeof(GLint) * i*/)) = (GLint) f; break;
  case GL_FLOAT:
    *((GLfloat*)(((char*) base) /*+ sizeof(GLfloat) * i*/)) = (GLfloat) f; break;
  case GL_DOUBLE:
    *((GLdouble*)(((char*) base) /*+ sizeof(GLdouble) * i*/)) = (GLdouble) f; break;
  case GL_UNSIGNED_BYTE:
    *((GLubyte*)(((char*) base) /*+ sizeof(GLubyte) * i*/)) = (GLubyte) f; break;
  case GL_UNSIGNED_SHORT:
    *((GLushort*)(((char*) base)/* + sizeof(GLushort) * i*/)) = (GLushort) f; break;
  case GL_UNSIGNED_INT:
    *((GLuint*)(((char*) base)/* + sizeof(GLuint) * i*/))= (GLuint) f; break;
  default:
    assert(false);
  }
}

void SetValAtOffseti(char* base, int i, int type, int val) {
  switch(type) {
  case GL_BYTE:
    *((GLbyte*)(((char*) base) + sizeof(GLbyte) * i)) = (GLbyte) val; break;
  case GL_SHORT:
    *((GLshort*)(((char*) base) + sizeof(GLshort) * i)) = (GLshort) val; break;
  case GL_INT:
    *((GLint*)(((char*) base) + sizeof(GLint) * i)) = (GLint) val; break;
  case GL_FLOAT:
    *((GLfloat*)(((char*) base) + sizeof(GLfloat) * i)) = (GLfloat) val; break;
  case GL_DOUBLE:
    *((GLdouble*)(((char*) base) + sizeof(GLdouble) * i)) = (GLdouble) val; break;
  case GL_UNSIGNED_BYTE:
    *((GLubyte*)(((char*) base) + sizeof(GLubyte) * i)) = (GLubyte) val; break;
  case GL_UNSIGNED_SHORT:
    *((GLushort*)(((char*) base) + sizeof(GLushort) * i)) = (GLushort) val; break;
  case GL_UNSIGNED_INT:
    *((GLuint*)(((char*) base) + sizeof(GLuint) * i))= (GLuint) val; break;
  default:
    assert(false);
  }
}



void GetVAState(VaState* va) {
  memset(va, 0, sizeof(VaState));
  // figure out counts and values
  if(glIsEnabled(GL_VERTEX_ARRAY)) {
    glGetPointerv(GL_VERTEX_ARRAY_POINTER, &va->va);
    glGetIntegerv(GL_VERTEX_ARRAY_SIZE, &va->va_size);
    glGetIntegerv(GL_VERTEX_ARRAY_TYPE, (GLint*) &va->va_type);
    glGetIntegerv(GL_VERTEX_ARRAY_STRIDE, &va->va_stride);
    if (va->va_stride==0) va->va_stride=GetStrideSize(va->va_size, va->va_type);
  }

  if(glIsEnabled(GL_NORMAL_ARRAY)) {
    glGetPointerv(GL_NORMAL_ARRAY_POINTER, &va->na);
    glGetIntegerv(GL_NORMAL_ARRAY_TYPE, (GLint*) &va->na_type);    
    glGetIntegerv(GL_NORMAL_ARRAY_STRIDE, &va->na_stride);  
    if (va->na_stride==0) va->na_stride=GetStrideSize(3, va->na_type);;
  }

  if(glIsEnabled(GL_TEXTURE_COORD_ARRAY)) {
    glGetPointerv(GL_TEXTURE_COORD_ARRAY_POINTER, &va->ta);
    glGetIntegerv(GL_TEXTURE_COORD_ARRAY_SIZE, &va->ta_size);
    glGetIntegerv(GL_TEXTURE_COORD_ARRAY_TYPE, (GLint*) &va->ta_type);
    glGetIntegerv(GL_TEXTURE_COORD_ARRAY_STRIDE, &va->ta_stride);
    if (va->ta_stride==0) va->ta_stride=GetStrideSize(va->ta_size, va->ta_type);
  }

  if(glIsEnabled(GL_COLOR_ARRAY)) {
    glGetPointerv(GL_COLOR_ARRAY_POINTER, &va->ca);
    glGetIntegerv(GL_COLOR_ARRAY_SIZE, &va->ca_size);    
    glGetIntegerv(GL_COLOR_ARRAY_TYPE, (GLint*) &va->ca_type);
    glGetIntegerv(GL_COLOR_ARRAY_STRIDE, &va->ca_stride);
    if (va->ca_stride==0) va->ca_stride=GetStrideSize(va->ca_size, va->ca_type);
  }
}

int GetStrideSize(int numElements, int type){
    switch (type){
	case GL_BYTE:
	    return numElements*sizeof(GLbyte);
	case GL_SHORT:
	    return numElements*sizeof(GLshort);
	case GL_INT:
	    return numElements*sizeof(GLint);
	case GL_FLOAT:
	    return numElements*sizeof(GLfloat);
	case GL_DOUBLE:
	    return numElements*sizeof(GLdouble);
	case GL_UNSIGNED_BYTE:
	    return numElements*sizeof(GLubyte);
	case GL_UNSIGNED_SHORT:
	    return numElements*sizeof(GLushort);
	case GL_UNSIGNED_INT:
	    return numElements*sizeof(GLuint);
	default:
	    assert(false);
    }
	return 0;
}

int PredictSizes(VaState* va, GLuint mode, int count, GLvoid* indices, int *num_triangles, int *num_vertices, int *num_elems) {
  switch(mode) {
  case GL_TRIANGLES:
    if(count < 3) {
      GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Too few vertices in tris while importing VA!\n");
      return 0;
    }

    *num_triangles = count / 3;
    *num_vertices  = count;
    *num_elems     = count / 3; // num_triangles
    break;
  case GL_TRIANGLE_STRIP:
    if(count < 3) {
      GLOD_SetError(GLOD_INVALID_DATA_FORMAT, "Too few vertices in Tri_Strip while importing VA!");
      return 0;
    }

    *num_triangles = count - 2;
    *num_elems = *num_triangles;
    *num_vertices = count;
    break;
  default:
    assert(false);
  }
  return 1;
}
 
static int TypeSize(int type) {
  switch(type) {
  case GL_BYTE:
    return sizeof(GLbyte);
  case GL_SHORT:
    return sizeof(GLshort);
  case GL_INT:
    return sizeof(GLint);
  case GL_FLOAT:
    return sizeof(GLfloat);
  case GL_DOUBLE:
    return sizeof(GLdouble);
  case GL_UNSIGNED_BYTE:
    return sizeof(GLubyte);
  case GL_UNSIGNED_SHORT:
    return sizeof(GLushort);
  case GL_UNSIGNED_INT:
    return sizeof(GLushort);
  default:
    assert(false);
  }
}

/***************************************************************************
 * $Log: RawConvert.cpp,v $
 * Revision 1.32  2005/03/12 19:35:16  gfx_friends
 * more or less final fix for the color readback problems. Essentially we were unable to read back anything other then float/ints from any field (tex, coord, etc) because we only allocated a float readback buffer, and then tried to write stuff as other data types to it, putting them in wrong places in the buffer. This should work now
 *
 * Revision 1.31  2005/02/23 20:38:10  gfx_friends
 * added the conversion from float to byte/short/int when reading back glod's vertex arrays
 *
 * Revision 1.30  2005/02/23 19:11:17  gfx_friends
 * Fixed problems with non-float arrays being passed into GLOD. The cause of the problem was not scaling the byte/short/int values to float number range, 0...1 for colors, normals, etc.
 *
 * Revision 1.29  2004/07/19 19:28:01  gfx_friends
 * Warning fixes in rawpatch.
 *
 * Revision 1.28  2004/07/19 19:18:41  gfx_friends
 * Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
 *
 * Revision 1.27  2004/07/16 16:57:53  gfx_friends
 * When using half-edge collapses, DiscreteHierarchy now stores only one vertex array for eah patch array instead of one for every patch on every per level. --Nat
 *
 * Revision 1.26  2004/07/08 16:15:52  gfx_friends
 * many changes to remove warnings during compilation, and allow it to compile using gcc3.5 (on osx anyway)
 *
 * Revision 1.25  2004/06/11 19:05:46  gfx_friends
 * Got Win32-debug to work after moving the directory structure around.
 *
 * Revision 1.24  2004/06/01 19:01:34  gfx_friends
 * Rich Holloways fix for multiple deletes
 *
 * Revision 1.23  2004/05/26 16:51:09  gfx_friends
 * Changed the slightly hacky fix for the windows stride problem to a more general one, should take into account the type of data and the number of elements
 *
 * Revision 1.22  2004/05/25 23:53:53  gfx_friends
 * Fix for windows and 0 stride vertex arrays. Apparently gl in windows return 0 for stride when the arrays are tightly packed, rather then the actual size between each vertex as unix/linux/osx do
 *
 * Revision 1.21  2004/02/04 07:21:03  gfx_friends
 * Huuuuge cleanup. I moved parameters out of the glod_objects and glod_groups code into new files in the api/. Same goes for vertex array [in and out] which go into a new file. I modified xbssimplifier to take a hierarchy directly instead of a enum to the hierarchy because glod can decide better how to create a hierarchy than xbs can. Most importantly, I cleaned up the build object process so that now discrete manual mode is implemented entirely with a custom DiscreteHierarchy::initialize(RawObject*) routine... which I haven't implemented. Also, I renamed DiscreteObject to DiscreteLevel, since calling it a DiscreteObject is a huge misnomer that is easily confused with GLOD_Object. -- Nat
 *
 * Revision 1.20  2003/07/26 01:17:14  gfx_friends
 * Fixed copyright notice. Added wireframe to sample apps. Minor
 * revisions to documentation.
 *
 * Revision 1.19  2003/07/23 19:55:26  gfx_friends
 * Added copyright notices to GLOD. I'm making a release.
 *
 * Revision 1.18  2003/07/23 00:24:46  gfx_friends
 * A bunch of usability patches.
 *
 * Revision 1.17  2003/07/22 19:52:05  gfx_friends
 * Patched a bug in the glodFillArrays code and updated the documentation correspondingly. --n
 *
 * Revision 1.16  2003/07/22 18:32:11  gfx_friends
 * Fixed the windows build. We've got a big bad windows bug that keeps the simplifier from working right now, but hopefully that'll be patched soon. -nat
 *
 * Revision 1.15  2003/07/22 03:28:29  gfx_friends
 * Fixed the Scene tool. Mostly. I need to do some more stuff, but its back to comipling. glodAdapt jams! --nat
 *
 * Revision 1.14  2003/07/21 23:10:50  gfx_friends
 * Added cut readback support. I'm still debugging, but need to move computers to my home. --n
 *
 * Revision 1.13  2003/07/18 22:19:34  gfx_friends
 * Fixed most of the build problems. The lights have mysteriously gone off in the simple test program... I'm trying to figure out why. But the rest works, I think
 *
 * Revision 1.12  2003/07/15 20:18:53  gfx_friends
 * Major documentation effort and basic distribution readiness. We now have pod-based documentation for each GLOD function. It will build HTML or Man pages on Linux. To use the man pages, append glod/doc/man to your manpath after running make in doc or doing a top-level make. Also new is a release target... a top level make release builds with -O2 and any flags you also set based on the release target (See glod.conf). Also, #define DEBUG is active when building for debug.
 *
 * Revision 1.11  2003/06/30 19:29:56  gfx_friends
 * (1) InsertElements now works completely. --nat
 * (2) Some XBS classes got moved from xbs.h into discrete.h and Hierarchy.h for
 *     cleanliness.
 *
 * Revision 1.10  2003/06/27 04:33:47  gfx_friends
 * We now have a functioning glodInsertElements. There is a bug in ModelShare.c that infiniteloops. I'm chasing that. -- n
 *
 * Revision 1.9  2003/06/26 18:52:57  gfx_friends
 * Major rewrite of GLOD-side Vertex Array handling routines (the so-called masseusse) to allow more robust inputs. Let me tell you, the VA interface is really pretty when you're using it, but using the data in a coherent way is a nightmare because of all of the different options you have as a user. This will allow me to implement the readback interface faster... in theory, although that is going to be an equal nightmare. -- nat
 *
 * Revision 1.8  2003/06/09 19:16:58  gfx_friends
 * RawConvert.c -> RawConvert.cpp
 *
 * Revision 1.7  2003/06/04 16:53:55  gfx_friends
 * Tore out CR.
 *
 * Revision 1.6  2003/01/20 04:14:40  gfx_friends
 * Fixed texturing bugs.
 *
 * Revision 1.5  2003/01/19 07:19:57  gfx_friends
 * Patches for C++ compatibility, better VA-conversion support, and a note about the new API call for InsertElements.
 *
 * Revision 1.4  2003/01/16 02:40:58  gfx_friends
 * Ported VDS callbacks and include support into GLOD.
 *
 * Revision 1.3  2003/01/15 20:12:41  gfx_friends
 * Basic functionality of GLOD with DiscreteHierarchy and EdgeCollapse.
 *
 * Revision 1.2  2003/01/15 00:02:30  gfx_friends
 * Needed more explicit includes.
 *
 * Revision 1.1  2003/01/14 23:39:31  gfx_friends
 * Major reorganization.
 *
 * Revision 1.2  2003/01/13 23:52:22  gfx_friends
 * Minor change.
 *
 ***************************************************************************/
