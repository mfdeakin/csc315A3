
#include "pngloader.h"

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
	ORTHOGRAPH,
	CUSTOM_ORTHO,
	CUSTOM_PROJ,
	CUSTOM_MTX
} viewmode;

struct vector {
	float x, y, z, w;
} viewmtx[4], shearmtx[4];

enum shearplanes {
	XYPLANE = 0,
	XZPLANE = 1,
	YXPLANE = 2,
	YZPLANE = 3,
	ZXPLANE = 4,
	ZYPLANE = 5
};

float rspeedx, rspeedy, rspeedz,
	rcurx, rcury, rcurz,
	tran[3], scale[3], shear[3];
bool axis;
unsigned viewwidth, viewheight;

struct image *bricks;

void drawAxes(void);
void drawMatrixType(void);
void drawHouse(void);
void resetHouse(void);
void updateView(void);
void initglobs(void);
void display(void);
void mpress(int btn, int state, int x, int y);
void resize(GLsizei width, GLsizei height);
void keypress(unsigned char key, int x, int y);
void timer(int val);
void createMenus(void);

void timer(int val)
{
	/* Updates the animation and the scene */
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
	/* Draw the current scene */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Save our projection matrix so we don't recalculate it */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Draw our HUD contents */
	drawMatrixType();

	/* Restore our matrix */
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	/* Draw the actual 3D stuff */
	glMatrixMode(GL_MODELVIEW);
 	gluLookAt(-5.0f, 5.0f, 4.0f,
						0.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f);
	if(axis)
		drawAxes();
	glRotatef(rcurx, 1.0f, 0.0f, 0.0f);
	glRotatef(rcury, 0.0f, 1.0f, 0.0f);
	glRotatef(rcurz, 0.0f, 0.0f, 1.0f);
	glMultMatrixf((GLfloat *)shearmtx);
	glTranslatef(tran[0], tran[1], tran[2]);
	glScalef(scale[0], scale[1], scale[2]);
	drawHouse();
	glFlush();
 	glutSwapBuffers();
}

void drawMatrixType(void)
{
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.4, 0.9, -1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(0, 0);
	char *mtxname[] = {"Projection",
										 "Orthographic",
										 "Custom Ortho",
										 "Custom Proj",
										 "Custom Matrix",};
	for(int i = 0; mtxname[viewmode][i]; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, mtxname[viewmode][i]);
	glPopMatrix();
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

	glPushMatrix();
	float tranRatio = 0.5f;
	glTranslatef(MAXX * tranRatio, 0.0f, 0.0f);
	glRasterPos2i(0, 0);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'X');
	glTranslatef(-MAXX * tranRatio, MAXY * tranRatio, 0.0f);
	glRasterPos2i(0, 0);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'Y');
	glTranslatef(0.0f, -MAXY * tranRatio, MAXZ * tranRatio);
	glRasterPos2i(0, 0);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'Z');
	glPopMatrix();
}

