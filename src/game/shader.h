/*
 * Shader object
 *
 * Date: 08-06-12 16:00
 *
 * Description: A class which can be used to load shaders from a file and use them in openGL
 *
 */

#ifndef SHADER_H
#define SHADER_H

#include <string>


namespace Assets{

using namespace std;

class Shader{
	//The shader assembler
	friend class ShaderProgram;
	
	GLuint shaderID;
	string readFile(string fileName);
	
	virtual GLuint compileShader(string shader) = 0;
	
	protected:
	
	void init(string fileName);
	
	public:
	Shader(){}
	
	~Shader();

};

class VertexShader : public Shader{
	
	GLuint compileShader(string shader);
	
	public:
	
	VertexShader(string fileName) {init(fileName);}

};


class FragmentShader : public Shader{
	
	GLuint compileShader(string shader);
	
	public:
	
	FragmentShader(string fileName) {init(fileName);}
};

//TODO primitives still need to be linked.
//Not yet used so only signature made for further usages, shader is also not widely supported.
class GeometryShader : public Shader{

	GLuint compileShader(string shader);
	
	public:
	
	GeometryShader(string fileName) {init(fileName);}


};

class ShaderProgram {

	GLuint program;
	void init(Shader** s, size_t length);
	public:
	
	//Constructors:
	ShaderProgram(Shader** s, size_t length);
	ShaderProgram(){ program = 0;}
	//Constructor by string
	ShaderProgram(string vsFileName, string fsFileName = "");
	~ShaderProgram();
	
	GLuint getProgramID(){return program;}
	void load();
	void unload();
};

}

#endif
