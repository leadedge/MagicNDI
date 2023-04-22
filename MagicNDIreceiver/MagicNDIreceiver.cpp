//
// Magic Module Development Kit (MDK) v2.13
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
//		MagicNDIreceiver
//
//		Module plugin for Magic https://magicmusicvisuals.com/
//		Using the Magic MDK
//
//		Using the NDI SDK to send frames over a network https://www.ndi.tv/
//		And send class files from ofxNDI Openframeworks addon https://github.com/leadedge/ofxNDI
//		Copyright(C) 2018-2023 Lynn Jarvis https://spout.zeal.co/
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
// 02.02.21		- Change back to RGBA due to NDI conversion problem
//				  Update GitHub release
//				  Version 1.014
// 21.08.21		- Update help text with NDI version number
//				  Release NDI receiver in destructor
//				  Update ofxNDI with NDI Version 5
//				  Rebuild x64 VS2017 / MT
//				  Version 1.015
// 26.09.21		- Update ofxNDI with NDI SDK Version 5.0.3
//				  Rebuild x64 VS2017 / MT
//				  Version 1.016
// 15.12.21		- Equalise version numbers for sender and receiver.
//				  Update to the latest ofxNDI source with NDI SDK Version 5.0.10.1
//				  Fix NDI version number length.
//				  For user selection, set the selected sender name or the old one will be used.
//				  This can happen if there is a name in the dialog at startup and it's not running.
//				  Version 1.017
// 11.12.22		- Suppress warnings
// 12.12.22		- Add ReleaseNDIreceiver
//				  Expand help and tooltips
//				  Remove unused definitions and pbos
//				  Remove unused Glee library
//				  Allow for sender name change for the same index
//				  to allow for Spout to NDI selecting another sender
//				  Rebuild with revised ofxNDI
// 26.12.22		- Rebuild with revised ofxNDI
//				  Version 1.018
// 22.04.23		- Rebuild with current SpoutGL beta and revised ofxNDI
//			      NDI Vers 5.5.4 - Version to match SpoutToNDI
//				  VS2022 x64 only /MT Version 1.020
//
// =======================================================================================

// Suppress warnings because the OpenGLfunctions can't be changed
#pragma warning(disable : 26482) // Only index into arrays using constant expressions
#pragma warning(disable : 26485) // Do not pass an array as a single pointer

// The following require a re-write
// All functions are used without reported problems
#pragma warning(disable : 26493) // c-style casts.
#pragma warning(disable : 26472) // static cast no casts for arithmetic conversion (gsl::narrow etc not available)
#pragma warning(disable : 26481) // Don't use pointer arithmetic (used without exceeding bounds)
#pragma warning(disable : 26496) // variable assigned only once mark it as const
#pragma warning(disable : 26446) // Prefer to use gsl::at() - not available
#pragma warning(disable : 26461) // Pointer argument can be marked as a pointer to const
#pragma warning(disable : 26408) // Avoid malloc and free. Prefer new and delete.

// Suppress the following for the MDK
#pragma warning(disable : 26433) // override
#pragma warning(disable : 26440) // noexcept
#pragma warning(disable : 26455) // constructor noexcept
#pragma warning(disable : 26447) // functions inside noexcept
#pragma warning(disable : 26432) // define or delete
#pragma warning(disable : 26409) // avoid calling new and delete explicitly
#pragma warning(disable : 26495) // always initialize a member variable

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <gl/gl.h>
#pragma comment(lib, "OpenGL32.Lib")

#include "MagicModule.h"
#include "ofxNDIreceive.h"

// Name list for the combo box
static std::string senderList; // Name list to compare for changes
static int nSenders = 0;
static char senderCharList[5120]{}; // 2560 = 10 or more sender names at 256 each
static bool bNeedsUpdating = false; // Update the combo box

// Convenience definitions
#define PARAM_SenderName  0
#define PARAM_Aspect      1
#define PARAM_Lowres      2

// Number of parameters
#define NumParams 3

// For OpenGL
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif


class MagicNDIreceiverModule : public MagicModule
{
	// see below (after class definition) for static value assignments
	static const MagicModuleSettings settings;
	static const MagicModuleParam params[NumParams];

public:

	MagicNDIreceiverModule() {
		
		// For debugging
		// Console window so printf works
		/*
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
		hlp.reserve(1024); // reserve instead of allocate on the stack

		receiver.SetAudio(false); // Set to receive no audio

	}

	~MagicNDIreceiverModule() {
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

		bNewContext = true; // New OpenGL context

		// Make sure there is a valid texture to draw in case of scene change.
		if (senderWidth > 0 && senderHeight > 0)
			InitTexture(senderWidth, senderHeight);

	};
	
	virtual void glClose() {
		// Close the NDI receiver
		// and release receiving buffer and texture
		ReleaseNDIreceiver();
	};

	void drawBefore(MagicUserData *userData) {

		unsigned int width = 0;
		unsigned int height = 0;
		int nsenders = 0;

		if(receiver.FindSenders(nsenders)) {

			// FindSenders returns true for a network change
			// Even if the last sender closed and nSenders = 0

			// FindSenders has only a 1msec timeout, so wait 2 frames 
			// and check again to allow time for senders to be closed
			// and the network refreshed.
			Sleep(33);
			receiver.FindSenders(nsenders);

			// Construct a new sender list for user selection
			nSenders = nsenders;

			// Update the sender name list
			std::string list;

			if (nSenders > 0) {
				for (int i = 0; i < nSenders; i++) {
					
					// Get the full NDI sender name
					std::string name = receiver.GetSenderName(i);

					// If initialized and the the index is current,
					// release the receiver if it's a different name.
					if (bInitialized && i == senderIndex && name != senderName) {
						ReleaseNDIreceiver();
					}
					
					// The full NDI name exceeds the combo box
					// name width and the prefix is always the same.
					// Create shortened display names for the combo box
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
						// Find the shortened dialog name within the whole name
						if (name.find(startName.c_str(), 0, startName.size()) != std::string::npos) {
							// Set the sender name and allow receiver creation with this index
							senderName = name;
							senderIndex = i;
							bStarted = true; // don't do this section again
						}
					}

				} // endif nSenders > 0

				// Tell Magic paramNeedsUpdating to update the combo box 
				// if the list is different or if a new OpenGL context
				// in case there has been a scene change.
				if (!list.empty() && list != senderList || bNewContext) {
					senderList = list; // update comparison string
					// Update char array for Module param[0].extrainfo
					// Assume max 256 characters each for 10 senders
					// Clear first in case the list is smaller
					memset((void*)senderCharList, 0, 2560);
					strcpy_s(senderCharList, 2560, list.c_str());
					bNeedsUpdating = true;
				} // endif new list
			} // endif nSenders > 0
		} // endif network change

		if (nSenders > 0) {
			// Wait for a sender before receiving
			if (bInitialized) {
				// Receive from the NDI sender
				// Frame rate might be much less than the draw cycle
				// ReceiveImage succeeds if it finds a sender
				// and ignores spout_buffer if null.
				if (receiver.ReceiveImage(spout_buffer, width, height, false)) {
					// Have the NDI sender dimensions changed ?
					// (initially senderWidth and senderHeight are 0 so the buffer gets created here)
					if (senderWidth != width || senderHeight != height) {
						senderWidth = width;
						senderHeight = height;
						// Update the local texture and receiving buffer
						InitTexture(senderWidth, senderHeight);
						return; // no more for this cycle
					}
					// Now that the receiving texture is the right size
					// update with the pixel buffer
					glBindTexture(GL_TEXTURE_2D, myTexture);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, senderWidth, senderHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)spout_buffer);
					glBindTexture(GL_TEXTURE_2D, 0);
				}

				// Could check for Metadata or Audio here if receiveimage fails.
				// Otherwise the connection could be down so draw the current texture 
				// and keep waiting for it to come back.
				if (senderWidth > 0 && senderHeight > 0 && myTexture > 0) {
					DrawReceivedTexture(myTexture, GL_TEXTURE_2D,
						senderWidth, senderHeight,
						userData->glState->viewportWidth,
						userData->glState->viewportHeight);
				}

			}
			else {
				// Update the name in case the one in this position has changed.
				senderName = receiver.GetSenderName(senderIndex);
				// Set the sender name or the old one will be used
				receiver.SetSenderName(senderName.c_str());
				bInitialized = receiver.CreateReceiver(senderIndex);
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
							// Release receiver and resources
							ReleaseNDIreceiver();
							// Reset the sender index
							senderIndex = i;
							// Reset the sender name (full NDI name)
							senderName = name;
							// Set the selected sender name or the old one will be used
							receiver.SetSenderName(senderName.c_str());
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
				// Release the receiver and resources
				ReleaseNDIreceiver();
				receiver.SetLowBandwidth(bLowres);
			}
			break;

		default:
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

		hlp =
			"  Magic NDI Receiver - Vers 1.020\n"
			"  Receives textures from NDI Senders\n\n"
			"      Sender name : select sender.\n\n"
			"      Aspect : preserve sender aspect ratio. Instead of \n"
			"      drawing to the size of the window. It has no effect if\n"
			"      Window > Magic window options > Graphics resolution\n"
			"      is set to the same aspect ratio\n\n"
			"      Low bandwidth : low resolution receiving mode.\n"
			"      A medium quality stream that takes almost no bandwidth\n"
			"      normally about 640 pixels on the longest side.\n\n"
			"  Lynn Jarvis 2018-2023 - https://spout.zeal.co \n"
			"  For Magic MDK Version 2.3\n"
			"  2012-2020 Color & Music, LLC.\n"
			"  Newtek - https://www.ndi.tv/ \n"
			"  Library version : ";

		// Get NewTek library (dll) version number
		// Version number is the last 8 chars - e.g 5.0.10.1
		std::string NDIversion = receiver.GetNDIversion();
		std::string NDInumber = receiver.GetNDIversion().substr(NDIversion.length() - 8, 8);
		hlp += NDInumber;

		return hlp.c_str();
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
	GLuint myTexture;

	bool bInitialized; // NDI is initialized
	bool bStarted; // module has started
	bool bNewContext; // glInit has been called
	bool bAspect; // preserve aspect ratio of received texture in draw
	bool bLowres; // low bandwidth receiving mode
	std::string hlp; // Help text

	// Initialize a local texture
	void InitTexture(unsigned int width, unsigned int height)
	{
		// Release any existing texture
		if (myTexture != 0)
			glDeleteTextures(1, &myTexture);

		// Generate a new one
		glGenTextures(1, &myTexture);
		glBindTexture(GL_TEXTURE_2D, myTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, spout_buffer);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Update the receiving buffer
		if (spout_buffer)
			free((void*)spout_buffer);
		spout_buffer = (unsigned char*)malloc(width*height * 4 * sizeof(unsigned char));

		// To prevent artefacts when the texture is first drawn, clear with zero chars.
		// Noticeable with low frame rate sources such as NDI "Test Pattern"
		// Can use the receiving buffer here for this.
		if (spout_buffer)
			memset(spout_buffer, 0, width * height * 4 * sizeof(unsigned char));

	}

	// Release receiver and resources
	void ReleaseNDIreceiver()
	{
		if (!bInitialized)
			return;

		// Release NDI receiver
		receiver.ReleaseReceiver();
		// Free the receiving buffer and texture 
		// because the receiving resolution might change
		if (spout_buffer) free((void*)spout_buffer);
		spout_buffer = nullptr;
		if (myTexture != 0) glDeleteTextures(1, &myTexture);
		myTexture = 0;
		senderWidth = 0;
		senderHeight = 0;
		bInitialized = false;

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

	MagicModuleParam("Sender", NULL, NULL, NULL, MVT_STRING, MWT_COMBOBOX, true, "The NDI sender name", senderCharList),

	MagicModuleParam("Aspect", "0", "0", "1", MVT_BOOL, MWT_TOGGLEBUTTON, true, "Preserve sender aspect ratio instead of drawing to the size of the window. It has no effect if the Magic window has the same aspect ratio"),
	MagicModuleParam("Low bandwidth", "0", "0", "1", MVT_BOOL, MWT_TOGGLEBUTTON, true, 
	"Activate low bandwidth receiving mode. This is a medium quality stream that takes almost no bandwidth. Normally about 640 pixels on the longest side")
};
