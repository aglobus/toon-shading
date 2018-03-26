// The standard headers included by all files


#ifndef HEADERS_H
#define HEADERS_H

#ifdef _WIN32
  #include <windows.h>
  #include <typeinfo>
  #include <sys/types.h>
  #include <sys/timeb.h>
  #include <io.h>
  #define M_PI 3.14159
  #define MAXFLOAT 9.9e10
  #define rint(x) floor((x)+0.5)
#endif

#ifdef LINUX
  #include <sys/timeb.h>	// includes ftime (to return current time)
  #include <unistd.h>		// includes usleep (to sleep for some time)
  #include <values.h>           // includes MAX_FLOAT
  #define sprintf_s sprintf
  #define _strdup strdup
  #define sscanf_s sscanf
#endif

#ifdef __APPLE_CC__
  #include <glew.h>
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
#else
  #include <GL/glew.h>
  #include <GL/freeglut.h>
  #include <GL/glu.h>
  #include <GL/gl.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

#include <math.h>


#define randIn01() (rand() / (float) RAND_MAX)   // random number in [0,1]

#endif
