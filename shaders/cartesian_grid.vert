#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "venus/debug.glsl"
#include "venus/vert_debug.glsl"

layout(set = 0, binding = 0) uniform Data {   

	mat4 view;
	mat4 proj;
	vec3 eye;

} ubo;

layout (location = 0) in vec3 pos;

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer { 
	Vertex vertices[];
};

layout( push_constant ) uniform constants
{
	mat4 inv_viewproj_matrix;
} ps;

// Grid plane is at Y = 0
vec3 unprojectPoint(float x, float y, float z, mat4 view, mat4 proj) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(proj);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

layout(location = 0) out vec3 near_point;
layout(location = 1) out vec3 far_point;

void main() {

	// 1. Create a full-screen triangle using the vertex index
    // Vertices: (-1,-1), (3,-1), (-1,3)
    vec2 gridUV[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

	vec2 p = gridUV[gl_VertexIndex];

	// 2. Project points onto the near and far planes
    // This creates a "ray" for every pixel on the screen
    near_point = unprojectPoint(p.x, p.y, -1.0, ubo.view, ubo.proj).xyz;
    far_point = unprojectPoint(p.x, p.y, 1.0, ubo.view, ubo.proj).xyz;

	gl_Position = vec4(p, 0.0, 1.0);
}

