#ifndef VENUS_DEBUG_FRAG
#define VENUS_DEBUG_FRAG

#extension GL_EXT_debug_printf : enable

int print_frag(in float v, in float x, in float y) {
  if(gl_FragCoord.x == x && gl_FragCoord.y == y) {
    print(v);
    return 1;
  }
  return 0;
}

int print_frag(in vec3 v, in float x, in float y) {
  if(gl_FragCoord.x == x && gl_FragCoord.y == y) {
    print(v);
    return 1;
  }
  return 0;
}

#endif