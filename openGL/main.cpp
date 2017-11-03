// Include standard headers
//#include "SceneNode.h"
#include "SceneGraph.h"
// Include GLFW

int initialize(){
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
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

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

int main( void )
{
	GLuint VertexArrayID = initialize();

	SceneGraph graph;
	graph.addChild(glm::vec3( 0.0f, 1.0f, 0.0f));
	graph.addChild(glm::vec3( 0.0f, -1.0f, 0.0f));
	
	do{
		graph.draw();
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	glDeleteVertexArrays(1, &VertexArrayID);
	graph.deleteBuffers();

	glfwTerminate();

	return 0;
}

