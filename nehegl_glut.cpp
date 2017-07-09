//========================================================================
//    NeHe OpenGL Wizard : nehegl_glut.cpp
//    Wizard Created by: Vic Hollis
//========================================================================
/***********************************************
*                                              *
*         Bruce's GLUT OpenGL Basecode         *
*  Specially made for Nehe's Gamedev Website   *
*            http://nehe.gamedev.net           *
*                April 2003                    *
*                                              *
************************************************

/****************************************
*										*
*			OPENCV POSIT TUTORIAL		*
*		Javier Barandiaran Martirena	*
*			jbarandiaran@gmail.com		*
*			       2009					*
*										*
****************************************/

#ifdef WIN32
#include <windows.h>
#endif
#include <GLUT/glut.h>
#include <iostream>
#include "POSIT.h"

using namespace std;

/* creates a enum type for mouse buttons */
enum {
    BUTTON_LEFT = 0,
    BUTTON_RIGHT,
    BUTTON_LEFT_TRANSLATE,
};

/* set global variables */
int mButton = -1;
int mOldY, mOldX;

/* windows size and position constants */
const int GL_WIN_INITIAL_WIDTH = 700;
const int GL_WIN_INITIAL_HEIGHT = 700;
const int GL_WIN_INITIAL_X = 0;
const int GL_WIN_INITIAL_Y = 0;
const int GL_NEAR = 1.0;
const int GL_FAR = 1000.0;
const int ESC = 27;

/* vectors that makes the rotation and translation of the cube */
float mEye[3] = {0.0f, 0.0f, 100.0f};
float mRot[3] = {45.0f, 45.0f, 0.0f};

////POSIT
int mWinWidth = GL_WIN_INITIAL_WIDTH;
int mWinHeight = GL_WIN_INITIAL_HEIGHT;
double mCubeSize = 10.0f;
POSIT mPosit;

// Window resize function
void glutResize(int width, int height)
{
	mWinWidth = width;
	mWinHeight = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	mPosit.initializeIntrinsics( width,height);
	mPosit.createOpenGLProjectionMatrix( width, height, 1.0, 1000.0 );
}

// Function that handles keyboard inputs
void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case ESC:
			exit(0);
	}
}

// If rotation angle is greater of 360 or lesser than -360,
// resets it back to zero.
void clamp(float *v)
{
    int i;

    for (i = 0; i < 3; i ++)
        if (v[i] > 360 || v[i] < -360)
            v[i] = 0;
}

// Moves the screen based on mouse pressed button
void glutMotion(int x, int y)
{
	if (mButton == BUTTON_LEFT)
	{
		mRot[0] -= (mOldY - y);
		mRot[1] -= (mOldX - x);
		clamp (mRot);
	}
	else if (mButton == BUTTON_RIGHT)
		mEye[2] -= (mOldY - y) * 0.1f;
    else if (mButton == BUTTON_LEFT_TRANSLATE)
    {
		mEye[0] -= (mOldX - x) * 0.1f;
		mEye[1] += (mOldY - y) * 0.1f;
    }

	mPosit.buildRealPose( mRot, mEye );
	mPosit.projectModelPoints( mPosit.poseReal, mPosit.srcImagePoints );
	mPosit.poseEstimation();
	mPosit.projectModelPoints( mPosit.posePOSIT, mPosit.estimatedImagePoints );
	mOldX = x;
    mOldY = y;
	glutPostRedisplay();
}

//------------------------------------------------------------------------
// Function that handles mouse input
//------------------------------------------------------------------------
void glutMouse(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN)
    {
        mOldX = x;
        mOldY = y;
        switch(button)
        {
			case GLUT_LEFT_BUTTON:
				if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
				{
					mButton = BUTTON_LEFT_TRANSLATE;
					break;
				} 
				else
				{
					mButton = BUTTON_LEFT;
					break;
				}
			case GLUT_RIGHT_BUTTON:
				mButton = BUTTON_RIGHT;
				break;
        }
    }
	else if (state == GLUT_UP)
		mButton = -1;
}

void renderCube(float size)
{
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f( 0.0f,  0.0f,  0.0f);
		glVertex3f( size,  0.0f,  0.0f);
		glVertex3f( size,  size,  0.0f);
		glVertex3f( 0.0f,  size,  0.0f);
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glVertex3f( 0.0f,  0.0f, size);
		glVertex3f( 0.0f,  size, size);
		glVertex3f( size,  size, size);
		glVertex3f( size,  0.0f, size);
		// Top Face
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glVertex3f( 0.0f,  size,  0.0f);
		glVertex3f( size,  size,  0.0f);
		glVertex3f( size,  size, size);
		glVertex3f( 0.0f,  size, size);
		// Bottom Face
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glVertex3f( 0.0f,  0.0f,  0.0f);
		glVertex3f( 0.0f,  0.0f, size);
		glVertex3f( size,  0.0f, size);
		glVertex3f( size,  0.0f,  0.0f);
		// Right face
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( size,  0.0f, 0.0f);
		glVertex3f( size,  0.0f, size);
		glVertex3f( size,  size, size);
		glVertex3f( size,  size, 0.0f);
		// Left Face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f( 0.0f,  0.0f, 0.0f);
		glVertex3f( 0.0f,  size, 0.0f);
		glVertex3f( 0.0f,  size, size);
		glVertex3f( 0.0f,  0.0f, size);
    glEnd();
}

