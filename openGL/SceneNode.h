#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

typedef std::vector<GLuint> IDs;


class SceneNode{
public:
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
	
	GLuint textureID;
	GLuint programID;
	GLuint lightID;

	mat4 model;

	int verticesNum;

public:
	SceneNode(IDs ids, glm::vec3 tr = glm::vec3(1.0f));
	void draw();
	void delete_buffers();

	void rotate( glm::vec3 axis, float degree);
	void translate( glm::vec3 tr);
};

SceneNode::SceneNode(IDs ids, glm::vec3 tr){
	programID = ids[0];
	textureID = ids[1];
	lightID   = ids[2];

	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(.2f, .2f, .2f));
    model = glm::translate(model, tr);
    glm::vec3 myRotationAxis( .0f, 1.0f, .0f);
	model = glm::rotate( model, glm::radians(0.0f) , myRotationAxis );

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; // Won't be used at the moment.
	bool res = loadOBJ("shaders/suzanne.obj", vertices, uvs, normals);

	verticesNum = vertices.size();
	
	vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

}

void SceneNode::draw(){
	glUseProgram(programID);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(textureID, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0,	(void*)0 );

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, verticesNum); 
	//glDrawElements(GL_TRIANGLES, verticesNum, GL_UNSIGNED_SHORT, (void*)0 );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void SceneNode::rotate( glm::vec3 axis, float degree){
	model = glm::rotate( model, glm::radians(degree) , axis );	
}

void SceneNode::translate(glm::vec3 tr){ 
	model = glm::translate(model, tr); 
}

void SceneNode::delete_buffers(){
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &textureID);
	
}