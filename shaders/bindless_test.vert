#version 450

#extension GL_EXT_debug_printf : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform  SceneData {   
	
  mat4 view;
	mat4 projection;
	mat4 view_projection;
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;

} scene_data;

// scalar layout can be used here
// we use float, instead of vec3 because alignment must be power of 2
// otherwise we would need to fit our positions in vec4 structs
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer VertexBuffer { 
	float vertices[];
};

//push constants block
layout( push_constant ) uniform PushConstants
{
	mat4 render_matrix;
	VertexBuffer vb;
} pc;

void printVec(in vec3 v) {
  debugPrintfEXT("\n%d -> %f %f %f\n", gl_VertexIndex,v.x,v.y,v.z);
}

void printMat(in mat4 m) {
  debugPrintfEXT("\n%d\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", 
      gl_VertexIndex,
      m[0][0], m[1][0], m[2][0], m[3][0],
      m[0][1], m[1][1], m[2][1], m[3][1],
      m[0][2], m[1][2], m[2][2], m[3][2],
      m[0][3], m[1][3], m[2][3], m[3][3]);
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
  printMat(scene_data.projection);
  printMat(scene_data.view);
	gl_Position =  scene_data.projection * scene_data.view * pos;	
}
