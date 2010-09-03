/***************************************************************************
 * PushOrtho(int w, int h)
 * DrawString(char* str, int x, int y)
 * PopOrtho()
 ****************************************************************************/
#define BITMAP_FONT_MODE GLUT_BITMAP_HELVETICA_12

static inline void PushOrtho(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0,w,0,h);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

static inline void DrawString(const char* str, int x, int y) {
  glRasterPos2i(x, y);
  int l = strlen(str);
  int i = 0;
  while(i < l)
    glutBitmapCharacter(BITMAP_FONT_MODE, str[i++]);
}

static inline void DrawString(const char* str, float x, float y) {
  glRasterPos2f(x, y);
  int l = strlen(str);
  int i = 0;
  while(i < l)
    glutBitmapCharacter(BITMAP_FONT_MODE, str[i++]);
}

static inline void DrawString3fv(const char* str, float* pos) {
  glRasterPos3fv(pos);
  int l = strlen(str);
  int i = 0;
  while(i < l)
    glutBitmapCharacter(BITMAP_FONT_MODE, str[i++]);
}

static inline int GetDrawStringWidth(const char* str) {
  return glutBitmapLength(BITMAP_FONT_MODE, (const unsigned char*)str);
}

static inline int GetDrawStringHeight() {
  return 12;
}

static inline void PopOrtho() {
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}
