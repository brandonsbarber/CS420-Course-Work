/*
	CSCI 480
	Assignment 2
 */

#include <tgmath.h>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "pic.h"

using namespace std;

/* represents one control point along the spline */
struct point {
	 double x;
	 double y;
	 double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
	 int numControlPoints;
	 struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}


int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;


	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf ("can't open file\n");
		exit(1);
	}
	
	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);

	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf ("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf", 
		 &g_Splines[j].points[i].x, 
		 &g_Splines[j].points[i].y, 
		 &g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

/******************************************************/
/*				Variables for scene view			*/
/******************************************************/

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;		/* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/******************************************************/
/*			These are base GLUT calls for window	*/
/*				operations. Fun times.				*/
/******************************************************/

void menufunc(int value)
{
	switch (value)
	{
		case 0:
			exit(0);
			break;
	}
}

void doIdle()
{
	/* do some stuff... */

	/* make the screen update */
	glutPostRedisplay();
}

void myinit()
{
	glClearColor(0.0,0.0,0.0,0.0);
}
/******************************************************/
/*				Done with the fun times.			*/
/******************************************************/

/******************************************************/
/*				These should be edited				*/
/*				for individual commands				*/
/******************************************************/

void mousedrag(int x, int y)
{
	int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
	
	switch (g_ControlState)
	{
		case TRANSLATE:	
			if (g_iLeftMouseButton)
			{
				g_vLandTranslate[0] += vMouseDelta[0]*0.01;
				g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
			}
			if (g_iMiddleMouseButton)
			{
				g_vLandTranslate[2] += vMouseDelta[1]*0.01;
			}
			break;
		case ROTATE:
			if (g_iLeftMouseButton)
			{
				g_vLandRotate[0] += vMouseDelta[1];
				g_vLandRotate[1] += vMouseDelta[0];
			}
			if (g_iMiddleMouseButton)
			{
				g_vLandRotate[2] += vMouseDelta[1];
			}
			break;
		case SCALE:
			if (g_iLeftMouseButton)
			{
				g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
				g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
			}
			if (g_iMiddleMouseButton)
			{
				g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
			}
			break;
	}
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{
	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			g_iLeftMouseButton = (state==GLUT_DOWN);
			break;
		case GLUT_MIDDLE_BUTTON:
			g_iMiddleMouseButton = (state==GLUT_DOWN);
			break;
		case GLUT_RIGHT_BUTTON:
			g_iRightMouseButton = (state==GLUT_DOWN);
			break;
	}
 
	switch(glutGetModifiers())
	{
		case GLUT_ACTIVE_CTRL:
			g_ControlState = TRANSLATE;
			break;
		case GLUT_ACTIVE_SHIFT:
			g_ControlState = SCALE;
			break;
		default:
			g_ControlState = ROTATE;
			break;
	}

	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

/******************************************************/
/*				Camera Control Variables			*/
/******************************************************/

bool orientedToTrack = false;
int cameraSegment = 0;
float cameraU = 0;
bool constantRendering = false;
int modeToRender = 1;
int imgNum = 0;

void keyPressed(unsigned char key, int x, int y)
{
	if(key == 'o')
	{
		orientedToTrack = !orientedToTrack;
	}
	if(key == '1')
	{
		modeToRender = 1;
	}
	if(key == '2')
	{
		modeToRender = 2;
	}
	if(key == 'p')
	{
		//get image name
	    ostringstream convert;
	    convert << std::setw(3) << setfill('0')<< imgNum++;
	    string temp = convert.str();
	    char* fileName = new char[temp.length()+5];

	    for(int i = 0; i < temp.length(); i++)
	    {
	      fileName[i] = temp[i];
	    }
	    fileName[temp.length()] = '.';
	    fileName[temp.length()+1] = 'j';
	    fileName[temp.length()+2] = 'p';
	    fileName[temp.length()+3] = 'g';
	    fileName[temp.length()+4] = '\0';

	    cout <<"Rendering " << fileName << endl;

	    saveScreenshot(fileName);
	}
	if(key == 'i')
	{
		constantRendering = !constantRendering;
	}
}

/******************************************************/
/*				Done with commands.					*/
/******************************************************/

GLuint groundTex;
GLuint skyTex;

void loadTextures()
{
	Pic * groundPic = jpeg_read("Textures/groundTexture.jpg", NULL);

	unsigned char * imageData = new unsigned char[3*512*512];

	for(int x = 0; x < groundPic->nx; x++)
	{
		for(int y = 0; y < groundPic->ny; y++)
		{
			imageData[(x*groundPic->ny+y)*3] = PIC_PIXEL(groundPic,x,y,0);
			imageData[(x*groundPic->ny+y)*3+1] = PIC_PIXEL(groundPic,x,y,1);
			imageData[(x*groundPic->ny+y)*3+2] = PIC_PIXEL(groundPic,x,y,2);
		}
	}

	glGenTextures(1,&groundTex);
	glBindTexture(GL_TEXTURE_2D,groundTex);

	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGB, 512, 512, 0 , GL_RGB, GL_UNSIGNED_BYTE, imageData);

	Pic * skyPic = jpeg_read("Textures/skyTexture.jpg", NULL);

	unsigned char * skyImageData = new unsigned char[3*512*512];

	for(int x = 0; x < skyPic->nx; x++)
	{
		for(int y = 0; y < skyPic->ny; y++)
		{
			skyImageData[(x*skyPic->ny+y)*3] = PIC_PIXEL(skyPic,x,y,0);
			skyImageData[(x*skyPic->ny+y)*3+1] = PIC_PIXEL(skyPic,x,y,1);
			skyImageData[(x*skyPic->ny+y)*3+2] = PIC_PIXEL(skyPic,x,y,2);
		}
	}

	glGenTextures(1,&skyTex);
	glBindTexture(GL_TEXTURE_2D,skyTex);

	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGB, 512, 512, 0 , GL_RGB, GL_UNSIGNED_BYTE, skyImageData);
}

