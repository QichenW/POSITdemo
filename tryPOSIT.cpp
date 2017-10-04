//
// Created by Qichen Wang on 10/4/17.
//

#include <cxcore.h>
#include <cv.h>
#include <GLUT/glut.h>
using namespace std;

/* windows size and position constants */
const int GL_WIN_INITIAL_WIDTH = 1080;
const int GL_WIN_INITIAL_HEIGHT = 720;
const int GL_WIN_INITIAL_X = 0;
const int GL_WIN_INITIAL_Y = 0;
const int GL_NEAR = 1.0;
const int GL_FAR = 1000.0;
const int ESC = 27;
const float FOCAL_LENGTH = 986.f;

int mWinWidth = GL_WIN_INITIAL_WIDTH;
int mWinHeight = GL_WIN_INITIAL_HEIGHT;

vector<CvPoint2D32f> estimatedImagePoints;
vector<CvPoint2D32f> srcImagePoints;
float projectionMatrix[16] = {};
float posePOSIT[16] = {};

void glutResize(int width, int height){
    mWinWidth = width;
    mWinHeight = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
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

void renderBox(float x, float y, float z)
{
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f( 0.0f,  0.0f,  0.0f);
		glVertex3f( x,  0.0f,  0.0f);
		glVertex3f( x,  y,  0.0f);
		glVertex3f( 0.0f,  y,  0.0f);
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glVertex3f( 0.0f,  y,  -1 * z);
		glVertex3f( x,  y, -1 * z);
		glVertex3f( x,  0.f, -1 * z);
		glVertex3f( 0.f,  0.0f, -1 * z);
		// Top Face
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glVertex3f( 0.0f,  y,  -1 * z);
		glVertex3f( 0.f,  y,  0.0f);
		glVertex3f( x,  y, 0.f);
		glVertex3f( x,  y, -1 * z);
		// Bottom Face
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glVertex3f( 0.0f,  0.0f,  0.0f);
		glVertex3f( 0.0f,  0.0f, -1 * z);
		glVertex3f( x,  0.0f, -1 * z);
		glVertex3f( x,  0.0f,  0.0f);
		// Right face
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( x,  y, 0.0f);
		glVertex3f( x,  0.0f, 0.f);
		glVertex3f( x,  0.0f, -1 * z);
		glVertex3f( x,  y, -1 * z);
		// Left Face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f( 0.0f,  y, -1 * z);
		glVertex3f( 0.0f,  0.0f, -1 * z);
		glVertex3f( 0.0f,  0.0f, 0.f);
		glVertex3f( 0.0f,  y, 0.f);
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
	glLoadMatrixf(projectionMatrix);
	glMatrixMode(GL_MODELVIEW);

	//Draw the object with the estimated pose
	glLoadIdentity();
	glScalef( 1.0f, 1.0f, -1.0f); // was 1,1, -1 in the example, donno why
	glMultMatrixf(posePOSIT );
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glColor3f( 0.0f, 0.7f, 0.7f );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	renderBox(105.f, 20.f, 50.f);
	glDisable( GL_LIGHTING );

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
	for ( size_t  p=0; p < srcImagePoints.size(); p++ )
		drawCross(srcImagePoints[p].x, srcImagePoints[p].y, 10 );

	glPointSize(4.0f);
	glColor3f( 0.0f, 1.0f, 0.0f);
	glBegin( GL_POINTS );
	for ( size_t  p=0; p < estimatedImagePoints.size(); p++ )
		glVertex2f(estimatedImagePoints[p].x, estimatedImagePoints[p].y);
	glEnd();

	glutSwapBuffers();

}

int main(int argc, char** argv) {
    /**
     * get the homogeneous matrix for glut
     */
    vector<CvPoint3D32f> modelPoints;
    modelPoints.push_back(cvPoint3D32f(0.0f, 0.0f, 0.0f));
    modelPoints.push_back(cvPoint3D32f(0.0f, 20.0f, 0.0f));
    modelPoints.push_back(cvPoint3D32f(0.0f, 0.0f, -50.0f));
    modelPoints.push_back(cvPoint3D32f(105.0f, 20.0f, 0.0f));

    CvPOSITObject* positObject = cvCreatePOSITObject( &modelPoints[0], (int)modelPoints.size() );
    CvMat* intrinsics = cvCreateMat( 3, 3, CV_32F );
	cvmSet( intrinsics , 0, 0, FOCAL_LENGTH);
	cvmSet( intrinsics , 1, 1, FOCAL_LENGTH);
	cvmSet( intrinsics , 0, 2, 1080 * 0.5 );//principal point in the centre of the image
	cvmSet( intrinsics , 1, 2, 720 * 0.5 );
	cvmSet( intrinsics , 2, 2, 1.0 );

//    srcImagePoints.push_back(cvPoint2D32f(593.f, 432.f));
//    srcImagePoints.push_back(cvPoint2D32f(590.f, 353.f));
//    srcImagePoints.push_back(cvPoint2D32f(485.f, 407.f));
//    srcImagePoints.push_back(cvPoint2D32f(840.f, 301.f));
	// convert the pixel coordinate to right hand coordinate
	srcImagePoints.push_back(cvPoint2D32f(53.f, -72.f));
    srcImagePoints.push_back(cvPoint2D32f(50.f, 7.f));
    srcImagePoints.push_back(cvPoint2D32f(-55.f, 047.f));
    srcImagePoints.push_back(cvPoint2D32f(340.f, 59.f));

	CvMatr32f rotation_matrix = new float[9];
	CvVect32f translation_vector = new float[3];
    CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 100, 1.0e-4f);
    cvPOSIT(positObject, &srcImagePoints[0], FOCAL_LENGTH, criteria, rotation_matrix, translation_vector);
    cout << "The rotation matrix is " << endl;
    for (int i = 0; i < 9; ++i) {
		cout << rotation_matrix[i] << "\t";
        if(i % 3 == 2) {
			cout << endl;
		}
    }
	//note that the estimated rotation matrix is not ortho-normal;
    for (int f=0; f<3; f++)
	{
		for (int c=0; c<3; c++)
		{
			posePOSIT[c*4+f] = rotation_matrix[f*3+c];	//transposed
		}
	}
	posePOSIT[3] = 0.0;
	posePOSIT[7] = 0.0;
	posePOSIT[11] = 0.0;
	posePOSIT[12] = translation_vector[0];
	posePOSIT[13] = translation_vector[1];
	posePOSIT[14] = translation_vector[2];
	posePOSIT[15] = 1.0;

	projectionMatrix[0] = (float) (2.0 * cvmGet( intrinsics, 0, 0 ) / mWinWidth);
	projectionMatrix[1] = 0.0;
	projectionMatrix[2] = 0.0;
	projectionMatrix[3] = 0.0;

	projectionMatrix[4] = 0.0;
	projectionMatrix[5] = (float) (2.0 * cvmGet( intrinsics, 1, 1 ) / mWinHeight);
	projectionMatrix[6] = 0.0;
	projectionMatrix[7] = 0.0;

	projectionMatrix[8] = (float)(2.0 * ( cvmGet( intrinsics, 0, 2 ) / mWinWidth) - 1.0);
	projectionMatrix[9] = (float)(2.0 * ( cvmGet( intrinsics, 1, 2 ) / mWinHeight) - 1.0);



	/**TODO, projectMatrix 10,11,14 are a bit diffrent from the matrix given in CSCI6554
		reference, http://www.songho.ca/opengl/gl_projectionmatrix.html
	**/
	projectionMatrix[10] = -( GL_FAR +GL_NEAR ) / ( GL_FAR - GL_NEAR );
	projectionMatrix[11] = -1.0f;
	projectionMatrix[12] = 0.0;
	projectionMatrix[13] = 0.0;
	projectionMatrix[14] = -2.f * GL_FAR * GL_NEAR/ (GL_FAR - GL_NEAR);
	projectionMatrix[15] = 0.0;

	//given model points and the pose estimation, calculate the estimated image points
	// The origin of the coordinates system is in the centre of the image
	estimatedImagePoints.clear();
	CvMat poseMatrix = cvMat( 4, 4, CV_32F, posePOSIT );
	for ( size_t  p=0; p<modelPoints.size(); p++ )
	{
		float modelPoint[] =  { modelPoints[p].x, modelPoints[p].y, modelPoints[p].z, 1.0f };
		CvMat modelPointMatrix = cvMat( 4, 1, CV_32F, modelPoint );
		float point3D[4];
		CvMat point3DMatrix = cvMat( 4, 1, CV_32F, point3D );
		cvGEMM( &poseMatrix, &modelPointMatrix, 1.0, NULL, 0.0, &point3DMatrix, CV_GEMM_A_T );

		//Project the transformed 3D points
		CvPoint2D32f point2D = cvPoint2D32f( 0.0, 0.0 );
		if ( point3D[2] != 0 )
		{
			//TODO, as in similar triangles x:X = y:Y = z : Z, z is focal length
			point2D.x = cvmGet( intrinsics, 0, 0 ) * point3D[0] / point3D[2];
			point2D.y = cvmGet( intrinsics, 1, 1 ) * point3D[1] / point3D[2];
		}
		estimatedImagePoints.push_back( point2D );
	}


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

	glutMainLoop();
	return 0;
}

