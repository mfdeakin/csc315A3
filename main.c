
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glut.h>

#define PI 3.141592653589793
#define REFRESHMS 5

#define VIEWWIDTH 500
#define VIEWHEIGHT 500

#define MINX -10.0f
#define MINY -10.0f
#define MINZ -10.0f

#define MAXX 10.0f
#define MAXY 10.0f
#define MAXZ 10.0f

float rspeedx, rspeedy, rspeedz,
	rcurx, rcury, rcurz;
GLenum mode;
bool axis;

void drawAxes(void);
void drawHouse();
void display(void);
void mpress(int btn, int state, int x, int y);
void resize(GLsizei width, GLsizei height);
void keypress(unsigned char key, int x, int y);
void timer(int val);

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
 	gluLookAt(-5.0f, 5.0f, 4.0f,
						0.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f);
	//	if(axis)
		drawAxes();
	drawHouse();
	glFlush();
 	glutSwapBuffers();
}

void drawAxes(void)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(MINX, 0.0f, 0.0f);
	glVertex3f(MAXX, 0.0f, 0.0f);
	glVertex3f(0.0f, MINY, 0.0f);
	glVertex3f(0.0f, MAXY, 0.0f);
	glVertex3f(0.0f, 0.0f, MINZ);
	glVertex3f(0.0f, 0.0f, MAXZ);
	glEnd();
}

#define FACECOUNT 7

void drawHouse(void)
{
	/* Define the coordinates to be used,
	 * then create the face matrices from them.
	 * Points are stored in a row of x, then a row of y, then a row of z */
	struct {
		float x, y, z;
	} vertices[FACECOUNT][5] = {
		// x      y      z
		//Closest 5 vertex side
		{{ 0.0f, -3.0f,  1.0f},
		 { 2.0f, -3.0f,  0.0f},
		 { 2.0f, -3.0f, -2.0f},
		 {-2.0f, -3.0f, -2.0f},
		 {-2.0f, -3.0f,  0.0f}},
		//Farthest 5 vertex side
		{{ 0.0f,  3.0f,  1.0f},
		 { 2.0f,  3.0f,  0.0f},
		 { 2.0f,  3.0f, -2.0f},
		 {-2.0f,  3.0f, -2.0f},
		 {-2.0f,  3.0f,  0.0f}},
		//Floor
		{{ 2.0f, -3.0f, -2.0f},
		 {-2.0f, -3.0f, -2.0f},
		 {-2.0f,  3.0f, -2.0f},
		 { 2.0f,  3.0f, -2.0f}},
		//Closest 4 vertex side
		{{ 2.0f, -3.0f,  0.0f},
		 { 2.0f, -3.0f, -2.0f},
		 { 2.0f,  3.0f, -2.0f},
		 { 2.0f,  3.0f,  0.0f}},
		//Farthest 4 vertex side
		{{-2.0f, -3.0f,  0.0f},
		 {-2.0f, -3.0f, -2.0f},
		 {-2.0f,  3.0f, -2.0f},
		 {-2.0f,  3.0f,  0.0f}},
		//Closest roof
		{{ 2.0f,  3.0f,  0.0f},
		 { 2.0f, -3.0f,  0.0f},
		 { 0.0f, -3.0f,  1.0f},
		 { 0.0f,  3.0f,  1.0f}},
		//Farthest roof
		{{-2.0f,  3.0f,  0.0f},
		 {-2.0f, -3.0f,  0.0f},
		 { 0.0f, -3.0f,  1.0f},
		 { 0.0f,  3.0f,  1.0f}}
	};
	struct {
		float r, g, b;
	} colors[FACECOUNT] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f},
		{1.0f, 0.7f, 0.1f},
		{0.7f, 1.0f, 0.1f},
		{0.1f, 0.5f, 0.8f},
		{0.5f, 0.1f, 0.8f}};
	unsigned vcount[FACECOUNT] = {5, 5, 4, 4, 4, 4, 4};
	for(int i = FACECOUNT - 1; i; i--) {
		glColor3f(colors[i].r,
							colors[i].g,
							colors[i].b);
		glBegin(mode);
		for(int j = 0; j < vcount[i]; j++) {
			glVertex3f(vertices[i][j].x, vertices[i][j].y, vertices[i][j].z);
		}
		glEnd();
	}
}

void timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(REFRESHMS, timer, val + 1);
}

void resize(GLsizei width, GLsizei height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(MINX, MAXX, MINY, MAXY, 0.0f, 30.0f);
	glMatrixMode(GL_MODELVIEW);
}

void mpress(int btn, int state, int x, int y)
{
	if(btn == GLUT_LEFT_BUTTON &&
		 state == GLUT_DOWN) {
	}
}

void keypress(unsigned char key, int x, int y)
{
	key = tolower(key);
	switch(key) {
	case 'a':
		axis = !axis;
		break;
	case 'f':
		if(mode == GL_POLYGON)
			mode = GL_LINE_LOOP;
		else
			mode = GL_POLYGON;
		break;
	case 'q':
		exit(0);
	case 'r':
		break;
	case 's':
		rspeedx = 0;
		rspeedy = 0;
		rspeedz = 0;
		break;
	}
}

int main(int argc, char **argv)
{
	mode = GL_POLYGON;
	axis = false;
	rspeedx = 0;
	rspeedy = 0;
	rspeedz = 0;
	rcurx = 0;
	rcury = 0;
	rcurz = 0;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(VIEWWIDTH,
										 VIEWHEIGHT);
	glutCreateWindow("Program 3");
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mpress);
	glutKeyboardFunc(keypress);
	glutTimerFunc(REFRESHMS, timer, 0);
	glPointSize(5);
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
  return 0;
}
