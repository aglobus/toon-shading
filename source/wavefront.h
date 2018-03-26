/* wavefront.h
 *
 * A Wavefront object defined from a .OBJ file.  This can represent
 * only a subset of all Wavefront objects.
 */


#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include "headers.h"

#include "seq.h"
#include "linalg.h"
#include "shadeMode.h"
#include "gpuProgram.h"


/* A material with lighting properties and perhaps a texture map
 */


class wfMaterial {

  unsigned char *readP6( char *filename ); /* read a P6 PPM file */
  unsigned char *readPNG( char *filename ); /* read a P6 PPM file */

  static unsigned char defaultTexmap[];

 public:
  char    *name;		/* name of material */
  GLfloat diffuse[4];		/* diffuse component */
  GLfloat ambient[4];		/* ambient component */
  GLfloat specular[4];		/* specular component */
  GLfloat emissive[4];		/* emmissive component */
  GLfloat shininess;		/* specular exponent */

  GLubyte *texmap;		/* texture map */
  unsigned int width, height;   /* texmap dimensions */
  GLuint  textureID;		/* the OpenGL ID for this texture */
  bool    hasAlpha;		/* texmap has alpha component */

  wfMaterial() {}

  wfMaterial( char *n ) {
    name = new char[ strlen(n)+1 ];
    strcpy( name, n );

    diffuse[0]  = 1.0;  diffuse[1] = 1.0;  diffuse[2] = 1.0;  diffuse[3] = 1.0;
    ambient[0]  = 0.2;  ambient[1] = 0.2;  ambient[2] = 0.2;  ambient[3] = 1.0;
    specular[0] = 0.5; specular[1] = 0.5; specular[2] = 0.5; specular[3] = 1.5;
    emissive[0] = 0.0; emissive[1] = 0.0; emissive[2] = 0.0; emissive[3] = 1.0;
    shininess = 0;
    texmap = NULL;
    width = height = 0;
  }

  ~wfMaterial() {
    delete [] name;
  }

  void loadTexmap( char *filename ); /* read a ppm texture map */
  void storeTexture();		     /* record texture with OpenGL */
  void setMaterial( bool useTex, bool useMat, GPUProgram * gpuProg ); /* set the current OpenGL context */
};


/* A triangle with vertices, vertex normals, texture coordinates, and
 * a face normal
 */


class wfTriangle {
 public:
  GLuint vindices[3];		/* array of triangle vertex indices */
  GLuint nindices[3];		/* array of triangle normal indices */
  GLuint tindices[3];		/* array of triangle texcoord indices*/
  GLuint findex;		/* index of triangle facet normal */
};


/* A group of triangles sharing the same material
 */


class wfGroup {
 public:
  char             *name;	/* name of this group */
  seq<wfTriangle*> triangles;	/* triangles of this group */
  wfMaterial       *material;	/* material for group */
  GLuint           VAO;
  bool             VAOinitialized;

  wfGroup() {}

  wfGroup( char *gname ) {
    name = new char[ strlen(gname)+1 ];
    strcpy( name, gname );
    VAOinitialized = false;
  }

  ~wfGroup() {
    delete [] name;
  }

  wfGroup( const wfGroup & source ) { // copy constructor
    name = strdup(source.name);
    triangles = source.triangles;
    material = source.material;
  }

  wfGroup const &operator=( wfGroup const &src ) { // assignment operator
    if (this != &src) {
      name = strdup(src.name);
      triangles = src.triangles;
      material = src.material;
    }
    return *this;
  }
};


/* A model consisting of groups
 */


class wfModel {
  char*    pathname;		/* path to this model */
  char*    mtllibname;		/* name of the material library */

  seq<vec3>  vertices;	/* vertices */
  seq<vec3>  normals;		/* face normals */
  seq<vec3>  texcoords;	/* texture coordinates */
  seq<vec3>  facetnorms;	/* face normals */

  seq<wfMaterial*> materials;	/* materials */
  seq<wfGroup*>    groups;	/* groups (which themselves store the triangles) */

  bool hasVertexNormals;	/* ALL vertices have normals */
  bool hasVertexTexCoords;	/* ALL vertices have texture coordinates */

  bool texturesInitialized;

  wfMaterial* findMaterial( char *name );            /* find a named material */
  wfGroup*    findGroup( char *name );               /* find a named group */
  void        readMaterialLibrary( char *filename ); /* read all materials */
  void        initTextures();	                     /* assign texture IDs and store all textures */

  int lineNum;

  unsigned int nFaces;

 public:

  vec3 centre;			/* centre of point cloud */
  float radius;			/* radius of point cloud */

  // Global vars controlling the rendering

  static bool newGroupWithNewMaterial; /* create a new group each time the material changes */
  static bool verticesAreCW;	       /* calculate opposite-to-usual face normals */

  vec3 min, max;		/* extents */

  wfModel() {
    texturesInitialized = false;
    pathname = mtllibname = NULL;
  }

  wfModel( char *filename ) {
    texturesInitialized = false;
    pathname = mtllibname = NULL;
    read( filename );
    setupVAO();
  }

  ~wfModel() {
  }

  void read( char *filename );         /* instantiate this model from a file */
  void draw( GPUProgram * gpuProg );
  void setupVAO();

  void checkVindex( int v ) {
    if (v < 0 || v >= vertices.size()) {
      cerr << "error on line " << lineNum
	   << ": vertex index " << v+1 << " is too large.  There are only "
	   << vertices.size() << " vertices." << endl;
      abort();
    }
  }
};

#endif
