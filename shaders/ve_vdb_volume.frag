#version 450

#extension GL_EXT_debug_printf : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_GOOGLE_include_directive : require

layout(buffer_reference, scalar) readonly buffer 
PNanoVDBBuffer { 

  // raw nanovdb data
	uint data[];

};

layout(set = 1, binding = 0) uniform 
MaterialData {   

  PNanoVDBBuffer vdb;
  vec3 eye;

} material_data;

#define pnanovdb_buf_data material_data.vdb.data
#define PNANOVDB_GLSL
#include "PNanoVDB.h"

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 frag_wp;


void printVec(in vec3 v) {
  debugPrintfEXT("\n %f %f %f\n", v.x,v.y,v.z);
}

void main()
{
  vec3 ray = normalize(frag_wp - material_data.eye);
  outColor = vec4(ray,1.0);
}

