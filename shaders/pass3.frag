// Pass 3 fragment shader
//
// Output fragment colour based using
//    (a) Cel shaded diffuse surface
//    (b) wide silhouette in black

#version 330

uniform vec3      lightDir;     // direction toward the light in the VCS
uniform vec2      texCoordInc;  // texture coord difference between adjacent texels

in vec2 texCoords;              // texture coordinates at this fragment
in vec4 gl_FragCoord;

// The following four textures are now available and can be sampled
// using 'texCoords'

uniform sampler2D colourSampler;
uniform sampler2D normalSampler;
uniform sampler2D depthSampler;
uniform sampler2D laplacianSampler;

out vec4 outputColour;          // the output fragment colour as RGBA with A=1

// comment to disable
#define DIFFUSE_COMPONENT
#define SPECULAR_COMPONENT
#define SILHOUETTE_BLEND

#define NUM_QUANTA 3

void main()

{
  // Look up values for the depth and Laplacian.  Use only
  // the R component of the texture as texture2D( ... ).r

  float d = texture2D(depthSampler, texCoords).r;
  float l = texture2D(laplacianSampler, texCoords).r;

  // Discard the fragment if it is a background pixel not
  // near the silhouette of the object.

  /* vec3 D = normalize(vec3(texture2D(depthSampler, texCoords))); */
  /* vec3 L = normalize(vec3(0.0,0.0,0.0) - vec3(texture2D(laplacianSampler, texCoords))); */
  /* float DdotL = abs(dot(D,L)); */

  if (d >= 1) {
    outputColour = vec4(1,1,1,1);
    return;
  }

  // Look up value for the colour and normal.  Use the RGB
  // components of the texture as texture2D( ... ).rgb or texture2D( ... ).xyz.

  vec3 N = texture2D(normalSampler, texCoords).xyz;
  vec3 C = texture2D(colourSampler, texCoords).xyz;

  // Compute Cel shading, in which the diffusely shaded
  // colour is quantized into four possible values.  Do not allow the
  // diffuse component, N dot L, to be below 0.2.  That will provide
  // some ambient shading.  Use the 'numQuata' below
  // to have that many divisions of quanta of colour.

  const int numQuanta = NUM_QUANTA;
  float ndotl = dot(normalize(N),normalize(lightDir));
  vec3 IOut;

  if (ndotl <= 0.2) {
    return;
  }

  //Cel-shading
  for (int i = numQuanta; i >= 1; i--) {
    float x = (1.0 / numQuanta) * i;
    if (ndotl > x) {
      IOut += x * C;
      break;
    }
  }

  //Phong
#ifdef DIFFUSE_COMPONENT
  IOut += ndotl * vec3(texture2D(depthSampler, texCoords));
#endif

#ifdef SPECULAR_COMPONENT
  vec3 R = (2.0 * ndotl) * N - lightDir;
  vec3 V = vec3(0,0,1);

  float rdotv = dot(R,V);

  if (rdotv > 0.0) {
    IOut += pow(rdotv, 200.0) * vec3(0.4, 0.4, 0.4);
  }
#endif

  /*   Modify this shader to have a silhouette that is black in */
  /*   the middle but a blend of black and the Phong-computed colour away */
  /*   from the silhouette middle.  Make sure that the shader is efficient */
  /*   and does not make unnecessary texture lookups in doing this. */

#ifdef SILHOUETTE_BLEND
  vec2 uv = gl_FragCoord.xy / vec2(600,450); //FIXME
  uv = 2.0 * uv - 1.0;
  float circle = uv.x * uv.x + uv.y * uv.y;
  vec4 o = 5 * mix(vec4(0,0,0,1), vec4(1,1,1,1), circle);
  IOut *= vec3(o);
#endif

  // Count number of fragments in the 3x3 neighbourhood of
  // this fragment with a Laplacian that is less than -0.1.  These are
  // the edge fragments.  Use the 'kernelRadius'
  // below and check all fragments in the range
  //
  //    [-kernelRadius,+kernelRadius] x [-kernelRadius,+kernelRadius]
  //
  // around this fragment.

  const int kernelRadius = 1;
  int nFragments = 0;
  vec4 sample[9];

  vec2 coords[9] = {
    vec2(-texCoordInc.x,-texCoordInc.y),
    vec2(0,-texCoordInc.y),
    vec2(texCoordInc.x,-texCoordInc.y),
    vec2(-texCoordInc.x,0),
    vec2(0,0),
    vec2(texCoordInc.x,0),
    vec2(-texCoordInc.x,texCoordInc.y),
    vec2(0,texCoordInc.y),
    vec2(texCoordInc.x,texCoordInc.y)
    };

  for (int i = 0; i < 9; i+=kernelRadius) {
    sample[i] = texture2D(laplacianSampler, texCoords + coords[i]);
    if (sample[i].r < -0.1)
      nFragments++;
  }

  // Output the fragment colour.  If there is an edge
  // fragment in the 3x3 neighbourhood of this fragment, output a
  // black colour.  Otherwise, output the cel-shaded colour.
  //
  // Since we're making this black is there is any edge in the 3x3
  // neighbourhood of the fragment, the silhouette will be wider
  // than if we test only the fragment.

  if (nFragments > 0)
    outputColour = vec4(0,0,0,1);
  else
    outputColour = vec4(IOut, 1.0);
}
