/* wavefront.cpp
*/


#include "headers.h"
#include "gpuProgram.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_PNG
#include <png.h>
#endif

#include "wavefront.h"


bool          wfModel::newGroupWithNewMaterial = false;
bool          wfModel::verticesAreCW = false;

unsigned char wfMaterial::defaultTexmap[] = { 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255 };


/* Read a Wavefront model into this structure.  See ObjectFile.html
* for a description of the Wavefront file format.  This code is from
* the Nate Robins GLM library.
*/

void wfModel::read( char *filename )

{
  FILE* file;
  char  buf[1000];
  float x, y, z;
  wfGroup    *currentGroup;
  wfMaterial *currentMaterial;
  int   nextGroupNum = 0;

  // Counts of different vertex formats

  int numVTN = 0;
  int numVT = 0;
  int numVN = 0;
  int numV = 0;

  /* init */

  vertices.clear();
  normals.clear();
  texcoords.clear();
  facetnorms.clear();
  materials.clear();
  groups.clear();

  pathname = strdup(filename);

  groups.add( new wfGroup( "default" ) );
  currentGroup = groups[0];

  materials.add( new wfMaterial( "default" ) );
  currentMaterial = materials[0];

  currentGroup->material = currentMaterial;

  /* open the file */

  file = fopen(filename, "r");
  if (!file) {
    cerr << "wfModel::read() failed: can't open data file '" << filename << "'." << endl;
    exit(-1);
  }

  /* process each line */

  lineNum = 0;

  while(fscanf(file, "%s", buf) != EOF) {

    lineNum++;

    int v = 0, n = 0, t = 0;
    wfTriangle *tri, *prevTri;

    switch(buf[0]) {

    case '#':				/* comment */
      /* eat up rest of line */
      fgets(buf, sizeof(buf), file);
      break;

    case 's':				/* smoothing group ... ignore */
      /* eat up rest of line */
      fgets(buf, sizeof(buf), file);
      break;

    case 'v':				/* v, vn, vt */
      switch(buf[1]) {

      case '\0':			/* vertex */
	fscanf(file, "%f %f %f", &x, &y, &z );
	vertices.add( vec3(x,y,z) );
	break;

      case 'n':				/* normal */
	fscanf(file, "%f %f %f", &x, &y, &z );
	normals.add( vec3(x,y,z) );
	break;

      case 't':				/* texcoord */
	fscanf(file, "%f %f", &x, &y );
	texcoords.add( vec3(x,y,0) );
	fgets(buf, sizeof(buf), file); // skip rest of line
	break;
      }
      break;

    case 'm':			        /* mtllib filename */
      fgets(buf, sizeof(buf), file);
      sscanf(buf, "%s %s", buf, buf);
      mtllibname = strdup(buf);
      readMaterialLibrary( buf );
      break;

    case 'u':			        /* usemtl name */

      if (newGroupWithNewMaterial) {
	char buffer[100];
	sprintf( buffer, "g%d", nextGroupNum++ );
	currentGroup = findGroup( buffer );
      }

      fgets(buf, sizeof(buf), file);
      sscanf(buf, "%s %s", buf, buf);
      currentGroup->material = currentMaterial = findMaterial( buf );
      break;

    case 'g':				/* group */
      /* eat up rest of line */
      fgets(buf, sizeof(buf), file);
      sscanf(buf, "%s", buf);
      if (buf[0] == '\n')
	currentGroup = findGroup( "default" );
      else
	currentGroup = findGroup( buf );
      currentGroup->material = currentMaterial;
      break;

    case 'f':				/* face */

      fscanf(file, "%s", buf);

      /* can be one of %d, %d//%d, %d/%d, or %d/%d/%d */

      tri = new wfTriangle();

      if (strstr(buf, "//")) {	/* v//n */

	numVN++;

	/* First three vertices define a triangle */

	sscanf(buf, "%d//%d", &v, &n);	v--; checkVindex(v); n--; tri->vindices[0] = v; tri->nindices[0] = n;
	fscanf(file, "%d//%d", &v, &n);	v--; checkVindex(v); n--; tri->vindices[1] = v; tri->nindices[1] = n;
	fscanf(file, "%d//%d", &v, &n);	v--; checkVindex(v); n--; tri->vindices[2] = v; tri->nindices[2] = n;

	currentGroup->triangles.add( tri );

	/* More vertices (a convex polygon) are converted to a fan of triangles: */

	while(fscanf(file, "%d//%d", &v, &n) > 0) {

	  v--; checkVindex(v); n--;

	  prevTri = tri;
	  tri = new wfTriangle();

	  tri->vindices[0] = prevTri->vindices[0];
	  tri->nindices[0] = prevTri->nindices[0];
	  tri->vindices[1] = prevTri->vindices[2];
	  tri->nindices[1] = prevTri->nindices[2];
	  tri->vindices[2] = v;
	  tri->nindices[2] = n;

	  currentGroup->triangles.add( tri );
	}

      } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {  /* v/t/n */

	numVTN++;

	v--; checkVindex(v); n--; t--;

	tri->vindices[0] = v;
	tri->tindices[0] = t;
	tri->nindices[0] = n;
	fscanf(file, "%d/%d/%d", &v, &t, &n);	v--; checkVindex(v); n--; t--;
	tri->vindices[1] = v;
	tri->tindices[1] = t;
	tri->nindices[1] = n;
	fscanf(file, "%d/%d/%d", &v, &t, &n);	v--; checkVindex(v); n--; t--;
	tri->vindices[2] = v;
	tri->tindices[2] = t;
	tri->nindices[2] = n;

	currentGroup->triangles.add( tri );

	while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {

	  v--; checkVindex(v); n--; t--;

	  prevTri = tri;
	  tri = new wfTriangle();

	  tri->vindices[0] = prevTri->vindices[0];
	  tri->tindices[0] = prevTri->tindices[0];
	  tri->nindices[0] = prevTri->nindices[0];
	  tri->vindices[1] = prevTri->vindices[2];
	  tri->tindices[1] = prevTri->tindices[2];
	  tri->nindices[1] = prevTri->nindices[2];
	  tri->vindices[2] = v;
	  tri->tindices[2] = t;
	  tri->nindices[2] = n;

	  currentGroup->triangles.add( tri );
	}

      } else if (sscanf(buf, "%d/%d", &v, &t) == 2) { /* v/t */

	numVT++;

	v--; checkVindex(v); t--;

	tri->vindices[0] = v;
	tri->tindices[0] = t;
	fscanf(file, "%d/%d", &v, &t);	v--; checkVindex(v); t--;
	tri->vindices[1] = v;
	tri->tindices[1] = t;
	fscanf(file, "%d/%d", &v, &t);	v--; checkVindex(v); t--;
	tri->vindices[2] = v;
	tri->tindices[2] = t;

	currentGroup->triangles.add( tri );

	while(fscanf(file, "%d/%d", &v, &t) > 0) {

	  v--; checkVindex(v); t--;

	  prevTri = tri;
	  tri = new wfTriangle();

	  tri->vindices[0] = prevTri->vindices[0];
	  tri->tindices[0] = prevTri->tindices[0];
	  tri->vindices[1] = prevTri->vindices[2];
	  tri->tindices[1] = prevTri->tindices[2];
	  tri->vindices[2] = v;
	  tri->tindices[2] = t;

	  currentGroup->triangles.add( tri );
	}

      } else {	/* v */

	numV++;

	sscanf(buf, "%d", &v); v--; checkVindex(v);
	tri->vindices[0] = v;
	fscanf(file, "%d", &v); v--; checkVindex(v);
	tri->vindices[1] = v;
	fscanf(file, "%d", &v); v--; checkVindex(v);
	tri->vindices[2] = v;

	currentGroup->triangles.add( tri );

	while(fscanf(file, "%d", &v) > 0) {

	  v--; checkVindex(v);

	  prevTri = tri;
	  tri = new wfTriangle();

	  tri->vindices[0] = prevTri->vindices[0];
	  tri->vindices[1] = prevTri->vindices[2];
	  tri->vindices[2] = v;

	  currentGroup->triangles.add( tri );
	}
      }
      break;

    default:
      cerr << "Warning: unrecognized Wavefront command on line " << lineNum << ": " << buf << endl;
      break;
    }
  }

  // Determine a consistent format for each vertex

  hasVertexNormals   = ((numVTN > 0 || numVN > 0) && numVT == 0 && numV == 0);
  hasVertexTexCoords = ((numVTN > 0 || numVT > 0) && numVN == 0 && numV == 0);

  // Compute all face normals

  for (int g=0; g<groups.size(); g++)
    for (int i=0; i<groups[g]->triangles.size(); i++) {

      wfTriangle &tri = *groups[g]->triangles[i];

      vec3 d01 = vertices[ tri.vindices[1] ] - vertices[ tri.vindices[0] ];
      vec3 d02 = vertices[ tri.vindices[2] ] - vertices[ tri.vindices[0] ];
      vec3 n;

      if (verticesAreCW)
	n = (d02 ^ d01).normalize();
      else
	n = (d01 ^ d02).normalize();

      tri.findex = facetnorms.size();
      facetnorms.add( n );
    }

  // Find bounding box

  min = vec3(MAXFLOAT,MAXFLOAT,MAXFLOAT);
  max = vec3(-MAXFLOAT,-MAXFLOAT,-MAXFLOAT);

  vec3 sum(0,0,0);

  for (int i=0; i<vertices.size(); i++) {
    sum = sum + vertices[i];

    if (vertices[i].x < min.x)
      min.x = vertices[i].x;
    if (vertices[i].y < min.y)
      min.y = vertices[i].y;
    if (vertices[i].z < min.z)
      min.z = vertices[i].z;
    if (vertices[i].x > max.x)
      max.x = vertices[i].x;
    if (vertices[i].y > max.y)
      max.y = vertices[i].y;
    if (vertices[i].z > max.z)
      max.z = vertices[i].z;
  }

  centre = 0.5 * (min + max);
  radius = 0.5 * (max - min).length();
}


