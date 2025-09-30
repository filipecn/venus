#version 450

#extension GL_EXT_debug_printf : enable
//#extension GL_EXT_nonuniform_qualifier : require

layout (set=0, binding = 0) uniform Buffer
{
  mat4 mvp;
} uniformBuffer;

layout (location = 0) in vec3 pos;
// layout (location = 1) in vec4 inColor;

//layout (location = 0) out vec4 outColor;
void main()
{

  if(gl_VertexIndex == 0)
    gl_Position = vec4(-1.0,-1.0,0.0,1.0);
  if(gl_VertexIndex == 2)
    gl_Position = vec4(1.0,1.0,0.0,1.0);
  if(gl_VertexIndex == 1)
    gl_Position = vec4(1.0,-1.0,0.0,1.0);
float myfloat = 3.f;
  debugPrintfEXT("My float is %f", myfloat);
  
  //gl_Position = uniformBuffer.mvp * vec4(pos,1.0);
  gl_Position = vec4(pos,1.0);
}

