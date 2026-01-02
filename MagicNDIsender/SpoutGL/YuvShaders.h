/*

				YuvShaders.h

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

*/
#pragma once
#ifndef __yuvShaders__
#define __yuvShaders__

#include <windows.h>
#include <string>
#include <algorithm> // for std::replace

// Spout OpenGL extensions including compute shader extensions
#include "../../../../apps/SpoutGL/SpoutGLextensions.h"

class yuvShaders {

	public:

		yuvShaders();
		~yuvShaders();

		// RGBA to YUV
		bool yuvShaders::RgbaToYUV(GLuint SourceID, GLuint DestID,
			unsigned int width, unsigned int height,
			bool bBT610);

		// YUV to RGBA
		bool yuvShaders::YUVtoRgba(GLuint SourceID, GLuint DestID,
			unsigned int width, unsigned int height, bool BT601);

		// Swap RGBA<>BGRA
		bool yuvShaders::Swap(GLuint SourceID, unsigned int width, unsigned int height);

		// Shader format
		void SetGLformat(GLint glformat);
		void CheckShaderFormat(std::string &shaderstr);

		// Globals
		GLuint m_yuvProgram     = 0;
		GLuint m_rgbaProgram    = 0;
		GLuint m_swapProgram    = 0;

	protected :

		bool ComputeShader(std::string &shader, GLuint &program, 
			GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height,
			float uniform0 = -1.0, float uniform1 = -1.0,
			float uniform2 = -1.0, float uniform3 = -1.0);

		GLuint CreateComputeShader(std::string shader, unsigned int nWgX, unsigned int nWgY);

		GLint m_GLformat = GL_RGBA8;
		std::string m_GLformatName = "rgba8";

		//
		// Shader source
		//

		//
		// RGBA > UYVY
		//
		std::string m_yuvstr = "layout(rgba8, binding=0) uniform readonly image2D src;\n"
		"layout(rgba8, binding=1) uniform writeonly image2D dst;\n"
		"layout (location = 0) uniform float BT601;\n"
		"void main() {\n"

			"ivec2 pos = ivec2(gl_GlobalInvocationID.xy);\n"

			// Only process even x coordinates (each writes two pixels)
			"if ((pos.x % 2) != 0) return;\n"

			"ivec2 p0 = pos;\n"
			"ivec2 p1 = pos + ivec2(1, 0);\n"

			"vec4 c0 = imageLoad(src, p0);\n"
			"vec4 c1 = imageLoad(src, p1);\n"

			"// Average two pixels for U/V (4:2:2)\n"
			"vec3 avg = (c0.rgb + c1.rgb)*0.5;\n"

			//
			// Convert RGBA to YUV
			//
			// The BT.601 aand BT.709  matrices in specs do not form a perfect
			// forward/inverse pair in "full-range" form. These coefficients
			// are produced by solving for the exact inverse numerically

			"float Y0, Y1, U, V;\n"
			"if(BT601 == 1.0) {\n"
			// BT601
			"    Y0 = 0.299*c0.r + 0.587*c0.g + 0.114*c0.b;\n"
			"    Y1 = 0.299*c1.r + 0.587*c1.g + 0.114*c1.b;\n"
			"    U = -0.167386*avg.r - 0.331264*avg.g + 0.500000*avg.b + 0.5;\n"
			"    V =  0.500000*avg.r - 0.418688*avg.g - 0.081312*avg.b + 0.5;\n"
			"}\n"
			" else {\n"
			// BT709
			"    Y0 = 0.2126*c0.r + 0.7152*c0.g + 0.0722*c0.b;\n"
            "    Y1 = 0.2126*c1.r + 0.7152*c1.g + 0.0722*c1.b;\n"
            "    U  = -0.1146*avg.r - 0.3854*avg.g + 0.5000*avg.b + 0.5;\n"
            "    V  =  0.5000*avg.r - 0.4542*avg.g - 0.0458*avg.b + 0.5;\n"
			"}\n"

			// Convert from full to video-range
			"Y0 = Y0 * 219.0/255.0 + 16.0/255.0;\n"
			"Y1 = Y1 * 219.0/255.0 + 16.0/255.0;\n"
			"U  = U  * 224.0/255.0 + 16.0/255.0;\n"
			"V  = V  * 224.0/255.0 + 16.0/255.0;\n"

			// Clamp
			"Y0 = clamp(Y0, 0.0, 1.0);\n"
			"Y1 = clamp(Y1, 0.0, 1.0);\n"
			"U  = clamp(U,  0.0, 1.0);\n"
			"V  = clamp(V,  0.0, 1.0);\n"

