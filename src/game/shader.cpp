/*
 * Shader object -- see header file for more info
 */
#include <string>
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

#include<iostream>
#include<fstream>
#include <sstream>
#include "shader.h"


namespace Assets {
using namespace std;

/*Shader::Shader(string fileName){
	program = 0;
	ifstream t;
	int length;
	char* buffer;
	t.open(fileName.c_str());     
	t.seekg(0, ios::end);
	length = t.tellg();
	t.seekg(0, ios::beg);
	buffer = new char[length];
	t.read(buffer, length);
	t.close();
	printf(buffer);
	delete[] buffer;
}*/

string Shader::readFile(string fileName){
	ifstream t(fileName.c_str());
	stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

void Shader::init(string fileName){
	string shadsrc	=	readFile(fileName);
	shaderID		=	compileShader(shadsrc);
	char* report 	= 	new char[10000];
	int l;
	glGetShaderInfoLog(shaderID, 10000, &l, report);
	if(l){
		printf("debug report for: %s \n %s\n shader source:\n %s\n----\n",fileName.c_str(), report, shadsrc.c_str());
		
	}
}
Shader::~Shader(){
	//glDeleteShader(shaderID);
}

GLuint VertexShader::compileShader(string shader){
	GLuint result = glCreateShader(GL_VERTEX_SHADER);
	const char* shadersrc = shader.c_str();
	glShaderSource(result, 1, &(shadersrc) , NULL);
	glCompileShader(result);
	return result;
}

GLuint FragmentShader::compileShader(string shader){
	GLuint result = glCreateShader(GL_FRAGMENT_SHADER);
	const char* shadersrc = shader.c_str();
	glShaderSource(result, 1, &(shadersrc) , NULL);
	glCompileShader(result);
	return result;
}

GLuint GeometryShader::compileShader(string shader){
	GLuint result = glCreateShader(GL_GEOMETRY_SHADER_EXT);
	const char* shadersrc = shader.c_str();
	glShaderSource(result, 1, &(shadersrc) , NULL);
	glCompileShader(result);
	return result;
}

void ShaderProgram::init(Shader** s, size_t length){
	program = glCreateProgram();
	
	for(int i = 0; i < length; i++){
		if(s[i]){
			glAttachShader(program, s[i]->shaderID);
		}
	}
	glLinkProgram(program);
	char* infoLog = new char[10000];
	int l;
	glGetProgramInfoLog(program, 10000, &l, infoLog);
	printf(infoLog);
}

ShaderProgram::ShaderProgram(Shader** s, size_t length){
	init ( s, length);
}

ShaderProgram::ShaderProgram(string vsFileName, string fsFileName){
	VertexShader* 	vs	=	new VertexShader(vsFileName);
	FragmentShader*	fs	=	fsFileName.empty()	?	NULL	:	new FragmentShader(fsFileName);
	Shader** s	= new Shader*[2];
	s[0]		= vs;
	s[1]		= fs;
	init ( s, 2);
	delete vs;
	delete fs;
	delete[] s;
}

void ShaderProgram::load(){
	if(!glIsProgram(program)){
		printf("dafuq\n");
	}
	printf("loading program: %d\n", program);
	glUseProgram(program);
	int p;
	glGetIntegerv(GL_CURRENT_PROGRAM, &p);
	printf("current program: %d\n", p);
}

void ShaderProgram::unload(){
	glUseProgram(0);
}

ShaderProgram::~ShaderProgram(){
	glDeleteProgram(program);
}

}