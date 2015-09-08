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
};

class Material {
public:
    vec3 ka; // Ambient color
    vec3 kd; // Diffuse color
    vec3 ks; // Specular color
    float sp; // Power coefficient of specular

    Material() : ka(0.0f), kd(0.0f), ks(0.0f), sp(0.0f) {
    }
};

class Light {
public:
    enum LIGHT_TYPE {POINT_LIGHT, DIRECTIONAL_LIGHT};

    vec3 posDir;  // Position (Point light) or Direction (Directional light)
    vec3 color;   // Color of the light
    LIGHT_TYPE type;

    Light() : posDir(0.0f), color(0.0f), type(POINT_LIGHT) {
    }
};

// Material and lights
Material material;
vector<Light> lights;

//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;
int 		drawX = 0;
int 		drawY = 0;
int         captureWidth = 0;
int         captureHeight = 0;
int         isCapture = 0;


void initScene() {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f); // Clear to black, fully transparent

    glViewport (0, 0, viewport.w, viewport.h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, viewport.w, 0, viewport.h);
}


//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
    viewport.w = w;
    viewport.h = h;

    glViewport (0, 0, viewport.w, viewport.h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, viewport.w, 0, viewport.h);

    drawX = (int)(viewport.w * 0.5f);
    drawY = (int)(viewport.h * 0.5f);

}

void setPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b) {
    glColor3f(r, g, b);
    glVertex2f(x + 0.5, y + 0.5);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//   ////////  ///////   ///////   //   //   //////   //   //   //////   ////    /////
//      //     //   //   //   //   //// //   ///      //   //   //  //   //  //  ///
//      //     //   //   //   //   // ////      ///   ///////   //////   //  //  ///
//      //     ///////   ///////   //  ///   //////   //   //   //  //   ////    /////
//////////////////////////////////////////////////////////////////////////////////////////////

int shade = 3;

float cutColor(float x) {
    float y = 100.0f / shade;
    if((x * 100.0f) / y - floor((x * 100.0f) / y) >= 0.48 && (x * 100.0f) / y - floor((x * 100.0f) / y) <= 0.52)
        return 0.7 * x;
    return ((x * 100.0f) / y - floor((x * 100.0f) / y) >= 0.5 ? ceil((x * 100.0f) / y) : floor((x * 100.0f) / y) ) * y / 100.0f;
}

vec3 toonShading(vec3 v, vec3 pos) {
    vec3 tmp = vec3(cutColor(v.x), cutColor(v.y), cutColor(v.z));

    if(0 <= pos.normalize().z && pos.normalize().z <= 0.2)
        tmp = 0.2 * tmp;

//    if(abs(v.x - tmp.x) >= 0.3)
//        tmp = 0.2 * tmp;

    return tmp;
}

vec3 computeShadedColor(vec3 pos) {
    // TODO: Your shading code mostly go here

//	return vec3(0.1f, 0.1f, 0.1f);
    vec3 sum = vec3(0, 0, 0);
    for(int i = 0; i < lights.size(); i++) {
        vec3 n = pos.normalize();
        vec3 l = lights[i].type == Light::DIRECTIONAL_LIGHT ? lights[i].posDir.normalize() : (lights[i].posDir - pos).normalize();
        vec3 lc = lights[i].color;
        vec3 r = (2 * (l * n) * n - l).normalize();
        vec3 v = vec3(0, 0, 1);

        vec3 amb = vec3(material.ka.r * lc.r, material.ka.g * lc.g, material.ka.b * lc.b);
        vec3 dif = vec3(material.kd.r * lc.r, material.kd.g * lc.g, material.kd.b * lc.b) * max(n * l, 0.0f);
        vec3 spc = vec3(material.ks.r * lc.r, material.ks.g * lc.g, material.ks.b * lc.b) * pow(max(r * v, 0.0f), material.sp);
        sum += amb + dif + spc;
    }
    //sum = toonShading(sum,pos);
    return  sum;
}

//****************************************************
// compute the bmp map vector[y position][x position][0 = red, 1 = green, 2 = blue]
//****************************************************
vector<vector<vec3>> computePixelMap(int iwidth, int iheight) {
    vector<vector<vec3>> pMap;
    pMap.resize(iheight);
    for (int i = 0; i < iheight; i++) {
        pMap[i].resize(iwidth);
        for (int j = 0; j < iwidth; j++) {
            pMap[i][j] = vec3(1.0f, 1.0f, 1.0f);
        }
    }

    int drawRadius = min(iwidth, iheight) / 2 - 10; // Make it almost fit the entire window
    float idrawRadius = 1.0f / drawRadius;



    for (int i = -drawRadius; i <= drawRadius; i++) {
        int width = floor(sqrt((float)(drawRadius * drawRadius - i * i)));
        for (int j = -width; j <= width; j++) {

            // Calculate the x, y, z of the surface of the sphere
            float x = j * idrawRadius;
            float y = i * idrawRadius;
            float z = sqrtf(1.0f - x * x - y * y);
            vec3 pos(x, y, z); // Position on the surface of the sphere

            vec3 col = computeShadedColor(pos);

            // Set the red pixel
            pMap[j + iheight / 2][i + iwidth / 2] = col;
        }
    }
    return pMap;
}

//****************************************************
// Write BMP & PPM file
//****************************************************
void writeImageFile(char* fileName, char* fileType, int WIDTH, int HEIGHT) {

    vector<vector<vec3>> pMap = computePixelMap(WIDTH, HEIGHT);

    cout << "Image calculated.\n";

    ofstream ofs;
    ofs.open(fileName);

    int red, green, blue;

    if(strcmp(fileType, "BMP") == 0) {

        int extrabytes = 4 - ((WIDTH * 3) % 4); // Yhe size of which must be a multiple of 4 bytes.
        if (extrabytes == 4)
            extrabytes = 0;

        int paddedsize = ((WIDTH * 3) + extrabytes) * HEIGHT;

        // BMP Header.

        BITMAPFILEHEADER bmfh;
        bmfh.bfType = 0x4d42;
        bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paddedsize;
        bmfh.bfReserved1 = 0;
        bmfh.bfReserved2 = 0;
        bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        BITMAPINFOHEADER bmih;
        memset(&bmih, 0, 40);
        bmih.biSize = sizeof(BITMAPINFOHEADER);
        bmih.biWidth = WIDTH;
        bmih.biHeight = HEIGHT;
        bmih.biPlanes = 1;
        bmih.biBitCount = 24;
        bmih.biCompression = 0;
        bmih.biSizeImage = paddedsize;

        ofs.write((char*)(&bmfh), sizeof(BITMAPFILEHEADER));
        ofs.write((char*)(&bmih), sizeof(BITMAPINFOHEADER));

        for (int y = HEIGHT - 1; y >= 0; y--) { // BMP is written from bottom to top.
            for (int x = 0; x <= WIDTH - 1; x++) {

                red = pMap[x][HEIGHT - y - 1].r * 255;
                green = pMap[x][HEIGHT - y - 1].g * 255;
                blue = pMap[x][HEIGHT - y - 1].b * 255;

                if (red > 255) red = 255;
                if (red < 0) red = 0;
                if (green > 255) green = 255;
                if (green < 0) green = 0;
                if (blue > 255) blue = 255;
                if (blue < 0) blue = 0;

                ofs << (char)blue << (char)green << (char)red;  // BMP is written in (b,g,r) format.

            }
            if (extrabytes > 0) // BMP lines must be of lengths divisible by 4.
                for (int n = 1; n <= extrabytes; n++)
                    ofs << (char)0;

        }
    } else if(strcmp(fileType, "PPM") == 0) {

        ofs << "P3\n" << WIDTH << " " << HEIGHT << "\n255\n"; // PPM header.

        for (int y = 0; y < HEIGHT - 1; y++) { // PPM body
            for (int x = 0; x <= WIDTH - 1; x++) {

                red = pMap[x][HEIGHT - y - 1].r * 255;
                green = pMap[x][HEIGHT - y - 1].g * 255;
                blue = pMap[x][HEIGHT - y - 1].b * 255;

                if (red > 255) red = 255;
                if (red < 0) red = 0;
                if (green > 255) green = 255;
                if (green < 0) green = 0;
                if (blue > 255) blue = 255;
                if (blue < 0) blue = 0;

                ofs << red << " " << green << " " << blue << " ";
            }
            ofs << "\n";
        }

    } else  {

        for (int y = 0; y < HEIGHT - 1; y++) { // Pixel Matrix
            for (int x = 0; x <= WIDTH - 1; x++) {

                red = pMap[x][HEIGHT - y - 1].r * 255;
                green = pMap[x][HEIGHT - y - 1].g * 255;
                blue = pMap[x][HEIGHT - y - 1].b * 255;

                if (red > 255) red = 255;
                if (red < 0) red = 0;
                if (green > 255) green = 255;
                if (green < 0) green = 0;
                if (blue > 255) blue = 255;
                if (blue < 0) blue = 0;

                ofs << red << " " << green << " " << blue << "   ";
            }
            ofs << "\n";
        }
    }

    ofs.close();

    cout << "File: " << fileName << " created!\n";

}
//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {

    glClear(GL_COLOR_BUFFER_BIT);				// clear the color buffer

    glMatrixMode(GL_MODELVIEW);					// indicate we are specifying camera transformations
    glLoadIdentity();							// make sure transformation is "zero'd"


    int drawRadius = min(viewport.w, viewport.h) / 2 - 10; // Make it almost fit the entire window
    float idrawRadius = 1.0f / drawRadius;
    // Start drawing sphere
    glBegin(GL_POINTS);

    for (int i = -drawRadius; i <= drawRadius; i++) {
        int width = floor(sqrt((float)(drawRadius * drawRadius - i * i)));
        for (int j = -width; j <= width; j++) {

            // Calculate the x, y, z of the surface of the sphere
            float x = j * idrawRadius;
            float y = i * idrawRadius;
            float z = sqrtf(1.0f - x * x - y * y);
            vec3 pos(x, y, z); // Position on the surface of the sphere

            vec3 col = computeShadedColor(pos);

            // Set the red pixel
            setPixel(drawX + j, drawY + i, col.r, col.g, col.b);
        }
    }
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
    dt = (float)(currentTime - lastTime) * 0.001f;
#else
    timeval currentTime;
    gettimeofday(&currentTime, NULL);
    dt = (float)((currentTime.tv_sec - lastTime.tv_sec) + 1e-6 * (currentTime.tv_usec - lastTime.tv_usec));
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
            material.ka.r = (float)atof(argv[i + 1]);
            material.ka.g = (float)atof(argv[i + 2]);
            material.ka.b = (float)atof(argv[i + 3]);
            i += 4;
        } else if (strcmp(argv[i], "-kd") == 0) {
            // Diffuse color
            material.kd.r = (float)atof(argv[i + 1]);
            material.kd.g = (float)atof(argv[i + 2]);
            material.kd.b = (float)atof(argv[i + 3]);
            i += 4;
        } else if (strcmp(argv[i], "-ks") == 0) {
            // Specular color
            material.ks.r = (float)atof(argv[i + 1]);
            material.ks.g = (float)atof(argv[i + 2]);
            material.ks.b = (float)atof(argv[i + 3]);
            i += 4;
        } else if (strcmp(argv[i], "-sp") == 0) {
            // Specular power
            material.sp = (float)atof(argv[i + 1]);
            i += 2;
        } else if ((strcmp(argv[i], "-pl") == 0) || (strcmp(argv[i], "-dl") == 0)) {
            Light light;
            // Specular color
            light.posDir.x = (float)atof(argv[i + 1]);
            light.posDir.y = (float)atof(argv[i + 2]);
            light.posDir.z = (float)atof(argv[i + 3]);
            light.color.r = (float)atof(argv[i + 4]);
            light.color.g = (float)atof(argv[i + 5]);
            light.color.b = (float)atof(argv[i + 6]);
            if (strcmp(argv[i], "-pl") == 0) {
                // Point
                light.type = Light::POINT_LIGHT;
            } else {
                // Directional
                light.type = Light::DIRECTIONAL_LIGHT;
            }
            lights.push_back(light);
            i += 7;
        } else if ((strcmp(argv[i], "-bmp") == 0) || (strcmp(argv[i], "-ppm")) == 0) {
            captureWidth = atoi(argv[i + 1]);
            captureHeight = atoi(argv[i + 2]);
            isCapture = 1;
            if (strcmp(argv[i], "-bmp") == 0) {
                // BMP
                isCapture = 1;
            } else {
                // PPM
                isCapture = 2;
            }
            i += 3;
        }
    }
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {

    parseArguments(argc, argv);

    if (isCapture == 1) {
        writeImageFile("image.bmp", "BMP", captureWidth, captureHeight);
        return 0;
    } else if (isCapture == 2) {
        writeImageFile("image.ppm", "PPM", captureWidth, captureHeight);
        return 0;
    }

    //This initializes glut
    glutInit(&argc, argv);

    //This tells glut to use a double-buffered window with red, green, and blue channels
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Initalize theviewport size
    viewport.w = 800;
    viewport.h = 800;

    //The size and position of the window
    glutInitWindowSize(viewport.w, viewport.h);
    glutInitWindowPosition(0, 0);
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
