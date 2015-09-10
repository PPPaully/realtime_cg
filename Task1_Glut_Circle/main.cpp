// Simple OpenGL example for CS184 F06 by Nuttapong Chentanez, modified from sample code for CS184 on Sp06
// Modified for Realtime-CG class

#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#define _WIN32

#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>
#include "algebra3.h"

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define PI 3.14159265358979323846

using namespace std;

//****************************************************
// Some Classes
//****************************************************

class Viewport;

class Viewport {
public:
	int w, h; // width and height
	int shade = 3; // shade of Toon Shading
	bool toonShade = false;
};

class Material{
public:
	vec3 ka; // Ambient color
	vec3 kd; // Diffuse color
	vec3 ks; // Specular color
	float sp; // Power coefficient of specular

	Material() : ka(0.0f), kd(0.0f), ks(0.0f), sp(0.0f) {
	}
};

class Light{
public:
	enum LIGHT_TYPE{POINT_LIGHT, DIRECTIONAL_LIGHT};

	vec3 posDir;  // Position (Point light) or Direction (Directional light)
	vec3 color;   // Color of the light
	LIGHT_TYPE type;

	Light() : posDir(0.0f), color(0.0f), type(POINT_LIGHT) {
	}
};

// Material and lights
Material material;
vector<Light> lights;
float pixels[1000][1000][3];
float sobel[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
float edgeX[1000][1000]={};
float edgeY[1000][1000]={};

//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;
int 		drawX = 0;
int 		drawY = 0;


void initScene(){
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f); // Clear to black, fully transparent
	for(int x=0;x<viewport.w;x++)
        for(int y=0;y<viewport.h;y++)
            pixels[x][y][0] = pixels[x][y][1] = pixels[x][y][2] = 1.0f;

	glViewport (0,0,viewport.w,viewport.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,viewport.w, 0, viewport.h);
}


//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
	viewport.w = w;
	viewport.h = h;

	glViewport (0,0,viewport.w,viewport.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, viewport.w, 0, viewport.h);

	drawX = (int)(viewport.w*0.5f);
	drawY = (int)(viewport.h*0.5f);

}

void setPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b) {
    if(viewport.toonShade) {
        pixels[x][y][0] = r;
        pixels[x][y][1] = g;
        pixels[x][y][2] = b;
    }
    else {
        glColor3f(r, g, b);
        glVertex2f(x+0.5, y+0.5);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//   ////////  ///////   ///////   //   //   //////   //   //   //////   ////    /////
//      //     //   //   //   //   //// //   ///      //   //   //  //   //  //  ///
//      //     //   //   //   //   // ////      ///   ///////   //////   //  //  ///
//      //     ///////   ///////   //  ///   //////   //   //   //  //   ////    /////
//////////////////////////////////////////////////////////////////////////////////////////////

float cutColor(float x) {
    return floor(x*viewport.shade) / viewport.shade;
}

vec3 toonShading(vec3 v,vec3 pos) {
    vec3 tmp = vec3(cutColor(v.x),cutColor(v.y),cutColor(v.z));
//    if(abs(v.x - tmp.x) >= 0.3)
//        tmp = 0.2 * tmp;
    return tmp;
}

vec3 computeShadedColor(vec3 pos) {
	// TODO: Your shading code mostly go here

//	return vec3(0.1f, 0.1f, 0.1f);
    vec3 sum = vec3(0,0,0);
    for(int i = 0; i < lights.size(); i++) {
        vec3 n = pos.normalize();
        vec3 l = lights[i].type == Light::DIRECTIONAL_LIGHT ? lights[i].posDir.normalize() : (lights[i].posDir - pos).normalize();
        vec3 lc = lights[i].color;
        vec3 r = (2*(l*n)*n-l).normalize();
        vec3 v = vec3(0,0,1);

        vec3 amb = vec3(material.ka.r*lc.r,material.ka.g*lc.g,material.ka.b*lc.b);
        vec3 dif = vec3(material.kd.r*lc.r,material.kd.g*lc.g,material.kd.b*lc.b)*max(n*l,0.0f);
        vec3 spc = vec3(material.ks.r*lc.r,material.ks.g*lc.g,material.ks.b*lc.b)*pow(max(r*v,0.0f),material.sp);
        sum += amb + dif + spc;
    }
    if(viewport.toonShade)sum = toonShading(sum,pos);
    return  sum;
}

void edgeFilter(int maxX, int maxY) {
    for(int x=0; x<maxX; x++) {
        for(int y=0; y<maxY; y++) {
            edgeX[x][y] = 0.0f;
            edgeY[x][y] = 0.0f;
        }
    }
    for(int x=0; x+3 < maxX; x++) {
        for(int y=0; y+3 < maxY; y++) {
            for(int i=0; i<3; i++) {
                for(int j=0; j<3; j++) {
                    for(int k=0; k<3; k++) {
                        float avg = (pixels[x+k][y+i][0]+pixels[x+k][y+i][1]+pixels[x+k][y+i][2])/3;
                        edgeX[x+i][y+j] += sobel[k][i]*avg;
                        edgeY[x+i][y+j] += sobel[i][k]*avg;
                    }
                }
            }
            for(int i=0; i<3; i++) {
                for(int j=0; j<3; j++) {
                    float e = (1-(sqrt(pow(edgeX[x+i][y+j],2)+pow(edgeY[x+i][y+j],2))));
//                    glColor3f(e,e,e);
//                    glColor3f(min(e,pixels[x+i][y+j][0]), min(e,pixels[x+i][y+j][1]), min(e,pixels[x+i][y+j][2]));
                    glColor3f(e*pixels[x+i][y+j][0], e*pixels[x+i][y+j][1], e*pixels[x+i][y+j][2]);
                    glVertex2f(x+i+0.5, y+j+0.5);
                }
            }
        }
    }
}
//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {

	glClear(GL_COLOR_BUFFER_BIT);				// clear the color buffer

	glMatrixMode(GL_MODELVIEW);					// indicate we are specifying camera transformations
	glLoadIdentity();							// make sure transformation is "zero'd"


	int drawRadius = min(viewport.w, viewport.h)/2 - 10;  // Make it almost fit the entire window
	float idrawRadius = 1.0f / drawRadius;
	// Start drawing sphere
	glBegin(GL_POINTS);

	for (int i = -drawRadius; i <= drawRadius; i++) {
		int width = floor(sqrt((float)(drawRadius*drawRadius-i*i)));
		for (int j = -width; j <= width; j++) {

			// Calculate the x, y, z of the surface of the sphere
			float x = j * idrawRadius;
			float y = i * idrawRadius;
			float z = sqrtf(1.0f - x*x - y*y);
			vec3 pos(x,y,z); // Position on the surface of the sphere

			vec3 col = computeShadedColor(pos);

			// Set the red pixel
			setPixel(drawX + j, drawY + i, col.r, col.g, col.b);
		}
	}
	if(viewport.toonShade)
        edgeFilter(viewport.w,viewport.h);
	glEnd();

	glFlush();
	glutSwapBuffers();					// swap buffers (we earlier set double buffer)

//    shade = shade < 1 ? 20 : shade - 1;
}


//****************************************************
// for updating the position of the circle
//****************************************************

void myFrameMove() {
	float dt;
	// Compute the time elapsed since the last time the scence is redrawn
#ifdef _WIN32
	DWORD currentTime = GetTickCount();
	dt = (float)(currentTime - lastTime)*0.001f;
#else
	timeval currentTime;
	gettimeofday(&currentTime, NULL);
	dt = (float)((currentTime.tv_sec - lastTime.tv_sec) + 1e-6*(currentTime.tv_usec - lastTime.tv_usec));
#endif

	// Store the time
	lastTime = currentTime;
	glutPostRedisplay();
}


void parseArguments(int argc, char* argv[]) {
	int i = 1;
	while (i < argc) {
		if (strcmp(argv[i], "-ka") == 0) {
			// Ambient color
			material.ka.r = (float)atof(argv[i+1]);
			material.ka.g = (float)atof(argv[i+2]);
			material.ka.b = (float)atof(argv[i+3]);
			i+=4;
		} else
		if (strcmp(argv[i], "-kd") == 0) {
			// Diffuse color
			material.kd.r = (float)atof(argv[i+1]);
			material.kd.g = (float)atof(argv[i+2]);
			material.kd.b = (float)atof(argv[i+3]);
			i+=4;
		} else
		if (strcmp(argv[i], "-ks") == 0) {
			// Specular color
			material.ks.r = (float)atof(argv[i+1]);
			material.ks.g = (float)atof(argv[i+2]);
			material.ks.b = (float)atof(argv[i+3]);
			i+=4;
		} else
		if (strcmp(argv[i], "-sp") == 0) {
			// Specular power
			material.sp = (float)atof(argv[i+1]);
			i+=2;
		} else
		if ((strcmp(argv[i], "-pl") == 0) || (strcmp(argv[i], "-dl") == 0)){
			Light light;
			// Specular color
			light.posDir.x = (float)atof(argv[i+1]);
			light.posDir.y = (float)atof(argv[i+2]);
			light.posDir.z = (float)atof(argv[i+3]);
			light.color.r = (float)atof(argv[i+4]);
			light.color.g = (float)atof(argv[i+5]);
			light.color.b = (float)atof(argv[i+6]);
			if (strcmp(argv[i], "-pl") == 0) {
				// Point
				light.type = Light::POINT_LIGHT;
			} else {
				// Directional
				light.type = Light::DIRECTIONAL_LIGHT;
			}
			lights.push_back(light);
			i+=7;
		}
		else if(strcmp(argv[i], "-ts") == 0) {
            // ToonShading
            viewport.toonShade = true;
            if(i+1 < argc && '0' <= argv[i+1][0] && argv[i+1][0] <= '9')
                viewport.shade = min(atoi(argv[++i]),1);
            i++;
		}
	}
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {

	parseArguments(argc, argv);

  	//This initializes glut
  	glutInit(&argc, argv);

  	//This tells glut to use a double-buffered window with red, green, and blue channels
  	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  	// Initalize theviewport size
  	viewport.w = 800;
  	viewport.h = 800;

  	//The size and position of the window
  	glutInitWindowSize(viewport.w, viewport.h);
  	glutInitWindowPosition(0,0);
  	glutCreateWindow(argv[0]);

   	// Initialize timer variable
	#ifdef _WIN32
	lastTime = GetTickCount();
	#else
	gettimeofday(&lastTime, NULL);
	#endif

  	initScene();							// quick function to set up scene

  	glutDisplayFunc(myDisplay);					// function to run when its time to draw something
  	glutReshapeFunc(myReshape);					// function to run when the window gets resized
  	glutIdleFunc(myFrameMove);

  	glutMainLoop();							// infinite loop that will keep drawing and resizing and whatever else

  	return 0;
}
