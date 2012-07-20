/*
 * Collision algorithms -- see header file for more info
 */

#include <stdlib.h>
#include <stdio.h>
#if defined _WIN32
	#define WIN32_LEAN_AND_MEAN 1
	#include <windows.h>
	#include <gl\gl.h>
	#include <gl\glu.h>
	#include <gl\glext.h>
#elif defined __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif
#include "collision.h"
#include "shader.h"


namespace Collision{

using namespace Assets;


ShaderProgram* collShader;
bool InCollisionPhase = true;
GLint objID_1, objID_2, subID, shaderProgram;


void MarkObject(uint16_t objID){
	if(InCollisionPhase){
		glUniform1f(objID_1, (objID >> 8 & 0xFF)/255.0);
		glUniform1f(objID_2, (objID & 0xFF)/255.0);
		glUniform1f(subID, 0.0);
	}
}

void MarkSubType(uint8_t stID){
	if(InCollisionPhase){
		glUniform1f(subID, (stID & 0xFF)/255.0);
	}
}

void UnregisterObject(ObjectHandle oh){

}

void RegisterObject(ObjectHandle oh){

}


void DetectCollision(Pd start, Vd direction, double maxLength, void (*onCollision)(ObjectHandle, uint8_t), double lineThickness){

}

void ProcessCollisions(){
	InCollisionPhase = true;
	
	InCollisionPhase = false;
}

void Initialize(){
	printf("error %d \n", glGetError());
	collShader = new ShaderProgram("coll.vert", "coll.frag");
	collShader->load();
 	printf("error %s \n", gluErrorString(glGetError()));
	//TODO Load shader program
	objID_1 = glGetUniformLocation(collShader->getProgramID(), "objID_1");
	objID_2	= glGetUniformLocation(collShader->getProgramID(), "objID_2");
	subID	= glGetUniformLocation(collShader->getProgramID(), "subID");
	MarkObject(0xFFFF);
	MarkSubType(255);
}

void Terminate(){

}

}
