#version 450

#extension GL_EXT_debug_printf : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform  
SceneData {   
	
  mat4 view;
	mat4 projection;
	mat4 view_projection;
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;

} scene_data;


layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer 
VertexBuffer { 

	float vertices[];

};

//push constants block
layout( push_constant ) uniform 
PushConstants {

	mat4 render_matrix;
	VertexBuffer vb;

} pc;

void printVec(in vec3 v) {
  debugPrintfEXT("\n%d -> %f %f %f\n", gl_VertexIndex,v.x,v.y,v.z);
}

layout (location = 0) out vec3 frag_wp;

void main() {
	vec3 v = vec3(
      pc.vb.vertices[gl_VertexIndex * 3  + 0],
      pc.vb.vertices[gl_VertexIndex * 3  + 1],
      pc.vb.vertices[gl_VertexIndex * 3  + 2]
      );
	vec4 pos = vec4(v, 1.0f);

	gl_Position =  scene_data.projection * scene_data.view * pos;	
  frag_wp = v;
}

