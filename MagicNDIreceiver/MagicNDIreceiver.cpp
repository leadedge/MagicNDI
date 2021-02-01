//
// Magic Module Development Kit (MDK) v2.11
// Copyright (c) 2012-2017 Color & Music, LLC.  All rights reserved.
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
//		MagicNDIreceiver
//
//		Module plugin for Magic https://magicmusicvisuals.com/
//		Using the Magic MDK
//
//		Using the NDI SDK to send frames over a network https://www.ndi.tv/
//		And send class files from ofxNDI Openframeworks addon https://github.com/leadedge/ofxNDI
//		Copyright(C) 2018-2021 Lynn Jarvis https://spout.zeal.co/
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
// 11.08.18		- Create constructor for initialization
//				  Change FindSenders in ofxReceive class to return true for network change
//				  Change logic of sender discovery to skip the section if no network change
//				  Change static data for combobox to char array
//				  Clear texture to black on initialization due to artefacts on first draw
// 13.10.18		- Fix aspect adjustment
//				  Version 1.002 - initial release
// 16.10.18		- Update to NDI Version 3.7
//				  Remove buffer option - slower than without
//				  Version 1.003
// 15.11.18		- Rebuild with MAGIC_MDK_VERSION 2.2
//				  Version 1.004
// 09.03.19		- Remove fbo check in drawbefore
//				  See : https://magicmusicvisuals.com/forums/viewtopic.php?f=2&t=1756&p=8464#p8464
//				  Version 1.005
// 10.03.19		- Update to NDI Version 3.8
// 12.03.19		- Cleanup for GitHub
//				  Version 1.006
// 15.03.19		- Remove LoadTexturePixels
//				  Retained by error during cleanup for GitHub
//				  Previously confirmed that it is slower than glTexSubImage2D
//				  Version 1.007
// 28.04.19		- Rebuild x86 and x64 VS2017 /MT
// 11.11.19		- Update to ofxNDI for NDI 4.0
//				  Version 1.008
// 15.11.19		- Update to ofxNDI with dynamic loading
//				- Bump version number to match with sender
//				  Version 1.010
// 19.11.19		- Disable audio receive
// 08.05.20		- Update to ofxNDI with NDI 4.5
//				  Version 1.011
// 27.10.20		- Update to Magic MDK Version 2.3
//				  No code changes. NDI remains at Version 4.5
//				  Version 1.012
// 29.01.21		- Changes for multiple scenes and context refresh
//				  Version 1.013
//
// =======================================================================================

// necessary for OpenGL
#ifdef _WIN32
#include <windows.h>
#include "lib/glee/GLee.h" // Include before gl.h
#include <gl/gl.h>
#pragma comment(lib, "OpenGL32.Lib")
#else // (assumes OS X)
#include <OpenGL/gl.h>
// TODO : extensions ?
#endif

#include <conio.h>
#include <stdio.h>
#include <thread>
#include <mutex>

#include "MagicModule.h"
#include "ofxNDIreceive.h" // TODO - OSX - should be compatible

// Name list for the combo box
static std::string senderList = ""; // Name list to compare for changes
static char senderCharList[5120]; // 2560 = 10 or more sender names at 256 each
static bool bNeedsUpdating = false; // Update the combo box

// Convenience definitions
#define PARAM_SenderName  0
#define PARAM_Aspect      1
#define PARAM_Lowres      2

// Number of parameters
#define NumParams 3

#ifndef GL_READ_FRAMEBUFFER_EXT
#define GL_READ_FRAMEBUFFER_EXT 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER_EXT
#define GL_DRAW_FRAMEBUFFER_EXT 0x8CA9
#endif

class MagicNDIreceiverModule : public MagicModule
{
	// see below (after class definition) for static value assignments
	static const MagicModuleSettings settings;
	static const MagicModuleParam params[NumParams];

public:

	MagicNDIreceiverModule() {
		
		
		/*
		// For debugging
		// Console window so printf works
		FILE* pCout; // should really be freed on exit
		AllocConsole();
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		printf("MagicNDIreceiverModule\n");
		*/

		spout_buffer = nullptr;
		senderName = "";
		startName = "";
		senderIndex = -1;
		senderWidth = 0;
		senderHeight = 0;

		bInitialized = false; // not initialized yet
		bStarted = false; // not started yet
		bNewContext = false; // no OpenGL context yet
		bAspect = false; // do not preserve aspect ratio of received texture in draw
		bLowres = false; // do not use low bandwidth receiving mode

		receiver.SetAudio(false); // Set to receive no audio

	}

	~MagicNDIreceiverModule() {

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

		bNewContext = true; // New OpenGL context

		// Make sure there is a valid texture to draw in case of scene change.
		if (senderWidth > 0 && senderHeight > 0)
			InitTexture(senderWidth, senderHeight);

	};
	