struct vector
{
	float x;
	float y;
	float z;
};

float mag(vector vec)
{
	return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

vector normalize(vector vec)
{
	float magnitude = mag(vec);
	vector toReturn;
	toReturn.x = vec.x/magnitude;
	toReturn.y = vec.y/magnitude;
	toReturn.z = vec.z/magnitude;
	return toReturn;
}

vector cross(vector vec1, vector vec2)
{
	vector toReturn;
	toReturn.x = vec1.y*vec2.z - vec1.z*vec2.y;
	toReturn.y = vec1.z*vec2.x - vec1.x*vec2.z;
	toReturn.z = vec1.x*vec2.y - vec1.y*vec2.x;
	return toReturn;
}

/******************************************************/
/*				Actual point population				*/
/******************************************************/
void render()
{
	float s = .5;

	spline sp = g_Splines[0];

	if(modeToRender == 1)
	{
		glBegin(GL_LINE_STRIP);

		glLineWidth(5);

		for(int index = 0; index + 3 < sp.numControlPoints; index++)
		{
			point p1 = sp.points[index];
			point p2 = sp.points[index+1];
			point p3 = sp.points[index+2];
			point p4 = sp.points[index+3];

			for(float u = 0; u <= 1; u+=.01)
			{
				float val1 = - ( u * u * u * s ) + ( 2 * u * u * s ) - ( s * u );
				float val2 = ( u * u * u * ( 2 - s ) ) + ( u * u * ( s - 3 ) ) + 1;
				float val3 = u * u * u * ( s - 2 ) + u * u * ( 3 - 2 * s ) + u * s;
				float val4 = u * u * u * s - u * u * s;

				float xPos = val1 * p1.x + val2 * p2.x + val3 * p3.x + val4 * p4.x;
				float yPos = val1 * p1.y + val2 * p2.y + val3 * p3.y + val4 * p4.y;
				float zPos = val1 * p1.z + val2 * p2.z + val3 * p3.z + val4 * p4.z;

				glVertex3f(xPos,yPos,zPos);
			}
		}
		glEnd();
	}
	else if(modeToRender == 2)
	{
		glBegin(GL_QUADS);

		glColor3f(37.0/255,23.0/255,9.0/255);

		vector n0;
		vector t0;
		vector b0;
		vector p0;
		bool previouslySet = false;

		for(int index = 0; index + 3 < sp.numControlPoints; index++)
		{
			point p1 = sp.points[index];
			point p2 = sp.points[index+1];
			point p3 = sp.points[index+2];
			point p4 = sp.points[index+3];

			for(float u = 0; u < 1;u+=.01)
			{
				float val1 = - ( u * u * u * s ) + ( 2 * u * u * s ) - ( s * u );
				float val2 = ( u * u * u * ( 2 - s ) ) + ( u * u * ( s - 3 ) ) + 1;
				float val3 = u * u * u * ( s - 2 ) + u * u * ( 3 - 2 * s ) + u * s;
				float val4 = u * u * u * s - u * u * s;

				//Position
				vector position1;

				position1.x = val1 * p1.x + val2 * p2.x + val3 * p3.x + val4 * p4.x;
				position1.y = val1 * p1.y + val2 * p2.y + val3 * p3.y + val4 * p4.y;
				position1.z = val1 * p1.z + val2 * p2.z + val3 * p3.z + val4 * p4.z;

				float valLook1 = - (3 * u * u * s ) + ( 4 * u * s ) - s ;
				float valLook2 = ( 3 * u * u * ( 2 - s ) ) + ( 2 * u * ( s - 3 ) );
				float valLook3 = 3 * u * u * ( s - 2 ) + 2 * u * ( 3 - 2 * s ) + s;
				float valLook4 = 3 * u * u * s - 2 * u * s;

				//Tangent
				vector t1;

				t1.x = valLook1 * p1.x + valLook2 * p2.x + valLook3 * p3.x + valLook4 * p4.x;
				t1.y = valLook1 * p1.y + valLook2 * p2.y + valLook3 * p3.y + valLook4 * p4.y;
				t1.z = valLook1 * p1.z + valLook2 * p2.z + valLook3 * p3.z + valLook4 * p4.z;
 
				t1 = normalize(t1);

				//initialize previous values
				if(!previouslySet)
				{
					previouslySet = true;
					p0 = position1;
					t0 = t1;

					vector v;
					v.x = 1;
					v.y = 1;
					v.z = 1;

					n0 = normalize(cross(v,t0));
					b0 = normalize(cross(t0,n0));
					continue;
				}
				//populate vectors from previous ones
				vector n1 = normalize(cross(b0,t1));
				vector b1 = normalize(cross(t1,n1));

				float d = .01;

				glColor3f(37.0/255,23.0/255,9.0/255);

				//Render plane v0,v1
				glVertex3f(p0.x + d*(n0.x - b0.x),p0.y + d*(n0.y - b0.y),p0.z + d*(n0.z - b0.z));
				glVertex3f(p0.x + d*(n0.x + b0.x),p0.y + d*(n0.y + b0.y),p0.z + d*(n0.z + b0.z));
				glVertex3f(position1.x + d*(n1.x + b1.x),position1.y + d*(n1.y + b1.y),position1.z + d*(n1.z + b1.z));
				glVertex3f(position1.x + d*(n1.x - b1.x),position1.y + d*(n1.y - b1.y),position1.z + d*(n1.z - b1.z));

				glColor3f(1,0,0);

				//Render plane v1,v2
				glVertex3f(p0.x + d*(n0.x + b0.x),p0.y + d*(n0.y + b0.y),p0.z + d*(n0.z + b0.z));
				glVertex3f(p0.x + d*( - n0.x + b0.x),p0.y + d*( - n0.y + b0.y),p0.z + d*( - n0.z + b0.z));
				glVertex3f(position1.x + d*( - n1.x + b1.x),position1.y + d*( - n1.y + b1.y),position1.z + d*( - n1.z + b1.z));
				glVertex3f(position1.x + d*(n1.x + b1.x),position1.y + d*(n1.y + b1.y),position1.z + d*(n1.z + b1.z));

				glColor3f(0,1,0);

				//Render plane v2, v3
				glVertex3f(p0.x + d*( - n0.x + b0.x),p0.y + d*( - n0.y + b0.y),p0.z + d*( - n0.z + b0.z));
				glVertex3f(p0.x + d*( - n0.x - b0.x),p0.y + d*( - n0.y - b0.y),p0.z + d*( - n0.z - b0.z));
				glVertex3f(position1.x + d*( - n1.x - b1.x),position1.y + d*( - n1.y - b1.y),position1.z + d*( - n1.z - b1.z));
				glVertex3f(position1.x + d*( - n1.x + b1.x),position1.y + d*( - n1.y + b1.y),position1.z + d*( - n1.z + b1.z));

				glColor3f(0,0,1);

				//Render plane v3, v0
				glVertex3f(p0.x + d*( - n0.x - b0.x),p0.y + d*( - n0.y - b0.y),p0.z + d*( - n0.z - b0.z));
				glVertex3f(p0.x + d*(n0.x - b0.x),p0.y + d*(n0.y - b0.y),p0.z + d*(n0.z - b0.z));
				glVertex3f(position1.x + d*(n1.x - b1.x),position1.y + d*(n1.y - b1.y),position1.z + d*(n1.z - b1.z));
				glVertex3f(position1.x + d*( - n1.x - b1.x),position1.y + d*( - n1.y - b1.y),position1.z + d*( - n1.z - b1.z));

				n0 = n1;
				p0 = position1;
				t0 = t1;
				b0 = b1;
			}
		}
		glEnd();
	}

	glColor3f(1,1,1);

	glBindTexture(GL_TEXTURE_2D,groundTex);

	glEnable (GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP);

	//Draw Ground
	glTexCoord2f(0.0,0.0); glVertex3f(-100,-100,-1);
	glTexCoord2f(1.0,0.0); glVertex3f(-100,100,-1);
	glTexCoord2f(0.0,1.0); glVertex3f(100,-100,-1);
	glTexCoord2f(1.0,1.0); glVertex3f(100,100,-1);

	glEnd();

	glBindTexture(GL_TEXTURE_2D,skyTex);

	/*Draw sky box*/
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0); glVertex3f(-100,-100,-1);
		glTexCoord2f(1.0,0.0); glVertex3f(-100,-100,100);
		glTexCoord2f(0.0,1.0); glVertex3f(100,-100,100);
		glTexCoord2f(1.0,1.0); glVertex3f(100,-100,-1);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0); glVertex3f(-100,-100,-1);
		glTexCoord2f(1.0,0.0); glVertex3f(-100,-100,100);
		glTexCoord2f(0.0,1.0); glVertex3f(-100, 100,100);
		glTexCoord2f(1.0,1.0); glVertex3f(-100, 100,-1);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0); glVertex3f(100,-100,-1);
		glTexCoord2f(1.0,0.0); glVertex3f(100,-100,100);
		glTexCoord2f(0.0,1.0); glVertex3f(100,100,100);
		glTexCoord2f(1.0,1.0); glVertex3f(100,100,-1);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0); glVertex3f(-100,100,-1);
		glTexCoord2f(1.0,0.0); glVertex3f(-100,100,100);
		glTexCoord2f(0.0,1.0); glVertex3f(100,100,100);
		glTexCoord2f(1.0,1.0); glVertex3f(100,100,-1);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0); glVertex3f(-100,100,100);
		glTexCoord2f(1.0,0.0); glVertex3f(100,100,100);
		glTexCoord2f(0.0,1.0); glVertex3f(100,-100,100);
		glTexCoord2f(1.0,1.0); glVertex3f(-100,-100,100);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void orientCamera();

