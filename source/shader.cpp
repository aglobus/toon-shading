// Phong shading demo
//
// Draw a Phong-shaded object


#include "headers.h"
#include "linalg.h"
#include "wavefront.h"
#include "renderer.h"
#include "font.h"


wfModel *obj;			// the object

Renderer *renderer;		// class to do multipass rendering

float theta = 0;
bool sleeping = false;

GLuint windowWidth = 600;
GLuint windowHeight = 450;
float factor = 0;

// Drawing function

float fovy;
vec3 eyePosition;


bool isTorso = false;		// for torso.obj model


void display()

{
  glClearColor( 1.0, 1.0, 1.0, 0.0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glEnable( GL_DEPTH_TEST );

  // OCS-to-WCS

  mat4 M;

  if (isTorso)
    M = rotate( theta, vec3(0,1,0) )
      * rotate( -M_PI/2.0, vec3(1,0,0) )
      * translate( -1 * obj->centre );
  else
    M = rotate( theta, vec3(0.5,2,0) )
      * translate( -1 * obj->centre );

  // model-view transform (i.e. OCS-to-VCS)

  mat4 MV = translate( -1 * eyePosition )
          * M;

  // model-view-projection transform (i.e. OCS-to-CCS)

  float n = (eyePosition - obj->centre).length() - obj->radius;
  float f = (eyePosition - obj->centre).length() + obj->radius;

  mat4 MVP = perspective( fovy, windowWidth / (float) windowHeight, n, f )
           * MV;

  // Light direction in VCS is above, to the right, and behind the
  // eye.  That's in direction (1,1,1) since the view direction is
  // down the -z axis.

  vec3 lightDir(1,1,0.2);
  lightDir = lightDir.normalize();

  // Draw the objects

  renderer->render( obj, M, MV, MVP, lightDir );

  // Output status message

  char buffer[1000];
  renderer->makeStatusMessage( buffer );
  glColor3f(0.3,0.3,1.0);
  printString( buffer, 10, 10, windowWidth, windowHeight );

  // Done

  glutSwapBuffers();
}


// Reshape the window


void reshape( int newWidth, int newHeight )

{
  windowWidth = newWidth;
  windowHeight = newHeight;

  glViewport( 0, 0, newWidth, newHeight );

  renderer->reshape( newWidth, newHeight );
}



// Update the object angle upon idle


void idle()

{
  // Determine time elapsed from start

  static bool firstTime = true;
  static struct timeb startTime;

  if (firstTime) {
    ftime( &startTime );
    firstTime = false;
  }

  struct timeb thisTime;
  ftime( &thisTime );
  float elapsedTime = (thisTime.time + thisTime.millitm / 1000.0) - (startTime.time + startTime.millitm / 1000.0);

  // Set angle based on elapsed time

  if (!sleeping)
    theta = elapsedTime * 0.3;

  // Wait a bit

#ifdef LINUX
  usleep(10000);
#endif

  glutPostRedisplay();
}


// Handle a key press


void keyPress( unsigned char key, int x, int y )

{
  switch (key) {
  case 27: exit(0);
  case 'p':
    sleeping = !sleeping;
    break;
  case 'd':
    renderer->incDebug();
    break;
  case 'F':
    factor += 0.01;
    cout << "factor = " << factor << endl;
    break;
  case 'f':
    factor -= 0.01;
    cout << "factor = " << factor << endl;
    break;
  }
}


void specialKeyPress( int key, int x, int y )

{
  switch (key) {
  case GLUT_KEY_UP:
    eyePosition = eyePosition / 1.1;
    break;
  case GLUT_KEY_DOWN:
    eyePosition = eyePosition * 1.1;
    break;
  case GLUT_KEY_LEFT:
    fovy *= 1.1;
    break;
  case GLUT_KEY_RIGHT:
    fovy /= 1.1;
    break;
  }

  glutPostRedisplay();
}


// Main program


int main( int argc, char **argv )

{
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " scene.obj" << endl;
    exit(1);
  }

  glutInit(&argc, argv);
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );

  int shift = (argc < 4 ? 0 : atoi(argv[3]) * 450);

  glutInitWindowSize( windowWidth, windowHeight );
  glutInitWindowPosition( 1700 + 50 + shift, 50 );
  glutCreateWindow( "toon shading" );

  GLenum status = glewInit();
  if (status != GLEW_OK) {
    std::cerr << "Error: " << glewGetErrorString(status) << std::endl;
    return 1;
  }

  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  glutIdleFunc( idle );
  glutKeyboardFunc( keyPress );
  glutSpecialFunc( specialKeyPress );

  // Set up world objects

  obj = new wfModel( argv[1] );

  isTorso = (strlen(argv[1]) >= 9 && strcmp( &argv[1][strlen(argv[1])-9] , "torso.obj" ) == 0);

  // Point camera to the model

  const float initEyeDistance = 5.0;

  eyePosition = (initEyeDistance * obj->radius) * vec3(0,0,1);
  fovy = 2 * atan2( 1, initEyeDistance );

  // Set up renderer

  renderer = new Renderer( windowWidth, windowHeight );

  // Go

  glutMainLoop();

  return 1;
}



