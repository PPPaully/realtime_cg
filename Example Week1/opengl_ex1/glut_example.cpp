// Simple OpenGL example for Realtime Graphics and Physics class
// Modified from UC Berkeley's CS184 sp08 by Trevor Standley, modified from sample code for CS184 on Sp06
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#include <windows.h>

#include <GL/glut.h>
#include <GL/glu.h>

#include <time.h>
#include <math.h>

static DWORD lastTime;

#define PI 3.14159265

using namespace std;

//****************************************************
// Some Classes
//****************************************************
class Viewport {
public:
	int w, h; // width and height
};


//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;

//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
	viewport.w = w;
	viewport.h = h;

	glViewport(0,0,viewport.w,viewport.h);// sets the rectangle that will be the window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// loading the identity matrix for the screen

	//----------- setting the projection -------------------------
	// glOrtho sets left, right, bottom, top, zNear, zFar of the chord system


	// glOrtho(-1, 1 + (w-400)/200.0 , -1 -(h-400)/200.0, 1, 1, -1); // resize type = add
	// glOrtho(-w/400.0, w/400.0, -h/400.0, h/400.0, 1, -1); // resize type = center

	glOrtho(-1, 1, -1, 1, -1, 1);	// resize type = stretch

	//------------------------------------------------------------
}


//****************************************************
// sets the window up
//****************************************************
void initScene(){
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent

	myReshape(viewport.w,viewport.h);
}

//***************************************************
// function that does the actual drawing
//***************************************************
void myDisplay() {

	glClear(GL_COLOR_BUFFER_BIT);				// clear the color buffer (sets everything to black)

	glMatrixMode(GL_MODELVIEW);					// indicate we are specifying camera transformations
	glLoadIdentity();							// make sure transformation is "zero'd"

	//----------------------- code to draw objects --------------------------
							// Rectangle Code
	//glColor3f(red component, green component, blue component);
	glColor3f(1.0f,0.0f,0.0f);					// setting the color to pure red 90% for the rect

	glBegin(GL_POLYGON);						// draw rectangle
		//glVertex3f(x val, y val, z val (won't change the point because of the projection type));
//		glVertex3f(-0.8f, 0.0f, 0.0f);			// bottom left corner of rectangle
//		glVertex3f(-0.8f, 0.5f, 0.0f);			// top left corner of rectangle
//		glVertex3f( 0.0f, 0.5f, 0.0f);			// top right corner of rectangle
//		glVertex3f( 0.0f, 0.0f, 0.0f);			// bottom right corner of rectangle

    float x = -0.5f;
    float y = 0.0f;
    float radius = 0.4f;
    int angleDivision = 6;
    float angleIncrement = 2.0f*PI / angleDivision;
    for (int i = 0; i <= angleDivision; i++)
	{
		float radian = angleIncrement * i;
		glVertex3f(x + cos(radian)*radius, y + sin(radian)*radius, 0.0f);
	}

	glEnd();
							// Triangle Code
//	glColor3f(1.0f,1.0f,0.0f);					// setting the color to orange for the triangle

	float basey = -sqrt(0.48f);					// height of triangle = sqrt(.8^2-.4^2)
//		glVertex3f(0.5f,  0.0f, 0.0f);			// top tip of triangle
//		glVertex3f(0.1f, basey, 0.0f);			// lower left corner of triangle
//		glVertex3f(0.9f, basey, 0.0f);			// lower right corner of triangle


	glBegin(GL_TRIANGLES);
    glColor3f(1.0f,1.0f,0.0f);

    x = 0.5f;
    y = 0.0f;
    radius = 0.4f;
    float radius2 = 0.2f;
    angleDivision = 10;
    angleIncrement = 2.0f*PI / angleDivision;

    static float spin = 0.01f;
    spin += 0.01f;

    for (int i = 0; i <= angleDivision; i++)
	{
	    glVertex3f(x, y, 0.0f);
		float radian = angleIncrement * i + spin;
		glVertex3f(x + cos(radian)*radius, y + sin(radian)*radius, 0.0f);
		glVertex3f(x + cos(radian + PI/angleDivision)*radius2, y + sin(radian + PI/angleDivision)*radius2, 0.0f);
	}

//	for (int i = 0; i <= angleDivision; i++)
//	{
//	    glVertex3f(x, y, 0.0f);
//		float radian = angleIncrement * i;
//		glVertex3f(x + cos(radian)*radius, y + sin(radian)*radius, 0.0f);
//		glVertex3f(x + cos(radian - angleIncrement + PI/angleDivision)*radius2, y + sin(radian - angleIncrement + PI/angleDivision)*radius2, 0.0f);
//	}

	glEnd();

//	glBegin(GL_POLYGON);
//    glColor3f(1.0f,0.0f,0.0f);
//
//    x = 0.5f;
//    y = 0.0f;
//    radius = 0.4f;
//    radius2 = 0.2f;
//    angleDivision = 5;
//    angleIncrement = 2.0f*PI / angleDivision;
//    for (int i = 0; i <= angleDivision; i++)
//	{
////		float radian = angleIncrement * i + PI/2;
////		glVertex3f(x + cos(radian)*radius, y + sin(radian)*radius, 0.0f);
//		float radian2 = angleIncrement * i + PI/5;
//		glVertex3f(x + cos(radian2)*radius2, y + sin(radian2)*radius2, 0.0f);
//	}
//
//	glEnd();
	//-----------------------------------------------------------------------

	glFlush();
	glutSwapBuffers();					// swap buffers (we earlier set double buffer)
}


//****************************************************
// called by glut when there are no messages to handle
//****************************************************
void myFrameMove() {
	//nothing here for now
#ifdef _WIN32
	Sleep(10);						//give ~10ms back to OS (so as not to waste the CPU)
#endif
	glutPostRedisplay(); // forces glut to call the display function (myDisplay())
}


//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
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
	glutCreateWindow("Real Time Graphics: Example 1");

  	initScene();							// quick function to set up scene

  	glutDisplayFunc(myDisplay);				// function to run when its time to draw something
  	glutReshapeFunc(myReshape);				// function to run when the window gets resized
  	glutIdleFunc(myFrameMove);				// function to run when not handling any other task
  	glutMainLoop();							// infinite loop that will keep drawing and resizing and whatever else

  	return 0;
}








