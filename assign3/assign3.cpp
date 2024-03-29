/*
CSCI 420
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
///*
#define WIDTH 640
#define HEIGHT 480
//*/
/*
#define WIDTH 640
#define HEIGHT 480
*/

//the field of view of the camera
#define fov 60.0

#define null 0

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

struct Vector
{
	double x;
	double y;
	double z;
	double magnitude()
	{
		return sqrt(x*x + y*y + z*z);
	}

	Vector normalize()
	{
		Vector toReturn;
		toReturn.x = x/magnitude();
		toReturn.y = y/magnitude();
		toReturn.z = z/magnitude();
		return toReturn;
	}

	Vector operator-(const Vector &other)
	{
		Vector toReturn;
		toReturn.x = x - other.x;
		toReturn.y = y - other.y;
		toReturn.z = z - other.z;
		return toReturn;
	}

	double dot(Vector o)
	{
		return x*o.x + y*o.y + z*o.z;
	}

	Vector operator*(const double &scaleFactor)
	{
		Vector toReturn;
		toReturn.x = x * scaleFactor;
		toReturn.y = y * scaleFactor;
		toReturn.z = z * scaleFactor;
		return toReturn;
	}

	void operator*=(const double &scaleFactor)
	{
		x *= scaleFactor;
		y *= scaleFactor;
		z *= scaleFactor;
	}

	Vector cross(Vector v)
	{
			Vector toReturn;
			toReturn.x = y * v.z - z * v.y;
			toReturn.y = z * v.x - x * v.z;
			toReturn.z = x * v.y - y * v.x;
			return toReturn;
	}

	Vector operator+(const Vector& other)
	{
		Vector toReturn;
		toReturn.x = x + other.x;
		toReturn.y = y + other.y;
		toReturn.z = z + other.z;
		return toReturn;
	}

	void print()
	{
		cout << x <<" "<<y<<" "<<z<<endl;
	}
};



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



struct V2
{
	double x;
	double y;
};

double area(V2 p0, V2 p1, V2 p2)
{
	return .5 * ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
}

double clamp(double d)
{
	if(d < 0)
	{
		return 0;
	}
	if(d > 1)
	{
		return 1;
	}
	return d;
}

Vertex subtract(Vertex v1, Vertex v2)
{
	Vertex toReturn;
	toReturn.position[0] = v1.position[0] - v2.position[0];
	toReturn.position[1] = v1.position[1] - v2.position[1];
	toReturn.position[2] = v1.position[2] - v2.position[2];
	return toReturn;
}

Vector vertexToVector(Vertex v)
{
	Vector toReturn;
	toReturn.x = v.position[0];
	toReturn.y = v.position[1];
	toReturn.z = v.position[2];
	return toReturn;
}

Vertex cross(Vertex v1, Vertex v2)
{
	Vertex toReturn;

	return toReturn;
}

Vector makeVector(double x, double y, double z)
{
	Vector toReturn;
	toReturn.x = x;
	toReturn.y = y;
	toReturn.z = z;
	return toReturn;
}

Vector makeVector(double* vals)
{
	Vector toReturn;
	toReturn.x = vals[0];
	toReturn.y = vals[1];
	toReturn.z = vals[2];
	return toReturn;
}

#define EPSILON .00001

double checkTriangleCollision(Vector startingLocation, Vector ray, int& triangleIndex, double* savedWeights)
{
	double collision_T = -1;

	for(int index = 0; index < num_triangles; index++)
	{
		Triangle triangle = triangles[index];

		Vector points[3];
		for(int i = 0; i < 3; i++)
		{
			points[i] = vertexToVector(triangle.v[i]);
		}

		Vector triangleNormal = (points[1] - points[0]).cross((points[2] - points[0])).normalize();

		double t_Denominator = ray.normalize().dot(triangleNormal.normalize());

		if(t_Denominator == 0)
		{
			//never intersects
			continue;
		}

		double t_Triangle = (points[0] - startingLocation).dot(triangleNormal) / t_Denominator;

		if(t_Triangle < 0)
		{
			continue;
		}

		Vector intersectLocation = ray * t_Triangle + startingLocation;

		double area = (points[1] - points[0]).cross((points[2] - points[0])).magnitude()*.5;
		double alpha = (points[1] - intersectLocation).cross((points[2] - intersectLocation)).magnitude() * .5 / area;
		double beta = (points[0] - intersectLocation).cross((points[2] - intersectLocation)).magnitude() * .5 / area;
		double gamma = (points[0] - intersectLocation).cross((points[1] - intersectLocation)).magnitude() * .5 / area;

		double epsilon = .0000000001;

		if(alpha >= epsilon && alpha <= 1 && beta >= epsilon && beta <= 1 && gamma >= epsilon && gamma <= 1 && ((alpha + beta + gamma) >= 1-epsilon && (alpha + beta + gamma) <= 1 + epsilon ) && t_Triangle > epsilon)
		{
			if(collision_T > 0)
			{
				if(t_Triangle < collision_T)
				{
					collision_T = t_Triangle;
					triangleIndex = index;
					savedWeights[0] = alpha;
					savedWeights[1] = beta;
					savedWeights[2] = gamma;
				}
			}
			else
			{
				collision_T = t_Triangle;
				triangleIndex = index;
				savedWeights[0] = alpha;
				savedWeights[1] = beta;
				savedWeights[2] = gamma;
			}
		}
	}
	return collision_T;
}

