/* AttribSet : Generic code for attribute manipulation
 ***************************************************************************
 * AttribSet(StandardAttribSet id)
 * AttribSet(GLOD Attribute Names)
 * AttribSet()
 * AttribSet(AttribSet& copy)
 *
 * addAttrib(name, int vec_count, GLenum type, bool normalized)
 * addAttrib(glod_attrib_name)
 * create()
 ***************************************************************************
 * int getVertexSize()
 * bool hasAttrib(attr_num)
 * void getAttrib(void* data, int attribNum, void* dst)
 * void* getAttribAddress(void* data, int attribNum)
 * void setAttrib(void* data, int attribNum, void* src)
 ***************************************************************************
 * Serialization:
 *   int create(void* src)
 *   int getStateSize();
 *   void copyState(void* dst);
 ***************************************************************************
 * StandardAttribSet enum:
 *    AS_STD_[xxxx]
 *    where xxxx is any combination of 
 *       P  C  N  T
 *    as long as they appear in the relative order above
 *
 * Attrib names:
 *    AS_USER0 ... AS_USER_8
 *    AS_POSITION, AS_NORMAL, AS_COLOR, AS_TEXTURE0 ... 7
 *    AS_FOGCOORD, AS_WEIGHTS, AS_COLOR2

 ***************************************************************************/
// AttribSet.h written by Gabriel Landau (night100@hotmail.com)
// Heavily modified by Nat
#ifndef INCLUDED_ATTRIBSET_H
#define INCLUDED_ATTRIBSET_H

#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <string.h>
#include <assert.h>
#else
#include <GL/gl.h>
#endif

#define AS_MAX_ATTRIBS 16 // stolen from GL... if necessary, increment this number.
#define AS_POSITION  0
#define AS_WEIGHTS   1
#define AS_NORMAL    2
#define AS_COLOR     3
#define AS_COLOR2    4
#define AS_FOGCOORD  5
#define AS_TEXTURE0  8
#define AS_TEXTURE1  9
#define AS_TEXTURE2  10
#define AS_TEXTURE3  11
#define AS_TEXTURE4  12
#define AS_TEXTURE5  13
#define AS_TEXTURE6  14
#define AS_TEXTURE7  15

#define AS_USER_0    9
#define AS_USER_1    10
#define AS_USER_2    11
#define AS_USER_3    12
#define AS_USER_4    13
#define AS_USER_5    14
#define AS_USER_6    15
#define AS_USER_7    6
#define AS_USER_8    7

enum StandardAttribSet {
    AS_STD_P,
    AS_STD_PN,
    AS_STD_PT,
    AS_STD_PC,
    AS_STD_PCN,
    AS_STD_PCT,
    AS_STD_PNT,
    AS_STD_PCNT
};

class AttribSet {
private:
   int m_VertexSize;
   int m_NAttribs;
   int m_AttribOffsets[AS_MAX_ATTRIBS];
   int m_AttribTypes[AS_MAX_ATTRIBS];
   short m_AttribSizes[AS_MAX_ATTRIBS];
   int m_AttribCount[AS_MAX_ATTRIBS];
   bool m_AttribNormalized[AS_MAX_ATTRIBS];
   bool m_Finalized;

public:
   
   int getVertexSize() const 
   {
      return m_VertexSize;
   }

   bool hasAttrib(int attribNum)
   { 
      return (m_AttribCount[attribNum]>0);
   }

   void getAttrib(void* data, int attribNum, void* dst) {
       memcpy(dst, ((char*) data) + m_AttribOffsets[attribNum], 
              m_AttribSizes[attribNum]);
   }
   
   void* getAttribAddress(void* data, int attribNum) {
       return (void*)((unsigned char*)data + m_AttribOffsets[attribNum]);
   }
   
   int getAttribSize(int attribNum) { return m_AttribSizes[attribNum]; }
   int getAttribOffset(int attribNum) { return m_AttribOffsets[attribNum]; }
   
   void setAttrib(void* data, int attribNum, void* src) {
       memcpy(((char*)data) + m_AttribOffsets[attribNum], src,
              m_AttribSizes[attribNum]);
   }


