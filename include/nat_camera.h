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
/* Nat Camera
 * natCamera()
 * AutoPlaceBall()
 * PlaceBall(x,y,radius,viewportw,viewporth)
 * LookAt(eye,
 *        center,
 *        up)
 * --
 * SetMode(one of NAT_CAM_MODE);
 * Draw()
 * SetGLState()
 * --
 * Drag(x,y)
 * BeginDrag(x,y)
 * EndDrag(x,y)
 ****************************************************************************/
#ifndef NAT_CAMERA_H
#define NAT_CAMERA_H

#include "nat_math.h"

enum NAT_CAM_MODE { CAMERA_ROTATE, CAMERA_FREE_ROTATE, CAMERA_PAN, 
                    CAMERA_ZOOM, CAMERA_MAX_MODES };
const char* NAT_CAM_NAMES[] = {"Rotate", "Free Rotate", "Pan", "Zoom", "Nothing"};

class natCamera {
 public:
  // camera stuff::: these are what are used to position the camera when nothing else is positioned
  Vec3 m_Eye;
  Vec3 m_Center; // used for CAMERA_ROTATE
  Vec3 m_Up;
  Quaternion m_Quat;
    

  // mousedown camera control
  Vec2 m_Viewport;
  int m_MouseDown;
  Vec3 m_vDown; Vec3 m_vNow;
  Vec2 m_mDown;

  Quaternion m_qDown; Vec3 m_eDown; Vec3 m_cDown; Vec3 m_uDown;
  
  // control
  Vec2 m_BallCenter;
  float m_BallRadius;

  float m_Speed;
 private:
  int m_Mode;
  
 public:
  int GetMode() { return m_Mode;}
  void SetMode(int mode) {
    if(mode == m_Mode) return;
    printf("Mode: %s\n", NAT_CAM_NAMES[mode]);
    m_Mode = mode;
  }

  void SetSpeed(float f) { m_Speed = f;}

  /***************************************************************************/
  natCamera() {
    VEC3_SET(m_Eye, 0, 0, 0);
    QUAT_IDENTITY(&m_Quat);
    m_Mode = CAMERA_ROTATE;
    m_MouseDown = 0;
    m_Speed = 1.0;
  }
  
  void LookAt(float eyeX, float eyeY, float eyeZ,
              float centerX, float centerY, float centerZ, 
              float upX, float upY, float upZ) {
    // set quaternion
    Quaternion q_rot;
    QUAT_LOOKAT(&q_rot,
                eyeX,eyeY,eyeZ,centerX,centerY,centerZ,upX,upY,upZ);
    QUAT_COPY(&m_Quat,&q_rot);
    
    // set alternate orientation and position vectors
    VEC3_SET(m_Eye,eyeX,eyeY,eyeZ);
    VEC3_SET(m_Center,centerX,centerY,centerZ);
    QUAT_TO_ORIENTATION(&m_Quat, NULL, m_Up, NULL);
  }

  void SetGLState() {
    Mat4 m;
    QUAT_TO_MAT4(&m_Quat, m);
    MAT_TRANSPOSE(4,(float*)m,(float*)m);
    
    glMultMatrixf((float*)m);
    glTranslatef(-m_Eye[0], -m_Eye[1], -m_Eye[2]); 
  }