			// Pack into 4 bytes (U, Y0, V, Y1)
			"vec4 uyvy = vec4(U, Y0, V, Y1);\n"

			// Write as one RGBA pixel (reinterpret as UYVY)
			"imageStore(dst, ivec2(pos.x/2, pos.y), uyvy);\n"

		"}\n";

		//
		//  UYVY > RGBA
		//
		std::string m_rgbastr = 
		"layout(rgba8, binding=0) uniform readonly image2D src;\n"
		"layout(rgba8, binding=1) uniform writeonly image2D dst;\n"
		"layout (location = 0) uniform float BT601;\n"
		"void main() {\n"

		    "ivec2 pos = ivec2(gl_GlobalInvocationID.xy);\n"

		    // Only process even X (each invocation outputs 2 RGBA pixels)
		    "if ((pos.x % 2) != 0) return;\n"

		    // Read UYVY block at half resolution
		    // Each src pixel contains U,Y0,V,Y1 for two RGBA output pixels
		    "ivec2 blockPos = ivec2(pos.x/2, pos.y);\n"

		    // Load UYVY packed as (U, Y0, V, Y1)
		    "vec4 uyvy = imageLoad(src, blockPos);\n"

		    // Show U,Y0,V,Y1 directly in RGB
		    "float U  = uyvy.r;\n"
		    "float Y0 = uyvy.g;\n"
		    "float V  = uyvy.b;\n"
		    "float Y1 = uyvy.a;\n"

		    // Convert from video range to full range
		    "Y0 = (Y0 - 16.0/255.0) * (255.0/219.0);\n"
		    "Y1 = (Y1 - 16.0/255.0) * (255.0/219.0);\n"
		    "U  = (U  - 16.0/255.0) * (255.0/224.0);\n"
		    "V  = (V  - 16.0/255.0) * (255.0/224.0);\n"

		    // Clamp
		    "Y0 = clamp(Y0, 0.0, 1.0);\n"
		    "Y1 = clamp(Y1, 0.0, 1.0);\n"
			"U  = clamp(U,  0.0, 1.0);\n"
		    "V  = clamp(V,  0.0, 1.0);\n"
		
			// Shift UV back to +/-0.5
			"float Uf = U - 0.5;\n"
		    "float Vf = V - 0.5;\n"
		
		    "vec3 rgb0;\n"
		    "vec3 rgb1;\n"

		    "if(BT601 == 1.0) {\n"
				 // BT.601 YUV -> RGB"
			"    rgb0.r = Y0 + 1.402   * Vf;\n"
		    "    rgb0.g = Y0 - 0.34414 * Uf - 0.71414 * Vf;\n"
		    "    rgb0.b = Y0 + 1.772   * Uf;\n"
		    "    rgb1.r = Y1 + 1.402   * Vf;\n"
		    "    rgb1.g = Y1 - 0.34414 * Uf - 0.71414 * Vf;\n"
		    "    rgb1.b = Y1 + 1.772   * Uf;\n"
			"}\n"
		    "else {\n"
				 // BT.709 YUV -> RGB
		    "    rgb0.r = Y0 + 1.5748 * Vf;\n"
		    "    rgb0.g = Y0 - 0.1873 * Uf - 0.4681 * Vf;\n"
		    "    rgb0.b = Y0 + 1.8556 * Uf;\n"
		    "    rgb1.r = Y1 + 1.5748 * Vf;\n"
		    "    rgb1.g = Y1 - 0.1873 * Uf - 0.4681 * Vf;\n"
		    "    rgb1.b = Y1 + 1.8556 * Uf;\n"
		    "}\n"

		    // Clamp output RGB
		    "rgb0 = clamp(rgb0, 0.0, 1.0);\n"
		    "rgb1 = clamp(rgb1, 0.0, 1.0);\n"
		
		    // Write two RGBA pixels
		    "imageStore(dst, ivec2(pos.x,     pos.y), vec4(rgb0, 1.0));\n"
		    "imageStore(dst, ivec2(pos.x + 1, pos.y), vec4(rgb1, 1.0));\n"

		"}\n";

		//
		// Swap RGBA <> BGRA
		//
		std::string m_swapstr =
		"layout(rgba8, binding=0) uniform image2D src;\n" // Read/Write
		"void main() {\n"
			"vec4 c0 = imageLoad(src, ivec2(gl_GlobalInvocationID.xy));\n"
			"imageStore(src, ivec2(gl_GlobalInvocationID.xy), vec4(c0.b, c0.g, c0.r, c0.a));\n"
		"}\n";

};

#endif
