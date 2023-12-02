//
// Magic Module Development Kit (MDK) v2.3
// Copyright (c) 2012-2020 Color & Music, LLC.  All rights reserved.
//
// The MDK is provided "as is" without any express or implied warranty
// of any kind, oral or written, including warranties of merchantability,
// fitness for any particular purpose, non-infringement, information
// accuracy, integration, interoperability, or quiet enjoyment.  In no
// event shall Color & Music, LLC or its suppliers be liable for any
// damages whatsoever (including, without limitation, damages for loss
// of profits, business interruption, loss of information, or physical
// damage to hardware or storage media) arising out of the use of, misuse
// of, or inability to use the MDK, your reliance on any content in the
// MDK, or from the modification, alteration or complete discontinuance
// of the MDK, even if Color & Music, LLC has been advised of the
// possibility of such damages.

// =======================================================================================
//
//		MagicNDIsender
//
//		Module plugin for Magic https://magicmusicvisuals.com/
//		Using the Magic MDK
//
//		Using the NDI SDK to send frames over a network http://NDI.Newtek.com/
//		And send class files from ofxNDI Openframeworks addon https://github.com/leadedge/ofxNDI
//		Copyright(C) 2018-2024 Lynn Jarvis https://spout.zeal.co/
//
// =======================================================================================
//	This program is free software : you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with this program.If not, see <http://www.gnu.org/licenses/>.
//
//	Revisions :
//
//	11.08.18	- Create constructor for initialization
//				  Change FindSenders in ofxReceive class to return true for network change
//				  Change logic of sender discovery to skip the section if no network change
//				  Change static data for combobox to char array
//				  Clear texture to black on initialization due to artefacts on first draw
// 13.08.18		- Simplify with CreateNDIsender, UpdateNDIsender and ReleaseNDIsender functions
//				  ReleaseSender() (NDIlib_send_destroy) can freeze for quick repeats
// 16.08.18		- GPU methods for flip image from host fbo
//				  Fix ofxNDIsender::UpdateSender to include aspect ratio and no clock video for async mode
//				  Increment version to 1.001
// 12.10.18		- Specifically set frame rate in CreateSender
//				- Remove Async mode option due to failure in 3.5
//				  Increment version to 1.002
// 13.10.18		- Thread version testing for async mode
//				  TODO Pause/Resume thread for sender change
//				  Hack using Sleep - but needs correct procedure
// 16.10.18		- Update to NDI 3.7 - Async now works clocked
//				  Remove thread testing 
//				  Version 1.003
// 15.11.18		- Rebuild with MAGIC_MDK_VERSION 2.2
//				  Version 1.004
// 10.03.19		- Update to NDI Version 3.8
// 12.03.19		- Cleanup for GitHub
//				  Version 1.005
// 15.03.19		- Set to clocked video for both normal and async sending modes due to
//				  problems with Studio Monitor. Changes made to ofxNDIsend.cpp.
//				  Fps should be set to refresh rate to avoid throttling the application.
//				  Version 1.006
// 18.03.19		- Introduced clock video toggle for 60fps / user fps
//				  Version 1.007
// 19.03.19		- Added tooltips
// 27.03.19		- Update with current ofxNDI source on GitHub
//				  Version 1.008
// 11.11.19		- Update to ofxNDI for NDI 4.0
//				  Version 1.009
// 11.11.19		- Update to ofxNDI for dynamic load of NDI dll
// 08.05.20		- Update local ofxNDI folder for NDI 4.5
//				  Version 1.010
// 27.10.20		- Update to Magic MDK Version 2.3
//				  No code changes. NDI remains at Version 4.5
//				  Version 1.011
// 21.08.21		- Update help text with NDI Version number
//				  Update ofxNDI with NDI Version 5
//				  Release NDI sender in destructor
//				  Rebuild x64 VS2017 / MT
//				  Version 1.012
// 28.09.21		- Update ofxNDI with NDI SDK Version 5.0.3
//				  Rebuild x64 VS2017 / MT
//				  Version 1.013
// 14.11.21		- Change to revised UnloadTexturePixels with bufferstorage
//				  Increase pbo buffer number to three after performance profile
//				  Use Spout extensions instead of Glee
//				  Version 1.014
// 15.12.21		- Change back to UnloadTexturePixels with glMapBuffer due to testing problems.
//				  Equalise version numbers for sender and receiver.
//				  Update to the latest ofxNDI source with NDI SDK Version 5.0.10.1
//				  Update to latest SpoutGLextensions
//				  Fix NDI version number length
//				  Version 1.017
// 11.12.22		- Suppress warnings (see below)
//				  Replace bInitialized with SenderCreated()
//				  Re-order parameters.
//				  Expand tooltip help.
// 13.12.22		  Rebuild with revised ofxNDI
// 26.12.23		- Rebuild with current SpoutGL
//				  Rebuild with revised ofxNDI
//				  Remove compiler warnings due to Microsoft "All rules"
//				  VS2022 /MT Version 1.018
// 22.04.23		- Rebuild with current SpoutGL beta and revised ofxNDI
//			      NDI Vers 5.5.4 - Version to match SpoutToNDI
//				  VS2022 x64 only /MT Version 1.020
// 02.12.23		- Rebuild with current SpoutGL beta 2.007.013 and revised ofxNDI
//			      NDI Vers 5.6.0 - Version to match SpoutToNDI
//				  VS2022 x64 only /MT Version 1.021
//
// =======================================================================================

