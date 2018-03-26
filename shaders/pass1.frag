// Pass 1 fragment shader
//
// Outputs colour, normal, depth to three textures

#version 330

in vec3 colour;
in vec3 normal;
in float depth;

// Output to the three textures.  Location i corresponds to
// GL_COLOUR_ATTACHMENT + i in the Framebuffer Object (FBO).

layout (location = 0) out vec3 fragColour;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec3 fragDepth;

// Interpolated inputs as simply copied to the three outputs.  You do
// not have to modify this shader.

void main()

{
  fragColour = colour;
  fragNormal = normal;
  fragDepth  = vec3( depth );   // depth is stored (inefficiently) in an RGB texture
}
