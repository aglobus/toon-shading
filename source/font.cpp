/*
 *  font.C
 *
 *  Draws some text in a bitmapped font.  Uses glBitmap() 
 *  and other pixel routines.
 *
 *  This fixed-width font has WIDTH 8 and HEIGHT 13 pixels.
 */


#include "headers.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;


void printString( char *s, float x, float y, float width, float height )

{
  // Set up pixel coord system [0,width-1]x[0,height-1]

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0, width, 0, height, 0.5, -0.5 );

  // Draw the characters

  glRasterPos2f( x, y );
  int len = (int) strlen(s);
  for (int i=0; i<len; i++)
    glutBitmapCharacter( GLUT_BITMAP_8_BY_13, (GLubyte) s[i] );

  // Restore matrices

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}