vector cameraN0;
vector cameraT0;
vector cameraB0;
vector cameraP0;
bool cameraPreviouslySet = false;

void setupCamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80,640.0/480.0,.001,10000.0);

	//manage transformations
	if(!orientedToTrack)
	{
		glTranslatef(g_vLandTranslate[0],g_vLandTranslate[1],g_vLandTranslate[2]);
		glRotatef(g_vLandRotate[0],1.0,0.0,0.0);
		glRotatef(g_vLandRotate[2],0.0,1.0,0.0);
		glRotatef(g_vLandRotate[1],0.0,0.0,1.0);
		glScalef(g_vLandScale[0],g_vLandScale[1],g_vLandScale[2]);
	}
	else
	{
		orientCamera();
	}

	glMatrixMode(GL_MODELVIEW);
}

void orientCamera()
{
	if(orientedToTrack)
	{
		float u = cameraU;
		float s = .5;
		spline sp = g_Splines[0];

		point p1 = sp.points[cameraSegment];
		point p2 = sp.points[cameraSegment+1];
		point p3 = sp.points[cameraSegment+2];
		point p4 = sp.points[cameraSegment+3];

		float val1 = - ( u * u * u * s ) + ( 2 * u * u * s ) - ( s * u );
		float val2 = ( u * u * u * ( 2 - s ) ) + ( u * u * ( s - 3 ) ) + 1;
		float val3 = u * u * u * ( s - 2 ) + u * u * ( 3 - 2 * s ) + u * s;
		float val4 = u * u * u * s - u * u * s;

		vector position1;

		position1.x = val1 * p1.x + val2 * p2.x + val3 * p3.x + val4 * p4.x;
		position1.y = val1 * p1.y + val2 * p2.y + val3 * p3.y + val4 * p4.y;
		position1.z = val1 * p1.z + val2 * p2.z + val3 * p3.z + val4 * p4.z;

		float valLook1 = - (3 * u * u * s ) + ( 4 * u * s ) - s ;
		float valLook2 = ( 3 * u * u * ( 2 - s ) ) + ( 2 * u * ( s - 3 ) );
		float valLook3 = 3 * u * u * ( s - 2 ) + 2 * u * ( 3 - 2 * s ) + s;
		float valLook4 = 3 * u * u * s - 2 * u * s;

		vector t1;

		t1.x = valLook1 * p1.x + valLook2 * p2.x + valLook3 * p3.x + valLook4 * p4.x;
		t1.y = valLook1 * p1.y + valLook2 * p2.y + valLook3 * p3.y + valLook4 * p4.y;
		t1.z = valLook1 * p1.z + valLook2 * p2.z + valLook3 * p3.z + valLook4 * p4.z;
 
		t1 = normalize(t1);

		//Initialize previous values
		if(!cameraPreviouslySet)
		{
			cameraPreviouslySet = true;
			cameraP0 = position1;
			cameraT0 = t1;

			vector v;
			v.x = 1;
			v.y = 1;
			v.z = 1;

			cameraN0 = normalize(cross(v,cameraT0));
			cameraB0 = normalize(cross(cameraT0,cameraN0));

			u+=.01;
			return;
		}


		vector n1 = normalize(cross(cameraB0,t1));

		vector b1 = normalize(cross(t1,n1));

		glMatrixMode(GL_PROJECTION);
		//moves along track and offsets a little from the track
		gluLookAt(position1.x-n1.x*.1,position1.y-n1.y*.1,position1.z-n1.z*.1,t1.x+position1.x,t1.y+position1.y,t1.z+position1.z,-n1.x,-n1.y,-n1.z);

		cameraN0 = n1;
		cameraP0 = position1;
		cameraT0 = t1;
		cameraB0 = b1;

		cameraU+=.01;
		if(cameraU > 1)
		{
			cameraU = 0;
			cameraSegment++;
			if(cameraSegment+3 == sp.numControlPoints)
			{
				cameraSegment = 0;
			}
		}
	}
}

