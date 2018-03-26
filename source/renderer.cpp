// G-buffer renderer


#include "headers.h"
#include "renderer.h"
#include "shader.h"


// Draw a quad over the full screen.  This generates a fragment for
// each pixel on the screen, allowing the fragment shader to run on
// each fragment.


void drawFullscreenQuad()

{
  vec2 verts[4] = { vec2( -1, -1 ), vec2( -1, 1 ), vec2( 1, -1 ), vec2( 1, 1 ) };
    
  GLuint VAO, VBO;

  glGenVertexArrays( 1, &VAO );
  glBindVertexArray( VAO );

  glGenBuffers( 1, &VBO );
  glBindBuffer( GL_ARRAY_BUFFER, VBO );

  glBufferData( GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW );

  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );

  glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

  glDeleteBuffers( 1, &VBO );
  glDeleteVertexArrays( 1, &VAO );
}


// Render the scene in three passes.


void Renderer::render( wfModel *obj, mat4 &M, mat4 &MV, mat4 &MVP, vec3 &lightDir )

{
  // Pass 1: Store colour, normal, depth in G-Buffers

  gbuffer->BindForWriting();

  pass1Prog->activate();

  pass1Prog->setMat4(  "M",        M );
  pass1Prog->setMat4(  "MV",       MV );
  pass1Prog->setMat4(  "MVP",      MVP );

  gbuffer->BindTexture( COLOUR_GBUFFER );
  gbuffer->BindTexture( NORMAL_GBUFFER );
  gbuffer->BindTexture( DEPTH_GBUFFER  );

  int activeDrawBuffers1[] = { COLOUR_GBUFFER, NORMAL_GBUFFER, DEPTH_GBUFFER };
  gbuffer->setDrawBuffers( 3, activeDrawBuffers1 );

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glEnable( GL_DEPTH_TEST );

  obj->draw( pass1Prog );

  pass1Prog->deactivate();

  if (debug == 1) {
    gbuffer->DrawGBuffers();
    return;
  }

  // Pass 2: Store Laplacian (computed from depths) in G-Buffer

  pass2Prog->activate();

  pass2Prog->setVec2( "texCoordInc", vec2( 1 / (float) windowWidth, 1 / (float) windowHeight ) );

  pass2Prog->setInt( "depthSampler", DEPTH_GBUFFER );

  gbuffer->BindTexture( LAPLACIAN_GBUFFER );

  int activeDrawBuffers2[] = { LAPLACIAN_GBUFFER };
  gbuffer->setDrawBuffers( 1, activeDrawBuffers2 );

  glClear( GL_COLOR_BUFFER_BIT );
  glDisable( GL_DEPTH_TEST );

  drawFullscreenQuad();

  pass2Prog->deactivate();

  if (debug == 2) {
    gbuffer->DrawGBuffers();
    return;
  }

  // Pass 3: Draw everything using data from G-Buffers

  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
  glClear( GL_COLOR_BUFFER_BIT );
  glDisable( GL_DEPTH_TEST );

  pass3Prog->activate();

  pass3Prog->setVec2( "texCoordInc", vec2( 1 / (float) windowWidth, 1 / (float) windowHeight ) );
  pass3Prog->setVec3( "lightDir", lightDir );

  pass3Prog->setInt( "colourSampler",    COLOUR_GBUFFER );
  pass3Prog->setInt( "normalSampler",    NORMAL_GBUFFER );
  pass3Prog->setInt( "depthSampler",     DEPTH_GBUFFER );
  pass3Prog->setInt( "laplacianSampler", LAPLACIAN_GBUFFER );

  gbuffer->BindTexture( COLOUR_GBUFFER );
  gbuffer->BindTexture( NORMAL_GBUFFER );
  gbuffer->BindTexture( DEPTH_GBUFFER );
  gbuffer->BindTexture( LAPLACIAN_GBUFFER  );

  drawFullscreenQuad();

  pass3Prog->deactivate();
}
