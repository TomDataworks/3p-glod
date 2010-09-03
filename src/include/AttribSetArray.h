/* Serialization:
 *   int create(void* src)
 *   int getStateSize();
 *   void copyState(void* dst);
 ***************************************************************************/
#ifndef _ATTRIBSET_ARRAY
#define _ATTRIBSET_ARRAY

#define ENABLE_HORRIBLE_STATE_HACK // have GLOD set the array Client state to match what we contain!

#include <AttribSet.h>

class AttribSetArray : public AttribSet {
 private:
    int numVerts;
    int maxVerts;
    unsigned char* verts;

#ifdef GLOD
 public:
    GLuint m_VBOid;
#endif
    
 public:
    AttribSetArray() { 
        numVerts = 0; maxVerts = 0; verts = NULL; 
#ifdef GLOD
        m_VBOid = UINT_MAX;
#endif
    }
    ~AttribSetArray() {
        if(verts != NULL)
            free(verts);
    }

    int getSize() { return numVerts; }
    
    void create(bool has_color, bool has_normal, bool has_texcoord,
           int nverts = 4) {
        
        // init attrib set
        addAttrib(AS_POSITION,3,GL_FLOAT,false);
        if(has_color)
            addAttrib(AS_COLOR,3,GL_UNSIGNED_BYTE,false);
        
        if(has_normal)
            addAttrib(AS_NORMAL,3,GL_FLOAT,false);
        
        if(has_texcoord)
            addAttrib(AS_TEXTURE0,2,GL_FLOAT,false);
        AttribSet::create();
        
        // allocate verts
        verts = (unsigned char*)malloc(getVertexSize() * nverts);
        maxVerts = nverts;
        numVerts = 0;
    }
    
    int addVert() { 
        if(numVerts == maxVerts)
            setSize((int)ceil(1.25f * (float)maxVerts));
        return numVerts++;
    }

    void setSize(int newsize) { /* grow the array ... */
        assert(newsize >= numVerts);
        if(newsize == numVerts) return;
        
        verts = (unsigned char*) realloc(verts, getVertexSize() * newsize);
        maxVerts = newsize;
    }

    void getVertex(unsigned int idx, void* dst) {
        assert(idx < numVerts);
        memcpy(dst,
               verts + getVertexSize() * idx,
               getVertexSize());
    }
    
    void setVertex(unsigned int idx, void* src) {
        assert(idx < numVerts);
        memcpy(verts + getVertexSize() * idx,
               src,
               getVertexSize());
    }
    
    float* getCoord(unsigned int idx) {
        assert(idx < numVerts);
        return (float*) getAttribAddress(verts + getVertexSize() * idx, 
                                AS_POSITION);
    }
    
    void setCoord(unsigned int idx, float* coord) {
        assert(idx < numVerts);
        AttribSet::setAttrib(verts + getVertexSize() * idx, 
                  AS_POSITION,
                  coord);
        return;
    }
    
    void setAttrib(unsigned int idx, int attr, void* coord) {
        assert(hasAttrib(attr));
        AttribSet::setAttrib(verts + getVertexSize() * idx,
                            attr,
                            coord);
        return;
    }

    float* getAttrib(unsigned int idx, int attr) {
        assert(hasAttrib(attr));
        return (float*) getAttribAddress(verts + getVertexSize() * idx,
                                            attr);
    }
    void getAttrib(unsigned int idx, int attr, void* dst) {
        assert(hasAttrib(attr));
        float* tmp = (float*) getAttribAddress(verts + getVertexSize() * idx,
                                               attr);
        memcpy(dst, tmp, getAttribSize(attr));
    }
    
    /***************************************************************************/
   int getStateSize() {
#define APPEND(value,amount) {size+=amount;}
       int size = 0;
       
       size += AttribSet::getStateSize();
       
       APPEND(&numVerts, sizeof(numVerts));
       if(numVerts > 0) {
           APPEND(verts, getVertexSize() * numVerts);
       }
#undef APPEND
       return size;
   }
   
