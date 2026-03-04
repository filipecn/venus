#version 450


layout(location = 0) noperspective in vec3 nearPoint;
layout(location = 1) noperspective in vec3 farPoint;

layout(location = 0) out vec4 outColor;

// --- Color Palettes ---
const vec3 skyDay     = vec3(0.2, 0.5, 1.0);
const vec3 skySunset  = vec3(0.4, 0.1, 0.1);
const vec3 skyNight   = vec3(0.02, 0.02, 0.05);

const vec3 horizDay    = vec3(0.7, 0.8, 1.0);
const vec3 horizSunset = vec3(1.0, 0.4, 0.2);
const vec3 horizNight  = vec3(0.05, 0.05, 0.1);


void main() {
    vec3 viewDir = normalize(farPoint - nearPoint);
vec3 sunDir = vec3(0.5,0.3,0.0);
    float sunHeight = sunDir.y; // Range [-1.0, 1.0]

    // 1. Determine the "Time of Day" factors
    // Day factor: 1.0 at noon, 0.0 at sunset
    float dayFactor = clamp(sunHeight * 5.0, 0.0, 1.0); 
    // Sunset factor: 1.0 at horizon, 0.0 elsewhere
    float sunsetFactor = clamp(1.0 - abs(sunHeight * 5.0), 0.0, 1.0);
    // Night factor: 1.0 at midnight
    float nightFactor = clamp(-sunHeight * 5.0, 0.0, 1.0);

    // 2. Interpolate Sky & Horizon colors based on time
    vec3 currentSky = (skyDay * dayFactor) + (skySunset * sunsetFactor) + (skyNight * nightFactor);
    vec3 currentHoriz = (horizDay * dayFactor) + (horizSunset * sunsetFactor) + (horizNight * nightFactor);

    // 3. Draw the Sky Gradient
    vec3 finalSky;
    if (viewDir.y > 0.0) {
        finalSky = mix(currentHoriz, currentSky, pow(viewDir.y, 0.5));
    } else {
        finalSky = mix(currentHoriz, skyNight * 0.5, pow(-viewDir.y, 0.5));
    }

    // 4. Draw the Sun & Moon
    float sunTarget = dot(viewDir, sunDir);
    float sunGlow = pow(max(0.0, sunTarget), 64.0);
    finalSky += vec3(1.0, 0.9, 0.5) * sunGlow * dayFactor; // Sun only visible in day/sunset

    float moonTarget = dot(viewDir, -sunDir); // Moon is opposite to Sun
    float moonGlow = pow(max(0.0, moonTarget), 128.0);
    finalSky += vec3(0.6, 0.7, 1.0) * moonGlow * nightFactor; // Moon only at night
   
        outColor = vec4(finalSky, 1.0);
}