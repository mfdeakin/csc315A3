
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glut.h>

#include "matrix.h"

#define PI 3.141592653589793
#define REFRESHMS 5

#define VIEWWIDTH 500
#define VIEWHEIGHT 500

#define FACECOUNT 7

struct house {
	struct matrix *faces[FACECOUNT];
	struct colorVal {
		float r, g, b;
	} colors[FACECOUNT];
};

float rspeedx, rspeedy, rspeedz,
	rcurx, rcury, rcurz;

struct house *createHouse(void);
void freeHouse(struct house *);
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
	if(axis)
		drawAxes();
	struct house *house = createHouse();
	drawHouse(house);
	freeHouse(house);
	glFlush();
 	glutSwapBuffers();
}

struct house *createHouse(void)
{
	struct house *house = malloc(sizeof(struct house));
	/* Define the coordinates to be used,
	 * then create the face matrices from them.
	 * Points are stored in a row of x, then a row of y, then a row of z, and finally a row of w (always 1) */
	float posVals[FACECOUNT][20] = {
		//Closest 5 vertex side
		{0, 2, 2, -2, -2,
		 -3, -3, -3, -3, -3,
		 1, 0, -2, -2, 0,
		 1, 1, 1, 1, 1},
		//Farthest 5 vertex side
		{0, 2, 2, -2, -2,
		 3, 3, 3, 3, 3,
		 1, 0, -2, -2, 0,
		 1, 1, 1, 1, 1},
		//Floor
		{2, -2, -2, 2,
		 -3, -3, 3, 3,
		 -2, -2, -2, -2,
		 1, 1, 1, 1},
		//Closest 4 vertex side
		{2, 2, 2, 2,
		 3, 3, -3, -3,
		 0, -2, -2, 0,
		 1, 1, 1, 1},
		//Farthest 4 vertex side
		{-2, -2, -2, -2,
		 3, 3, -3, -3,
		 0, -2, -2, 0,
		 1, 1, 1, 1},
		//Closest 4 vertex roof
		{2, 2, 0, 0,
		 3, -3, -3, 3,
		 0, 0, 1, 1,
		 1, 1, 1, 1},
		//Farthest 4 vertex roof
		{-2, -2, 0, 0,
		 3, -3, -3, 3,
		 0, 0, 1, 1,
		 1, 1, 1, 1},
	};
	struct {
		float r, g, b;
	} colors[FACECOUNT] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f},
		{0.8f, 0.5f, 0.1f},
		{0.5f, 0.8f, 0.1f},
		{0.1f, 0.5f, 0.8f},
		{0.5f, 0.1f, 0.8f}};

	unsigned vcount[FACECOUNT] = {5, 5, 4, 4, 4, 4, 4};
	for(int i = 0; i < FACECOUNT; i++) {
		house->faces[i] = mtxFromArray(posVals[i], vcount[i], 4);
		house->colors[i].r = colors[i].r;
		house->colors[i].g = colors[i].g;
		house->colors[i].b = colors[i].b;
	}
	return house;
}

void freeHouse(struct house *h)
{
	for(int i = 0; i < FACECOUNT; i++)
		mtxFree(h->faces[i]);
	free(h);
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
	glOrtho(0.0, width, 0.0,
					height, 0.0, 1.0);
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
	glutMainLoop();
  return 0;
}
