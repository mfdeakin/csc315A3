
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glut.h>

#define PI 3.141592653589793
#define REFRESHMS 1

#define MINX -10.0f
#define MINY -10.0f
#define MINZ -10.0f

#define MAXX 10.0f
#define MAXY 10.0f
#define MAXZ 10.0f

#define OFFVIEW 0.3f
#define VIEWRATIO 1 - OFFVIEW

enum view {
	PERSPECTIVE,
	ORTHOGRAPH
} viewmode;

float rspeedx, rspeedy, rspeedz,
	rcurx, rcury, rcurz,
	tranx, trany, tranz,
	shearx, sheary, shearz;
GLenum polymode;
bool axis;
unsigned viewwidth, viewheight;

void drawAxes(void);
void drawHouse();
void display(void);
void mpress(int btn, int state, int x, int y);
void resize(GLsizei width, GLsizei height);
void keypress(unsigned char key, int x, int y);
void timer(int val);

void timer(int val)
{
	rcurx += (float)rspeedx * 18.0f / PI / 30.0f;
	if(rcurx > 360)
		rcurx -= 360;
	else if(rcurx < 0)
		rcurx += 360;
	rcury += (float)rspeedy * 18.0f / PI / 30.0f;
	if(rcury > 360)
		rcury -= 360;
	else if(rcury < 0)
		rcury += 360;
	rcurz += (float)rspeedz * 18.0f / PI / 30.0f;
	if(rcurz > 360)
		rcurz -= 360;
	else if(rcurz < 0)
		rcurz += 360;
	glutPostRedisplay();
	glutTimerFunc(REFRESHMS, timer, val + 1);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
 	gluLookAt(-5.0f, 5.0f, 4.0f,
						0.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f);
	if(axis)
		drawAxes();
	glRotatef(rcurx, 1.0f, 0.0f, 0.0f);
	glRotatef(rcury, 0.0f, 1.0f, 0.0f);
	glRotatef(rcurz, 0.0f, 0.0f, 1.0f);
	glTranslatef(tranx, trany, tranz);
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

void drawHouse(void)
{
#define FACECOUNT 7
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
	for(int i = 0; i < FACECOUNT; i++) {
		glColor3f(colors[i].r,
							colors[i].g,
							colors[i].b);
		glBegin(polymode);
		for(int j = 0; j < vcount[i]; j++) {
			glVertex3f(vertices[i][j].x, vertices[i][j].y, vertices[i][j].z);
		}
		glEnd();
	}
}

void updateView()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(viewmode == ORTHOGRAPH)
		glOrtho(MINX, MAXX, MINY, MAXY, 0.0f, 30.0f);
	else if(viewmode == PERSPECTIVE)
		glFrustum(MINX / 10, MAXX / 10, MINY / 10, MAXY / 10, 1.0f, 30.0f);
	glMatrixMode(GL_MODELVIEW);
}

void resize(GLsizei width, GLsizei height)
{
	glViewport(0, 0, width, height);
	updateView();
}

void mpress(int btn, int state, int mxp, int myp)
{
	if(state == GLUT_DOWN) {
		float x, y;
		float factor = viewwidth * viewheight;
		x = (float)-mxp * 2 / viewwidth + 1;
		y = (float)-myp * 2 / viewheight + 1;
		if(fabs(x) > VIEWRATIO || fabs(y) > VIEWRATIO) {
		}
		else if(btn == GLUT_LEFT_BUTTON) {
			if(y > 0.0f) {
				if(x < 0.0f && rspeedy < 10)
					rspeedy++;
				if(x >= 0.0f && rspeedx < 10)
					rspeedx++;
			}
			if(y < -fabs(x) && rspeedz < 10) {
				rspeedz++;
			}
			if(y >= -fabs(x) && x < 0.0f && rspeedy < 10)
				rspeedy++;
			if(y >= -fabs(x) && x >= 0.0f && rspeedx < 10)
				rspeedx++;
		}
		else if(btn == GLUT_RIGHT_BUTTON) {
			if(y > 0.0f) {
				if(x < 0.0f && rspeedy > -10)
					rspeedy--;
				if(x >= 0.0f && rspeedx > -10)
					rspeedx--;
			}
			if(y < -fabs(x) && rspeedz > -10) {
				rspeedz--;
			}
			if(y >= -fabs(x) && x < 0.0f && rspeedy > -10)
				rspeedy--;
			if(y >= -fabs(x) && x >= 0.0f && rspeedx > -10)
				rspeedx--;
		}
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
		if(polymode == GL_POLYGON)
			polymode = GL_LINE_LOOP;
		else
			polymode = GL_POLYGON;
		break;
	case 'q':
		exit(0);
	case 'r':
		rcurx = 0;
		rcury = 0;
		rcurz = 0;
		break;
	case 's':
		rspeedx = 0;
		rspeedy = 0;
		rspeedz = 0;
		break;
	}
}

void changeView(enum view newmode)
{
	viewmode = newmode;
	updateView();
}

void nil(int n) {}

int main(int argc, char **argv)
{
	polymode = GL_POLYGON;
	axis = false;
	rspeedx = 0;
	rspeedy = 0;
	rspeedz = 0;
	rcurx = 0;
	rcury = 0;
	rcurz = 0;
	viewwidth = 500;
	viewheight = 500;
	tranx = 5.0;
	trany = 0;
	tranz = 0;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(viewwidth,
										 viewheight);
	glutCreateWindow("Program 3");
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mpress);
	glutKeyboardFunc(keypress);
	glutTimerFunc(REFRESHMS, timer, 0);

	int view, solid, axes, trans;
	view = glutCreateMenu((void (*)(int))&changeView);
	glutAddMenuEntry("Perspective", PERSPECTIVE);
	glutAddMenuEntry("Orthographic", ORTHOGRAPH);

	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	glPointSize(5);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glutMainLoop();
  return 0;
}