void wfModel::readMaterialLibrary( char *name )

{
  FILE* file;
  char  buf[1000];
  wfMaterial *currentMaterial;
  int i;

  /* prepend path to the directory of the model file */

  char *dir = new char[ strlen( pathname )+1 ];

  strcpy( dir, pathname );

  char *s = strrchr(dir, '/');
  if (s)
    s[1] = '\0';
  else
    dir[0] = '\0';

  char *filename = new char[ strlen(dir) + strlen(name) + 1 ];

  strcpy(filename, dir);
  strcat(filename, name);

  /* open the file */

  file = fopen(filename, "r");
  if (!file) {
    cerr << "wfModel::readMaterialLibrary() couldn't open file '" << filename << "'" << endl;
    exit(-1);
  }

  /* set the default material */

  if (materials.size() == 0)
    materials.add( new wfMaterial("default") );

  currentMaterial = materials[0];

  /* now, read in the data */

  while(fscanf(file, "%s", buf) != EOF) {
    switch(buf[0]) {

    case '#':				/* comment */
      /* eat up rest of line */
      fgets(buf, sizeof(buf), file);
      break;

    case 'n':				/* newmtl */
      fgets(buf, sizeof(buf), file);
      sscanf(buf, "%s %s", buf, buf);
      for (i=0; i<materials.size(); i++)
	if (strcmp(materials[i]->name, buf) == 0)
	  break;
      if (i<materials.size())
	currentMaterial = materials[ i ];
      else {
	materials.add( new wfMaterial(buf) );
	currentMaterial = materials[ materials.size()-1 ];
      }
      break;

    case 'N':			        /* Ns */ 
      fscanf(file, "%f", &currentMaterial->shininess);
      /* wavefront shininess is from [0, 1000], so scale for OpenGL */
      //currentMaterial->shininess /= 1000.0;
      //currentMaterial->shininess *= 128.0;
      break;

    case 'K':
      switch(buf[1]) {

      case 'd':
	fscanf( file, "%f %f %f",
		&currentMaterial->diffuse[0],
		&currentMaterial->diffuse[1],
		&currentMaterial->diffuse[2] );
	break;

      case 's':
	fscanf( file, "%f %f %f",
		&currentMaterial->specular[0],
		&currentMaterial->specular[1],
		&currentMaterial->specular[2] );
	break;

      case 'a':
	fscanf( file, "%f %f %f",
		&currentMaterial->ambient[0],
		&currentMaterial->ambient[1],
		&currentMaterial->ambient[2]);
	break;

      default:
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
      }
      break;

    case 'm':			/* map_Kd filename */
      fgets(buf, sizeof(buf), file);
      sscanf(buf, "%s %s", buf, buf);

      {
	// prepend path to the directory of the model file

	char *dir = new char[ strlen(pathname) ];
	strcpy( dir, pathname );

	char *s = strrchr(dir, '/');
	if (s != NULL) s[1] = '\0'; else dir[0] = '\0';

	char *filename = new char[ strlen(dir) + strlen(buf) + 1 ];

	strcpy(filename, dir);
	strcat(filename, buf);

	// load the texture

	currentMaterial->loadTexmap( filename );

	delete [] dir;
	delete [] filename;
      }

      break;

    default:
      /* eat up rest of line */
      fgets(buf, sizeof(buf), file);
      break;
    }
  }

  delete [] dir;
  delete [] filename;
}


