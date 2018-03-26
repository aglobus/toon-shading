// Pass 1 vertex shader
//
// Stores colour, normal, depth

#version 330

uniform mat4 M;
uniform mat4 MV;
uniform mat4 MVP;

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec3 vertTexCoord;

// shader should compute the colour, normal (in the VCS), and
// depth (in the range [0,1] with 0=near and 1=far) and store these
// values in the corresponding variables.

out vec3 colour;
out vec3 normal;
out float depth;

void main()

{
  // calc vertex position in CCS (always required)

  gl_Position = MVP * vec4( vertPosition, 1.0 );

  // Provide a colour

  colour = 2.0 * vec3(.2,.3,.4);

  // calculate normal in VCS

  normal = vec3(0,1,0);
  normal = vec3(MV * vec4(vertNormal, 0.0));

  // Calculate the depth in [0,1]

  depth = 0.5 * ((gl_Position.z / gl_Position.w) + 1.0);
}
