/*
				YuvShaders.cpp

		Functions to manage RGBA <> UYVY compute shaders

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Copyright (c) 2026, Lynn Jarvis. All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, 
	are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	========================

	25.11.25 - first version

*/

#include "yuvShaders.h"

//
// Class: yuvShaders
//
// Functions to manage compute shaders
//

yuvShaders::yuvShaders() {
	loadGLextensions();
}


yuvShaders::~yuvShaders() {

	if (m_yuvProgram      > 0) glDeleteProgram(m_yuvProgram);
	if (m_rgbaProgram     > 0) glDeleteProgram(m_rgbaProgram);
	if (m_swapProgram     > 0) glDeleteProgram(m_swapProgram);

}

//---------------------------------------------------------
// Function: RgbaToYUV
//
bool yuvShaders::RgbaToYUV(GLuint SourceID, GLuint DestID,
	unsigned int width, unsigned int height,
	bool BT601)
{
	return ComputeShader(m_yuvstr, m_yuvProgram, SourceID, DestID,
		width, height, (float)BT601);
}

//---------------------------------------------------------
// Function: YUVtoRGBA
//
bool yuvShaders::YUVtoRgba(GLuint SourceID, GLuint DestID,
	unsigned int width, unsigned int height, bool BT601)
{
	return ComputeShader(m_rgbastr, m_rgbaProgram, SourceID, DestID,
		width, height, (float)BT601);
}

//---------------------------------------------------------
// Function: Swap
// Swap RGBA <> BGRA
bool yuvShaders::Swap(GLuint SourceID, unsigned int width, unsigned int height)
{
	// return ComputeShader(m_swapstr, m_swapProgram, SourceID, 0, width, height);
	return ComputeShader(m_swapstr, m_swapProgram, SourceID, 0, width, height);
}


//---------------------------------------------------------
// Function: SetGLformat
// Set OpenGL format for shaders
// GL_RGBA8, GL_RGBA16, GL_RGBA16F, GL_RGBA32F
// https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
void yuvShaders::SetGLformat(GLint glformat)
{
	if (m_GLformat != glformat) {

		GLint oldformat = m_GLformat;
		std::string oldformatname = m_GLformatName;

		m_GLformat = glformat;
		switch (m_GLformat) {
		case GL_RGBA:
		case GL_RGBA8:
			// GL_RGBA not supported by shaders
			m_GLformat = GL_RGBA8;
			m_GLformatName = "rgba8";
			break;
		case GL_RGBA16:
			m_GLformatName = "rgba16";
			break;
		case GL_RGBA16F:
			m_GLformatName = "rgba16f";
			break;
		case GL_RGBA32F:
			m_GLformatName = "rgba32f";
			break;
		default:
			break;
		}

		// Shaders have to be re-compiled
		if (m_yuvProgram      > 0) glDeleteProgram(m_yuvProgram);
		if (m_rgbaProgram     > 0) glDeleteProgram(m_rgbaProgram);
		if (m_swapProgram     > 0) glDeleteProgram(m_swapProgram);
		m_yuvProgram      = 0;
		m_rgbaProgram     = 0;
		m_swapProgram     = 0;

		// No notice for GL_RGBA -> GL_RGBA8
		if (glformat != GL_RGBA) {
			printf("YuvShaders::SetGLformat - shader format reset from 0x%X (%s) to 0x%X (%s)\n",
				oldformat, oldformatname.c_str(), m_GLformat, m_GLformatName.c_str());
		}

	}

}

//---------------------------------------------------------
// Function: CheckShaderFormat
// Check shader source for correct format description
void yuvShaders::CheckShaderFormat(std::string &shaderstr)
{
	// Find existing format name "layout(rgba8, etc
	size_t pos1 = shaderstr.find("(");
	pos1 += 1; // Skip the '(' character
	size_t pos2 = shaderstr.find(",");
	std::string formatname = shaderstr.substr(pos1, pos2-pos1);

	// Find matching format name
	if (m_GLformatName != formatname) {
		// Replace shader format name
		size_t pos = 0;
		while (pos += m_GLformatName.length()) {
			pos = shaderstr.find(formatname, pos);
			if (pos == std::string::npos) {
				break;
			}
			shaderstr.replace(pos, formatname.length(), m_GLformatName);
		}
	}
}


