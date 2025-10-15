#version 450

#extension GL_EXT_debug_printf : enable
#extension GL_EXT_buffer_reference : require

layout (set=0, binding = 0) uniform Buffer
{
  mat4 projection;
  mat4 view;
} ub;

// scalar layout can be used here
// we use float, instead of vec3 because alignment must be power of 2
// otherwise we would need to fit our positions in vec4 structs
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer VertexBuffer { 
	float vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{
	mat4 render_matrix;
	VertexBuffer vb;
} pc;

void printVec(in vec3 v) {
  debugPrintfEXT("\n%d -> %f %f %f\n", gl_VertexIndex,v.x,v.y,v.z);
}

void main() 
{
	vec3 v = vec3(
      pc.vb.vertices[gl_VertexIndex * 3  + 0],
      pc.vb.vertices[gl_VertexIndex * 3  + 1],
      pc.vb.vertices[gl_VertexIndex * 3  + 2]
      );
	vec4 pos = vec4(v, 1.0f);

  printVec(v);
	gl_Position =  ub.projection * ub.view * pos;	
}
