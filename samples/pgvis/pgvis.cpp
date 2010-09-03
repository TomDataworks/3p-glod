
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <mathlib.h>
#include <time.h>

// global variables: user interface control
static int		width = 1024, height = 768;
static float	xpos = 0, ypos = 0, zpos = 0;
static float	roth = 0, rotv = 0;
static float	rothLighting = 0, rotvLighting = 0;

static int		mouse_state = 0;
static int		mouse_x = 0, mouse_y = 0;
static bool	ctrl_pressed = false;
static int		show_lighting = 0;
static float	znear = 0.01f;
static float	zfar = 100.f;
static float	fov = 30.f;
static float	motion_speed = 1.f;
static float	rotate_speed = 1.f;

static float	xmax, ymax, zmax;
static float	xmin, ymin, zmin;
static Point	center;

typedef unsigned char GridDataType;
GridDataType * gridData;
GLfloat * vertexData;
int pgx, pgy, pgz;
int bits_per_data;
GLuint * onindices;
int num_onindices;
GLuint * offindices;
int num_offindices;
GLfloat onpointsize = 10.0;
GLfloat offpointsize = 1.0f;
static bool drawLines = true;

// glut routines 
void GlutResize(int w, int h)
{
	width = w;
	height = h;
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
	glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		gluPerspective(fov, (float) width / (float) height, znear, zfar);
	glMatrixMode (GL_MODELVIEW);
}

void InitViewParameters()
{
	xpos = ypos = zpos = 0;
	roth = rotv = 0;
	rothLighting = rotvLighting = 0;

	float ball_r = sqrtf((xmax - xmin) * (xmax - xmin)
						+ (ymax - ymin) * (ymax - ymin)
						+ (zmax - zmin) * (zmax - zmin));
	center = Point((xmax + xmin) / 2.f, (ymax + ymin) / 2.f, (zmax + zmin) / 2.f);

	xpos = center.x;
	ypos = center.y;
	zpos = ball_r / sinf(fov * M_PI / 180.f) + center.z;

	znear = 0.2f;
	zfar  = zpos - center.z + 3 * ball_r;

	motion_speed = 0.002f * ball_r;
	rotate_speed = 0.1f;
}


void AppKeyboard(unsigned char key, int x, int y)
{
	switch(key) {
		case 'h':
		case 'H':
			printf("================ Key Control ================\n");
			printf("q:   quit\n");
			printf("r:   reset\n");
			printf("+/-: increase/decrease size of ON cells\n");
			printf("]/[: increase/decrease size of OFF cells\n");
			printf("l:   toggle lines\n");
			printf("==============================================\n");
			break;
		case 'q':
		case 27:
			exit(0);
			break;
		case 'r':
		case 'R':
			InitViewParameters();
			break;
		case 's':
		case 'S':
			{
                /*
				unsigned char *framedata = NULL;
				static int framenum = 1;
				int w = glutGet(GLUT_WINDOW_WIDTH);
				int h = glutGet(GLUT_WINDOW_HEIGHT);
				framedata = new uchar[3 * w * h];
				char filename[256];
				sprintf (filename, "frame%i.ppm", framenum);
				glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, framedata);
				WritePPM(filename, w, h, framedata);
				framenum++;
				delete [] framedata;
                break;
                */
			}
		case '+':
		case '=':
            onpointsize+=1.0f;
			break;
		case '-':
		case '_':
            onpointsize-=1.0f;
			break;
		case ']':
        case '}':
            offpointsize+=1.0f;
			break;
		case '[':
        case '{':
            offpointsize-=1.0f;
			break;
        case 'l':
        case 'L':
            drawLines = !drawLines;
            break;
		default:
			return;
	}
	glutPostRedisplay();
}

void AppMouse(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON) {
		if(state == GLUT_DOWN) {
			mouse_state = 2;
			mouse_x = x;
			mouse_y = y;
		} else if(state == GLUT_UP)
			mouse_state = 0;
	} else if (button == GLUT_LEFT_BUTTON) {
		if(state == GLUT_DOWN) {
			mouse_state = 1;
			mouse_x = x;
			mouse_y = y;
		} else if(state == GLUT_UP)
			mouse_state = 0;
	} else if(button == GLUT_MIDDLE_BUTTON) {
		if(state == GLUT_DOWN) {
			mouse_state = 3;
			mouse_x = x;
			mouse_y = y;
		} else if(state == GLUT_UP)
			mouse_state = 0;
	}
	if(glutGetModifiers() & GLUT_ACTIVE_CTRL)
		ctrl_pressed = true;
	else
		ctrl_pressed = false;
}

void AppMotion(int x, int y) 
{
	if(mouse_state == 1) {
		if(ctrl_pressed)
		{
			rothLighting -= (x - mouse_x) * rotate_speed;
			rotvLighting += (y - mouse_y) * rotate_speed;
		}
		else
		{
			roth -= (x - mouse_x) * rotate_speed;
			rotv += (y - mouse_y) * rotate_speed;
		}
		mouse_x = x;
		mouse_y = y;
	} else if(mouse_state == 2) {
		zpos -= (y - mouse_y) * motion_speed;
		mouse_x = x;
		mouse_y = y;
	} else if(mouse_state == 3) {
		xpos -= (x - mouse_x) * motion_speed;
		ypos += (y - mouse_y) * motion_speed;
		mouse_x = x;
		mouse_y = y;
	}
	if(mouse_state)
	{
		glutPostRedisplay();
	}
}

void CleanUp()
{
}