double checkSphereCollision(Vector startingLocation, Vector ray, int& sphereIndex)
{
	double collision_T = -1;

	for(int index = 0; index < num_spheres; index++)
	{
		Sphere sphere = spheres[index];
		Vector sphereLoc = makeVector(sphere.position[0],sphere.position[1],sphere.position[2]);
		double a = 1;
		double b = 2 * (ray.x * (startingLocation.x - sphereLoc.x) + ray.y * (startingLocation.y - sphereLoc.y) + ray.z * (startingLocation.z - sphereLoc.z));
		double c = (startingLocation.x - sphereLoc.x)*(startingLocation.x - sphereLoc.x) + (startingLocation.y - sphereLoc.y)*(startingLocation.y - sphereLoc.y) + (startingLocation.z - sphereLoc.z)*(startingLocation.z - sphereLoc.z) - sphere.radius * sphere.radius;
	
		double discriminant = b*b - 4 * a * c;

		if(discriminant < 0)
		{
			continue;
		}
		float t_0 = (-b + sqrt(discriminant))/(2*a);
		float t_1 = (-b - sqrt(discriminant))/(2*a);

		double sphere_T;

		if(t_0 >= EPSILON && t_1 >= EPSILON)
		{
			sphere_T = min(t_0,t_1);
		}
		else if(t_0 >= EPSILON)
		{
			sphere_T = t_0;
		}
		else if(t_1 >= EPSILON)
		{
			sphere_T = t_1;
		}
		else
		{
			continue;
		}

		if(sphere_T >= 0)
		{
			if(collision_T >= 0)
			{
				if(sphere_T < collision_T)
				{
					collision_T = sphere_T;
					sphereIndex = index;
				}
			}
			else
			{
				collision_T = sphere_T;
				sphereIndex = index;
			}
		}
	}

	return collision_T;
}

