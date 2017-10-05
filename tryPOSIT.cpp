//This is a demo for POSIT algorithm,
// given the 3d model and 4 vertices in both model space and 2d image, approximately calculate its transformation
//in 3d. Draw the estimated transformation of the model in 3d, then project the estimated position of 4 vertices back
//to the 2d image, compare them with their original mark.

//note the image taken by webcam should not be mirrored like in video chat.
//note that the coordinate system of image should be a normal 2d with the origin at the center of image.
//note that the cvPOSIT function requires all model points in left-hand coordinate system,
//and out put the transformation matrix in left-hand coordinate system.
//to properly draw the transformed model, must first do glScale(1,1,-1); then the matrix can be used as normal
// Created by Qichen Wang on 10/4/17.
//

#include <cxcore.h>
#include <cv.h>
#include <GLUT/glut.h>
using namespace std;

/* windows size and position constants */
const int IMAGE_WIDTH = 1080;
const int IMAGE_HEIGHT = 720;
const int GL_WIN_INITIAL_X = 0;
const int GL_WIN_INITIAL_Y = 0;
const int GL_NEAR = 1.f;
const int GL_FAR = 1000.f;
const int ESC = 27;
const float FOCAL_LENGTH = 896.7f;

vector<CvPoint2D32f> estimatedImagePoints;
vector<CvPoint2D32f> srcImagePoints;
float projectionMatrix[16] = {};
float posePOSIT[16] = {};


float boxLengthInPixel = 105.f;
float boxHeightInPixel = 20.f;
float boxDepthInPixel = 50.f;



void glutResize(int width, int height){
	glViewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

/**
 * create a ppm file
 */
void PPMWriter(unsigned char *in,char *name,int dimx, int dimy)
{

	// note pixel coordinate and coordinate in image file are different in y-direction
    int i, j;
    FILE *fp = fopen(name, "wb"); /* b - binary mode */
    (void) fprintf(fp, "P6\n%d %d\n255\n", dimx, dimy);
    for (j = dimy - 1; j > - 1; --j)
    {
        for (i = 0; i < dimx; ++i)
        {
            static unsigned char color[3];
            color[0] = in[3*i+3*j*dimx];  /* red */
            color[1] = in[3*i+3*j*dimx+1];  /* green */
            color[2] = in[3*i+3*j*dimx+2];  /* blue */
            (void) fwrite(color, 1, 3, fp);
        }
    }
    (void) fclose(fp);
}

/**
 * helper function to save the image
 */
void saveImage()
{
    unsigned char* image = (unsigned char*)malloc(sizeof(unsigned char) * 3 * IMAGE_WIDTH * IMAGE_HEIGHT);
    glReadPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, image);
    char buffer [33];
    sprintf(buffer, "capture/%d SV-%f ms.ppm", 1, 1.0f);

    PPMWriter(image,buffer, IMAGE_WIDTH, IMAGE_HEIGHT);
}

// Function that handles keyboard inputs
void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case ESC:
			exit(0);
            break;
        case 's':
            saveImage();
            break;
        default:
            break;
	}
}