   /***************************************************************************/
   int addAttrib(int name, int count, GLenum type, bool normalized)
   {
       if (m_Finalized) return -1;
       int size = 0;
       switch(type) {
       case GL_BYTE:           size = sizeof(GLbyte);    break;
       case GL_UNSIGNED_BYTE:  size = sizeof(GLubyte);   break;
       case GL_SHORT:          size = sizeof(GLshort);   break;
       case GL_UNSIGNED_SHORT: size = sizeof(GLushort);  break;
       case GL_INT:            size = sizeof(GLint);     break;
       case GL_UNSIGNED_INT:   size = sizeof(GLuint);    break;
       case GL_FLOAT:          size = sizeof(GLfloat);   break;
       case GL_DOUBLE:         size = sizeof(GLdouble);  break;
       default: return -1; break;
       }
       
       int index = name;
       m_AttribTypes[index] = type;
       m_AttribSizes[index] = count * size;
       m_AttribCount[index] = count;
       m_AttribNormalized[index] = normalized;
       m_NAttribs++;
       return index;
   }
   
   bool create() {
       if (m_Finalized) return false;
       m_VertexSize = 0;
       for (int i=0;i<AS_MAX_ATTRIBS;i++) {
           if(m_AttribCount[i] == 0) continue;
           m_AttribOffsets[i] = m_VertexSize;
           m_VertexSize += m_AttribSizes[i];
       }
       m_Finalized = true;
       return m_Finalized;
   }

   /***************************************************************************/
   AttribSet()   {  init(); }

   AttribSet(AttribSet& copy) {
       this->m_VertexSize = copy.m_VertexSize;
       this->m_NAttribs = copy.m_NAttribs;

#define COPY_TOPTR_FROMREF(dst, src, attr) memcpy(dst->attr, src.attr, sizeof(src.attr));
       COPY_TOPTR_FROMREF(this, copy, m_AttribOffsets);
       COPY_TOPTR_FROMREF(this, copy, m_AttribTypes);
       COPY_TOPTR_FROMREF(this, copy, m_AttribSizes);
       COPY_TOPTR_FROMREF(this, copy, m_AttribCount);
       COPY_TOPTR_FROMREF(this, copy, m_AttribNormalized);
#undef COPY_TOPTR_FROMREF
       this->m_Finalized = copy.m_Finalized;
   }
   
   int getStateSize() {
#define APPEND(value,amount) {size+=amount;}
       int size = 0;
       APPEND(&m_VertexSize,  sizeof(m_VertexSize));
       APPEND(&m_NAttribs,    sizeof(m_NAttribs));
       
       APPEND(m_AttribOffsets,      sizeof(m_AttribOffsets));
       APPEND(m_AttribTypes,        sizeof(m_AttribTypes));
       APPEND(m_AttribSizes,        sizeof(m_AttribSizes));
       APPEND(m_AttribCount,        sizeof(m_AttribCount));
       APPEND(m_AttribNormalized,   sizeof(m_AttribNormalized));

       APPEND(&m_Finalized,   sizeof(m_Finalized));
#undef APPEND
       return size;
   }
   
   int copyState(void* vdst) {
       char* dst = (char*)vdst;
#if 1
#define APPEND(value,amount) {memcpy(dst,(value),(amount)); dst+=(amount);}
#else
#define APPEND(value,amount) { \
    memcpy(dst,(value),(amount)); dst+=(amount); \
    printf(" +%i\n", amount);}
#endif
       APPEND(&m_VertexSize,  sizeof(m_VertexSize));
       APPEND(&m_NAttribs,    sizeof(m_NAttribs));
       
       APPEND(m_AttribOffsets,      sizeof(m_AttribOffsets));
       APPEND(m_AttribTypes,        sizeof(m_AttribTypes));
       APPEND(m_AttribSizes,        sizeof(m_AttribSizes));
       APPEND(m_AttribCount,        sizeof(m_AttribCount));
       APPEND(m_AttribNormalized,   sizeof(m_AttribNormalized));

       APPEND(&m_Finalized,   sizeof(m_Finalized));
#undef APPEND
       return dst - (char*)vdst;
   }

   int create(void* vsrc) {
       char* src = (char*)vsrc;
#if 1
#define APPEND(value,amount) {memcpy((value),src,(amount)); src+=(amount);}
#else
#define APPEND(value,amount) {memcpy((value),src,(amount)); src+=(amount); \
                              printf(" +%i: ", amount);\
                              for(int k = 0; k < amount; k++) printf("%02x ", (unsigned int)((char*)src)[k]); \
                              printf("\n"); }