/* read a ppm texture map into the material
*/


void wfMaterial::loadTexmap( char *filename )

{
  char *p = strrchr( filename, '.' );
  if (p == NULL || strcmp( p, ".ppm" ) == 0)
    texmap = readP6( filename );
  else
    texmap = readPNG( filename );
}


wfMaterial* wfModel::findMaterial( char *name )

{
  int i;

  for (i=0; i<materials.size(); i++)
    if (strcmp( name, materials[i]->name ) == 0)
      break;

  if (i < materials.size())
    return materials[i];

  cerr << "Error: Can't find material '" << name << "'" << endl;
  return materials[0];
}


wfGroup* wfModel::findGroup( char *name )

{
  int i;

  for (i=0; i<groups.size(); i++)
    if (strcmp( name, groups[i]->name ) == 0)
      break;

  if (i < groups.size())
    return groups[i];

  // create a new group of this name

  groups.add( new wfGroup(name) );
  return groups[ groups.size() - 1 ];
}


class VertexSignature {
public:
  unsigned int sig[3];
  bool operator == (const VertexSignature p) {
    return sig[0] == p.sig[0] && sig[1] == p.sig[1] && sig[2] == p.sig[2];
  }
};



void wfModel::setupVAO()

{
  // Note that positions, normals, and texture coordinates can all be
  // indexed differently in a Wavefront file.  But OpenGL permits only
  // one index per vertex, and the OpenGL vertex encapsulates all
  // attributes, including position, normal, and texture coordinates.
  //
  // So we have to create *another* array of vertices where each
  // vertex stores position, normal, and texture coordinates and the
  // face indices index into this new array.

  unsigned int vertexSize = 3;

  if (hasVertexNormals)
    vertexSize += 3;

  if (hasVertexTexCoords)
    vertexSize += 2;

  // Process each group separately

  for (int i=0; i<groups.size(); i++) {

    wfGroup *thisGroup = groups[i];

    int numTriangles = thisGroup->triangles.size();

    if (numTriangles > 0) {

      GLfloat *vertexBuffer =  new GLfloat[ numTriangles * 3 * vertexSize ];
      GLuint *faceIndexBuffer= new GLuint[ numTriangles * 3 ];

      unsigned int nVerts = 0;
      nFaces = 0;

      VertexSignature *vertSig = new VertexSignature[ numTriangles * 3 ];

      for (int j=0; j<thisGroup->triangles.size(); j++) {

	wfTriangle *tri = thisGroup->triangles[j];

	for (int k=0; k<3; k++) {

	  // Find an already-stored vertex with this signature (brute force)

	  VertexSignature vs;

	  vs.sig[0] = tri->vindices[k];
	  vs.sig[1] = tri->nindices[k];
	  vs.sig[2] = tri->tindices[k];

	  unsigned int l;
	  for (l=0; l<nVerts; l++)
	    if (vs == vertSig[l])
	      break;

	  if (l == nVerts) {	// none found ... create a new vertex
	    * (vec3*) &vertexBuffer[nVerts*vertexSize] = vertices[ tri->vindices[k] ];
	    if (hasVertexNormals)
	      * (vec3*) &vertexBuffer[nVerts*vertexSize+3] = normals[ tri->nindices[k] ];
	    if (hasVertexTexCoords)
	      if (hasVertexNormals)
		* (vec2*) &vertexBuffer[nVerts*vertexSize+6] = * (vec2*) &texcoords[ tri->tindices[k] ];
	      else
		* (vec2*) &vertexBuffer[nVerts*vertexSize+3] = * (vec2*) &texcoords[ tri->tindices[k] ];

	    vertSig[ nVerts ] = vs;

	    nVerts++;
	  }

	  // Store this vertex index

	  faceIndexBuffer[ nFaces * 3 + k ] = l;
	}

	nFaces++;

      }

      //cout << "stored " << nVerts << " verts, " << nFaces << " faces" << endl;

      // Set up the VAO

      glGenVertexArrays( 1, &thisGroup->VAO );
      glBindVertexArray( thisGroup->VAO );

      GLuint bufferID;

      // store vertices

      glGenBuffers( 1, &bufferID );
      glBindBuffer( GL_ARRAY_BUFFER, bufferID );
      glBufferData( GL_ARRAY_BUFFER, nVerts * vertexSize * sizeof(GLfloat), vertexBuffer, GL_STATIC_DRAW );

      // store faces

      glGenBuffers( 1, &bufferID );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bufferID );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, nFaces * 3 * sizeof(GLuint), faceIndexBuffer, GL_STATIC_DRAW );

      // define attributes

      int attribIndex = 0;
      unsigned long int accumulatedOffset = 0;

      // position = attribute 0

      glEnableVertexAttribArray( attribIndex );
      glVertexAttribPointer( attribIndex, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (const GLvoid*) accumulatedOffset );
      attribIndex++;
      accumulatedOffset += 3 * sizeof( float );

      // normals = next attribute

      if (hasVertexNormals) {
	glEnableVertexAttribArray( attribIndex );
	glVertexAttribPointer( attribIndex, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (const GLvoid*) accumulatedOffset );
	attribIndex++;
	accumulatedOffset += 3 * sizeof( float );
      }

      // texture coordinates = next attribute

      if (hasVertexTexCoords) {
	glEnableVertexAttribArray( attribIndex );
	glVertexAttribPointer( attribIndex, 2, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (const GLvoid*) accumulatedOffset );
	attribIndex++;
	accumulatedOffset += 2 * sizeof( float );
      }

      thisGroup->VAOinitialized = true;

      delete [] vertexBuffer;
      delete [] faceIndexBuffer;
      delete [] vertSig;
    }
  }

  initTextures();
}