void drawHouse(void)
{
#define FACECOUNT 7
	/* Define the coordinates to be used,
	 * then create the face matrices from them.
	 * Points are stored in a row of x, then a row of y, then a row of z */
	struct vector vertices[FACECOUNT][5] = {
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

	/* The image loader I've got is bad,
	 * so I'm cheating and not using the
	 * whole image.
	 * I'm exploiting it to make the roof
	 * a different color, too. 
	 * I will fix this bug eventually. */
	struct {
		float u, v;
	} uvmap[FACECOUNT][5] = {
		//Closest 5 vertex side
		{{0.15f, 0.3f},
		 {0.0f, 0.225f},
		 {0.0f, 0.0f},
		 {0.3f, 0.0f},
		 {0.3f, 0.225f}},
		//Farthest 5 vertex side
		{{0.15f, 0.3f},
		 {0.0f, 0.225f},
		 {0.0f, 0.0f},
		 {0.3f, 0.0f},
		 {0.0f, 0.225f}},
		//Floor
		{{0.0f, 0.0f},
		 {0.3f, 0.0f},
		 {0.3f, 0.3f},
		 {0.0f, 0.3f}},
		//Closest 4 vertex side
		{{0.3f, 0.0f},
		 {0.3f, 0.1f},
		 {0.0f, 0.1f},
		 {0.0f, 0.0f}},
		//Farthest 4 vertex side
		{{0.3f, 0.0f},
		 {0.3f, 0.1f},
		 {0.0f, 0.1f},
		 {0.0f, 0.0f}},
		//Closest roof
		{{0.67f, 0.3f},
		 {1.0f, 0.3f},
		 {1.0f, 0.0f},
		 {0.67f, 0.0f}},
		//Farthest roof
		{{0.67f, 0.15f},
		 {1.0f, 0.15f},
		 {1.0f, 0.0f},
		 {0.67f, 0.0f}},
	};
	unsigned vcount[FACECOUNT] = {5, 5, 4, 4, 4, 4, 4};
	for(int i = 0; i < FACECOUNT; i++) {
		if(!bricks || !bricks->texture) {
			glColor3f(colors[i].r,
								colors[i].g,
								colors[i].b);
		}
		glBegin(GL_POLYGON);
		for(int j = 0; j < vcount[i]; j++) {
			if(bricks && bricks->texture) {
				glTexCoord2d(uvmap[i][j].u, uvmap[i][j].v);
			}
			glVertex3fv((GLfloat *)&vertices[i][j]);
		}
		glEnd();
	}
}

void updateView()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(viewmode == ORTHOGRAPH) {
		glOrtho(MINX, MAXX, MINY, MAXY, -5.0f, 30.0f);
	}
	else if(viewmode == PERSPECTIVE) {
		glFrustum(MINX / 10, MAXX / 10, MINY / 10, MAXY / 10, 1.0f, 30.0f);
	}
	else
		glLoadMatrixf((float *)viewmtx);
	struct vector v[4];
	glGetFloatv(GL_PROJECTION_MATRIX, (float *)&v);
	
	for(int i = 0; i < 4; i++)
		printf("%10.7f ", v[i].x);
	printf("\n");
	
	for(int i = 0; i < 4; i++)
		printf("%10.7f ", v[i].y);
	printf("\n");
	
	for(int i = 0; i < 4; i++)
		printf("%10.7f ", v[i].z);
	printf("\n");
	
	for(int i = 0; i < 4; i++)
		printf("%10.7f ", v[i].w);
	printf("\n\n");
		
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
		/* Scale the coordinates so the viewport *
		 * can be easily divided into rectangles */
		float x, y;
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
	case 'q':
		exit(0);
	case 'r':
		resetHouse();
		break;
	case 's':
		rspeedx = 0;
		rspeedy = 0;
		rspeedz = 0;
		break;
	}
}

void showAxes(int on)
{
	if(on == -1)
		axis = !axis;
	else if(on)
		axis = 1;
	else
		axis = 0;
}

