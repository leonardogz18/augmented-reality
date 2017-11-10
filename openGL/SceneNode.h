#include "cali/calibrator.h"
// Include GLEW
#include <GL/glew.h>

#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include <glm/gtc/type_ptr.hpp>

typedef std::vector<GLuint> IDs;
typedef std::vector<GLuint> Buffers;

vec3 zeros(0.0f);
vec3 ones (1.0f);
vec3 xAxis(1.0f, 0.0f, 0.0f);
vec3 yAxis(0.0f, 1.0f, 0.0f);
vec3 zAxis(0.0f, 0.0f, 1.0f);


mat4 ViewM = glm::lookAt(
    	glm::vec3(0,0,8), // Camera is at (4,3,3), in World Space
    	glm::vec3(0,0,0), // and looks at the origin
    	glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

/*float values[16] = {
	1.77446e-317, 0, 0, 0, 
	1.7652e-317, 2.33777e-317, 0, 0, 
	3.60266e+15, -0.999967, -1.00002, -1,
	0, 0, -0.200002, 0 
};

mat4 ProjectionM = glm::make_mat4(values);
*/mat4 ProjectionM = glm::perspective(glm::radians(45.0f),4.0f / 3.0f, 0.1f, 100.0f);


class SceneNode{
public:
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
	
	GLuint textureID;
	GLuint programID;
	GLuint lightID;

	mat4 model;
	//mat4 ViewM;

	int verticesNum;

public:
	SceneNode* parent;
	std::vector<SceneNode*> childs;

	vec3 translate_vec;
	vec3 scale_vec;
	vec3 degrees;

public:
	SceneNode (void) {}
	SceneNode(IDs ids, vec3 tr = zeros);
	void draw(GLuint ViewMatrixID, GLuint MatrixID, GLuint ModelMatrixID);
	void delete_buffers();

	void update();
	void updateIds(IDs ids);
	void addChild( SceneNode* n);

	void rotateX( float degree );
	void rotateY( float degree );
	void rotateZ( float degree );
	void translate( vec3 tr );
	void scale(float sc);
};

SceneNode::SceneNode(IDs ids, glm::vec3 tr){
	parent = NULL;
	translate_vec = tr;
	scale_vec = ones;	
	degrees = zeros;

	programID = ids[0];
	textureID = ids[1];
	lightID   = ids[2];

	vertexbuffer = ids[3];
	uvbuffer     = ids[4];
	normalbuffer = ids[5];

	verticesNum  = ids[6];

	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(.2f, .2f, .2f));
    model = glm::translate(model, tr);
	model = glm::rotate( model, glm::radians(0.0f) , yAxis );
}

void SceneNode::update(){
	if (parent != NULL){	//if its not root
		model = parent->model;
		model = glm::translate(model, translate_vec);
		model = glm::scale(model, scale_vec); 
		model = glm::rotate( model, degrees[0], xAxis );
		model = glm::rotate( model, degrees[1], yAxis );
		model = glm::rotate( model, degrees[2], zAxis );
	}
	else{
		model = glm::translate(model, translate_vec);
		model = glm::scale(model, scale_vec); 
		model = glm::rotate( model, degrees[0], xAxis );
		model = glm::rotate( model, degrees[1], yAxis );
		model = glm::rotate( model, degrees[2], zAxis );
		translate_vec = zeros;
		scale_vec = ones;
		degrees = zeros;	
	}

	for (SceneNode* n : childs)
		n->update();
}

void SceneNode::updateIds(IDs ids){
	programID = ids[0];
	textureID = ids[1];
	lightID   = ids[2];
	vertexbuffer = ids[3];
	uvbuffer     = ids[4];
	normalbuffer = ids[5];
	verticesNum  = ids[6];	
}

void SceneNode::addChild( SceneNode* n ){
	n->parent = this;
	childs.push_back(n);
}

void SceneNode::draw(GLuint ViewMatrixID, GLuint MatrixID, GLuint ModelMatrixID){
	//cout<<"drawn\n" << to_string(ViewM)<<endl;
	
	glUseProgram(programID);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(textureID, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0,	(void*)0 );


	glm::mat4 mvp = ProjectionM * ViewM * model;
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewM[0][0]);
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &model[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, verticesNum); 
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	for(SceneNode* n : childs)
		n->draw(ViewMatrixID, MatrixID, ModelMatrixID);
}

void SceneNode::rotateX( float degree ){
	degrees[0] += radians(degree);
}

void SceneNode::rotateY( float degree ){
	degrees[1] += radians(degree);
}

void SceneNode::rotateZ( float degree ){
	degrees[2] += radians(degree);
}

void SceneNode::translate(glm::vec3 tr){ 
	translate_vec += tr;
}

void SceneNode::scale(float sc){ 
	scale_vec[0] *= sc;
	scale_vec[1] *= sc;
	scale_vec[2] *= sc;
}

void SceneNode::delete_buffers(){
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &textureID);	
}