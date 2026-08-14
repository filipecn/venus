#version 450

layout(location = 0) flat in uint inID;
layout(location = 0) out uvec4 outPickingColor;

void main() {
    outPickingColor = uvec4(inID, 0, 0, 0);
}