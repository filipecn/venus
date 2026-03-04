#ifndef VENUS_DEBUG_VERT
#define VENUS_DEBUG_VERT

#extension GL_EXT_debug_printf : enable

void printVec(in vec3 v) {
  debugPrintfEXT("\n%d -> %f %f %f\n", gl_VertexIndex,v.x,v.y,v.z);
}

void printVec(in vec4 v) {
  debugPrintfEXT("\n%d -> %f %f %f %f\n", gl_VertexIndex,v.x,v.y,v.z, v.w);
}

#endif