 /***************************************************************************
  * Mouse Motion 
  ***************************************************************************/
 public:
  void Drag(float x, float y) {
    if(m_MouseDown) {
      if(m_Mode == CAMERA_ROTATE || m_Mode == CAMERA_FREE_ROTATE) {
        // Generate a rotation
        Vec3 now;
        ScreenToSphere(x,y,now);
        
	// now apply this rotation to the camera        
        if(m_Mode == CAMERA_ROTATE) { // rotate m_Eye around m_Center
          Quaternion qDrag;
          Vec3 wv1,wv2; 
          QUAT_TRANSFORM_INVERSE(&m_qDown, m_vDown, wv1);
          QUAT_TRANSFORM_INVERSE(&m_qDown, now, wv2);
          QUAT_FROM_VEC3(wv1, wv2, &qDrag);// what's been dragged so far

          // transform the eye
          Vec3 eyevec; Vec3 tmp; 
          VEC3_OP(eyevec,m_eDown,-,m_Center);
          QUAT_TRANSFORM_INVERSE(&qDrag, eyevec, tmp);
          VEC3_OP(m_Eye,tmp,+,m_Center);

          // transform the quaternion
          QUAT_MULT(&m_qDown,&qDrag,&m_Quat);
        } else { // eye doesn't move, center does, plus rotation changes
          
          Quaternion qDrag;
          QUAT_FROM_VEC3(now, m_vDown, &qDrag);// what's been dragged so far
          QUAT_MULT(&qDrag,&m_qDown,&m_Quat);

	  // move the center
          Vec3 eye_to_cam; Vec3 tmp;
          VEC3_OP(eye_to_cam,m_cDown,-,m_Eye);
          QUAT_TRANSFORM_INVERSE(&qDrag,eye_to_cam,tmp);
          VEC3_OP(m_Center,tmp,+,m_Eye);
          VEC3_PRINTLN(m_Center);
        }
      } else if(m_Mode == CAMERA_PAN) {
        float dx,dy; dx = (m_mDown[0] - x)/m_Viewport[0]; dy = (m_mDown[1] - y)/m_Viewport[1];
        float SPEED = 16*m_Speed;
        
        Vec3 right,up; Vec3 motion;

        QUAT_TO_ORIENTATION(&m_Quat,right,up,NULL);
        VEC3_NORM(right); VEC3_NORM(up); // just in case
        
        // now, scale these by the amount of motion

        VEC3_SCALE(right,dx*SPEED);
        VEC3_SCALE(up,dy*SPEED);
        VEC3_OP(motion,right,+,up);

        // translate center and op to the left by diff's x value
        // translate center and op to the up by diff's y value
        VEC3_OP(m_Eye,m_eDown,+,motion);
        VEC3_OP(m_Center,m_cDown,+,motion);
      } else if(m_Mode == CAMERA_ZOOM) {
        float dy = (y - m_mDown[1])/m_Viewport[1];
        Vec3 forward;
        QUAT_TO_ORIENTATION(&m_Quat,NULL,NULL,forward);
        
        float SPEED = 20*m_Speed;

        // scale forward
        VEC3_SCALE(forward,SPEED * dy);

        // move eye by dy * forward... do not pass center
        Vec3 tmp; VEC3_OP(tmp,m_eDown,-,m_Center);
        float dot = VEC3_DOT(tmp,forward);
        if(VEC3_LEN(forward) > VEC3_LEN(tmp) && dot < -.9) { // too close
          VEC3_NORM(forward);
          VEC3_SCALE(forward, .9 * VEC3_LEN(tmp));
        } 
        VEC3_OP(m_Eye,m_eDown,+,forward);
      }
    }
  }

  void BeginDrag(float x, float y) {
    // store the orientation for the first click
    ScreenToSphere(x,y,m_vDown);
    QUAT_COPY(&m_qDown, &m_Quat);
    VEC3_COPY(&m_cDown, &m_Center);
    VEC3_COPY(&m_eDown, &m_Eye);
    VEC3_COPY(&m_uDown, &m_Up);

    VEC2_SET(m_mDown,x,y);
    m_MouseDown = 1;
  }

  void EndDrag(float x, float y) {
    m_MouseDown = 0;
  }
  
  
  /***************************************************************************
   * Aux
   ***************************************************************************/
 public:
  
  void AutoPlaceBall() {
    // get the viewport
    int v[4];
    glGetIntegerv(GL_VIEWPORT,(GLint*) v);
    float w = v[2] - v[0]; float h = v[3]-v[1];
    float r;
    if(w > h)
      r = h / 2;
    else 
      r = w / 2;
    r *= .8;
    VEC2_SET(m_BallCenter, ((float)w / 2), ((float)h / 2));
    m_BallRadius = r;
    //printf("(%f %f)-%f\n", m_BallCenter[0], m_BallCenter[1], m_BallRadius);

	m_Viewport[0] = v[2]; m_Viewport[1] = v[3];
  }


  
  

