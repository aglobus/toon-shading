/*
    Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "headers.h"
#include "gbuffer.h"


GBuffer::GBuffer( unsigned int width, unsigned int height, int nTextures )

{
  windowWidth = width;
  windowHeight = height;
  numTextures = nTextures;

  // Create the FBO

  glGenFramebuffers( 1, &FBO );
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, FBO );

  // Create the gbuffer textures

  textures = new GLuint[ numTextures ];
  glGenTextures( numTextures, textures );
  glGenTextures( 1, &depthTexture );
    
  for (int i = 0 ; i < numTextures; i++) {

    glBindTexture( GL_TEXTURE_2D, textures[i] );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0 );
  }

  // depth

  glBindTexture( GL_TEXTURE_2D, depthTexture );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );

  glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0 );

  // Declare the drawBuffers

  GLenum *drawBuffers = new GLenum[numTextures];

  for (int i=0; i<numTextures; i++)
    drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

  glDrawBuffers( numTextures, drawBuffers );

  GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    printf("FB error, status: 0x%x\n", status);
    return;
  }

  // restore default FBO

  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
}


GBuffer::~GBuffer()

{
  glDeleteFramebuffers( 1, &FBO );
  glDeleteTextures( numTextures, textures );
  glDeleteTextures( 1, &depthTexture );
}


void GBuffer::BindForWriting()

{
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, FBO );
}


void GBuffer::BindForReading()

{
  glBindFramebuffer( GL_READ_FRAMEBUFFER, FBO );
}


void GBuffer::BindTexture( int textureNumber )

{
  glActiveTexture( GL_TEXTURE0 + textureNumber );
  glBindTexture( GL_TEXTURE_2D, textures[ textureNumber ] );
}


void GBuffer::SetReadBuffer( int textureNumber )

{
  glReadBuffer( GL_COLOR_ATTACHMENT0 + textureNumber );
}


void GBuffer::setDrawBuffers( int numDrawBuffers, int *bufferIDs )

{
  GLenum *drawBuffers = new GLenum[numDrawBuffers];

  for (int i=0; i<numDrawBuffers; i++)
    drawBuffers[i] = GL_COLOR_ATTACHMENT0 + bufferIDs[i];

  glDrawBuffers( numDrawBuffers, drawBuffers );

  delete [] drawBuffers;
}


// Debugging output
//
// LL = position
// UL = colour
// UR = normal
// LR = depth


void GBuffer::DrawGBuffers()

{
  // Clear window

  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Blit textures onto window

  glBindFramebuffer( GL_READ_FRAMEBUFFER, FBO );

  GLsizei halfWidth = (GLsizei)(windowWidth / 2.0f);
  GLsizei halfHeight = (GLsizei)(windowHeight / 2.0f);

  SetReadBuffer( 0 );
  glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0,                 halfWidth, halfHeight,      GL_COLOR_BUFFER_BIT, GL_LINEAR);

  SetReadBuffer( 1 );
  glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, halfHeight,         halfWidth, windowHeight,   GL_COLOR_BUFFER_BIT, GL_LINEAR);

  SetReadBuffer( 2 );
  glBlitFramebuffer(0, 0, windowWidth, windowHeight, halfWidth, halfHeight, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  SetReadBuffer( 3 );
  glBlitFramebuffer(0, 0, windowWidth, windowHeight, halfWidth, 0,          windowWidth, halfHeight,   GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
