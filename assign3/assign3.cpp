/*
CSCI 480
Assignment 3 Raytracer

Name: Brandon Barber
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <string.h>
#include <cmath>
#include <iostream>

using namespace std;

#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10

#define PI 3.14159265

char *filename=0;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode=MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 160
#define HEIGHT 120

//the field of view of the camera
#define fov 60.0

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

typedef struct _Triangle
{
  struct Vertex v[3];
} Triangle;

typedef struct _Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
} Sphere;

typedef struct _Light
{
  double position[3];
  double color[3];
} Light;

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles=0;
int num_spheres=0;
int num_lights=0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

Vertex subtract(Vertex v1, Vertex v2)
{
    Vertex toReturn;
    for(int i = 0; i <= 2; i++)
    {
        toReturn.position[i] = v1.position[i] - v2.position[i];
    }
    return toReturn;
}

Vertex cross(Vertex v1, Vertex v2)
{
    Vertex toReturn;
    toReturn.position[0] = v1.position[1] * v2.position[2] - v1.position[2] * v2.position[1];
    toReturn.position[1] = v1.position[2] * v2.position[0] - v1.position[0] * v2.position[2];
    toReturn.position[2] = v1.position[0] * v2.position[1] - v1.position[1] * v2.position[0];

    return toReturn;
}

double dot(Vertex v1, Vertex v2)
{
    return v1.position[0] * v2.position[0] + v1.position[1] * v2.position[1] + v1.position[2] * v2.position[2];
}

Vertex scale(Vertex v, double scaleFactor)
{
    Vertex toReturn;
    toReturn.position[0] *= scaleFactor;
    toReturn.position[1] *= scaleFactor;
    toReturn.position[2] *= scaleFactor;
    return toReturn;
}

double magnitude(Vertex v)
{
    return sqrt(v.position[0]*v.position[0] + v.position[1]*v.position[1] + v.position[2]*v.position[2]); 
}

Vertex normalize(Vertex v)
{
    Vertex toReturn;

    double mag = magnitude(v);

    toReturn.position[0] = v.position[0] / mag;
    toReturn.position[1] = v.position[1] / mag;
    toReturn.position[2] = v.position[2] / mag;

    return toReturn;
}

//MODIFY THIS FUNCTION
void draw_scene()
{
    //generate rays
    float ar = 1.0f*WIDTH/HEIGHT;

    float tanVal = tan(fov/2.0 * PI / 180.0);

    for(float x = 0; x < 2 * ar * tanVal-1.0f*(2.0f*ar*tanVal)/WIDTH; x+=1.0f*(2.0f*ar*tanVal)/WIDTH)
    {
        for(float y = 0; y < 2*tanVal-1.0f*(2.0f*tanVal)/HEIGHT; y+=1.0f*(2.0f*tanVal)/HEIGHT)
        {
            float rayX = x - ar * tanVal;
            float rayY = tanVal - y;
            float rayZ = -1;
            float d = sqrt(rayX * rayX + rayY * rayY + rayZ * rayZ);

            rayX /= d;
            rayY /= d;
            rayZ /= d;

            float t = 0;

            for(int triangleIndex = 0; triangleIndex < num_triangles; triangleIndex++)
            {
                Triangle triangle = triangles[triangleIndex];

                Vertex planeNormal = cross(subtract(triangle.v[1],triangle.v[0]),subtract(triangle.v[2],triangle.v[0]));
                planeNormal = normalize(planeNormal);

                Vertex zeroVector;
                zeroVector.position[0] = 0;
                zeroVector.position[1] = 0;
                zeroVector.position[2] = 0;

                Vertex rayVector;
                rayVector.position[0] = rayX;
                rayVector.position[1] = rayY;
                rayVector.position[2] = rayZ;

                double t_Triangle = -1 * dot(subtract(zeroVector,triangle.v[0]),planeNormal) / dot(rayVector, planeNormal);

                if(t_Triangle <= 0)
                {
                    continue;
                }

                rayVector = scale(rayVector,t_Triangle);

                double area = .5 * magnitude(cross(subtract(triangle.v[1],triangle.v[0]),subtract(triangle.v[2],triangle.v[0])));
                double alpha = magnitude(cross(subtract(triangle.v[1],rayVector),subtract(triangle.v[2],rayVector))) * .5 / area;
                double beta = magnitude(cross(subtract(triangle.v[0],rayVector),subtract(triangle.v[2],rayVector))) * .5 / area;
                double gamma = magnitude(cross(subtract(triangle.v[0],rayVector),subtract(triangle.v[1],rayVector))) * .5 / area;

                //double p0Val = dot(cross(subtract (triangle.v[1], triangle.v[0]), subtract(scale(rayVector,t_Triangle),triangle.v[0])),planeNormal);
                //double p1Val = dot(cross(subtract (triangle.v[2], triangle.v[1]), subtract(scale(rayVector,t_Triangle),triangle.v[1])),planeNormal);
                //double p2Val = dot(cross(subtract (triangle.v[0], triangle.v[2]), subtract(scale(rayVector,t_Triangle),triangle.v[2])),planeNormal);
                
                /*double p0X = triangle.v[0].position[0];
                double p0Y = triangle.v[0].position[1];
                double p1X = triangle.v[1].position[0];
                double p1Y = triangle.v[1].position[1];
                double p2X = triangle.v[2].position[0];
                double p2Y = triangle.v[2].position[1];

                cout << "A: "<<p0X << " "<<p0Y << endl;
                cout << "B: "<<p1X << " "<<p1Y << endl;
                cout << "C: "<<p2X << " "<<p2Y << endl;

                double hitX = rayX * t_Triangle;
                double hitY = rayY * t_Triangle;

                double ABx = p1X - p0X;
                double ABy = p1Y - p0Y;

                double ACx = p2X - p0X;
                double ACy = p2Y - p0Y;

                double totalArea = abs(ABx*ACy - ABy*ACx)/2.0;

                cout << ABx << " " << ABy << endl;
                cout << ABy<< " "<< ACx << endl;
                cout << p1X << " " <<p0X << endl;
                cout << p1Y << " " <<p0Y << endl;

                double PAx = p0X - hitX;
                double PAy = p0Y - hitY;

                double PBx = p1X - hitX;
                double PBy = p1Y - hitY;

                double PCx = p2X - hitX;
                double PCy = p2Y - hitY;

                double alpha = abs(PBx * PCy - PBy * PCx) / ( 2 * totalArea);
                double beta = abs(PCx * PAy - PCy * PAx) / ( 2 * totalArea);
                double gamma = abs(PAx * PBy - PAy * PBx) / ( 2 * totalArea);

                cout << "Total Area: "<<totalArea<<endl;

                cout << alpha << " "<<beta<<" "<<gamma << endl;*/

                if(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gamma >= 0 && gamma <= 1)
                {
                    t = t_Triangle;
                }
            }

            for(int sphereIndex = 0;sphereIndex < num_spheres; sphereIndex++)
            {
                Sphere sphere = spheres[sphereIndex];
                float a = 1;
                float b = 2 * (rayX * (-sphere.position[0]) + rayY * (-sphere.position[1]) + rayZ * (-sphere.position[2]));
                float c = sphere.position[0]*sphere.position[0] + sphere.position[1]*sphere.position[1] + sphere.position[2]*sphere.position[2] - sphere.radius * sphere.radius;

                float discriminant = b * b - 4 * a * c;

                if(discriminant < 0)
                {
                    continue;
                }
                float t_0 = (-b + sqrt(discriminant))/2;
                float t_1 = (-b - sqrt(discriminant))/2;

                //cout << "Found a T for "<<sphereIndex<<endl;

                float sphere_t = 0;

                if(t_0 > 0 && t_1 > 0)
                {
                    sphere_t = min(t_0,t_1);
                }
                else if(t_0 > 0)
                {
                    sphere_t = t_0;
                }
                else if(t_1 > 0)
                {
                    sphere_t = t_1;
                }
                else
                {
                    continue;
                }

                //cout << "Sphere: "<<sphereIndex<<endl;

                if(sphere_t > 0)
                {
                    if(t > 0)
                    {
                        t = min(sphere_t,t);
                    }
                    else
                    {
                        t = sphere_t;
                    }
                }
            } 

            if(t > 0)
            {
                //Do shadow ray
                float startX = rayX * t;
                float startY = rayY * t;
                float startZ = rayZ * t;

                /*

                float shadow_t = 0;

                for(int sphereIndex = 0; sphereIndex < num_spheres; sphereIndex++)
                {
                    Sphere s = spheres[sphereIndex];
                    float a = 1;
                    float b = 2 * (rayX * (-s.position[0]) + rayY * (-s.position[1]) + rayZ * (-s.position[2]));
                    float c = s.position[0]*s.position[0] + s.position[1]*s.position[1] + s.position[2]*s.position[2] - s.radius * s.radius;

                    float discriminant = b * b - 4 * a * c;

                    if(discriminant < 0)
                    {
                        continue;
                    }
                    float t_0 = (-b + sqrt(discriminant))/2;
                    float t_1 = (-b - sqrt(discriminant))/2;

                    float sphere_t = 0;

                    if(t_0 > 0 && t_1 > 0)
                    {
                        sphere_t = min(t_0,t_1);
                    }
                    else if(t_0 > 0)
                    {
                        sphere_t = t_0;
                    }
                    else if(t_1 > 0)
                    {
                        sphere_t = t_1;
                    }
                    else
                    {
                        continue;
                    }

                    if(sphere_t > 0)
                    {
                        if(shadow_t > 0)
                        {
                            shadow_t = min(sphere_t,shadow_t);
                        }
                        else
                        {
                            shadow_t = sphere_t;
                        }
                    }
                }*/



                glPointSize(2.0);  
                glBegin(GL_POINTS);
                plot_pixel(x/((2.0f*ar*tanVal)/WIDTH),y/((2.0f*tanVal)/HEIGHT),255,255,255);
                glEnd();
                glFlush();
            }
            else
            {
                glPointSize(2.0);  
                glBegin(GL_POINTS);
                plot_pixel(x/((2.0f*ar*tanVal)/WIDTH),y/((2.0f*tanVal)/HEIGHT),0,0,0);
                glEnd();
                glFlush();
            }

        }
    }
    printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  glColor3f(((double)r)/256.f,((double)g)/256.f,((double)b)/256.f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  buffer[HEIGHT-y-1][x][0]=r;
  buffer[HEIGHT-y-1][x][1]=g;
  buffer[HEIGHT-y-1][x][2]=b;
}