void renderAxis(float size)
{
	glBegin(GL_LINES);
		glColor3f(1.0,0.0,0.0);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(size, 0.0f, 0.0f);
		glColor3f(0.0,1.0,0.0);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, size, 0.0f);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, size);
	glEnd();
}

void drawCross( float x, float y, float size )
{
	glBegin( GL_LINES );
		glVertex2f( x-size, y+size );
		glVertex2f( x+size, y-size );
		glVertex2f( x-size, y-size );
		glVertex2f( x+size, y+size );
	glEnd();
}

void glutDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf( mPosit.projectionMatrix );
	glMatrixMode(GL_MODELVIEW);

	//Draw the object with the estimated pose
	glLoadIdentity();
	glScalef( 1.0f, 1.0f, -1.0f);
	glMultMatrixf( mPosit.posePOSIT );
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glColor3f( 0.7f, 0.7f, 0.7f );
	renderCube( mCubeSize );
	glDisable( GL_LIGHTING );

	//Draw the object with the real pose
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable( GL_POLYGON_OFFSET_LINE );
	glPolygonOffset( -0.1f, -0.1f );
	glLoadIdentity();
	glScalef( 1.0, 1.0, -1.0 );
	glMultMatrixf( mPosit.poseReal );
	glColor3f( 1.0f, 0.0f, 1.0f );
	glLineWidth( 2.0f );
	renderCube( mCubeSize );
	glDisable( GL_POLYGON_OFFSET_LINE );
	glLineWidth( 4.0f );	
	renderAxis( mCubeSize * 2.0 );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Draw the calculated 2D points 
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//2D projection with (0,0) in the centre of the image
	glOrtho( -mWinWidth *0.5, mWinWidth*0.5, -mWinHeight * 0.5, mWinHeight * 0.5, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLineWidth( 2.0f );
	glColor3f( 1.0f, 0.0f, 0.0f);
	for ( size_t  p=0; p < mPosit.srcImagePoints.size(); p++ )
		drawCross( mPosit.srcImagePoints[p].x, mPosit.srcImagePoints[p].y, 10 );
	glPointSize(4.0f);
	glColor3f( 1.0f, 1.0f, 1.0f);
	glBegin( GL_POINTS );	
	for ( size_t  p=0; p < mPosit.estimatedImagePoints.size(); p++ )
		glVertex2f( mPosit.estimatedImagePoints[p].x, mPosit.estimatedImagePoints[p].y);
	glEnd();
    glutSwapBuffers();
}

int main(int argc, char** argv)
{
	cout << "OpenCV POSIT tutorial" << endl;
	cout << "by Javier Barandiaran(jbarandiaran@gmail.com)" << endl;
	cout << "-Gray solid cube: model projected with the pose estimated by POSIT" << endl;
	cout << "-Red wired cube: model projected with the real pose" << endl;
	cout << "-Red crosses: Source image points" << endl;
	cout << "-White dots: Estimated image points" << endl;
	cout << "CONTROLS:" << endl;
	cout << "-Left mouse button: Rotate the cube" << endl;
	cout << "-Ctrl+Left mouse button: Translate the cube" << endl;
	cout << "-Right mouse button: Translate the cube in z-axis" << endl;

    /*
        Glut's initialization code. Set window's size and type of display.
        Window size is put half the 800x600 resolution as defined by above
		constants. The windows is positioned at the top leftmost area of
		the screen.
    */
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( GL_WIN_INITIAL_X, GL_WIN_INITIAL_Y );
    glutInitWindowSize( GL_WIN_INITIAL_WIDTH, GL_WIN_INITIAL_HEIGHT );
    glutInit( &argc, argv );
    glutCreateWindow("OpenCV POSIT Tutorial");

    /*
       The function below are called when the respective event
       is triggered.
    */
    glutReshapeFunc(glutResize);       // called every time  the screen is resized
    glutDisplayFunc(glutDisplay);      // called when window needs to be redisplayed
    glutKeyboardFunc(glutKeyboard);    // called when the application receives a input from the keyboard
    glutMouseFunc(glutMouse);          // called when the application receives a input from the mouse
    glutMotionFunc(glutMotion);        // called when the mouse moves over the screen with one of this button pressed
    //glutSpecialFunc(glutSpecial);      // called when a special key is pressed like SHIFT

   	/*
		POSIT initialization
	*/
	mPosit.initialize( mCubeSize, mWinWidth, mWinHeight, GL_NEAR, GL_FAR );
	mPosit.buildRealPose( mRot, mEye );
	mPosit.projectModelPoints( mPosit.poseReal, mPosit.srcImagePoints );
	mPosit.poseEstimation();
	mPosit.projectModelPoints( mPosit.posePOSIT, mPosit.estimatedImagePoints );

    /*
       Application's main loop. All the above functions
	 are called whe the respective events are triggered
    */
	glutMainLoop();
	return 0;
}