   int copyState(void* vdst) {
       char* dst = (char*)vdst;
#define APPEND(value,amount) {memcpy(dst,(value),(amount)); dst+=(amount);}
       int size = 0;
       dst += AttribSet::copyState(vdst);
       
       APPEND(&numVerts, sizeof(numVerts));
       if(numVerts > 0) {
           APPEND(verts, getVertexSize() * numVerts);
       }
       return dst - (char*)vdst;
#undef APPEND
   }
   
   int create(void* vsrc) {
       char* src = (char*)vsrc;
#define APPEND(value,amount) {memcpy((value),src,(amount)); src+=(amount);}
       int size = 0;
       
       src += AttribSet::create(vsrc);
       APPEND(&numVerts, sizeof(numVerts));
       if(numVerts > 0) {
           verts = (unsigned char*) malloc(numVerts * getVertexSize());
           APPEND(verts, getVertexSize() * numVerts);
       }
       return src - (char*)vsrc;
#undef APPEND
   }

    
#ifdef XBSVERTEX
    void setFrom(int idx, xbsVertex* xvert) {
        unsigned char* data = verts + getVertexSize() * idx;

        xbsVec3 coord; xbsColor color; xbsVec3 normal; xbsVec2 texcoord;
        xvert->fillData(coord, color, normal, texcoord);

        AttribSet::setAttrib(data, AS_POSITION, (float*)&coord);
        if(hasAttrib(AS_COLOR))
            AttribSet::setAttrib(data, AS_COLOR, (void*)&color);
        if(hasAttrib(AS_NORMAL))
            AttribSet::setAttrib(data, AS_NORMAL, (float*)&normal);
        if(hasAttrib(AS_TEXTURE0))
            AttribSet::setAttrib(data, AS_TEXTURE0, (float*)&texcoord);
    }

    void getAt(int idx, 
               xbsVec3& coord, xbsColor& color, xbsVec3& normal, xbsVec2& texcoord) {
        unsigned char* data = verts + getVertexSize() * idx;
        
        AttribSet::getAttrib(data, AS_POSITION, (float*)&coord);
        if(hasAttrib(AS_COLOR))
            AttribSet::getAttrib(data, AS_COLOR, (void*)&color);
        if(hasAttrib(AS_NORMAL))
            AttribSet::getAttrib(data, AS_NORMAL, (float*)&normal);
        if(hasAttrib(AS_TEXTURE0))
            AttribSet::getAttrib(data, AS_TEXTURE0, (float*)&texcoord);
    }
#endif /* xbsvertex */

#ifdef GLOD
    void setFrom(int dst_idx, GLOD_RawPatch* src_patch, int src_idx) {
        setAttrib(dst_idx, AS_POSITION,
                  &src_patch->vertices[src_idx*3]);
        if (src_patch->data_flags & GLOD_HAS_VERTEX_COLORS_3) {
            float tmp[3] = {255 * src_patch->vertices[src_idx*3],
                            255 * src_patch->vertices[src_idx*3+1],
                            255 * src_patch->vertices[src_idx*3+2]};
            setAttrib(dst_idx, AS_COLOR,
                      tmp);
        }
        

        if (src_patch->data_flags & GLOD_HAS_VERTEX_NORMALS)
            setAttrib(dst_idx, AS_NORMAL,
                      &src_patch->vertex_normals[src_idx*3]);


        if (src_patch->data_flags & GLOD_HAS_TEXTURE_COORDS_2)
            setAttrib(dst_idx, AS_TEXTURE0,
                      &src_patch->vertex_texture_coords[src_idx*2]);
    }

