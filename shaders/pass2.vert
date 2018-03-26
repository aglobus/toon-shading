// Pass 2 vertex shader
//
// Generate texture coordinates from raw vertex position.  The vertex
// position is in [-1,1]x[-1,1] and is mapped to [0,1]x[0,1].

#version 330

layout (location = 0) in vec3 vertPosition;

out vec2 texCoords;

void main()

{
  gl_Position = vec4( vertPosition, 1.0 );

  // Calculate the texture coordinates at this vertex.  X and Y vertex
  // coordinates are in the range [-1,1] in the window.  You have to
  // map this to the range [0,1] of texture coordinates.

  texCoords = vec2((gl_Position.x + 1) / 2, (gl_Position.y + 1) / 2);
}
