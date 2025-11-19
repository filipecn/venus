#ifndef VENUS_SAMPLING_GLSL
#define VENUS_SAMPLING_GLSL

#include "math.glsl"

/*
 * Random generation taken from nvpro-samples/vk_raytracing_tutorial_KHR jitter camera
 * https://github.com/nvpro-samples/vk_raytracing_tutorial_KHR
 */

// Generate a random unsigned int from two unsigned int values, using 16 pairs
// of rounds of the Tiny Encryption Algorithm. See Zafar, Olano, and Curtis,
// "GPU Random Numbers via the Tiny Encryption Algorithm"
uint tea(uint val0, uint val1) {
    uint v0 = val0;
    uint v1 = val1;
    uint s0 = 0;

    for(uint n = 0; n < 16; n++) {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }

    return v0;
}

// Generate a random unsigned int in [0, 2^24) given the previous RNG state
// using the Numerical Recipes linear congruential generator
uint lcg(inout uint prev) {
    uint LCG_A = 1664525u;
    uint LCG_C = 1013904223u;
    prev       = (LCG_A * prev + LCG_C);
    return prev & 0x00FFFFFF;
}

// Generate a random float in [0, 1) given the previous RNG state
float rand(inout uint prev) {
    return (float(lcg(prev)) / float(0x01000000));
}

vec2 rand2(inout uint prev) {
    return vec2(rand(prev), rand(prev));
}

vec3 rand3(inout uint prev) {
    return vec3(rand(prev), rand(prev), rand(prev));
}

vec3 sampleUniformSphere(inout uint seed) {
  vec2 r = rand2(seed);
  float z = 1.0 - 2.0 * r[0];
  float radius = sqrt(1.0 - z*z);
  float phi = VENUS_2PI * r[1];
  return vec3(radius * cos(phi), radius * sin(phi), z);
}

#endif
