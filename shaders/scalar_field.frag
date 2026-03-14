#version 450

#extension GL_GOOGLE_include_directive : require

#include "venus/debug.glsl"
#include "venus/frag_debug.glsl"

layout(location = 0) in vec3 color;

layout (location = 0) out vec4 outColor;

void main()
{
  outColor = vec4(color,1.0);
}