#endif
       APPEND(&m_VertexSize,  sizeof(m_VertexSize));
       APPEND(&m_NAttribs,    sizeof(m_NAttribs));
       
       APPEND(m_AttribOffsets,      sizeof(m_AttribOffsets));
       APPEND(m_AttribTypes,        sizeof(m_AttribTypes));
       APPEND(m_AttribSizes,        sizeof(m_AttribSizes));
       APPEND(m_AttribCount,        sizeof(m_AttribCount));
       APPEND(m_AttribNormalized,   sizeof(m_AttribNormalized));

       APPEND(&m_Finalized,   sizeof(m_Finalized));
#undef APPEND
       return src - (char*)vsrc;
   }

 /***************************************************************************/
   AttribSet(StandardAttribSet ID) {
     init();
     setupStandardSet(ID);
   }

   void init()
   {
      m_VertexSize = 0;
      m_NAttribs = 0;
      m_Finalized = false;
      memset(m_AttribOffsets, 0, AS_MAX_ATTRIBS * sizeof(int));
      memset(m_AttribTypes, 0, AS_MAX_ATTRIBS * sizeof(int));
      memset(m_AttribSizes, 0, AS_MAX_ATTRIBS * sizeof(int));
      memset(m_AttribNormalized, 0, AS_MAX_ATTRIBS * sizeof(bool));
      memset(m_AttribCount, 0, AS_MAX_ATTRIBS * sizeof(int));
   }


   /***************************************************************************/
   static AttribSet STANDARD_P;     // Position only
   static AttribSet STANDARD_PN;    // Position and Normal
   static AttribSet STANDARD_PT;    // Position and TexCoord
   static AttribSet STANDARD_PC;    // Position and Color
   static AttribSet STANDARD_PCN;   // Position, Color, and Normal
   static AttribSet STANDARD_PCT;   // Position, Color, and TexCoord
   static AttribSet STANDARD_PNT;   // Position, Normal, TexCoord
   static AttribSet STANDARD_PCNT;  // Position, Color, Normal, and TexCoord
   
   void setupStandardSet(StandardAttribSet &ID) {
       switch (ID) {
       case AS_STD_P :
           addAttrib(AS_POSITION,3,GL_FLOAT,false); // Position
           break;
       case AS_STD_PN:
           addAttrib(AS_POSITION,3,GL_FLOAT,false);// Position
           addAttrib(AS_NORMAL,3,GL_FLOAT,false);// Normal
           break;
       case AS_STD_PT:
           addAttrib(AS_POSITION,3,GL_FLOAT,false);// Position
           addAttrib(AS_TEXTURE0,2,GL_FLOAT,false);// TexCoord
           break;
       case AS_STD_PC:
           addAttrib(AS_POSITION,3,GL_FLOAT,false);// Position
           addAttrib(AS_COLOR,3,GL_FLOAT,false);// Color
           break;
       case AS_STD_PCN:
           addAttrib(AS_POSITION,3,GL_FLOAT,false);// Position
           addAttrib(AS_COLOR,3,GL_FLOAT,false);// Color
           addAttrib(AS_NORMAL,3,GL_FLOAT,false);// Normal
           break;
       case AS_STD_PCT:
           addAttrib(AS_POSITION,3,GL_FLOAT,false);// Position
           addAttrib(AS_COLOR,3,GL_FLOAT,false);// Color
           addAttrib(AS_TEXTURE0,2,GL_FLOAT,false);// TexCoord
           break;
       case AS_STD_PNT:
           addAttrib(AS_POSITION,3,GL_FLOAT,false);// Position
           addAttrib(AS_NORMAL,3,GL_FLOAT,false);// Normal
           addAttrib(AS_TEXTURE0,2,GL_FLOAT,false);// TexCoord
           break;
       case AS_STD_PCNT:
           addAttrib(AS_POSITION,3,GL_FLOAT,false);// Position
           addAttrib(AS_COLOR,3,GL_FLOAT,false);// Color
           addAttrib(AS_NORMAL,3,GL_FLOAT,false);// Normal
           addAttrib(AS_TEXTURE0,2,GL_FLOAT,false);// TexCoord
           break;
       default:
           assert(false);
       }
       create();
   }
};

#endif
