// Include standard headers
#include "SceneGraph.h"
// Include GLFW

Calibrator cali;
VideoCapture capture(0);

Mat toOGL(Mat ocvImage){
	Mat oglImage;

	flip(ocvImage, oglImage,0);
	cvtColor(oglImage, oglImage, CV_BGR2RGB);

	/*for (int i = 0; i < oglImage.size().width; i++)
		for (int j = 0; j < oglImage.size().height; j++)
			swap(oglImage.ptr<Vec3b>(j)[i][0], oglImage.ptr<Vec3b>(j)[i][2]);
	*/
	return oglImage;
}

int initOGL(){
	// Initialise GLFW
	glfwInit();

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Learnin", NULL, NULL);
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	glewInit();
	// Ensure we can capture the escape key being pressed below
		// Ensure we can capture the escape key being pressed below
	//glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(.73f, .56f, .56f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 

	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	return VertexArrayID;
}

void initOCV(){
	int th, max, min;	float mult;
	th = 90;	max = 160;	min=35;	mult=1.6;
	cali.init("Calibrar_Prueba.wmv",th, max, min, mult);
	cali.patternSize(4,3);
	cali.read_intrinsic_data("data");
}

vec3 matToVec3(Mat &mat){
	vec3 vec(mat.at<double>(0,0), mat.at<double>(1,0), mat.at<double>(2,0) );
	//printf("%f %f %f\n", vec[0], vec[1], vec[2]);
	return vec;	
}

mat4 fromCV2GLM(const cv::Mat& cvmat) {
	mat4 glmmat;
    for (int i=0; i<4; i++)
    	for (int j=0; j<4; j++)
    		glmmat[i][j] = cvmat.at<double>(i,j);
    return glmmat;
}

int main( void )
{
	
	GLuint VertexArrayID = initOGL();
	initOCV();

	SceneGraph graph;

	SceneNode n1(graph.ids[1], glm::vec3( -2.0f, 2.0f, 0.0f));
	SceneNode n1_1(graph.ids[1], glm::vec3( 0.0f, -2.0f, 0.0f));
	SceneNode n1_2(graph.ids[1], glm::vec3( 0.0f, -2.0f, 0.0f));
	n1.addChild(&n1_1);
	n1_1.addChild(&n1_2);

	SceneNode n2(graph.ids[1], glm::vec3(  2.0f, 0.0f, 0.0f));
	SceneNode n2_1(graph.ids[1], glm::vec3( 2.0f, -2.0f, 0.0f));
	n2.addChild(&n2_1);

	graph.addNode(&n1);
	graph.addNode(&n2);

	//graph.translate_world(glm::vec3(0.0f, -3.0f, 0.0f));

	Mat webCam;
	Mat oglImage;
	vector<Point2d> imagePoints;
	//namedWindow( "OCV", WINDOW_AUTOSIZE );// Create a window for display.
	float scale = 30.0f;
	do{
		capture.read(webCam);
		
		imshow( "OCV", webCam );
	    waitKey(1);
		bool found = cali.findCirclesGrid(webCam, imagePoints);
		if ( found ){
			Mat rvec = Mat(Size(3,1), CV_64F);
    		Mat tvec = Mat(Size(3,1), CV_64F);
			Mat cvViewMatrix = cali.get_glMatrix(webCam, rvec, tvec);
			//graph.translate_world(matToVec3(tvec));
			mat4 glViewMatrix = fromCV2GLM(cvViewMatrix);
			/*glViewMatrix[3][0] = 1;//= scale;
			glViewMatrix[3][1] = 1;//= scale;
			glViewMatrix[3][2] = 1;//= scale;*/
			//cout<<to_string(glViewMatrix)<<endl;
			//graph.root.model = glViewMatrix;
			graph.root.model[0] = glViewMatrix[0];
			graph.root.model[1] = glViewMatrix[1];
			graph.root.model[2] = glViewMatrix[2];
			cout<<to_string(graph.root.model[3])<<endl;
		}
		cout<<to_string(graph.root.model[2])<<endl;
		oglImage = toOGL(webCam);
		graph.draw(oglImage);
		
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	glDeleteVertexArrays(1, &VertexArrayID);
	graph.deleteBuffers();

	glfwTerminate();

	return 0;
}

