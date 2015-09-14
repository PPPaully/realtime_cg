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
    bool writeIFile = false;
    char* iFileType;
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

// Edge Detector
float sobel[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
vector<vector<float> > pixels[3];
vector<float> edgeX[3];
vector<float> edgeY[3];

// Pixel Map for Write to file Mode
vector<vector<vec3>> pMap;

//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;
int 		drawX = 0;
int 		drawY = 0;


void initScene() {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f); // Clear to black, fully transparent

	for(int i=0;i<3;i++) {
        edgeX[i].resize(viewport.w);
        edgeY[i].resize(viewport.w);
        pixels[i].resize(viewport.w);
        for(int x=0;x<viewport.w;x++) {
            pixels[i][x].resize(viewport.h);
            for(int y=0;y<viewport.h;y++)
                pixels[i][x][y] = 1.0f;
        }
	}
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

	for(int i=0;i<3;i++) {
        edgeX[i].resize(viewport.w);
        edgeY[i].resize(viewport.w);
        pixels[i].resize(viewport.w);
        for(int x=0;x<viewport.w;x++) {
            pixels[i][x].resize(viewport.h);
            for(int y=0;y<viewport.h;y++)
                pixels[i][x][y] = 1.0f;
        }
	}

	glViewport (0,0,viewport.w,viewport.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, viewport.w, 0, viewport.h);

    drawX = (int)(viewport.w * 0.5f);
    drawY = (int)(viewport.h * 0.5f);

}