void changeView(enum view newmode)
{
	viewmode = newmode;
	if(viewmode == CUSTOM_ORTHO || viewmode == CUSTOM_PROJ) {
		float top, bottom, right, left, near, far;
		printf("Left: ");
		scanf("%f", &left);
		printf("Right: ");
		scanf("%f", &right);

		printf("Bottom: ");
		scanf("%f", &bottom);
		printf("Top: ");
		scanf("%f", &top);

		printf("Near: ");
		scanf("%f", &near);
		printf("Far: ");
		scanf("%f", &far);
		near = fabs(near);
		far = fabs(far);

		if(viewmode == CUSTOM_ORTHO) {
			viewmtx[0].x = 2 / (right - left);
			viewmtx[1].x = 0;
			viewmtx[2].x = 0;
			viewmtx[3].x = -(right + left) / (right - left);

			viewmtx[0].y = 0;
			viewmtx[1].y = 2 / (top - bottom);
			viewmtx[2].y = 0;
			viewmtx[3].y = -(top + bottom) / (top - bottom);

			viewmtx[0].z = 0;
			viewmtx[1].z = 0;
			viewmtx[2].z = 2 / (near - far);
			viewmtx[3].z = (near + far) / (near - far);
		
			viewmtx[0].w = 0;
			viewmtx[1].w = 0;
			viewmtx[2].w = 0;
			viewmtx[3].w = 1;
		}
		else if(viewmode == CUSTOM_PROJ) {
			viewmtx[0].x = 2 * near / (right - left);
			viewmtx[1].x = 0;
			viewmtx[2].x = (left + right) / (left - right);
			viewmtx[3].x = 0;

			viewmtx[0].y = 0;
			viewmtx[1].y = 2 * near / (top - bottom);
			viewmtx[2].y = (bottom + top) / (bottom - top);
			viewmtx[3].y = 0;

			viewmtx[0].z = 0;
			viewmtx[1].z = 0;
			viewmtx[2].z = (far + near) / (near - far);
			viewmtx[3].z = 2 * far * near / (near - far);
		
			viewmtx[0].w = 0;
			viewmtx[1].w = 0;
			viewmtx[2].w = -1;
			viewmtx[3].w = 0;
		}
	}
	else if(viewmode == CUSTOM_MTX) {
		printf("Enter matrix values (row major order):\n");
		for(int i = 0; i < 4; i++)
			scanf("%f", &viewmtx[i].x);
		for(int i = 0; i < 4; i++)
			scanf("%f", &viewmtx[i].y);
		for(int i = 0; i < 4; i++)
			scanf("%f", &viewmtx[i].z);
		for(int i = 0; i < 4; i++)
			scanf("%f", &viewmtx[i].w);
	}
	updateView();
}

void changeSolid(int solid)
{
	glPolygonMode(GL_FRONT_AND_BACK, solid);
}

void translate(int axis)
{
	char *axes[] = {"X", "Y", "Z"};
	printf("Translate %s amount: ", axes[axis]);
	scanf("%f", &tran[axis]);
}

void rescale(int axis)
{
	char *axes[] = {"X", "Y", "Z"};
	printf("Scale %s amount: ", axes[axis]);
	scanf("%f", &scale[axis]);
}

void reshear(enum shearplanes plane)
{
	char *planes[] = {"X by Y",
										"X by Z",
										"Y by X",
										"Y by Z",
										"Z by X",
										"Z by Y"};
	printf("Shear %s amount: ", planes[plane]);
	float amount;
	scanf("%f", &amount);
	switch(plane) {
	case XYPLANE:
		shearmtx[1].x = amount;
		break;
	case XZPLANE:
		shearmtx[2].x = amount;
		break;
	case YXPLANE:
		shearmtx[0].y = amount;
		break;
	case YZPLANE:
		shearmtx[1].y = amount;
		break;
	case ZXPLANE:
		shearmtx[0].z = amount;
		break;
	case ZYPLANE:
		shearmtx[1].z = amount;
		break;
	}
}

void quit(int n)
{
	glDeleteTextures(1, &bricks->texture);
	free(bricks->data);
	free(bricks);
	exit(0);
}

/* Just for glut menus */
void nil(int n) {}

void resetHouse(void)
{
	rspeedx = 0;
	rspeedy = 0;
	rspeedz = 0;
	
	rcurx = 0;
	rcury = 0;
	rcurz = 0;
	
	tran[0] = 5.0;
	tran[1] = 0;
	tran[2] = 0;
	
	scale[0] = 1.0;
	scale[1] = 1.0;
	scale[2] = 1.0;
	
	shearmtx[0] = (struct vector){1.0f, 0.0f, 0.0f, 0.0f};
	shearmtx[1] = (struct vector){0.0f, 1.0f, 0.0f, 0.0f};
	shearmtx[2] = (struct vector){0.0f, 0.0f, 1.0f, 0.0f};
	shearmtx[3] = (struct vector){0.0f, 0.0f, 0.0f, 1.0f};
}