// Suppress warnings because the OpenGL functions can't be changed
#pragma warning(disable : 26482) // Only index into arrays using constant expressions
#pragma warning(disable : 26485) // Do not pass an array as a single pointer
#pragma warning(disable : 26451) // Arithmetic overflow

// Necessary for OpenGL
// Spout is Windows only
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <gl/gl.h>
#pragma comment(lib, "OpenGL32.Lib")

#include "MagicModule.h"
#include "ofxNDIsend.h"
#include "ofxNDIutils.h" // for SSE CopyImage function
#include "SpoutGL\SpoutGLextensions.h" // Spout extensions (standalone define)

// Convenience definitions
#define PARAM_SenderName 0
#define PARAM_Fps        1
#define PARAM_Clock      2
#define PARAM_Async      3
#define PARAM_Buffer     4

// Number of parameters
#define NumParams 5

#ifndef GL_READ_FRAMEBUFFER_EXT
#define GL_READ_FRAMEBUFFER_EXT 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER_EXT
#define GL_DRAW_FRAMEBUFFER_EXT 0x8CA9
#endif

class MagicNDIsenderModule : public MagicModule
{
	// see below (after class definition) for static value assignments
	static const MagicModuleSettings settings;
	static const MagicModuleParam params[NumParams];

public:

	MagicNDIsenderModule() {
		
		// Console window for debugging
		/*
		FILE* pCout; // should really be freed on exit
		AllocConsole();
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		printf("MagicNDIsenderModule\n");
		*/

		SenderName[0] = 0;
		UserSenderName[0] = 0; // sender is created when the user enters a name
		m_Width = 0;
		m_Height = 0;
		spout_buffer = nullptr;
		m_fbo = 0;
		m_glTexture = 0;
		bClock = true;
		bBuffer = true;
		bAsync = false;
		m_pbo[0] = 0;
		m_pbo[1] = 0;
		m_pbo[2] = 0;
		PboIndex = 0;
		NextPboIndex = 0;
		m_frate_N = 60000; // default 60 fps
		m_frate_D = 1000;
		hlp.reserve(1024); // reserve plenty instead of allocate on the stack
	}

	~MagicNDIsenderModule() {
		// All has been done in glClose
	}

	// getSettings() and getParams() are called whenever
	// a new instance of the module is created.
	const MagicModuleSettings *getSettings() {
		return &settings;
	}

	const MagicModuleParam *getParams() { 
		return params; 
	}
	
	virtual void glInit(MagicUserData *userData) {

		// Load Spout extensions instead of using Glee
		loadGLextensions();

		// pbo for pixel data transfer
		if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
		glGenBuffers(3, m_pbo);

		// fbo and texture for invert rather than cpu invert using ofxUtils::CopyImage
		if (m_fbo) glDeleteFramebuffersEXT(1, &m_fbo);
		glGenFramebuffersEXT(1, &m_fbo);

		if (m_glTexture) glDeleteTextures(1, &m_glTexture);
		glGenTextures(1, &m_glTexture);

	};
	
	virtual void glClose() {

		// Release sender and resources
		ReleaseNDIsender();
		if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
		if (m_fbo) glDeleteFramebuffersEXT(1, &m_fbo);
		if (m_glTexture) glDeleteTextures(1, &m_glTexture);

	};

	void drawBefore(MagicUserData *userData) {

	};

