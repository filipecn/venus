#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

#include "venus/debug.glsl"
#include "venus/vert_debug.glsl"

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer VertexBuffer { 
	float vertices[];
};

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer ScalarFieldBuffer { 
	float values[];
};

layout(set = 0, binding = 0) uniform Data {   
	VertexBuffer vb;
	ScalarFieldBuffer sf;
} ubo;

layout( push_constant ) uniform constants
{
	mat4 proj_view;
} ps;

void main() {

	vec3 v = vec3(
      ubo.vb.vertices[gl_VertexIndex * 3  + 0],
      ubo.vb.vertices[gl_VertexIndex * 3  + 1],
      ubo.vb.vertices[gl_VertexIndex * 3  + 2]
      );

	  gl_Position = ps.proj_view * vec4(v,1.0);

	  vert_print(ubo.sf.values[gl_VertexIndex]);
}