void wfModel::draw( GPUProgram * gpuProg )

{
  for (int i=0; i<groups.size(); i++)
    if (groups[i]->VAOinitialized) {

      // Set up material properties

      groups[i]->material->setMaterial( true, true, gpuProg );

      // Render

      glBindVertexArray( groups[i]->VAO );
      glDrawElements( GL_TRIANGLES, nFaces * 3, GL_UNSIGNED_INT, 0 );
    }
}


void wfMaterial::setMaterial( bool useTextures, bool useMaterial, GPUProgram * gpuProg )

{
  if (useMaterial) {
    gpuProg->setVec3( "kd", vec3( &diffuse[0] ) );
    gpuProg->setVec3( "ks", vec3( &specular[0] ) );
    gpuProg->setVec3( "Ia", vec3( &ambient[0] ) );
    gpuProg->setVec3( "Ie", vec3( &emissive[0] ) );
    gpuProg->setFloat( "shininess", shininess );
  }

  if (useTextures) {
    if (texmap == NULL) {
      glDisable( GL_TEXTURE_2D );
      glDisable(GL_BLEND);
    } else {
      glEnable( GL_TEXTURE_2D );
      glActiveTexture( GL_TEXTURE0 ); // use texture unit zero
      glBindTexture( GL_TEXTURE_2D, textureID );
      gpuProg->setInt( "texSampler", 0 );
      if (hasAlpha) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      } else
	glDisable(GL_BLEND);
    }
  }
}



