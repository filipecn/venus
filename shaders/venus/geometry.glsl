#ifndef VENUS_GEOMETRY_GLSL
#define VENUS_GEOMETRY_GLSL

struct Ray {
  vec3 o; // origin
  vec3 d; // direction
  float t_max; // max ray travel end
  float t_min; // min ray travel start
};

#endif
