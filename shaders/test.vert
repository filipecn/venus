#version 450

#extension GL_EXT_debug_printf : enable
//#extension GL_EXT_nonuniform_qualifier : require

layout (set=0, binding = 0) uniform Buffer
{
  mat4 projection;
  mat4 view;
} ub;

layout (location = 0) in vec3 pos;
// layout (location = 1) in vec4 inColor;

void printVec(in vec3 v) {
  debugPrintfEXT("\n%d -> %f %f %f\n", gl_VertexIndex,v.x,v.y,v.z);
}

void printVec(in vec4 v) {
  debugPrintfEXT("\n%d -> %f %f %f %f\n", gl_VertexIndex,v.x,v.y,v.z, v.w);
}

void printMat(in mat4 m) {
  debugPrintfEXT("\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", 
      m[0][0], m[1][0], m[2][0], m[3][0],
      m[0][1], m[1][1], m[2][1], m[3][1],
      m[0][2], m[1][2], m[2][2], m[3][2],
      m[0][3], m[1][3], m[2][3], m[3][3]);
}

//layout (location = 0) out vec4 outColor;
void main()
{

  if(gl_VertexIndex == 0)
    gl_Position = vec4(0, 0, 11.f / 6.f,1);
  if(gl_VertexIndex == 2)
    gl_Position = vec4(1, 0, 11.f / 6.f,1);
  if(gl_VertexIndex == 1)
    gl_Position = vec4(1, 1, 11.f / 6.f,1);

  //gl_Position = vec4(pos,1.0);
  gl_Position = ub.projection * ub.view * vec4(pos,1.0);
  //gl_Position = ub.projection * gl_Position;
  //gl_Position = uniformBuffer.projection * gl_Position;

  //mat4 M = ub.projection * ub.model_view;
  //gl_Position = M * vec4(pos,1.0);
  if(gl_VertexIndex == 1) {
    printMat(ub.projection);
    printMat(ub.view);
    
    //printVec(ub.view * vec4(pos,1.0));
    //printVec(gl_Position);
  }

    printVec(gl_Position);
}