void castRay(int x, int y, bool debugMode)
{

	//generate rays
	double ar = 1.0*WIDTH/HEIGHT;

	double tanVal = tan(fov/2.0 * PI / 180.0);

	double rayX = 2.0 * ar * tanVal * x / (1.0*WIDTH) - ar * tanVal;
	double rayY = 2.0 * tanVal * y / (1.0*HEIGHT) - tanVal;
	double rayZ = -1;
	double d = sqrt(rayX * rayX + rayY * rayY + rayZ * rayZ);

	rayX /= d;
	rayY /= d;
	rayZ /= d;

	double t = -1;

	Triangle foundTriangle;
	Sphere foundSphere;
	bool triangleWasFound = false;
	bool sphereWasFound = false;

	double savedAlpha,savedBeta,savedGamma;


	int triangleIndex = 0;
	double savedWeight[3];

	t = checkTriangleCollision(makeVector(0,0,0),makeVector(rayX,rayY,rayZ),triangleIndex,savedWeight);
	if(t >= 0)
	{
		triangleWasFound = true;
		foundTriangle = triangles[triangleIndex];
		savedAlpha = savedWeight[0];
		savedBeta = savedWeight[1];
		savedGamma = savedWeight[2];
	}

	double saved_T = t;

	int sphereIndex = 0;

	t = checkSphereCollision(makeVector(0,0,0),makeVector(rayX,rayY,rayZ),sphereIndex);

	if(t >= 0)
	{
		sphereWasFound = true;
		foundSphere = spheres[sphereIndex];
	}

	if((t > saved_T && saved_T >= 0) || t < 0)
	{
		t = saved_T;
	}

	if(t >= 0)
	{
			Vector hitLocation = makeVector(rayX,rayY,rayZ) * t;

			double redVal = 0;
			double greenVal = 0;
			double blueVal = 0;

			//calc ambient light
			redVal = ambient_light[0];
			greenVal = ambient_light[1];
			blueVal = ambient_light[2];

			//Cast shadow rays
			for(int lightIndex = 0; lightIndex < num_lights; lightIndex++)
			{
				Light light = lights[lightIndex];
				Vector lightPosition;
				lightPosition.x = light.position[0];
				lightPosition.y = light.position[1];
				lightPosition.z = light.position[2];

				Vector rayToLight = (lightPosition - hitLocation);
				double rayToLightMagnitude = rayToLight.magnitude();
				rayToLight = rayToLight.normalize();

				double triangleHit = -1;
				double sphereHit = -1;

				int hitTriangle = -1;
				int throwawayVal = 0;
				double throwAwayArray[3];

				triangleHit = checkTriangleCollision(hitLocation, rayToLight,hitTriangle,throwAwayArray);

				sphereHit = checkSphereCollision(hitLocation,rayToLight,throwawayVal);

				double epsilon = .0000000000000001;

				if(debugMode)
				{
					cout <<"TRIANGLE HIT: "<< triangleHit << endl;
					cout <<"SPHERE HIT: "<< sphereHit << endl;
				}
				
				if(triangleHit > epsilon && triangleHit < rayToLightMagnitude + epsilon)
				{
					if(hitTriangle != triangleIndex && !sphereWasFound || sphereWasFound)
						continue;
				}
				if(sphereHit > epsilon && sphereHit < rayToLightMagnitude + epsilon)
				{
					continue;
				}

				if(sphereWasFound)
				{
					Vector sphereNormal = ((hitLocation - makeVector(foundSphere.position[0],foundSphere.position[1],foundSphere.position[2])) * (1.0/foundSphere.radius)).normalize();

					Vector toViewer = makeVector(-rayX,-rayY,-rayZ).normalize();

					Vector incoming = rayToLight;
					Vector perfectReflect = (sphereNormal * (2 * (incoming.normalize().dot(sphereNormal.normalize())))- incoming.normalize()).normalize();

					//Diffuse
					redVal += foundSphere.color_diffuse[0] * clamp(rayToLight.normalize().dot(sphereNormal.normalize())) * light.color[0];
					greenVal += foundSphere.color_diffuse[1] * clamp(rayToLight.normalize().dot(sphereNormal.normalize())) * light.color[1];
					blueVal += foundSphere.color_diffuse[2] * clamp(rayToLight.normalize().dot(sphereNormal.normalize())) * light.color[2];

					//Specular
					redVal += foundSphere.color_specular[0] * pow(clamp((perfectReflect.dot(toViewer))),foundSphere.shininess) * light.color[0];
					greenVal += foundSphere.color_specular[1] * pow(clamp((perfectReflect.dot(toViewer))),foundSphere.shininess) * light.color[1];
					blueVal += foundSphere.color_specular[2] * pow(clamp((perfectReflect.dot(toViewer))),foundSphere.shininess) * light.color[2];
				}
				else if(triangleWasFound)
				{
					double redColor = (savedAlpha * foundTriangle.v[0].color_diffuse[0] + savedBeta * foundTriangle.v[1].color_diffuse[0] + savedGamma * foundTriangle.v[2].color_diffuse[0]);
					double greenColor = (savedAlpha * foundTriangle.v[0].color_diffuse[1] + savedBeta * foundTriangle.v[1].color_diffuse[1] + savedGamma * foundTriangle.v[2].color_diffuse[1]);
					double blueColor = (savedAlpha * foundTriangle.v[0].color_diffuse[2] + savedBeta * foundTriangle.v[1].color_diffuse[2] + savedGamma * foundTriangle.v[2].color_diffuse[2]);

					double shiny = savedAlpha * foundTriangle.v[0].shininess + savedBeta * foundTriangle.v[1].shininess + savedGamma * foundTriangle.v[2].shininess;

					Vector pointNormal =  (makeVector(foundTriangle.v[0].normal) * savedAlpha + makeVector(foundTriangle.v[1].normal) * savedBeta + makeVector(foundTriangle.v[2].normal) * savedGamma).normalize();
					
					Vector toViewer = (makeVector(0,0,0) - hitLocation).normalize();

					Vector incoming = rayToLight;

					Vector perfectReflect = pointNormal * (2 * incoming.normalize().dot(pointNormal)) - rayToLight.normalize();

					//Diffuse
					redVal += redColor * clamp(rayToLight.normalize().dot(pointNormal.normalize())) * light.color[0];
					greenVal += greenColor * clamp(rayToLight.normalize().dot(pointNormal.normalize())) * light.color[1];
					blueVal += blueColor * clamp(rayToLight.normalize().dot(pointNormal.normalize())) * light.color[2];

					//Specular
					redVal += redColor * pow(clamp(perfectReflect.dot(toViewer)),shiny) * light.color[0];
					greenVal += greenColor * pow(clamp(perfectReflect.dot(toViewer)),shiny) * light.color[1];
					blueVal += blueColor * pow(clamp(perfectReflect.dot(toViewer)),shiny) * light.color[2];
				}
			}

			glPointSize(2.0);  
			glBegin(GL_POINTS);

			plot_pixel(x,y,clamp(redVal)*255,clamp(greenVal)*255,clamp(blueVal)*255);

			glEnd();
			glFlush();
	}
	else
	{
			glPointSize(2.0);  
			glBegin(GL_POINTS);
			plot_pixel(x,y,255,255,255);
			glEnd();
			glFlush();
	}
}

//MODIFY THIS FUNCTION
void draw_scene()
{
	for(int x = 0; x < WIDTH; x++)
	{
		for(int y = 0; y < HEIGHT; y++)
		{
			castRay(x,y,false);
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

void mouseButton(int button, int state, int x, int y)
{
	cout << "Mouse Click At: " <<x << " "<<(HEIGHT - y) << endl;
	castRay(x,HEIGHT-y,true);
	cout << "Shot ray"<<endl;
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
	glutMouseFunc(mouseButton);
	init();
	glutMainLoop();
}
