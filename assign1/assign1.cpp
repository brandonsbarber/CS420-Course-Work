/*
  Brandon Barber
  CSCI 420 Computer Graphics
  Assignment 1: Height Fields
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>

using namespace std;

int imgNum;
int height, width;
int mode = 0;
double heightField = 1024.0;

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;

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

void myinit()
{
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void setupCamera()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60,0,.001,10000.0);

  //manage transformations
  glTranslatef(g_vLandTranslate[0],g_vLandTranslate[1],g_vLandTranslate[2]);
  glRotatef(g_vLandRotate[0],1.0,0.0,0.0);
  glRotatef(g_vLandRotate[2],0.0,1.0,0.0);
  glRotatef(g_vLandRotate[1],0.0,0.0,1.0);
  glScalef(g_vLandScale[0],g_vLandScale[1],g_vLandScale[2]);

  glMatrixMode(GL_MODELVIEW);
}

/**
 * Renders the scene
 */
void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  setupCamera();

  //loop through the height-1 (because I look one ahead)
  for(int j = 0; j < height-1; j++)
  {

    //Switches what the render mode is
    switch(mode)
    {
      case 0:glBegin(GL_POINTS);break;
      case 1:glBegin(GL_LINE_STRIP);break;
      default:glBegin(GL_TRIANGLE_STRIP);
    }

    //loop through whole width
    for(int i = 0; i < width; i++)
    {
      //set color and render current point
      glColor3f(1.0,(int)PIC_PIXEL(g_pHeightData,i,j,0)/256.0,(int)PIC_PIXEL(g_pHeightData,i,j,0)/256.0);
      glVertex3f(1.0*i/width,1.0*j/height,(int)PIC_PIXEL(g_pHeightData,i,j,0)/heightField);
      //set color and render next point
      glColor3f(1.0,(int)PIC_PIXEL(g_pHeightData,i,j+1,0)/256.0,(int)PIC_PIXEL(g_pHeightData,i,j+1,0)/256.0);
      glVertex3f(1.0*i/width,1.0*(j+1)/height,(int)PIC_PIXEL(g_pHeightData,i,j+1,0)/heightField);
    }
    glEnd();
  }
  
  glFlush();
  glutSwapBuffers();
}

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

/* converts mouse drags into information about 
rotation/translation/scaling */
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

/**
 * Generates a response based on a key press
 * @param key the char representing the key pressed
 * @param x the x cursor position
 * @param y the y cursor position
 */
void keyPressed(unsigned char key, int x, int y)
{
  //switches to point rendering mode
  if(key == '1')
  {
    mode = 0;
  }
  //switches to line rendering mode
  else if(key == '2')
  {
    mode = 1;
  }
  //switches to poly rendering mode
  else if(key == '3')
  {
    mode = 2;
  }
  //saves a screenshot
  else if(key == 'p')
  {
    //get image name
    ostringstream convert;
    convert << std::setw(3) << setfill('0')<< imgNum++;
    string temp = convert.str();
    cout << temp << endl;
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

    cout << fileName << endl;

    saveScreenshot(fileName);
  }
  //increase height map
  else if(key == 'q')
  {
    heightField-=128;
  }
  //decrease height map
  else if(key == 'a')
  {
    heightField+=128;
  }
}

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }

  g_pHeightData = jpeg_read(argv[1], NULL);
  if (!g_pHeightData)
  {
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }

  width = g_pHeightData->nx;
  height = g_pHeightData->ny;

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  
  glutInitWindowSize(640,480);
  glutInitWindowPosition(100,100);

    glutCreateWindow(argv[0]);

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

  return(0);
}