void plot_pixel(int x,int y,unsigned char r,unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
      plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  Pic *in = NULL;

  in = pic_alloc(WIDTH, HEIGHT, 3, NULL);
  printf("Saving JPEG file: %s\n", filename);

  memcpy(in->pix,buffer,3*WIDTH*HEIGHT);
  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);      

}

void parse_check(char *expected,char *found)
{
  if(strcasecmp(expected,found))
    {
      char error[100];
      printf("Expected '%s ' found '%s '\n",expected,found);
      printf("Parse error, abnormal abortion\n");
      exit(0);
    }

}

void parse_doubles(FILE*file, char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE*file,double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE*file,double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE *file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  int i;
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i",&number_of_objects);

  printf("number of objects: %i\n",number_of_objects);
  char str[200];

  parse_doubles(file,"amb:",ambient_light);

  for(i=0;i < number_of_objects;i++)
    {
      fscanf(file,"%s\n",type);
      printf("%s\n",type);
      if(strcasecmp(type,"triangle")==0)
	{

	  printf("found triangle\n");
	  int j;

	  for(j=0;j < 3;j++)
	    {
	      parse_doubles(file,"pos:",t.v[j].position);
	      parse_doubles(file,"nor:",t.v[j].normal);
	      parse_doubles(file,"dif:",t.v[j].color_diffuse);
	      parse_doubles(file,"spe:",t.v[j].color_specular);
	      parse_shi(file,&t.v[j].shininess);
	    }

	  if(num_triangles == MAX_TRIANGLES)
	    {
	      printf("too many triangles, you should increase MAX_TRIANGLES!\n");
	      exit(0);
	    }
	  triangles[num_triangles++] = t;
	}
      else if(strcasecmp(type,"sphere")==0)
	{
	  printf("found sphere\n");

	  parse_doubles(file,"pos:",s.position);
	  parse_rad(file,&s.radius);
	  parse_doubles(file,"dif:",s.color_diffuse);
	  parse_doubles(file,"spe:",s.color_specular);
	  parse_shi(file,&s.shininess);

	  if(num_spheres == MAX_SPHERES)
	    {
	      printf("too many spheres, you should increase MAX_SPHERES!\n");
	      exit(0);
	    }
	  spheres[num_spheres++] = s;
	}
      else if(strcasecmp(type,"light")==0)
	{
	  printf("found light\n");
	  parse_doubles(file,"pos:",l.position);
	  parse_doubles(file,"col:",l.color);

	  if(num_lights == MAX_LIGHTS)
	    {
	      printf("too many lights, you should increase MAX_LIGHTS!\n");
	      exit(0);
	    }
	  lights[num_lights++] = l;
	}
      else
	{
	  printf("unknown type in scene description:\n%s\n",type);
	  exit(0);
	}
    }
  return 0;
}

void display()
{

}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
      draw_scene();
      if(mode == MODE_JPEG)
	save_jpg();
    }
  once=1;
}

int main (int argc, char ** argv)
{
  if (argc<2 || argc > 3)
  {  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
    {
      mode = MODE_JPEG;
      filename = argv[2];
    }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}