void initglobs(void) {
	axis = false;

	viewwidth = 500;
	viewheight = 500;
	
	resetHouse();

	bricks = readPNG("bricks.png");
	if(!bricks) {
		printf("Could not load bricks.png!\n");
	}
}

int main(int argc, char **argv)
{
	initglobs();
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

	createMenus();

	glPointSize(5);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D,
									GL_TEXTURE_MIN_FILTER,
									GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,
									GL_TEXTURE_MAG_FILTER,
									GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,
									GL_TEXTURE_WRAP_S,
									GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,
									GL_TEXTURE_WRAP_T,
									GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV,
						GL_TEXTURE_ENV_MODE,
						GL_MODULATE);
	if(bricks) {
 		createTexture(bricks);
		printf("Texture: %d\n", bricks->texture);
	}
	glDisable(GL_CULL_FACE);
	glutMainLoop();
	/* Cleanup */
	glDeleteTextures(1, &bricks->texture);
	free(bricks->data);
	free(bricks);
  return 0;
}

void createMenus(void)
{
	int view, solid, axes, trans, mult, shear, reset, quitmenu;
	view = glutCreateMenu((void (*)(int))&changeView);
	glutAddMenuEntry("Perspective", PERSPECTIVE);
	glutAddMenuEntry("Orthographic", ORTHOGRAPH);
	glutAddMenuEntry("Custom Orthographic", CUSTOM_ORTHO);
	glutAddMenuEntry("Custom Projection", CUSTOM_PROJ);
	glutAddMenuEntry("Custom Matrix", CUSTOM_MTX);

	solid = glutCreateMenu((void (*)(int))&changeSolid);
	glutAddMenuEntry("Solid", GL_FILL);
	glutAddMenuEntry("Wireframe", GL_LINE);

	axes = glutCreateMenu(&showAxes);
	glutAddMenuEntry("On", true);
	glutAddMenuEntry("Off", false);
	glutAddMenuEntry("Toggle", -1);

	trans = glutCreateMenu((void (*)(int))&translate);
	glutAddMenuEntry("X", 0);
	glutAddMenuEntry("Y", 1);
	glutAddMenuEntry("Z", 2);

	mult = glutCreateMenu((void (*)(int))&rescale);
	glutAddMenuEntry("X", 0);
	glutAddMenuEntry("Y", 1);
	glutAddMenuEntry("Z", 2);

	shear = glutCreateMenu((void (*)(int))reshear);
	glutAddMenuEntry("Shear X by Y", XYPLANE);
	glutAddMenuEntry("Shear X by Z", XZPLANE);
	glutAddMenuEntry("Shear Y by X", YXPLANE);
	glutAddMenuEntry("Shear Y by Z", YZPLANE);
	glutAddMenuEntry("Shear Z by X", ZXPLANE);
	glutAddMenuEntry("Shear Z by Y", ZYPLANE);

	/* I normally don't exploit function casting like this, I promise */
	reset = glutCreateMenu((void (*)(int))resetHouse);
	glutAddMenuEntry("Reset", 0);

	quitmenu = glutCreateMenu(quit);
	glutAddMenuEntry("Exit", 0);

	glutCreateMenu(&nil);
	glutAddSubMenu("View", view);
	glutAddSubMenu("Polygon Fill", solid);
	glutAddSubMenu("Show Axes", axes);
	glutAddSubMenu("Translate", trans);
	glutAddSubMenu("Scale", mult);
	glutAddSubMenu("Shear", shear);
	glutAddSubMenu("Reset", reset);
	glutAddSubMenu("Quit", quitmenu);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}
