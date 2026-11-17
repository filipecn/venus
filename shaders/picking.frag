#version 450

layout(location = 0) flat in uint inObjectID;
layout(location = 1) flat in uint inPrimitiveID;
layout(location = 0) out uvec4 outPickingColor;

void main() {
    outPickingColor = uvec4(inObjectID, inPrimitiveID, 0, 0);
}