//---------------------------------------------------------
// Function: ComputeShader
//    Apply compute shader on source to dest
//    or to read/write source with provided uniforms
bool yuvShaders::ComputeShader(std::string &shaderstr, GLuint &program,
	GLuint SourceID, GLuint DestID,
	unsigned int width, unsigned int height,
	float uniform0, float uniform1, float uniform2, float uniform3)
{
	if (shaderstr.empty() || SourceID == 0) {
		printf("yuvShaders::ComputeShader - no shader or texture\n");
		return false;
	}

	// Opengl context is necessary
	if(!wglGetCurrentContext()) {
		printf("yuvShaders::ComputeShader - no OpenGL context\n");
		return false;
	}

	// Local size must match in the shader (see CreateComputeShader)
	// 32x32 is better for NVIDIA but Intel may require 16x16
	const GLuint local_size_x = 16;
	const GLuint local_size_y = 16;
	// The number of Y and Y work groups should match image width and height
	GLuint nWgX = (width  + local_size_x - 1) / local_size_x;
	GLuint nWgY = (height + local_size_y - 1) / local_size_y;

	if (program == 0) {

		// Check shader source for correct format name
		CheckShaderFormat(shaderstr);

		program = CreateComputeShader(shaderstr, local_size_x, local_size_y);

		if (program == 0) {
			printf("yuvShaders::ComputeShader - CreateComputeShader failed\n");
			return false;
		}
	}

	glUseProgram(program);
	glBindImageTexture(0, SourceID, 0, GL_FALSE, 0, GL_READ_WRITE, m_GLformat);
	if(DestID > 0)
	  glBindImageTexture(1, DestID, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_GLformat);

	if (uniform0 != -1.0) glUniform1f(0, uniform0);
	if (uniform1 != -1.0) glUniform1f(1, uniform1);
	if (uniform2 != -1.0) glUniform1f(2, uniform2);
	if (uniform3 != -1.0) glUniform1f(3, uniform3);

	glDispatchCompute(nWgX, nWgY, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, m_GLformat);
	glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_GLformat);
	glUseProgram(0);

	return true;

}

//---------------------------------------------------------
// Function: CreateComputeShader
// Create compute shader from a source string
unsigned int yuvShaders::CreateComputeShader(std::string shader, unsigned int nWgX, unsigned int nWgY)
{
	// Compute shaders are only supported since openGL 4.3
	int major = 0;
	int minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	float version = (float)major + (float)minor / 10.0f;
	if (version < 4.3f) {
		printf("CreateComputeShader - OpenGL version > 4.3 required\n");
		return 0;
	}

	//
	// Compute shader source initialized in header
	//

	// Common
	std::string shaderstr = "#version 440\n";
	shaderstr += "layout(local_size_x = ";
	shaderstr += std::to_string(nWgX);
	shaderstr += ", local_size_y = ";
	shaderstr += std::to_string(nWgY);
	shaderstr += ", local_size_z = 1) in;\n";

	// Full shader string
	shaderstr += shader;

	// Create the compute shader program
	GLuint computeProgram = glCreateProgram();

	if (computeProgram > 0) {
		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
		if (computeShader > 0) {
			// Compile and link shader
			GLint status = 0;
			const char* source = shaderstr.c_str();
			glShaderSource(computeShader, 1, &source, NULL);
			glCompileShader(computeShader);
			glAttachShader(computeProgram, computeShader);
			glLinkProgram(computeProgram);
			glGetProgramiv(computeProgram, GL_LINK_STATUS, &status);
			if (status == 0) {
				printf("CreateComputeShader - glGetProgramiv failed\n");
				glDetachShader(computeProgram, computeShader);
				glDeleteProgram(computeShader);
				glDeleteProgram(computeProgram);
			}
			else {

				// After linking, the shader object is not needed
				glDeleteShader(computeShader);

				return computeProgram;
			}
		}
		else {
			printf("CreateComputeShader - glCreateShader failed\n");
			return 0;
		}
	}
	printf("CreateComputeShader - glCreateProgram failed");
	return 0;
}