	virtual void glClose() {

		// Release all objects
		if (myTexture != 0) glDeleteTextures(1, &myTexture);
		if (spout_buffer) free((void *)spout_buffer);
		if (m_pbo[0]) glDeleteBuffers(2, m_pbo);
		myTexture = 0;
		spout_buffer = nullptr;
		m_pbo[0] = 0;
		m_pbo[1] = 0;

	};

	void drawBefore(MagicUserData *userData) {

		unsigned int width = 0;
		unsigned int height = 0;
		int nSenders = 0;

		if (receiver.FindSenders(nSenders) || bNewContext) {

			// FindSenders returns true for a network change
			// Even if the last sender closed and nSenders = 0

			// Update the sender name list
			std::string list = "";
			if (nSenders > 0) {
				for (int i = 0; i < nSenders; i++) {

					// Get the full NDI sender name
					std::string name = receiver.GetSenderName(i);

					// Can optionally use full NDI name for the list
					// but the prefix is always the same and exceeds
					// the combo box name width
					// list += name;

					// Work out the display names for the combo box
					std::string newname = name.substr(name.find_first_of("(") + 1, name.find_last_of(")"));
					newname.resize(newname.length() - 1); // drop the last ')'
					list += newname;
					if (i < nSenders - 1)
						list += "\n";

					// This section is to allow waiting for
					// the sender saved in the comobox to start
					// if the user does not select anything else
					// At this stage there is no senderName established
					if (!bInitialized && !startName.empty() && !bStarted) {
						if (name.find(startName.c_str(), 0, startName.size()) != std::string::npos) {
							// Set the sender name and allow receiver creation with this index
							senderName = name;
							senderIndex = i;
							bStarted = true; // don't do this section again
						}
					}
				}
			} // endif nSenders > 0

			// Tell Magic to update the combo box if the list is different
			// or if a new OpenGL context in case there has been a scene change.
			if (list != senderList || bNewContext) {
				senderList = list; // update comparison string
				// Update char array for Module param[0].extrainfo
				// Assume max 256 characters each for 10 senders
				memset((void *)senderCharList, 0, 2560);
				if (!list.empty()) strcpy_s(senderCharList, 2560, list.c_str());
				bNeedsUpdating = true;
			}

		} // endif network change

		if (nSenders > 0) {
			// Wait for a sender before doing anything
			if (bInitialized) {
				// Receive from the NDI sender
				// Frame rate might be much less than the draw cycle
				if (receiver.ReceiveImage(spout_buffer, width, height)) {
					// Have the NDI sender dimensions changed ?
					// (initially senderWidth and senderHeight are 0 so the buffer gets created here)
					if (senderWidth != width || senderHeight != height) {
						senderWidth = width;
						senderHeight = height;
						// Update the local texture and receiving buffer
						InitTexture(senderWidth, senderHeight);
						return; // no more for this cycle
					}
					// Now the receiving texture is the right size
					// Update with the pixel buffer
					glBindTexture(GL_TEXTURE_2D, myTexture);
					// TODO : Check RGBA/BGRA for NDI 4.5
					// glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, senderWidth, senderHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)spout_buffer);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, senderWidth, senderHeight, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)spout_buffer);
					glBindTexture(GL_TEXTURE_2D, 0);
				}

				// Can check for Metadata or Audio here on receive fail
				// Otherwise if receiveimage fails, the connection could
				// be down so keep waiting for it to come back
				if (senderWidth > 0 && senderHeight > 0 && myTexture > 0) {
					DrawReceivedTexture(myTexture, GL_TEXTURE_2D,
						senderWidth, senderHeight,
						userData->glState->viewportWidth,
						userData->glState->viewportHeight);
				}

			}
			else {
				// Create a new receiver for this index if we have a name for it
				if (!senderName.empty()) {
					if (receiver.CreateReceiver(senderIndex)) {
						// senderName is already set by start or combo box selection
						bInitialized = true;
					}
				}
			}
		}

		bNewContext = false; // Context has been used

	};

	void drawAfter(MagicUserData *userData) {

	}

	bool fixedParamValueChanged(const int whichParam, const char* newValue) {
		
		if (!newValue || !newValue[0])
			return false;

		int iValue = atoi(newValue);
		int nSenders = 0;
		std::string name = "";

		switch (whichParam) {

		// Sender name combo box
		case PARAM_SenderName:

			// Is there anything in the combo box at startup?
			// There might not be any senders running yet.
			if (newValue && newValue[0] && !bStarted) {
				startName = newValue; // set the starting name
			}

			// Find the index of the selected name in the NDI names list
			nSenders = receiver.GetSenderCount();
			if (nSenders > 0) {
				for (int i = 0; i < nSenders; i++) {
					name = receiver.GetSenderName(i);
					if (name.find(newValue, 0, strlen(newValue)) != std::string::npos) {
						// For a different name, reset to the selected index and start again
						if (name != senderName) {
							senderIndex = i;
							// Reset the sender name (full NDI name)
							senderName = name;
							receiver.ReleaseReceiver();
							bInitialized = false;
						}
					}
				}
			}
			break;

			// Aspect preserve
		case PARAM_Aspect:
			bAspect = (iValue == 1);
			break;

			// Low bandwidth
		case PARAM_Lowres:
			if (iValue != (int)bLowres) {
				bLowres = (iValue == 1);
				receiver.SetLowBandwidth(bLowres);
				receiver.ReleaseReceiver();
				bInitialized = false;
			}
			break;
		}

		return true;
	
	}

