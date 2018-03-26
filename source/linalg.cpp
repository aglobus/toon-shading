// linalg.cpp


#include "linalg.h"


// ---------------- vec2 ----------------


vec2 operator * ( float k, vec2 const& p )

{
  vec2 q;

  q.x = p.x * k;
  q.y = p.y * k;

  return q;
}

vec2 operator * ( vec2 const& p, float k )

{
  vec2 q;

  q.x = p.x * k;
  q.y = p.y * k;

  return q;
}

vec2 operator / ( vec2 const& p, float k )

{
  vec2 q;

  q.x = p.x / k;
  q.y = p.y / k;

  return q;
}


// I/O operators

std::ostream& operator << ( std::ostream& stream, vec2 const& p )

{
  stream << p.x << " " << p.y;
  return stream;
}

std::istream& operator >> ( std::istream& stream, vec2 & p )

{
  stream >> p.x >> p.y;
  return stream;
}


// ---------------- vec3 ----------------


vec3 operator * ( float k, vec3 const& p )

{
  vec3 q;

  q.x = p.x * k;
  q.y = p.y * k;
  q.z = p.z * k;

  return q;
}

vec3 operator * ( vec3 const& p, float k )

{
  vec3 q;

  q.x = p.x * k;
  q.y = p.y * k;
  q.z = p.z * k;

  return q;
}

vec3 operator / ( vec3 const& p, float k )

{
  vec3 q;

  q.x = p.x / k;
  q.y = p.y / k;
  q.z = p.z / k;

  return q;
}


vec3 vec3::perp1()

{
  vec3 result(0,0,0);

  if (x == 0)
    if (y == 0 || z == 0)
      result.x = 1;     /* v = (0 0 z) or (0 y 0) */
    else {
      result.y = -z; /* v = (0 y z) */
      result.z = y;
    }
  else if (y == 0)
    if (z == 0)
      result.z = 1;     /* v = (x 0 0) */
    else {
      result.x = -z; /* v = (x 0 z) */
      result.z = x;
    }
  else {
    result.x = -y;   /* v = (x y z) or (x y 0) */
    result.y = x;
  }

  float lenRecip = 1.0 / sqrt( result.x*result.x + result.y*result.y + result.z*result.z );
  result.x *= lenRecip;
  result.y *= lenRecip;
  result.z *= lenRecip;

  return result;
}


vec3 vec3::perp2()

{
  vec3 result(0,0,0);

  if (x == 0)
    if (y == 0)
      result.y = 1;
    else if (z == 0)
      result.z = 1;
    else
      result.x = 1;
  else
    if (y == 0)
      result.y = 1;
    else if (z == 0)
      result.z = 1;
    else {
      result.x = x * z;
      result.y = y * z;
      result.z = - x*x - y*y;

      float lenRecip = 1.0 / sqrt( result.x*result.x + result.y*result.y + result.z*result.z );
      result.x *= lenRecip;
      result.y *= lenRecip;
      result.z *= lenRecip;
    }

  return result;
}


// I/O operators

std::ostream& operator << ( std::ostream& stream, vec3 const& p )

{
  stream << p.x << " " << p.y << " " << p.z;
  return stream;
}

std::istream& operator >> ( std::istream& stream, vec3 & p )

{
  stream >> p.x >> p.y >> p.z;
  return stream;
}


// ---------------- vec4 ----------------


vec4 operator * ( float k, vec4 const& p )

{
  vec4 q;

  q.x = p.x * k;
  q.y = p.y * k;
  q.z = p.z * k;
  q.w = p.w * k;

  return q;
}

vec4 operator * ( vec4 const& p, float k )

{
  vec4 q;

  q.x = p.x * k;
  q.y = p.y * k;
  q.z = p.z * k;
  q.w = p.w * k;

  return q;
}

vec4 operator / ( vec4 const& p, float k )

{
  vec4 q;

  q.x = p.x / k;
  q.y = p.y / k;
  q.z = p.z / k;
  q.w = p.w / k;

  return q;
}


// I/O operators

std::ostream& operator << ( std::ostream& stream, vec4 const& p )

{
  stream << p.x << " " << p.y << " " << p.z << " " << p.w;
  return stream;
}

std::istream& operator >> ( std::istream& stream, vec4 & p )

{
  stream >> p.x >> p.y >> p.z >> p.w;
  return stream;
}


// ---------------- mat4 ----------------


mat4 operator * ( float k, mat4 const& m )

{
  mat4 out;

  out.rows[0] = k * m.rows[0];
  out.rows[1] = k * m.rows[1];
  out.rows[2] = k * m.rows[2];
  out.rows[3] = k * m.rows[3];

  return out;
}

vec4 operator * ( mat4 const& m, vec4 const& v )