/******************************************************/
/*				Simple display method				*/
/******************************************************/

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setupCamera();

	render();

	glFlush();
	glutSwapBuffers();

	if(constantRendering)
	{
		//get image name
	    ostringstream convert;
	    convert << std::setw(3) << setfill('0')<< imgNum++;
	    string temp = convert.str();
	    char* fileName = new char[temp.length()+5];

	    for(int i = 0; i < temp.length(); i++)
	    {
	      fileName[i] = temp[i];
	    }
	    fileName[temp.length()] = '.';
	    fileName[temp.length()+1] = 'j';
	    fileName[temp.length()+2] = 'p';
	    fileName[temp.length()+3] = 'g';
	    fileName[temp.length()+4] = '\0';

	    cout <<"Rendering "<< fileName << " automatically" << endl;

	    saveScreenshot(fileName);
	}
}

int g_iMenuId;

int main (int argc, char ** argv)
{
	if (argc<2)
	{	
		printf ("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	loadSplines(argv[1]);

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	
	glutInitWindowSize(640,480);
	glutInitWindowPosition(100,100);

	glutCreateWindow(argv[0]);

	loadTextures();

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	
	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit",0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	/* replace with any animate code */
	glutIdleFunc(doIdle);

	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);
	/* callback for key press*/
	glutKeyboardFunc(keyPressed);

 /* do initialization */
	myinit();

	glutMainLoop();

	return 0;
}
