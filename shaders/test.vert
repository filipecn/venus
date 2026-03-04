#version 450

// #extension GL_EXT_debug_printf : enable
// #extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "venus/debug.glsl"
#include "venus/vert_debug.glsl"

layout(set = 0, binding = 0) uniform  SceneData{   

	mat4 view;
	mat4 proj;
	vec3 eye;

} scene_data;

layout (location = 0) in vec3 pos;
// layout (location = 1) in vec4 inColor;



//layout (location = 0) out vec4 outColor;
void main()
{

  if(gl_VertexIndex == 0)
    gl_Position = vec4(0, 0, 0,1);//11.f / 6.f,1);
  if(gl_VertexIndex == 2)
    gl_Position = vec4(1, 0, 0,1);//11.f / 6.f,1);
  if(gl_VertexIndex == 1)
    gl_Position = vec4(1, 1, 0,1);//11.f / 6.f,1);

  gl_Position = scene_data.proj * scene_data.view * vec4(pos,1.0);
  //gl_Position = vec4(pos,1.0);
  //gl_Position = scene_data.proj * gl_Position;
  // gl_Position = uniformBuffer.projection * gl_Position;

  // mat4 M = ub.projection * ub.model_view;
  // gl_Position = M * vec4(pos,1.0);
  if(gl_VertexIndex == 1) {
    //printMat(scene_data.proj);
    
    // printVec(ub.view * vec4(pos,1.0));
    // printVec(gl_Position);
  }

    //printVec(gl_Position);
}