	bool paramNeedsUpdating(const int whichParam) {

		if (whichParam == 0 && bNeedsUpdating) {
			// params[0].extraInfo is reset with the new list array
			bNeedsUpdating = false;
			return true;
		}

		return false;

	}


	const char *getHelpText() {
		return "Magic NDI Receiver - Vers 1.013\n"
			"Lynn Jarvis 2018-2021 - https://spout.zeal.co/ \n\n"
			"Receives textures from NDI Senders\n"
			"Newtek - https://www.ndi.tv/ \n\n"
			"Sender name : select sender\n"
			"Aspect : preserve sender aspect ratio\n"
			"Low bandwidth : low resolution receiving mode";
	}

protected:

	// Initialize in constructor
	ofxNDIreceive receiver; // NDI receiver object
	unsigned char *spout_buffer; // Buffer used for image transfer
	std::string senderName; // full NDI sender name used by a receiver
	std::string startName;// used to wait for a selected sender to start
	int senderIndex; // index into the list of NDI senders
	unsigned int senderWidth;
	unsigned int senderHeight;

	GLuint m_pbo[2];
	int PboIndex;
	int NextPboIndex;
	GLuint myTexture;

	bool bInitialized; // NDI is initialized
	bool bStarted; // module has started
	bool bNewContext; // glInit has been called
	bool bAspect; // preserve aspect ratio of received texture in draw
	bool bLowres; // low bandwidth receiving mode

	// Initialize a local texture
	void InitTexture(unsigned int width, unsigned int height)
	{
		// Set up for pbo pixel data transfer
		if (m_pbo[0])
			glDeleteBuffers(2, m_pbo);
		glGenBuffers(2, m_pbo);

		// Update the receiving buffer
		if (spout_buffer)
			free((void *)spout_buffer);
		spout_buffer = (unsigned char *)malloc(senderWidth*senderHeight * 4 * sizeof(unsigned char));

		// To prevent artefacts when the texture is first drawn, clear with zero chars.
		// Noticeable with low frame rate sources such as NDI "Test Pattern"
		// Can use the receiving buffer here for this.
		memset(spout_buffer, 0, width * height * 4 * sizeof(unsigned char));

		// Release any existing texture
		if (myTexture != 0)
			glDeleteTextures(1, &myTexture);

		glGenTextures(1, &myTexture);
		glBindTexture(GL_TEXTURE_2D, myTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, spout_buffer);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	void DrawReceivedTexture(GLuint TextureID, GLuint TextureTarget,
		unsigned int width, unsigned int height, int viewportWidth, int viewportHeight)
	{

		// Calculate aspect ratios to scale to the aspect ratio required
		float vp_aspect = (float)viewportWidth / (float)viewportHeight;
		float image_aspect = (float)width / (float)height;

		// Always fit draw width to the viewport
		float vx = (float)viewportWidth / (float)viewportHeight;
		float vy = 1.0;

		// Preserve image aspect ratio for draw
		if (bAspect) {
			if (image_aspect > vp_aspect) {
				// Calculate the offset in Y
				vy = (float)height / (float)width;
				// Adjust to the viewport aspect ratio
				vy = vy * vp_aspect;
			}
			else {
				// Otherwise adjust the horizontal
				vx = (float)width / (float)height;
			}
		}

		// Invert the texture coords from DirectX to OpenGL
		GLfloat tc[] = {
			0.0, 1.0,
			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0 };

		GLfloat verts[] = {
			-vx,  -vy,   // bottom left
			-vx,   vy,   // top left
			 vx,   vy,   // top right
			 vx,  -vy }; // bottom right

		glPushMatrix();

		glColor4f(1.f, 1.f, 1.f, 1.f);

		glEnable(TextureTarget);
		glBindTexture(TextureTarget, TextureID);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, tc);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, verts);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glBindTexture(TextureTarget, 0);
		glDisable(TextureTarget);
		glPopMatrix();
	}
};


MagicModule *CreateInstance() {
	return new MagicNDIreceiverModule();
}

const bool HasInputPin() {
	return false;
} // No inputs for a receiver

const MagicModuleSettings MagicNDIreceiverModule::settings = MagicModuleSettings(NumParams);

const MagicModuleParam MagicNDIreceiverModule::params[NumParams] = {
	MagicModuleParam("Sender", NULL, NULL, NULL, MVT_STRING, MWT_COMBOBOX, true, NULL, senderCharList),
	MagicModuleParam("Aspect", "0", "0", "1", MVT_BOOL, MWT_TOGGLEBUTTON, true),
	MagicModuleParam("Low bandwidth", "0", "0", "1", MVT_BOOL, MWT_TOGGLEBUTTON, true)
};
