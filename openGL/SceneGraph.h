#include "SceneNode.h"
#include <unistd.h>

float rads = 10.0f;
float big = 1.5f;
float small = 0.75f;

class SceneGraph{
public:
	std::vector<SceneNode*> nodes;
	int pos;

	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;

	vector<IDs> ids;
	vector<Buffers> buffers;

public:
	SceneNode root;

public:
	SceneGraph(void);
	void loadSuzanne();
	void loadSuzanneDark();
	
	void addChilds(SceneNode* n);
	void addNode(SceneNode * n);
	
	void update();
	void background(Mat image);
	void keyboard();
	void draw(Mat oglImage);

	void translate_world(glm::vec3 tr);


	void deleteBuffers();
};

SceneGraph::SceneGraph(void){
	
    loadSuzanne();
    loadSuzanneDark();

    root = SceneNode(ids[0]);
    nodes.push_back(&root);
    pos = 0;
}

void SceneGraph::addChilds(SceneNode* n){
	nodes.push_back(n);
	for(SceneNode* nn : n->childs)
		addChilds(nn);
}

void SceneGraph::addNode(SceneNode* n){
	MatrixID      = glGetUniformLocation(n->programID, "MVP");
	ViewMatrixID  = glGetUniformLocation(n->programID, "V");
	ModelMatrixID = glGetUniformLocation(n->programID, "M");

	addChilds(n);
	root.addChild(n);
}

void SceneGraph::update(){
	root.update();
}


void SceneGraph::draw( Mat oglImage ){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//background(oglImage);
	
	//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);

	keyboard();
	update();
	for(SceneNode* n : root.childs)	
		n->draw(ViewMatrixID, MatrixID, ModelMatrixID);

	glfwSwapBuffers(window);
	glfwPollEvents();
//	sleep(0.5);
}

void SceneGraph::keyboard(){
	bool hold = false;
	if(	glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS ){
		nodes[pos]->updateIds(ids[1]);
		pos = (pos + 1)%nodes.size();
		nodes[pos]->updateIds(ids[0]);
	}
	
	if(	glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS ){
		nodes[pos]->updateIds(ids[1]);
		if(pos>0)	pos = (pos - 1);
		else 		pos = nodes.size() - 1;
		nodes[pos]->updateIds(ids[0]);
	}
		
	if(	glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS )
		nodes[pos]->rotateY(-rads);

	if(	glfwGetKey(window, GLFW_KEY_D ) == GLFW_PRESS )
		nodes[pos]->rotateY(rads);
	
	if(	glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS )
		nodes[pos]->rotateX(-rads);

	if(	glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS )
		nodes[pos]->rotateX(rads);

	if(	glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS )
		nodes[pos]->rotateZ(rads);

	if(	glfwGetKey(window, GLFW_KEY_E ) == GLFW_PRESS )
		nodes[pos]->rotateZ(-rads);
	
	if(	glfwGetKey(window, GLFW_KEY_T ) == GLFW_PRESS )
		nodes[pos]->scale(big);

	if(	glfwGetKey(window, GLFW_KEY_G ) == GLFW_PRESS )
		nodes[pos]->scale(small);
		
	//printf("%d\n", pos);
}

void SceneGraph::background( Mat image ){
	/*int width = image.cols;
	int height = image.rows;	
	unsigned char *imageData = image.ptr();
	GLuint m_texture;

	if (imageData == NULL)
		std::cerr << "Texture loading failed for texture: " << std::endl;

	glGenTextures(1, &m_texture);
	
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

	glUseProgram(ids2[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glBindVertexArray(m_vertexArrayObject);

	glDrawElements(GL_TRIANGLES, m_drawCount, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLES, 0, m_drawCount);

	glBindVertexArray(0);
*/
}

void SceneGraph::translate_world(glm::vec3 tr){
	//root.model = mat4(1.0);
	root.model = glm::translate(root.model, tr);
}

void SceneGraph::deleteBuffers(){
	for (SceneNode* n : nodes)
		n->delete_buffers();
}

void SceneGraph::loadSuzanne(){
	GLuint programID = LoadShaders( "shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader" );
	GLuint Texture = loadDDS("shaders/uvmap.DDS");
	GLuint textureID  = glGetUniformLocation(programID, "myTextureSampler");

	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	
	glm::vec3 lightPos = glm::vec3(0,5,3);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	////////////////////////////////////////////////////////////

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; 
	bool res = loadOBJ("shaders/suzanne.obj", vertices, uvs, normals);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	//////////////////////////////////////////////////////////////
	IDs _ids = {programID, textureID, LightID, vertexbuffer, uvbuffer, normalbuffer, (GLuint)vertices.size()};
	ids.push_back(_ids);
}

void SceneGraph::loadSuzanneDark(){
	GLuint programID = LoadShaders( "shaders/low.vertexshader", "shaders/low.fragmentshader" );
	GLuint Texture = loadDDS("shaders/uvmap.DDS");
	GLuint textureID  = glGetUniformLocation(programID, "myTextureSampler");

	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	
	glm::vec3 lightPos = glm::vec3(0,5,3);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	////////////////////////////////////////////////////////////

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; 
	bool res = loadOBJ("shaders/suzanne.obj", vertices, uvs, normals);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	//////////////////////////////////////////////////////////////
	IDs _ids = {programID, textureID, LightID, vertexbuffer, uvbuffer, normalbuffer, (GLuint)vertices.size()};
	ids.push_back(_ids);
}