{
  vec4 out;

  out[0] = m.rows[0] * v;
  out[1] = m.rows[1] * v;
  out[2] = m.rows[2] * v;
  out[3] = m.rows[3] * v;

  return out;
}

mat4 operator * ( mat4 const& m, mat4 const& n )

{
  mat4 out;

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) {

      float sum=0;

      for (int k=0; k<4; k++)
	sum += m[i][k] * n[k][j];

      out[i][j] = sum;
    }

  return out;
}

mat4 identity()

{
  mat4 out;

  out.rows[0] = vec4( 1, 0, 0, 0 );
  out.rows[1] = vec4( 0, 1, 0, 0 );
  out.rows[2] = vec4( 0, 0, 1, 0 );
  out.rows[3] = vec4( 0, 0, 0, 1 );

  return out;
}


mat4 scale( float x, float y, float z )

{
  mat4 out;

  out.rows[0] = vec4( x, 0, 0, 0 );
  out.rows[1] = vec4( 0, y, 0, 0 );
  out.rows[2] = vec4( 0, 0, z, 0 );
  out.rows[3] = vec4( 0, 0, 0, 1 );

  return out;
}

mat4 translate( float x, float y, float z )

{
  mat4 out;

  out.rows[0] = vec4( 1, 0, 0, x );
  out.rows[1] = vec4( 0, 1, 0, y );
  out.rows[2] = vec4( 0, 0, 1, z );
  out.rows[3] = vec4( 0, 0, 0, 1 );

  return out;
}


mat4 translate( vec3 v )

{
  mat4 out;

  out.rows[0] = vec4( 1, 0, 0, v.x );
  out.rows[1] = vec4( 0, 1, 0, v.y );
  out.rows[2] = vec4( 0, 0, 1, v.z );
  out.rows[3] = vec4( 0, 0, 0,   1 );

  return out;
}


mat4 rotate( float theta, vec3 axis )

{
  axis = axis.normalize();

  float v1 = axis.x;
  float v2 = axis.y;
  float v3 = axis.z;
  
  float t1 =  cos(theta);
  float t2 =  1 - t1;
  float t3 =  v1*v1;
  float t6 =  t2*v1;
  float t7 =  t6*v2;
  float t8 =  sin(theta);
  float t9 =  t8*v3;
  float t11 = t6*v3;
  float t12 = t8*v2;
  float t15 = v2*v2;
  float t19 = t2*v2*v3;
  float t20 = t8*v1;
  float t24 = v3*v3;

  mat4 out;

  out.rows[0] = vec4( t1 + t2*t3,     t7 - t9,   t11 + t12, 0 );
  out.rows[1] = vec4(    t7 + t9, t1 + t2*t15,   t19 - t20, 0 );
  out.rows[2] = vec4(  t11 - t12,   t19 + t20, t1 + t2*t24, 0 );
  out.rows[3] = vec4(          0,           0,           0, 1 );

  return out;
}


mat4 frustum( float l, float r, float b, float t, float n, float f )

{
  mat4 out;

  out.rows[0] = vec4( 2*n/(r-l),         0, (r+l)/(r-l),           0 );
  out.rows[1] = vec4(         0, 2*n/(t-b), (t+b)/(t-b),           0 );
  out.rows[2] = vec4(         0,         0, (f+n)/(n-f), 2*f*n/(n-f) );
  out.rows[3] = vec4(         0,         0,          -1,           0 );

  return out;
}


mat4 perspective( float fovy, float aspect, float n, float f )

{
  mat4 out;

  float s = 1 / tan( fovy / 2.0 );

  out.rows[0] = vec4( s/aspect,          0,           0,           0 );
  out.rows[1] = vec4(         0,         s,           0,           0 );
  out.rows[2] = vec4(         0,         0, (f+n)/(n-f), 2*f*n/(n-f) );
  out.rows[3] = vec4(         0,         0,          -1,           0 );

  return out;
}
    

mat4 ortho( float l, float r, float b, float t, float n, float f )

{
  mat4 out;

  out.rows[0] = vec4( 2/(r-l),       0,       0, (l+r)/(l-r) );
  out.rows[1] = vec4(       0, 2/(t-b),       0, (b+t)/(b-t) );
  out.rows[2] = vec4(       0,       0, 2/(n-f), (n+f)/(n-f) );
  out.rows[3] = vec4(       0,       0,       0,           1 );

  return out;
}
    


// I/O operators

std::ostream& operator << ( std::ostream& stream, mat4 const& m )

{
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++)
      stream << (j>0 ? " " : "") << m[i][j];
    stream << std::endl;
  }

  return stream;
}

std::istream& operator >> ( std::istream& stream, mat4 & m )

{
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++)
      stream >> m[i][j];

  return stream;
}
