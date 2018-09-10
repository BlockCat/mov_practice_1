===============================================================================
How to make it work under linux (similar steps for OSX):
===============================================================================

-- Unofficial stuff starts here.

0a) Download the dll folder from: http://www.cs.uu.nl/docs/vakken/mov/
0b) Preferably download the dll from: http://www.cs.uu.nl/docs/vakken/mov/2017/files/LinuxMacWin.zip


-- From here it will be official again
1) Install necessary libraries like cmake (try to get version 3.4 or higher), glew, glm, sdl2, freeimage, etc:

sudo apt-get install cmake libglew-dev libglm-dev libsdl2-dev libfreeimage-dev
(I think the OSX variant is: brew install <insert packages listed above> )


2) In order to compile & run, do either ./cleanrun.sh, or ./run.sh

3) If you encounter any errors about missing libraries etc, install those (usually you need the -dev packages) or google the errors ;)

4) If it runs, you should see a window with crisp, sharp, clear letters saying "Hello world" (worst case you see blocky/minecraft-like letters), and a red line underneath it.



===============================================================================
When its running... (The following holds for Windows, Linux, and OSX)
===============================================================================
Add your own code by adding files to the src folder, and adding your normal header files to the bottom of src/template.h (e.g. #include "something.h" )
Make sure your header and normal c++ code is in the same namespace, Tmpl8....:
For example:

File something.h :

	#pragma once
	
	namespace Tmpl8 {
	
	// Your header stuff here
	
	};

And then file something.cpp :

	#include "template.h"	// No other headers... they should all be in template.h 
	
	namespace Tmpl8 {
	
	// Your code here
	
	};

	

Good luck!