  // draw the ball & stuff
  void Draw() {
    int v[4];
    glGetIntegerv(GL_VIEWPORT,(GLint*)v);

    glPushMatrix();
	glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0,v[2],0,v[3]);
    {
      float theta = 0; float x,y;
      glColor3f(.5,.5,.5);
      glBegin(GL_LINE_LOOP);
      while(theta < 2 * M_PI) {
	x = m_BallRadius * cos(theta) + m_BallCenter[0];
	y = m_BallRadius * sin(theta) + m_BallCenter[1];
	glVertex2f(x,y);
	theta += M_PI/10;
      }
      glEnd();
    }
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
  }


  void ScreenToSphere(float x, float y, Vec3 out) {
    float mouse[2] = {x,y};
    Vec3 ballMouse;
    float mag;
    ballMouse[0] = (mouse[0] - m_BallCenter[0]) / m_BallRadius;
    ballMouse[1] = (mouse[1] - m_BallCenter[1]) / m_BallRadius;
    mag = ballMouse[0]*ballMouse[0] + ballMouse[1]*ballMouse[1];
    if (mag > 1.0) {
      float scale = 1.0/sqrt(mag);
      ballMouse[0] *= scale; ballMouse[1] *= scale;
      ballMouse[2] = 0.0;
    } else {
      ballMouse[2] = sqrt(1 - mag);
    }
    VEC3_COPY(out,ballMouse);
  }
  /*void ScreenToSphere(float x, float y, Vec3 out) {
    float mouse[2] = {x,y};
    Vec3 ballMouse;
    float mag;
    ballMouse[0] = (mouse[0] - m_BallCenter[0]) / m_BallRadius;
    ballMouse[1] = (mouse[1] - m_BallCenter[1]) / m_BallRadius;
    mag = ballMouse[0]*ballMouse[0] + ballMouse[1]*ballMouse[1];
    if (mag > 1.0) {
      float scale = 1.0/sqrt(mag);
      ballMouse[0] *= scale; ballMouse[1] *= scale;
      ballMouse[2] = 0.0;
    } else {
      ballMouse[2] = sqrt(1 - mag);
    }
    VEC3_COPY(out,ballMouse);
  }*/
  
  void PlaceBall(float x, float y, float radius, float vw, float vh) {
    m_BallCenter[0] = x;
    m_BallCenter[1] = y;
    m_BallRadius = radius;
    m_Viewport[0] = vw; m_Viewport[1] = vh;
  }


  /* Force sphere point to be perpendicular to axis. */
  /*natVec3 ConstrainToAxis(natVec3 loose, natVec3 axis) {
    natVec3 onPlane;
    float norm;
    onPlane = loose - (axis * axis.dot(loose));
    norm = onPlane.SquaredLength();
    if (norm > 0.0) {
      if (onPlane[2] < 0.0) onPlane.set(-onPlane[0], -onPlane[1], -onPlane[2]);
      return ( onPlane * (1/sqrt(norm)) );
    }
    if (axis[2] == 1) {
      onPlane.set(1.0, 0.0, 0.0);
    } else {
      onPlane.set(-axis[1], axis[0], 0.0);
      onPlane.normalize();
    }
    return (onPlane);
  }*/
  
  /* Find the index of nearest arc of axis set. */
  /*int NearestConstraintAxis(natVec3 loose, natVec3 *axes, int nAxes) {
    natVec3 onPlane;
    float max, dot;
    int i, nearest;
    max = -1; nearest = 0;
    for (i=0; i<nAxes; i++) {
      onPlane = ConstrainToAxis(loose, axes[i]);
      dot = onPlane.dot(loose);
      if (dot>max) {
	max = dot; nearest = i;
      }
    }
    return (nearest);
    }*/
  
 
};

static void DebugCamera(natCamera* cam) {
  Vec3 side; Vec3 up; Vec3 forward;
  QUAT_TO_ORIENTATION(&cam->m_Quat,side,up,forward);
  printf("Camera debug:\n\tSide:\t"); VEC3_PRINTLN(side);
  printf("\tForward:"); VEC3_PRINTLN(forward);
  printf("\tEye:\t"); VEC3_PRINTLN(cam->m_Eye);
  printf("\tUp:\t"); VEC3_PRINTLN(up);
  printf("\n\n");
}

#endif
