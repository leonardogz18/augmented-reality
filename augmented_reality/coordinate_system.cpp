#include "functions.h"

int main(int argc, char** argv)
{
	int th, max, min;	float mult;
	th = 90;	max = 160;	min=35;	mult=1.6;
	cali.init("Calibrar_Pruebaa.wmv",th, max, min, mult);

	//th = 250;	max = 150;	min=25;	mult=2.;
	//cali.init("Prueba.wmv",th, max, min, mult);
	
	cali.read_intrinsic_data("data");
	cali.get_intrinsic(intrinsics, distortion);
	//FileStorage fs;
	/*fs.open(filename, FileStorage::READ);
	// read camera matrix and distortion coefficients from file
	fs["Camera_Matrix"] >> intrinsics;
	fs["Distortion_Coefficients"] >> distortion;
	fs.release();*/

	//generate vectors for the points on the chessboard
	for (int i=0; i<boardWidth; i++)
	{
		for (int j=0; j<boardHeight; j++)
		{
			//boardPoints.push_back( Point3d( double(i), double(j), 0.0) );
			boardPoints.push_back( Point3d( double(67.5 * i), double(67.5 * j), 0.0f) );			
			cout<<Point3d( double(67.5 * i), double(67.5 * j), 0.0f) <<endl;
		}
	}
	
	//generate points in the reference frame
	framePoints.push_back( Point3d( 0.0, 0.0, 0.0 ) );
	framePoints.push_back( Point3d( 5.0, 0.0, 0.0 ) );
	framePoints.push_back( Point3d( 0.0, 5.0, 0.0 ) );
	framePoints.push_back( Point3d( 0.0, 0.0, -5.0 ) );

	//VideoCapture capture;
	//capture.open(0);
	capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	namedWindow("OpenCV Webcam", 0);

	
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);  
    glutInitWindowPosition( GL_WIN_INITIAL_X, GL_WIN_INITIAL_Y );
    glutInitWindowSize( FRAME_WIDTH, FRAME_HEIGHT );
    glutCreateWindow("OpenGL");        
    
    glClearColor(0.0,0.0,0.0,0.0);
    
    glutDisplayFunc(glutDisplay);	//glutDisplayFunc(display);
    glutReshapeFunc(glutResize);       
    glutIdleFunc(myidle); 

	glutMainLoop();

	return 0;
}
