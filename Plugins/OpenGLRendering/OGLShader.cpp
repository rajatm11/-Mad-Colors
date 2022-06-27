/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "OGLShader.h"
#include "../../Common/Assets.h"
#include <iostream>
#include "../../CSC8503/CSC8503Common/Debug.h"

using namespace NCL;
using namespace NCL::Rendering;

GLuint shaderTypes[(int)ShaderStages::SHADER_MAX] = {
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_GEOMETRY_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER
};

string ShaderNames[(int)ShaderStages::SHADER_MAX] = {
	"Vertex",
	"Fragment",
	"Geometry",
	"Tess. Control",
	"Tess. Eval"
};

OGLShader::OGLShader(const string& vertex, const string& fragment, const string& geometry, const string& domain, const string& hull) :
	ShaderBase(vertex, fragment, geometry, domain, hull) {

	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		shaderIDs[i]	= 0;
		shaderValid[i]	= 0;
	}
	programID = 0;

	ReloadShader();
}

OGLShader::~OGLShader()	{
	DeleteIDs();
}

void OGLShader::ReloadShader() {
	DeleteIDs();
	programID = glCreateProgram();
	string fileContents = "";
	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		if (!shaderFiles[i].empty()) {
			if (Assets::ReadTextFile(Assets::SHADERDIR + shaderFiles[i], fileContents)) {
				shaderIDs[i] = glCreateShader(shaderTypes[i]);

				string msg = "Reading " + ShaderNames[i] + " shader " + shaderFiles[i];
				Debug::PrintToConsole(msg);

				const char* stringData	 = fileContents.c_str();
				int			stringLength = (int)fileContents.length();
				glShaderSource(shaderIDs[i], 1, &stringData, &stringLength);
				glCompileShader(shaderIDs[i]);

				glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &shaderValid[i]);
		
				if (shaderValid[i] != GL_TRUE) {
					string msg = ShaderNames[i] + " shader " + " has failed!";
					Debug::PrintToConsole(msg);
				}
				else {
					glAttachShader(programID, shaderIDs[i]);
				}
				PrintCompileLog(shaderIDs[i]);
			}
		}
	}	
	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &programValid);

	PrintLinkLog(programID);

	if (programValid != GL_TRUE) {
		string msg = "This shader has failed!";
		Debug::PrintToConsole(msg);
	}
	else {
		string msg = "Shader loaded!";
		Debug::PrintToConsole(msg);
	}
}

void	OGLShader::DeleteIDs() {
	if (!programID) {
		return;
	}
	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		if (shaderIDs[i]) {
			glDetachShader(programID, shaderIDs[i]);
			glDeleteShader(shaderIDs[i]);
		}
	}
	glDeleteProgram(programID);
	programID = 0;
}

void	OGLShader::PrintCompileLog(GLuint object) {
	int logLength = 0;
	glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength) {
		char* tempData = new char[logLength];
		glGetShaderInfoLog(object, logLength, NULL, tempData);
		string msg = "Compile Log:\n" + *tempData;
		Debug::PrintToConsole(msg);
		delete tempData;
	}
}

void	OGLShader::PrintLinkLog(GLuint program) {
	int logLength = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength) {
		char* tempData = new char[logLength];
		glGetProgramInfoLog(program, logLength, NULL, tempData);
		string msg = "Link Log:\n" + *tempData;
		Debug::PrintToConsole(msg);
		delete tempData;
	}
}