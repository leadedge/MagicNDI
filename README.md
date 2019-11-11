# MagicNDI
MagicNDIsender and MagicNDIreceiver are module plugins for Magic (https://magicmusicvisuals.com/) to send and receive images over a network using the NewTek Network Device Protocol.

## Updates
12.03.19 - Create repository\
18.03.19 - MagicNDIsender\
o Introduced clock video toggle for 60fps / user fps\
o Version 1.007\
27.03.19 - MagicNDIsender\
o Added tooltips\
o Update with current ofxNDI source\
o Version 1.008\
11.11.19\
o Update with ofxNDI for NDI 4.0\
o Version 1.009

## Setup

1. MagicNDIsender and MagicNDIreceiver project files are for Visual Studio 2017 for Windows.
Before opening the solution files, the following files must be added to both projects.
2. The modules use the Magic Music Visuals Module Development Kit (MDK)
    - [Visit the MDK download page](https://magicmusicvisuals.com/developers) and download "Magic MDK v2.2" and save to a convenient folder.
    - Extract the single file "MagicModule.h"
    - Copy MagicModule.h to both project folders containing either MagicNDIsender.cpp or MagicNDIreceiver.cpp.
3. The projects use files from the Openframeworks addon "ofxNDI" which is separately maintained.
    - [Visit the ofxNDI repository](https://github.com/leadedge/ofxNDI)
    - Click "Clone or Download", "Download ZIP" and save to a convenient folder.
    - Unzip the contents and copy the following files to the project "ofxNDI" folder\
        ofxNDI-master\src\ofxNDIreceive.cpp\
        ofxNDI-master\src\ofxNDIreceive.h\
        ofxNDI-master\src\ofxNDIsend.cpp\
        ofxNDI-master\src\ofxNDIsend.h\
        ofxNDI-master\src\ofxNDIutils.cpp\
        ofxNDI-master\src\ofxNDIutils.h
    - Other files in the addon are not required.
4. [Visit Newtek](https://www.ndi.tv/) and download the NDI SDK. Install it and copy files as follows.
    - Copy all the files in "../NDI 4 SDK/Include" to "ofxNDI/include"
    - Copy the "x86" and "x64" folders in "../NDI 4 SDK/Lib" to "ofxNDI/libs/NDI/Lib"
    - Copy the "x86" and "x64" folders in "../NDI 4 SDK/Bin" to "ofxNDI/lib"
5. Copy "ofxNDI/libs/x64/Processing.NDI.Lib.x64.dll" to the folder containing Magic.exe 64bit.
6. Copy "ofxNDI/libs/x86/Processing.NDI.Lib.x86.dll" to the folder containing Magic.exe 32bit.
7. Open the project solution files with Visual Studio 2017, change to release and build.

## Binaries

If you want pre-built modules to use straight way, select the "Release" tab and download the release zip file.

## Copyrights

MagicNDIsender and MagicNDIreceiver - Copyright(C) 2018-2019 Lynn Jarvis [http://spout.zeal.co/]

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser  General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details. 
You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses).

----------------------
Magic Module Development Kit (MDK) v2.11

Copyright (c) 2012-2017 Color & Music, LLC.  All rights reserved.
The MDK is provided "as is" without any express or implied warranty
of any kind, oral or written, including warranties of merchantability,
fitness for any particular purpose, non-infringement, information
accuracy, integration, interoperability, or quiet enjoyment.  In no
event shall Color & Music, LLC or its suppliers be liable for any
damages whatsoever (including, without limitation, damages for loss
of profits, business interruption, loss of information, or physical
damage to hardware or storage media) arising out of the use of, misuse
of, or inability to use the MDK, your reliance on any content in the
MDK, or from the modification, alteration or complete discontinuance
of the MDK, even if Color & Music, LLC has been advised of the
possibility of such damages.
 
----------------------
GLee (OpenGL Easy Extension library)        
Version : 5.4
Copyright (c)2009  Ben Woodhouse  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer as
the first lines of this file unmodified.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY BEN WOODHOUSE AS IS AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL BEN WOODHOUSE BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Web: http://elf-stone.com/glee.php

----------------------
NDI SDK - Copyright NewTek Inc. [http://NDI.NewTek.com](http://NDI.NewTek.com).

A license agreement is included with the Newtek SDK when you receive it after registration with NewTek.
The SDK is used by you in accordance with the license you accepted by clicking “I accept” during installation. This license is available for review from the root of the SDK folder.
Read the conditions carefully. You can include the NDI dlls as part of your own application, but the Newtek SDK and specfic SDK's which may be contained within it may not be re-distributed.
Your own EULA must cover the specific requirements of the NDI SDK EULA.

----------------------
