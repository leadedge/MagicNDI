## MagicNDI

MagicNDIsender and MagicNDIreceiver are module plugins for [Magic Music Visuals](https://magicmusicvisuals.com/) to send and receive images over a network using the NewTek Network Device Protocol.

## Setup

### Plugins

Pre-built plugins can be downloaded from the [Releases](https://github.com/leadedge/MagicNDI/releases) page and instructions for installation are detailed there.

### Build

1. MagicNDIsender and MagicNDIreceiver project files are for Visual studio 2022 for Windows.
   The individual projects contain all the required files. 
2. The projects use the Magic Music Visuals Module Development Kit (MDK) 
   If this requires updating : 
    - [Visit the MDK download page](https://magicmusicvisuals.com/developers) and download "Magic MDK v2.2" and save to a convenient folder.
    - Extract the single file "MagicModule.h"
    - Copy MagicModule.h to the repository root containing either MagicNDIsender.cpp or MagicNDIreceiver.cpp.
3. Each project contains files from the Spout2 library for OpenGL extensions. 
   If these files require updating : 
   - [Visit the Spout2 repository](https://github.com/leadedge/Spout2/tree/master/SPOUTSDK/SpoutGL)
   - Copy the following files to the project "SpoutGL" folder
       SpoutGLextensions.cpp
	   SpoutGLextensions.h
4. Each project contains files from the Openframeworks addon "ofxNDI". 
   If this requires updating : 
    - [Visit the ofxNDI repository](https://github.com/leadedge/ofxNDI)
    - Click "Clone or Download", "Download ZIP" and save to a convenient folder.
    - Unzip the contents and copy the complete "ofxNDI" folder to the project "ofxNDI" folder
5. Open the project solution files with Visual Studio 2022, change to Release x64 and build. 

## Copyrights

MagicNDIsender and MagicNDIreceiver - Copyright(C) 2018-2025 Lynn Jarvis [http://spout.zeal.co/]

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser  General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details. 
You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses).

----------------------

 * Magic Module Development Kit (MDK) v2.3
 * Copyright (c) 2012-2020 Color & Music, LLC.  All rights reserved.
 *
 * The MDK is provided "as is" without any express or implied warranty
 * of any kind, oral or written, including warranties of merchantability,
 * fitness for any particular purpose, non-infringement, information
 * accuracy, integration, interoperability, or quiet enjoyment.  In no
 * event shall Color & Music, LLC or its suppliers be liable for any
 * damages whatsoever (including, without limitation, damages for loss
 * of profits, business interruption, loss of information, or physical
 * damage to hardware or storage media) arising out of the use of, misuse
 * of, or inability to use the MDK, your reliance on any content in the
 * MDK, or from the modification, alteration or complete discontinuance
 * of the MDK, even if Color & Music, LLC has been advised of the
 * possibility of such damages.
 
----------------------

NDI SDK - Copyright NewTek Inc. [https://ndi.video/](https://ndi.video/).

A license agreement is included with the Newtek SDK when you receive it after registration with NewTek.
The SDK is used by you in accordance with the license you accepted by clicking “I accept” during installation. This license is available for review from the root of the SDK folder.
Read the conditions carefully. You can include the NDI dlls as part of your own application, but the Newtek SDK and specfic SDK's which may be contained within it may not be re-distributed.
Your own EULA must cover the specific requirements of the NDI SDK EULA.

----------------------

