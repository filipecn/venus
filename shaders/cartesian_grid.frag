#version 450

#extension GL_GOOGLE_include_directive : require

#include "venus/debug.glsl"
#include "venus/frag_debug.glsl"

layout(set = 0, binding = 0) uniform Data {   

	mat4 view;
	mat4 proj;
	vec3 eye;

} ubo;

layout( push_constant ) uniform constants
{
	mat4 inv_viewproj_matrix;
} ps;

layout(location = 0) noperspective in vec3 nearPoint;
layout(location = 1) noperspective in vec3 farPoint;

layout(location = 0) out vec4 outColor;

const float lineThickness = 1.0;
const float majorStep = 1.0;
const float minorStep = 0.1;

float drawGrid(vec2 uv, float stepSize, float thickness) {
    vec2 grid = abs(fract(uv / stepSize - 0.5) - 0.5) * stepSize;
    float d = min(grid.x, grid.y);
    float derivative = fwidth(d);
    return 1.0 - smoothstep(0.0, derivative * thickness, d);
}

// Check if we are near the X (fragPos.x) or Z (fragPos.y) axes
float axisLine(float coord, float pixelSize, float thickness) {
    return 1.0 - smoothstep(0.0, pixelSize * thickness, abs(coord));
}

vec3 blend_axis(in vec2 fragPos, in vec3 color, float thickness) {
    float pSize_x = fwidth(fragPos.x); // Size of a pixel in world space
    float pSize_y = fwidth(fragPos.y); // Size of a pixel in world space

    float xAxis = axisLine(fragPos.y, pSize_y, thickness); // Red line (X-axis)
    float zAxis = axisLine(fragPos.x, pSize_x, thickness); // Blue line (Z-axis)
    
    vec3 finalColor = color;
    if (xAxis > 0.5) finalColor = vec3(1.0, 0.2, 0.2); // Red
    if (zAxis > 0.5) finalColor = vec3(0.2, 0.2, 1.0); // Blue
    return finalColor;
}

void main() {

  float dy = farPoint.y - nearPoint.y;
    
    // 1. Prevent division by zero (parallel to plane) 
    // and ensure we only hit the plane in FRONT of the camera.
    // If nearPoint.y and dy have the same sign, we are looking AWAY from the plane.
    if (abs(dy) < 0.0001 || (nearPoint.y * dy >= 0.0)) {
        discard;
    }

    float t = -nearPoint.y / dy;

    // 2. Extra safety: discard intersections "behind" the near plane
    if (t > 0.0 && t < 1.0) {
        discard;
    }

    // 3. Reconstruct the world-space position on the plane
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
    vec2 fragPos = fragPos3D.xz; // Use XZ for the 2D grid

    // 4. Calculate depth for proper Z-Buffer behavior
    // This allows objects to clip through the floor correctly
    float depth = (farPoint.z - nearPoint.z) * t + nearPoint.z;
    vec4 clipPos = ubo.proj * ubo.view * vec4(fragPos3D, 1.0);
    gl_FragDepth = clipPos.z / clipPos.w;

    vec2 derivative = fwidth(fragPos);
    float pixelSize = length(derivative);
    float minorFade = 1.0 - smoothstep(minorStep * 0.5, minorStep * 2.0, pixelSize);


    // 5. Run your grid logic (from previous answer)
    float majorGrid = drawGrid(fragPos, 1.0, 3.0);
    float minorGrid = drawGrid(fragPos, 0.1, 2.0);

    // 4. Composition
    vec3 bgColor = nearPoint.y < 0.0 ? vec3(0.1) : vec3(0.2);
    vec3 minorColor = vec3(0.25);
    vec3 majorColor = vec3(0.5);

    // Apply the fade to the minor grid color before mixing
    vec3 color = mix(bgColor, minorColor, minorGrid * minorFade);
    
    // Major grid stays visible longer
    color = mix(color, majorColor, majorGrid);
    color = blend_axis(fragPos, color, 4.0);

    // 5. Global Alpha Fade (Optional: fades everything at the horizon)
    float distanceFade = exp(-length(fragPos) * 0.02);

    // ... coloring and fading logic ...
    outColor = vec4(color, 0.5);//distanceFade );
}