	void drawAfter(MagicUserData *userData) {

		if (userData->glState->currentFramebuffer == 0)
			return;

		// If there is no sender name yet, the sender cannot be created
		if (!UserSenderName[0])
			return; // keep waiting for a name

		// Otherwise create a sender if not initialized yet
		if (!ndisender.SenderCreated()) {
			if (!CreateNDIsender(UserSenderName, userData)) {
				MessageBeep(0);
				UserSenderName[0] = 0; // wait for another name to be entered
				return;
			}
			return; // give it one frame to initialize
		}
		// Has the user entered a new sender name ?
		if (strcmp(SenderName, UserSenderName) != 0) {
			// Release sender and start again
			ReleaseNDIsender();
			return; // initialize on the next frame
		}

		// Has the input size changed ?
		if (m_Width != (unsigned int)userData->glState->viewportWidth
		|| m_Height != (unsigned int)userData->glState->viewportHeight) {
			// Update sender for new size
			// m_Width and m_Height are updated
			UpdateNDIsender((unsigned int)userData->glState->viewportWidth, (unsigned int)userData->glState->viewportHeight);
			return; // return for the next frame
		}

		if (ndisender.SenderCreated()) {
			// Get a texture from the host fbo and flip at the same time
			// to avoid flipping the pixel buffer using cpu memory
			if (FlipTexture(m_Width, m_Height, userData->glState->currentFramebuffer)) {
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
				if (bBuffer) {
					UnloadTexturePixels(m_Width, m_Height,  spout_buffer, GL_RGBA, userData->glState->currentFramebuffer);
				}
				else {
					glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)spout_buffer);
				}
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, userData->glState->currentFramebuffer);
				// The video frame send is clocked at the sender fps
				ndisender.SendImage(spout_buffer, m_Width, m_Height, false, false);
			}
		}
	}

	bool fixedParamValueChanged(const int whichParam, const char* newValue) {
		
		if (!newValue)
			return false;

		if(newValue[0] == 0)
			return false;

		int iValue = atoi(newValue);

		switch (whichParam) {

			// Sender name
			case PARAM_SenderName:
				strcpy_s(UserSenderName, 256, newValue);
				break;

			// NDI sender fps
			case PARAM_Fps:
				m_frate_N = (int)(atof(newValue) * 1000.0);
				m_frate_D = 1000;
				if (bClock) {
					ndisender.SetFrameRate(m_frate_N, m_frate_D);
					if (ndisender.SenderCreated())
						ndisender.UpdateSender(m_Width, m_Height);
				}
				break;

			case PARAM_Clock:
				bClock = (iValue == 1);
				ndisender.SetClockVideo(bClock);
				if (!bClock) {
					// default to 60 fps
					ndisender.SetFrameRate(60000, 1000);
				}
				else {
					// Restore user fps for NDI
					ndisender.SetFrameRate(m_frate_N, m_frate_D);
				}
				if (ndisender.SenderCreated()) {
					ndisender.ReleaseSender(); // no need to reset buffers
					// Re-create the sender because clock_video is
					// part of NDI_send_create_desc used to create the sender
				}

				break;

			// Async mode
			case PARAM_Async:
				bAsync = (iValue == 1);
				ndisender.SetAsync(bAsync);
				if (ndisender.SenderCreated())
					ndisender.ReleaseSender();
				break;

			// Buffering
			case PARAM_Buffer:
				// Local only
				bBuffer = (iValue == 1);
				break;

			default:
				break;

		}

		return true;
	
	}

	// paramCurrentlyEnabled() is called when the module needs to determine
	// what parameters are being used. It can return false if a specific parameter
	// temporarily needs to be displayed as greyed out in the module GUI.
	bool paramCurrentlyEnabled(const int whichParam) {

		switch (whichParam) {

			case PARAM_Fps:
				if (!bClock)
					return false;
				break;

			default:
				break;
		}
		
		return true; 
	
	}


	const char *getHelpText() {

		hlp = "Magic NDI Sender - Vers 1.021\n"
			  "Sends textures to NDI Receivers\n\n"
			  "  Sender : sender name\n"
			  "  Fps : set frame rate for sending\n"
			  "  Clock video : clock frame rate to fps\n"
			  "  Async : asynchronous sending\n"
			  "  Buffering : use OpenGL pixel buffering\n\n"
			  "Lynn Jarvis 2018-2024 - https://spout.zeal.co/ \n"
			  "For Magic MDK Version 2.3\n"
			  "2012-2020 Color & Music, LLC.\n"
			  "Newtek - https://www.ndi.tv/ \nLibrary version : ";
			  // Get NewTek library (dll) version number
			  // Version number is the last 8 chars - e.g 5.1.0.10
			  std::string NDIversion = ndisender.GetNDIversion();
			  std::string NDInumber = ndisender.GetNDIversion().substr(NDIversion.length() - 8, 8);
			  hlp += NDInumber;

		return hlp.c_str();

	}