    void getAt(int src_idx, GLOD_RawPatch* dst_patch, int dst_idx) {
        assert(src_idx < numVerts);
        getAttrib(src_idx, AS_POSITION,
                  dst_patch->vertices + dst_idx*3);
        if (dst_patch->data_flags & GLOD_HAS_VERTEX_COLORS_3) {
            unsigned char tmp[3];
            getAttrib(src_idx, AS_COLOR,
                      tmp);
            dst_patch->vertex_colors[dst_idx*3] = (float)tmp[0] / 255.0f;
            dst_patch->vertex_colors[dst_idx*3+1] = (float)tmp[1] / 255.0f;
            dst_patch->vertex_colors[dst_idx*3+2] = (float)tmp[2] / 255.0f;
        }

        if (dst_patch->data_flags & GLOD_HAS_VERTEX_NORMALS)
            getAttrib(src_idx, AS_NORMAL,
                      dst_patch->vertex_normals + 3*dst_idx);


        if (dst_patch->data_flags & GLOD_HAS_TEXTURE_COORDS_2)
            getAttrib(src_idx, AS_TEXTURE0,
                      dst_patch->vertex_texture_coords + 2*dst_idx);
    }



 public:

    bool glInitialized() { return m_VBOid != UINT_MAX; }
    void glInit(bool force_va = false) {
        assert(! glInitialized());
        if(glodHasVBO() && force_va == false)
            glInitVBO();
        else
            m_VBOid = UINT_MAX  - 1;
    }
    
    void glInitVBO() {
        assert(glodHasVBO());
        _glGenBuffersARB( 1, &m_VBOid );                                        // Get A Valid Name
        _glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_VBOid );                  // Bind The Buffer
        _glBufferDataARB( GL_ARRAY_BUFFER_ARB, 
                          numVerts*getVertexSize(), 
                          verts, GL_STATIC_DRAW_ARB );
        GLenum error = glGetError();
        if(error == GL_OUT_OF_MEMORY) {
            printf("GLOD: out of memory in VBO \n");
            exit(0);
        }

        //XXX: free(verts)
    }

    void glBind(bool force_va = false) {
        
        unsigned char* base;
        if(m_VBOid != UINT_MAX - 1 && force_va == false) {
            _glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_VBOid);
            base = NULL;
        } else
            base = verts;
        
        int vs = getVertexSize();

        glVertexPointer( 3, GL_FLOAT, vs, base + getAttribOffset(AS_POSITION));

#ifdef ENABLE_HORRIBLE_STATE_HACK
        glEnableClientState(GL_VERTEX_ARRAY);
        if(hasAttrib(AS_COLOR))
            glEnableClientState(GL_COLOR_ARRAY);
        
        if(hasAttrib(AS_NORMAL))
            glEnableClientState(GL_NORMAL_ARRAY);
        
        if(hasAttrib(AS_TEXTURE0))
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

        if(hasAttrib(AS_COLOR))
            glColorPointer(3, GL_UNSIGNED_BYTE, vs, base + getAttribOffset(AS_COLOR));
        
        if(hasAttrib(AS_NORMAL))
            glNormalPointer(GL_FLOAT, vs, base + getAttribOffset(AS_NORMAL));
        
        if(hasAttrib(AS_TEXTURE0))
            glTexCoordPointer(2, GL_FLOAT, vs, base + getAttribOffset(AS_TEXTURE0));
    }

    void glUnbind(bool force_va = false) {
        if(m_VBOid != UINT_MAX - 1 && force_va == false)
            _glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
#endif /* ifdef glod */

    void shuffle(int* new_locations) {
        if(numVerts == 0) return;
        int vs = getVertexSize();
        unsigned char* buf = (unsigned char*) malloc(numVerts * vs);
        for(int i= 0; i < numVerts; i++) {
            memcpy(buf + new_locations[i] * vs, verts + i * vs,
                   vs);
        }
        free(verts);
        verts = buf;
        maxVerts = numVerts;
    }

};    
#endif /*_ATTRIBSET_ARRAY */

