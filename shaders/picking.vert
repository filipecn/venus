#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    uint objectID;
    uint primitiveID;
    uint firstVertex;
    uint derivePrimitiveID;
} pc;

layout(location = 0) flat out uint outObjectID;
layout(location = 1) flat out uint outPrimitiveID;

void main() {
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    gl_PointSize = 1.0;
    outObjectID = pc.objectID;
    outPrimitiveID = pc.derivePrimitiveID != 0
        ? (uint(gl_VertexIndex) - pc.firstVertex) / 3
        : pc.primitiveID;
}