protected:

	// Initialize variables in constructor

	ofxNDIsend ndisender;
	char UserSenderName[256];
	char SenderName[256];
	unsigned int m_Width;
	unsigned int m_Height;
	int m_frate_N; // default 60 fps
	int m_frate_D;
	bool bClock;
	bool bBuffer;
	bool bAsync;
	unsigned char * spout_buffer;
	GLuint m_pbo[3];
	int PboIndex;
	int NextPboIndex;
	GLuint m_fbo;
	GLuint m_glTexture;
	std::string hlp;
	
	bool FlipTexture(unsigned int width, unsigned int height, GLuint HostFBO)
	{
		GLenum status = 0;

		if (HostFBO == 0)
			return false;

		// Create an fbo if not already
		if (m_fbo == 0) {
			glGenFramebuffersEXT(1, &m_fbo);
		}

		// Create pbos if not already
		if (m_pbo[0] == 0) {
			glGenBuffers(3, m_pbo);
		}

		// Resize texture and global size if necessary
		if (m_glTexture == 0 || width != m_Width || height != m_Height) {
			m_Width = width;
			m_Height = height;
			InitTexture(m_glTexture, GL_TEXTURE_2D, m_Width, m_Height);
		}

		// Host fbo is already bound for read

		// Bind our local fbo for draw
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_fbo);
		// Draw to the first attachment point
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		// Attach the texture we write into
		glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
			// copy one texture buffer to the other while flipping upside down 
			glBlitFramebufferEXT(0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
		else {
			PrintFBOstatus(status);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
			return false;
		}

		// restore the host fbo
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

		return true;

	} // end FlipTexture


	//
	// Asynchronous Read-back from a texture
	//
	// Adapted from : http://www.songho.ca/opengl/gl_pbo.html
	//
	// Assumes RGBA image format
	//

	bool UnloadTexturePixels(unsigned int width, unsigned int height,
		unsigned char* data, GLenum glFormat, GLuint HostFBO)
	{

		if (data == nullptr || m_glTexture == 0 || m_fbo == 0 || m_pbo[0] == 0) {
			return false;
		}

		void* pboMemory = nullptr;
		PboIndex = (PboIndex + 1) % 2;
		NextPboIndex = (PboIndex + 1) % 2;

		// Attach the texture to the fbo
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_glTexture, 0);

		// Set the target framebuffer to read
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

		// Bind the PBO
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[PboIndex]);

		// Null existing data to avoid a stall
		glBufferData(GL_PIXEL_PACK_BUFFER, width*height * 4, 0, GL_STREAM_READ);

		// Read pixels from framebuffer to PBO - glReadPixels() should return immediately.
		glReadPixels(0, 0, width, height, glFormat, GL_UNSIGNED_BYTE, (GLvoid*)0);

		// If there is data in the next pbo from the previous call, read it back

		// Map the PBO to process its data by CPU
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[NextPboIndex]);

		// glMapBuffer can return NULL when called the first time
		// when the next pbo has not been filled with data yet
		pboMemory = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

		if (pboMemory) {
			// Update data directly from the mapped pbo buffer with SSE optimisations
			ofxNDIutils::CopyImage((const unsigned char*)pboMemory, (unsigned char*)data, width, height, width * 4);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}
		else {
			glGetError(); // soak up the last error
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);
			return false;
		}

		// Back to conventional pixel operation
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		// Restore the previous fbo binding
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

		return true;
	}


	void PrintFBOstatus(GLenum status)
	{
		char tmp[256];
		sprintf_s(tmp, 256, "FBO status error 0x%x (%u) - ", status, status);
		if (status == GL_FRAMEBUFFER_UNSUPPORTED_EXT)
			strcat_s(tmp, 256, "GL_FRAMEBUFFER_UNSUPPORTED_EXT");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT)
			strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT)
			strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
			strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT - width-height problems?");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT)
			strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT)
			strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT)
			strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
		// else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT)
		// 	strcat_s(tmp, 256, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT\n");
		else
			strcat_s(tmp, 256, "Unknown Code");
		printf("fbo error [%s]\n", tmp);
		GLerror();
	}

	void GLerror() {
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			printf("GLerror = %u (0x%x)\n", err, err);
			// gluErrorString needs glu32.lib
			// printf("GL error = %u (0x%x) %s\n", err, err, gluErrorString(err));
		}
	}

	// Initialize local OpenGL texture
	void InitTexture(GLuint &texID, GLenum GLformat, unsigned int width, unsigned int height)
	{
		if (texID != 0) 
			glDeleteTextures(1, &texID);
		glGenTextures(1, &texID);

		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GLformat, width, height, 0, GLformat, GL_UNSIGNED_BYTE, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	bool CreateNDIsender(const char * name, MagicUserData * userdata)
	{
	
		// Close existing sender
		if (ndisender.SenderCreated())
			ReleaseNDIsender();

		// Update the sender name so that SenderName can be checked for change
		strcpy_s(SenderName, 256, name);

		// Set global width and height so any size change can be tested
		m_Width  = (unsigned int)userdata->glState->viewportWidth;
		m_Height = (unsigned int)userdata->glState->viewportHeight;

		// Create an RGBA buffer to send to NDI
		if (spout_buffer)
			free((void *)spout_buffer);
		spout_buffer = (unsigned char *)malloc(m_Width*m_Height * 4 * sizeof(unsigned char));
		if (!spout_buffer)
			return false;

		// Create or resize the flip texture
		InitTexture(m_glTexture, GL_RGBA, m_Width, m_Height);

		// Reset pbos because for a name change, NextPboIndex might still have data in it
		if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
		m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;
		PboIndex = NextPboIndex = 0;
		
		// Set current modes except frame rate which is set by the user
		ndisender.SetAsync(bAsync);
		ndisender.SetClockVideo(bClock);

		// Create a new sender
		return(ndisender.CreateSender(SenderName, m_Width, m_Height));

	}

	bool UpdateNDIsender(unsigned int width, unsigned int height)
	{
		if (!ndisender.SenderCreated())
			return false;

		// Update the RGBA buffer to send to NDI
		if (spout_buffer) free((void *)spout_buffer);
		spout_buffer = (unsigned char *)malloc(width*height * 4 * sizeof(unsigned char));
		if (!spout_buffer) {
			printf("UpdateNDIsender : spout_buffer not initialized\n");
			return false;
		}

		// Update the texture used for flipping
		InitTexture(m_glTexture, GL_RGBA, width, height);

		// Update width and height
		m_Width = width;
		m_Height = height;

		// Set current modes
		ndisender.SetAsync(bAsync);
		ndisender.SetClockVideo(bClock);

		// Reset pbos because NextPboIndex might still have data in it
		if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
		m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;
		PboIndex = NextPboIndex = 0;

		// Update existing sender
		return(ndisender.UpdateSender(m_Width, m_Height));


	}

	void ReleaseNDIsender()
	{
		if (!ndisender.SenderCreated()) {
			printf("ReleaseNDIsender : sender not initialized\n");
			return;
		}

		m_Width = m_Height = 0;
		if (spout_buffer) free((void *)spout_buffer);
		spout_buffer = nullptr;
		SenderName[0] = 0;
		ndisender.ReleaseSender();

	}


};


MagicModule *CreateInstance() {
	return new MagicNDIsenderModule();
}

const MagicModuleSettings MagicNDIsenderModule::settings = MagicModuleSettings(NumParams);

const MagicModuleParam MagicNDIsenderModule::params[NumParams] ={
	MagicModuleParam("Sender", NULL, NULL, NULL, MVT_STRING, MWT_TEXTBOX, true, "Sender name"),
	MagicModuleParam("Fps", NULL, NULL, NULL, MVT_STRING, MWT_TEXTBOX, true,
	"Set frame rate"),
	MagicModuleParam("Clock video", "1", "0", "1", MVT_BOOL, MWT_TOGGLEBUTTON, true, 
	"If fps is set, the output rate is clocked to the fps value"),
	MagicModuleParam("Async", "0", "0", "1", MVT_BOOL, MWT_TOGGLEBUTTON, true, "Can improve performance because NDI buffers the output frames and does not wait for the last frame to be finished"),
	MagicModuleParam("Buffering", "0", "0", "1", MVT_BOOL, MWT_TOGGLEBUTTON, true, "This activates OpenGL pixel buffering of texture to pixel conversion. Three buffers are used and consume CPU memory. This is well within capabilities of modern hardware but you can disable if you have insufficent memory. ")
};
