// GPUProgram class

#ifndef GPUPROGRAM_H
#define GPUPROGRAM_H


#include "headers.h"
#include "linalg.h"


class GPUProgram {

  unsigned int program_id;
  unsigned int shader_vp;
  unsigned int shader_fp;

 public:

  GPUProgram() {};

  GPUProgram( const char *vsFile, const char *fsFile ) {
    initFromFile( vsFile, fsFile );
  }

  void initFromFile( const char *vsFile, const char *fsFile ) {
    
    char* vsText = textFileRead(vsFile);	
    
    if (vsText == NULL) {
      std::cerr << "Vertex shader file '" << vsFile << "' not found." << std::endl;
      return;
    }
    
    char* fsText = textFileRead(fsFile);
    
    if (fsText == NULL) {
      std::cerr << "Fragment shader file '" << fsFile << "' not found." << std::endl;
      return;
    }
    
    init( vsText, fsText );
  }

  ~GPUProgram() {
    glDetachShader( program_id, shader_vp );
    glDeleteShader( shader_vp );

    glDetachShader( program_id, shader_fp );
    glDeleteShader( shader_fp );

    glDeleteProgram( program_id );
  }

  void init( char *vsText, char *fsText );

  int id() {
    return program_id;
  }

  void activate() {
    glUseProgram( program_id );
  }

  void deactivate() {
    glUseProgram( 0 );
  }

  void setMat4( char *name, mat4 &M ) {
    glUniformMatrix4fv( glGetUniformLocation( program_id, name ), 1, GL_TRUE, &M[0][0] );
  }

  void setVec2( char *name, vec2 v ) {
    glUniform2fv( glGetUniformLocation( program_id, name ), 1, &v[0] );
  }

  void setVec3( char *name, vec3 v ) {
    glUniform3fv( glGetUniformLocation( program_id, name ), 1, &v[0] );
  }

  void setVec4( char *name, vec4 v ) {
    glUniform4fv( glGetUniformLocation( program_id, name ), 1, &v[0] );
  }

  void setFloat( char *name, float f ) {
    glUniform1f( glGetUniformLocation( program_id, name ), f );
  }

  void setInt( char *name, int i ) {
    glUniform1i( glGetUniformLocation( program_id, name ), i );
  }

  char* textFileRead(const char *fileName);

  void glErrorReport( char *where ) {

    GLuint errnum;
    const char *errstr;

    while ((errnum = glGetError())) {
      errstr = reinterpret_cast<const char *>(gluErrorString(errnum));
      std::cerr << where << ": " << errstr << std::endl;
    }
  }

};

#endif