void InitDisplay(char *title)
{
	glutInitDisplayMode ( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH ); 
	glutInitWindowSize(width, height); 
	glutCreateWindow (title);

	InitViewParameters();

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    /*
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float tmp[] = {1,1,0,1};
    glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    glLightfv(GL_LIGHT0,
              GL_POSITION,
              tmp);
    */
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    //glCullFace(GL_BACK);	
	//glEnable(GL_CULL_FACE);
	//glShadeModel(GL_SMOOTH);

	// clear color
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
    glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	GlutResize(width, height);
}

void GlutDisplay(void)
{
    int x,y,z;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

	// gltransform according to user movement
	glLoadIdentity();
	glTranslatef(-xpos, -ypos, -zpos);
	
	glTranslatef(center.x, center.y, center.z);

	glRotatef(rotv, 1.0f, 0, 0);
	glRotatef(360.0f - roth, 0, 1.0f, 0);

	glTranslatef(-center.x, -center.y, -center.z);

    if (onpointsize > 0.0)
    {
        glPointSize(onpointsize);
        glDrawElements(GL_POINTS, num_onindices, GL_UNSIGNED_INT, onindices);
    }
    if (offpointsize > 0.0)
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glPointSize(offpointsize);
        glDrawElements(GL_POINTS, num_offindices, GL_UNSIGNED_INT, offindices);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
    if (drawLines)
    {
        glLineWidth(0.2);
        glDisableClientState(GL_COLOR_ARRAY);
        glColor3f(0,0,1.0);
        glBegin(GL_LINES);
        for (y=0; y<pgy; ++y)
        {
            for (x=0; x<pgx; ++x)
            {
                glArrayElement(y*pgx+x);
                glArrayElement(y*pgx+x+(pgz-1)*pgx*pgy);
            }
        }
        for (z=0; z<pgz; ++z)
        {
            for (y=0; y<pgy; ++y)
            {
                glArrayElement(y*pgx+z*pgx*pgy);
                glArrayElement(y*pgx+z*pgx*pgy+pgx-1);
            }
        }

        glEnd();
        glEnableClientState(GL_COLOR_ARRAY);
    }

	glutSwapBuffers();
}

void LoadPermissionGrid(const char * filename)
{
    FILE * fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf (stderr, "error opening file.  exiting.\n");
        exit(-1);
    }

    //fscanf(fp, "%i %i %i\n", &pgx, &pgy, &pgz);
    fread(&bits_per_data, sizeof(int), 1, fp);
    fread(&pgx, sizeof(int), 1, fp);
    fread(&pgy, sizeof(int), 1, fp);
    fread(&pgz, sizeof(int), 1, fp);
    fprintf (stdout, " Reading permission grid of %ix%ix%i cells...", pgx, pgy, pgz);
    gridData = new GridDataType[pgx*pgy*pgz/bits_per_data + 1];
    fread(gridData, sizeof(GridDataType), pgx*pgy*pgz/bits_per_data+1, fp);
    fprintf (stdout, " done.\n Creating vertex array... ");

    int stride = 7;
    vertexData = new GLfloat[stride*pgx*pgy*pgz];
    onindices = new GLuint[pgx*pgy*pgz];
    offindices = new GLuint[pgx*pgy*pgz];
    num_onindices=0;
    num_offindices=0;
    int index = 0;
    const unsigned char one = 0x80;
    for (int z=0; z<pgz; ++z)
    {
        for (int y=0; y<pgy; ++y)
        {
            for (int x=0; x<pgx; ++x)
            {
                // probably eventually want to unify grid and this file
                // can have a single "is on" test for both programs
                if ((gridData [index / bits_per_data] & (one >> (index % bits_per_data))) != 0)
                //if (gridData[index])
                {
                    vertexData[stride*index+0] = 1.0f;
                    vertexData[stride*index+1] = 0.0f;
                    vertexData[stride*index+2] = 0.0f;
                    vertexData[stride*index+3] = 1.0f;
                    onindices[num_onindices] = index;
                    num_onindices++;
                }
                else 
                {
                    vertexData[stride*index+0] = 0.0f;
                    vertexData[stride*index+1] = 1.0f;
                    vertexData[stride*index+2] = 0.0f;
                    vertexData[stride*index+3] = 0.1f;
                    offindices[num_offindices] = index;
                    num_offindices++;
                }
                vertexData[stride*index+4] = (GLfloat)x / (GLfloat)pgx;
                vertexData[stride*index+5] = (GLfloat)y / (GLfloat)pgy;
                vertexData[stride*index+6] = (GLfloat)z / (GLfloat)pgz;
                
                index++;
            }
        }
    }
    fprintf (stdout, "done.\n");
    //glInterleavedArrays(GL_C3F_V3F, 0, vertexData);
    glVertexPointer(3, GL_FLOAT, stride*sizeof(GL_FLOAT), &vertexData[4]);
    glColorPointer(4, GL_FLOAT, stride*sizeof(GL_FLOAT), vertexData);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
}

int main(int argc, char *argv[])
{
    char *filename;
	if(argc < 2)
        filename = strdup("pg.dat");
    else filename = argv[1];
	
	/* compute bounding box information */
	xmin = 0.0f;
	ymin = 0.0f;
	zmin = 0.0f;
	xmax = 1.0f;
	ymax = 1.0f;
	zmax = 1.0f;


	glutInit(&argc, argv);
	char title[256];
	sprintf(title, "PGVIS [%s]", filename);
	InitDisplay(title);

	glutDisplayFunc(GlutDisplay); 
	glutReshapeFunc(GlutResize);
	glutKeyboardFunc(AppKeyboard);
	glutMouseFunc(AppMouse);
	glutMotionFunc(AppMotion);
    
    LoadPermissionGrid(filename);

	glutMainLoop();

	return 0;
}

