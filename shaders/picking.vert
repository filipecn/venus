#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    uint elementType; // 0: Face, 1: Edge, 2: Vertex
} pc;

layout(location = 0) flat out uint outID;

void main() {
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    
    if (pc.elementType == 2) {
        outID = gl_VertexIndex;
        gl_PointSize = 10.0; // Larger hit area for vertices
    } else if (pc.elementType == 1) {
        outID = gl_VertexIndex / 2;
    } else {
        outID = gl_VertexIndex / 3;
    }
}