#ifndef VENUS_DEBUG_GLSL
#define VENUS_DEBUG_GLSL

#extension GL_EXT_debug_printf : enable

void print(in float v) {
  debugPrintfEXT("%f\n", v);
}

void print(in vec3 v) {
  debugPrintfEXT("\n %f %f %f\n", v.x,v.y,v.z);
}

void print(in vec4 v) {
  debugPrintfEXT("\n %f %f %f %f\n", v.x,v.y,v.z,v.w);
}

void printMat(in mat4 m) {
  debugPrintfEXT("\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", 
      m[0][0], m[1][0], m[2][0], m[3][0],
      m[0][1], m[1][1], m[2][1], m[3][1],
      m[0][2], m[1][2], m[2][2], m[3][2],
      m[0][3], m[1][3], m[2][3], m[3][3]);
}

void print_frag(in float v) {
  if(gl_FragCoord.x == 100.5 && gl_FragCoord.y == 100.5) {
    print(v);
  }
}

void print_frag(in vec3 v) {
  if(gl_FragCoord.x == 100.5 && gl_FragCoord.y == 100.5) {
    print(v);
  }
}

#endif