void renderBox(float x, float y, float z)
{
    glBegin(GL_QUADS);
    // Front Face
    glColor3f(1, 0, 1);
    glVertex3f( -x,  y,  0.0f);
    glVertex3f( -x,  0.0f,  0.0f);
    glVertex3f( 0.0f,  0.0f,  0.0f);
    glVertex3f( 0.0f,  y,  0.0f);
    // Back Face
    glColor3f(1,1,0);
    glVertex3f( -x,  0.f, z);
    glVertex3f( -x,  y, z);
    glVertex3f( 0.0f,  y,  z);
    glVertex3f( 0.f,  0.0f, z);
    // Top Face
    glColor3f(1,0,0);
    glVertex3f( -x,  y, z);
    glVertex3f( -x,  y, 0.f);
    glVertex3f( 0.f,  y,  0.0f);
    glVertex3f( 0.0f,  y,  z);
    // Bottom Face
    glColor3f(0,1,0);
    glVertex3f( -x,  0.0f,  0.0f);
    glVertex3f( -x,  0.0f, z);
    glVertex3f( 0.0f,  0.0f, z);
    glVertex3f( 0.0f,  0.0f,  0.0f);
    // Right face
    glColor3f(0,1,1);
    glVertex3f( 0.0f,  y, 0.f);
    glVertex3f( 0.0f,  0.0f, 0.f);
    glVertex3f( 0.0f,  0.0f, z);
    glVertex3f( 0.0f,  y, z);
    // Left Face
    glColor3f(0, 0, 1);
    glVertex3f( -x,  y, 0.0f);
    glVertex3f( -x,  y, z);
    glVertex3f( -x,  0.0f, z);
    glVertex3f( -x,  0.0f, 0.f);
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
	glScalef( 1.0f, 1.0f, -1.0f); // first reverse z axis, so we are in left-handed coordinate system
	glMultMatrixf(posePOSIT );
//	glEnable( GL_LIGHTING );
//	glEnable( GL_LIGHT0 );
	glColor3f( 0.0f, 0.7f, 0.7f );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	renderBox(boxLengthInPixel, boxHeightInPixel, boxDepthInPixel);
//	glDisable( GL_LIGHTING );

	//Draw the calculated 2D points
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//2D projection with (0,0) in the centre of the image
    //the dot and cross markers are drawn in Orthographic Volume
	glOrtho( -IMAGE_WIDTH*0.5, IMAGE_WIDTH*0.5,
             -IMAGE_HEIGHT *  0.5, IMAGE_HEIGHT * 0.5, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLineWidth( 2.0f );
	glColor3f( 1.0f, 1.0f, 1.0f);
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
    modelPoints.push_back(cvPoint3D32f(0.0f, boxHeightInPixel, 0.0f));
    modelPoints.push_back(cvPoint3D32f(0.0f, 0.0f, boxDepthInPixel));
    modelPoints.push_back(cvPoint3D32f(-boxLengthInPixel, boxHeightInPixel, 0.0f));

    CvPOSITObject* positObject = cvCreatePOSITObject( &modelPoints[0], (int)modelPoints.size() );
    CvMat* intrinsics = cvCreateMat( 3, 3, CV_32F );
	cvmSet( intrinsics , 0, 0, FOCAL_LENGTH);
	cvmSet( intrinsics , 1, 1, FOCAL_LENGTH);
	cvmSet( intrinsics , 0, 2, IMAGE_WIDTH * 0.5 );//principal point in the centre of the image
	cvmSet( intrinsics , 1, 2, IMAGE_HEIGHT * 0.5 );
	cvmSet( intrinsics , 2, 2, 1.0 );

	// convert the pixel coordinate to right hand coordinate, origin at center
	srcImagePoints.push_back(cvPoint2D32f(-85.f, -91.f));
    srcImagePoints.push_back(cvPoint2D32f(-80.f, -10.f));
    srcImagePoints.push_back(cvPoint2D32f(28.f, -67.f));
    srcImagePoints.push_back(cvPoint2D32f(-347.f, 40.f));
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
    cout << "The translation matrix is: ";
    cout << translation_vector[0] << " " << translation_vector[1] << " " << translation_vector[2] << endl;
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

	projectionMatrix[0] = (float) (2.0 * cvmGet( intrinsics, 0, 0 ) / IMAGE_WIDTH);
	projectionMatrix[1] = 0.0;
	projectionMatrix[2] = 0.0;
	projectionMatrix[3] = 0.0;

	projectionMatrix[4] = 0.0;
	projectionMatrix[5] = (float) (2.0 * cvmGet( intrinsics, 1, 1 ) / IMAGE_HEIGHT);
	projectionMatrix[6] = 0.0;
	projectionMatrix[7] = 0.0;

	projectionMatrix[8] = (float)(2.0 * ( cvmGet( intrinsics, 0, 2 ) / IMAGE_WIDTH) - 1.0);
	projectionMatrix[9] = (float)(2.0 * ( cvmGet( intrinsics, 1, 2 ) / IMAGE_HEIGHT) - 1.0);



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
            //TODO, as in similar triangles x:X = y:Y = z : Z, z is focal length,
            // TODO, note that here Point3D is in left-hand coordinate system
			point2D.x = (float)(cvmGet(intrinsics, 0, 0 ) * point3D[0] / point3D[2]);
			point2D.y = (float)(cvmGet(intrinsics, 1, 1 ) * point3D[1] / point3D[2]);
		}
		estimatedImagePoints.push_back( point2D );
	}

    glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( GL_WIN_INITIAL_X, GL_WIN_INITIAL_Y );
    glutInitWindowSize( IMAGE_WIDTH, IMAGE_HEIGHT );
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

