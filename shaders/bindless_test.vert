#version 450

#extension GL_EXT_buffer_reference : require

layout (set=0, binding = 0) uniform Buffer
{
  mat4 projection;
  mat4 view;
} ub;

struct Vertex {
	vec3 position;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer { 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{
	VertexBuffer vb;
} pc;

void main() 
{
	Vertex v = pc.vb.vertices[gl_VertexIndex];
	vec4 pos = vec4(v.position, 1.0f);

	gl_Position =  ub.projection * ub.view * pos;	
}
