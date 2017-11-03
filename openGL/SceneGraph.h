#include "SceneNode.h"

class SceneGraph{
public:
	std::vector<SceneNode> nodes;

	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;

	glm::mat4 View;
	glm::mat4 Projection;

	IDs ids;


public:
	SceneGraph(void);
	void loadSuzanne();
	void addChild(glm::vec3 pos = glm::vec3(1.0f));
	void draw();

	void deleteBuffers();
};

SceneGraph::SceneGraph(void){
	View = glm::lookAt(
    	glm::vec3(0,0,2), // Camera is at (4,3,3), in World Space
    	glm::vec3(0,0,0), // and looks at the origin
    	glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    Projection = glm::perspective(glm::radians(45.0f),4.0f / 3.0f, 0.1f, 100.0f);

    loadSuzanne();
}

void SceneGraph::loadSuzanne(){
	GLuint programID = LoadShaders( "shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader" );
	
	GLuint Texture = loadDDS("shaders/uvmap.DDS");
	GLuint textureID  = glGetUniformLocation(programID, "myTextureSampler");

	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	glm::vec3 lightPos = glm::vec3(0,5,3);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	ids.push_back(programID);
	ids.push_back(textureID);
	ids.push_back(LightID);
}

void SceneGraph::addChild(glm::vec3 pos){
	
	SceneNode node(ids, pos);
	nodes.push_back(node);

	MatrixID = glGetUniformLocation(node.programID, "MVP");
	ViewMatrixID = glGetUniformLocation(node.programID, "V");
	ModelMatrixID = glGetUniformLocation(node.programID, "M");

}

void SceneGraph::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);

	for(SceneNode n: nodes){
		glm::mat4 mvp = Projection * View * n.model;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &n.model[0][0]);

		n.draw();
	}

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void SceneGraph::deleteBuffers(){
	for (SceneNode n : nodes)
		n.delete_buffers();
}