void wfMaterial::storeTexture()

{
  unsigned int xdim, ydim;
  GLubyte *p, *q;

  xdim = width;
  ydim = height;

  // Create texture map of size 2^k x 2^l, filling blank space with black

  GLubyte *newTexMap = new GLubyte[ xdim * ydim * (hasAlpha ? 4 : 3) ];

  p = &newTexMap[0];		// destination
  q = texmap;			// source

  for (unsigned int y=0; y<ydim; y++)
    for (unsigned int x=0; x<xdim; x++)
      if (y >= height || x >= width) {
	*p++ = 0; *p++ = 0; *p++ = 0; // black outside of map
	if (hasAlpha) *p++ = 0;
      } else {
	*p++ = *q++; *p++ = *q++; *p++ = *q++; // copy from source
	if (hasAlpha) { *p++ = *q++; }
      }

  // Register it with OpenGL

  glBindTexture( GL_TEXTURE_2D, textureID );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

#if 1

  // ordinary texture

  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  glTexImage2D( GL_TEXTURE_2D, 0, (hasAlpha ? GL_RGBA : GL_RGB), xdim, ydim, 0,
		(hasAlpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, newTexMap );

#else

  // mip-mapped texture

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

  gluBuild2DMipmaps( GL_TEXTURE_2D, (hasAlpha ? GL_RGBA : GL_RGB), xdim, ydim,
		     (hasAlpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, newTexMap );

#endif

  delete [] newTexMap;
}


/* Initialize the textures by assigning each an OpenGL ID and storing
* each with OpenGL.
*/


void wfModel::initTextures()

{
  // Count the textures

  int count = 0;
  int countGroups = 0;
  for (int i=0; i<groups.size(); i++) {
    if (groups[i]->material->texmap != NULL)
      count++;
    if (groups[i]->triangles.size() > 0)
      countGroups++;
  }

  // Generate OpenGL texture IDs

  GLuint *ids = new GLuint[ count ];
  glGenTextures( count, &ids[0] );

  // Assign the IDs and store the textures

  int j = 0;

  for (int i=0; i<groups.size(); i++)
    if (groups[i]->material->texmap != NULL) {
      groups[i]->material->textureID = ids[j++];
      groups[i]->material->storeTexture();
    }

  delete [] ids;
}



/* Read a texture from a P6 PPM file
*/


unsigned char *wfMaterial::readP6( char *filename )

{
  char buffer[1000];
  int i, xdim, ydim;
  unsigned char *a, *b, *pa, *pb;

  int f = open( filename, O_RDONLY );

  if (f == -1) {
    cerr << "Open of `" << filename << "' failed.\n";
    exit(1);
  }

  // first line

  do {
    i = 0;
    do 
      read(f,&buffer[i],1);
    while (buffer[i++] != '\n');
  } while (buffer[0] == '#');

  if (strncmp( buffer, "P6", 2 ) != 0) {
    cerr << filename << " is not a P6 file.\n";
    exit(1);
  }

  // second line

  do {
    i = 0;
    do 
      read(f,&buffer[i],1);
    while (buffer[i++] != '\n');
  } while (buffer[0] == '#');
  buffer[i] = '\0';
  sscanf( buffer, "%d %d", &xdim, &ydim );

  width = xdim;
  height = ydim;

  // third line

  do {
    i = 0;
    do 
      read(f,&buffer[i],1);
    while (buffer[i++] != '\n');
  } while (buffer[0] == '#');
  if (strncmp( buffer, "255", 3 ) != 0) {
    cerr << filename << " is not a 24-bit file.\n";
    exit(1);
  }

  // read the data (stored top-to-bottom, left-to-right)

  a = new unsigned char[ xdim * ydim * 3 ];
  read( f, a, xdim*ydim*3 );

  // flip the image vertically (stored bottom-to-top, left-to-right)

  b = new unsigned char[ xdim * ydim * 3 ];

  for (int i=0; i<ydim; i++) {
    pa = a + (i)*xdim*3;
    pb = b + (ydim-1-i)*xdim*3;
    for (int j=0; j<xdim*3; j++)
      *(pb++) = *(pa++);
  }

  delete [] a;

  hasAlpha = false;

  close(f);
  return b;
}



// Read a PNG file.  Most of this code is taken from example.c, which
// is provided with the libpng distribution.


#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif


#define PNG_BYTES_TO_CHECK 8

unsigned char *wfMaterial::readPNG( char *filename )

{
  unsigned char *b;

#ifndef HAVE_PNG
  cerr << "Trying to read PNG file \"" << filename << "\", but the program wasn't compiled with -DHAVE_PNG." << endl;
  exit(-1);
  return b;
#else

  png_structp png_ptr;
  png_infop info_ptr;
  unsigned int sig_read = 0;
  int bit_depth, color_type, interlace_type;
  char header[PNG_BYTES_TO_CHECK];

  // Open file

  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    cerr << "Can't open PNG texture file '" << filename << "'." << endl;
    exit(-1);
  }

  // Check header

  fread( header, 1, PNG_BYTES_TO_CHECK, fp );
  bool is_png = !png_sig_cmp( (png_byte*) &header[0], 0, PNG_BYTES_TO_CHECK);
  if (!is_png) {
    cerr << "Texture file '" << filename << "' is not in PNG format." << endl;
    exit(-1);
  }

  /* Create and initialize the png_struct with the desired error handler
   * functions.  If you want to use the default stderr and longjump method,
   * you can supply NULL for the last three parameters.  We also supply the
   * the compiler header file version, so that we know if the application
   * was compiled with a compatible version of the library.  REQUIRED
   */

  png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

  if (png_ptr == NULL) {
    cerr << "Can't initialize PNG file for reading: " << filename << endl;
    fclose(fp);
    exit(-1);
  }

  /* Allocate/initialize the memory for image information.  REQUIRED. */

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL)
    {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      cerr << "Can't allocate memory to read PNG file: " << filename << endl;
      exit(-1);
    }

  /* Set error handling if you are using the setjmp/longjmp method (this is
   * the normal method of doing things with libpng).  REQUIRED unless you
   * set up your own error handlers in the png_create_read_struct() earlier.
   */

  if (setjmp(png_jmpbuf(png_ptr))) {
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fp);
    /* If we get here, we had a problem reading the file */
    cerr << "Exception occurred while reading PNG file: " << filename << endl;
    exit(-1);
  }

  /* Set up the input control if you are using standard C streams */

  png_init_io(png_ptr, fp);

  /* If we have already read some of the signature */

  png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

  // Warning: the following does NOT convert grey to RGB:

  png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL );


  // Store in texmap

  int numChannels = png_get_channels(png_ptr, info_ptr);

  if (png_get_bit_depth(png_ptr, info_ptr) != 8) {
    cerr << "Can't handle PNG files with bit depth other than 8.  '" << filename
	 << "' has " << png_get_bit_depth(png_ptr, info_ptr) << " bits per pixel." << endl;
    exit(-1);
  }


  width = png_get_image_width(png_ptr,info_ptr);
  height = png_get_image_height(png_ptr,info_ptr);

  int imageSize;

  if (numChannels == 4)
    imageSize = 4 * width * height;
  else
    imageSize = 3 * width * height;

  b = pb = new unsigned char[ imageSize ];

  for (int r=(int)info_ptr->height - 1; r >= 0; r--) {
    png_bytep row = info_ptr->row_pointers[r];
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    for (int c=0; c < rowbytes; c++)
      switch (numChannels) {
      case 1:
	*(pb)++ = row[c];
	*(pb)++ = row[c];
	*(pb)++ = row[c];
	break;
      case 2:
	cerr << "Can't handle a two-channel PNG file: " << filename << endl;
	exit(-1);
	break;
      case 3:
      case 4:
	*(pb)++ = row[c];
	break;
      }
  }

  hasAlpha = (numChannels == 4);

  // Clean up PNG stuff

  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
  fclose(fp);

  return b;
#endif
}