////////////////////////////////////////////////////////////////////////////
// mode "ts" - ToonShade, "gl" - OpenGL, "pic" - save to file
////////////////////////////////////////////////////////////////////////////
void setPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b, string mode) {
    if(strcmp(mode.c_str(), "ts") == 0) {
        pixels[0][x][y] = r;
        pixels[1][x][y] = g;
        pixels[2][x][y] = b;
    } else if(strcmp(mode.c_str(), "pic") == 0) {
        pMap[x][y] = vec3(r,g,b);
    }
    else if(strcmp(mode.c_str(), "gl") == 0){
        glColor3f(r, g, b);
        glVertex2f(x+0.5, y+0.5);
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//  #####  ##  ##  ##### ###### ##### ##   ##    #####  ##  ##  ######  #####  #####
//  ##     ##  ##  ###     ##   ##  # ### ###    ###    ######  ##  ##  ##  #  ## #
//  ##     ##  ##    ###   ##   ##  # ## # ##      ###  ######  ######  #####  ###
//  #####  ######  #####   ##   ##### ##   ##    #####  ##  ##  ##  ##  ##     #####
//////////////////////////////////////////////////////////////////////////////////////
vec3 circleShape(float x,float y) {
    return vec3(x,y,sqrt(1.0f-x*x-y*y));
}

vec3 customShape(float u,float v) {
    float C = 4*2;
    float x = (sin(atan(u/v)*C));
    float y = (cos(atan(u/v)*C));
    float z = sin(u*u+v*v);
    return vec3(x,y,z);
}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// ######  #######  #######  ##   ##  #####  ##  ##  ######  ####   #####
//   ##    ##   ##  ##   ##  #### ##  ###    ######  ##  ##  ##  #  ## #
//   ##    ##   ##  ##   ##  ## ####    ###  ######  ######  ##  #  ###
//   ##    #######  #######  ##  ###  #####  ##  ##  ##  ##  ####   #####
//////////////////////////////////////////////////////////////////////////////
float cutColor(float x) {
    float tmp = floor(x*viewport.shade) / viewport.shade;
    if(tmp > 0.7f) return 1.0f;
    else return tmp;
}
vec3 toonShading(vec3 v) {
    vec3 tmp = vec3(cutColor(v.x),cutColor(v.y),cutColor(v.z));
    return tmp;
}
void edgeFilter(int maxX, int maxY,string drawMode) {
    for(int x=0; x<maxX; x++) {
        for(int y=0; y<3; y++) {
            edgeX[y][x] = 0.0f;
            edgeY[y][x] = 0.0f;
        }
    }
    for(int y=0; y+3 < maxY; y++) {
        for(int x=0; x+3 < maxX; x++) {
            for(int i=0; i<3; i++) {
                for(int j=0; j<3; j++) {
                    for(int k=0; k<3; k++) {
                        float avg = (pixels[0][x+k][y+i]+pixels[1][x+k][y+i]+pixels[2][x+k][y+i])/3;
                        edgeX[j][x+i] += sobel[k][i]*avg;
                        edgeY[j][x+i] += sobel[i][k]*avg;
                    }
                }
            }
            // Clear Black L/R-Edge
            edgeX[0][0] = edgeX[0][1] = 0.0f;
            edgeY[0][0] = edgeY[0][1] = 0.0f;
            edgeX[0][maxX-1] = edgeX[0][maxX-2] = 0.0f;
            edgeY[0][maxX-1] = edgeY[0][maxX-2] = 0.0f;

            for(int i=0; i<3; i++) {
                float e = (1-(sqrt(pow(edgeX[0][x+i],2)+pow(edgeY[0][x+i],2))));
                setPixel(x+i+0.5, y+0.5, e*pixels[0][x+i][y], e*pixels[1][x+i][y], e*pixels[2][x+i][y], drawMode);
                edgeX[0][x+i] = edgeX[1][x+i];
                edgeX[1][x+i] = edgeX[2][x+i];
                edgeX[2][x+i] = 0.0f;
                edgeY[0][x+i] = edgeY[1][x+i];
                edgeY[1][x+i] = edgeY[2][x+i];
                edgeY[2][x+i] = 0.0f;
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////
//  #####  ##  ##  ######  ####   #####  ####
//  ###    ######  ##  ##  ##  #  ## #   ##  #
//    ###  ######  ######  ##  #  ###    ####
//  #####  ##  ##  ##  ##  ####   #####  ##  #
/////////////////////////////////////////////////
vec3 computeShadedColor(vec3 pos) {
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

    if(viewport.toonShade)
        sum = toonShading(sum);
    return  sum;
}
void display() {
    cout << "Displaying.." << endl;
    string drawMode1 = viewport.toonShade ? "ts" :
                       viewport.writeIFile ? "pic" :
                       "gl";
    float scale = 1.0f;
	int drawRange = min(viewport.w, viewport.h)/2 - 10;  // Make it almost fit the entire window
	float drawRadius = scale/drawRange;

	for (int y = -drawRange; y <= drawRange; y++) {
		for (int x = -drawRange; x <= drawRange; x++) {
			vec3 pos = circleShape(x*drawRadius,y*drawRadius);
			if(pos.x != pos.x || pos.y != pos.y || pos.z != pos.z )
                continue;
			vec3 col = computeShadedColor(pos);

			// Set the red pixel
			setPixel(drawX + x, drawY + y, col.r, col.g, col.b, drawMode1);
		}
	}

	// Filtering
	if(viewport.toonShade) {
        cout << "Edge Filtering..." << endl;
        string drawMode2 = viewport.writeIFile ? "pic" : "gl";
        edgeFilter(viewport.w,viewport.h, drawMode2);
	}
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
// ##   ##  ####   ######  ######  #####    #####  ######  #####      #####  ######  ##     #####
// ## # ##  ##  #    ##      ##    ## #     ##  #    ##    ##         ##       ##    ##     ## #
// ### ###  ####     ##      ##    ###      #####    ##    ##         ####     ##    ##     ###
// ##   ##  ##  #  ######    ##    #####    ##     ######  ##### ##   ##     ######  #####  #####
///////////////////////////////////////////////////////////////////////////////////////////////////
//****************************************************
// compute the bmp map vector[y position][x position][0 = red, 1 = green, 2 = blue]
//****************************************************
vector<vector<vec3>> computePixelMap(int iwidth, int iheight) {
    pMap.resize(iwidth);
    for (int i = 0; i < iwidth; i++) {
        pMap[i].resize(iheight);
        for (int j = 0; j < iheight; j++) {
            pMap[i][j] = vec3(1.0f, 1.0f, 1.0f);
        }
    }

    display();

    return pMap;
}
void initVariable() {
    if(viewport.toonShade) {
        for(int i=0;i<3;i++) {
            edgeX[i].resize(viewport.w);
            edgeY[i].resize(viewport.w);
            pixels[i].resize(viewport.w);
            for(int x=0;x<viewport.w;x++) {
                pixels[i][x].resize(viewport.h);
                for(int y=0;y<viewport.h;y++)
                    pixels[i][x][y] = 1.0f;
            }
        }
    }

    drawX = (int)(viewport.w * 0.5f);
    drawY = (int)(viewport.h * 0.5f);

}
void writeImageFile(char* fileName, char* fileType) {
    int width = viewport.w;
    int height = viewport.h;
    initVariable();

    vector<vector<vec3>> pMap = computePixelMap(width, height);

    cout << "Image calculated.\n";

    char file[256]="";
    strcat(file,fileName);

    FILE* of;
    //ofstream ofs;
    int red, green, blue;

    if(strcmp(fileType, "BMP") == 0) {

        strcat(file,".bmp");
        of = fopen(file,"wb");
        //ofs.open(file);

        int extrabytes = 4 - ((width * 3) % 4); // Yhe size of which must be a multiple of 4 bytes.
        if (extrabytes == 4)
            extrabytes = 0;

        int paddedsize = ((width * 3) + extrabytes) * height;

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
        bmih.biWidth = width;
        bmih.biHeight = height;
        bmih.biPlanes = 1;
        bmih.biBitCount = 24;
        bmih.biCompression = 0;
        bmih.biSizeImage = paddedsize;

        fwrite(&bmfh, 1, sizeof(bmfh), of);
        //ofs.write((char*)(&bmfh), sizeof(BITMAPFILEHEADER));
        fwrite(&bmih, 1, sizeof(bmih), of);
        //ofs.write((char*)(&bmih), sizeof(BITMAPINFOHEADER));

        for (int y = height - 1; y >= 0; y--) { // BMP is written from bottom to top.
            for (int x = 0; x <= width - 1; x++) {

                red = pMap[x][height - y - 1].r * 255;
                green = pMap[x][height - y - 1].g * 255;
                blue = pMap[x][height - y - 1].b * 255;

                if (red > 255) red = 255;
                if (red < 0) red = 0;
                if (green > 255) green = 255;
                if (green < 0) green = 0;
                if (blue > 255) blue = 255;
                if (blue < 0) blue = 0;

                fprintf(of,"%c%c%c",blue,green,red);
                //ofs << (char)blue << (char)green << (char)red;  // BMP is written in (b,g,r) format.

            }
            if (extrabytes > 0) // BMP lines must be of lengths divisible by 4.
                for (int n = 1; n <= extrabytes; n++)
                    fprintf(of,"%c",0);
                    //ofs << (char)0;

        }
    } else if(strcmp(fileType, "PPM") == 0) {

        strcat(file,".ppm");
        of = fopen(file,"wb");
        //ofs.open(file);

        fprintf(of,"P3\n%d %d\n255\n",width,height);
        //ofs << "P3\n" << width << " " << height << "\n255\n"; // PPM header.

        for (int y = 0; y < height - 1; y++) { // PPM body
            for (int x = 0; x <= width - 1; x++) {

                red = pMap[x][height - y - 1].r * 255;
                green = pMap[x][height - y - 1].g * 255;
                blue = pMap[x][height - y - 1].b * 255;

                if (red > 255) red = 255;
                if (red < 0) red = 0;
                if (green > 255) green = 255;
                if (green < 0) green = 0;
                if (blue > 255) blue = 255;
                if (blue < 0) blue = 0;

                fprintf(of,"%d %d %d",blue,green,red);
                //ofs << red << " " << green << " " << blue << " ";
            }
            fprintf(of,"\n");
            //ofs << "\n";
        }

    } else  {

        strcat(file,".txt");
        of = fopen(file,"wb");
        //ofs.open(file);

        for (int y = 0; y < height - 1; y++) { // Pixel Matrix
            for (int x = 0; x <= width - 1; x++) {

                red = pMap[x][height - y - 1].r * 255;
                green = pMap[x][height - y - 1].g * 255;
                blue = pMap[x][height - y - 1].b * 255;

                if (red > 255) red = 255;
                if (red < 0) red = 0;
                if (green > 255) green = 255;
                if (green < 0) green = 0;
                if (blue > 255) blue = 255;
                if (blue < 0) blue = 0;

                fprintf(of,"%3d %3d %3d  ",blue,green,red);
                //ofs << red << " " << green << " " << blue << "   ";
            }
            fprintf(of,"\n");
            //ofs << "\n";
        }
    }

    fclose(of);
    //ofs.close();

    cout << "File: " << file << " created!\n";

}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {

    glClear(GL_COLOR_BUFFER_BIT);				// clear the color buffer

	// Start drawing sphere
	glBegin(GL_POINTS);

    // New draw function
	display();

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
		} else if(strcmp(argv[i], "-ts") == 0) {
            // ToonShading
            cout << "Toon Shade" << endl;
            viewport.toonShade = true;
            if(i+1 < argc && '0' <= argv[i+1][0] && argv[i+1][0] <= '9')
                viewport.shade = min(atoi(argv[++i]),1);
            i++;
		} else if ((strcmp(argv[i], "-bmp") == 0) || (strcmp(argv[i], "-ppm")) == 0) {
		    // Write to file
            cout << "Write to file" << endl;
            viewport.w = atoi(argv[i + 1]);
            viewport.h = atoi(argv[i + 2]);
            viewport.writeIFile = true;
            if (strcmp(argv[i], "-bmp") == 0) {
                // BMP
                viewport.iFileType = "BMP";
            } else {
                // PPM
                viewport.iFileType = "PPM";
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

    if (viewport.writeIFile) {
        writeImageFile("image", viewport.iFileType);
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
