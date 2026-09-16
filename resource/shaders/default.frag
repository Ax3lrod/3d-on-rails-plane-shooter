#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 VertexColor;

out vec4 FragColor;

uniform vec3 uLightDir;         // Direction TO light
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uCameraPos;
uniform int uUseLighting;       // 1 for shaded, 0 for pure emissive/unlit (lasers, reticle, HUD)
uniform int uUseFog;            // 1 to apply distance fog, 0 to skip fog (HUD, lasers, UI)
uniform int uUseColorOverride;  // 1 to use uColorOverride, 0 to use VertexColor
uniform vec3 uColorOverride;    // Custom tint color
uniform float uAlpha;           // Transparency factor
uniform int uUseFlatShading;    // 1 for retro arcade faceted flat normals, 0 for smooth
uniform int uUseDithering;      // 1 for retro ordered Bayer dithering (Ex-Zodiac / Sega 32X)

// 4x4 Ordered Bayer Matrix
const float bayer4[16] = float[](
     0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
    12.0/16.0,  4.0/16.0, 14.0/16.0,  6.0/16.0,
     3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
    15.0/16.0,  7.0/16.0, 13.0/16.0,  5.0/16.0
);

float GetBayer4() {
    int x = int(mod(gl_FragCoord.x, 4.0));
    int y = int(mod(gl_FragCoord.y, 4.0));
    return bayer4[y * 4 + x];
}

void main() {
    // Authentic retro hardware alpha stippling / mesh transparency
    if (uUseDithering == 1 && uAlpha < 0.98) {
        float dither = GetBayer4();
        if (dither > uAlpha) {
            discard;
        }
    }

    vec3 baseColor = (uUseColorOverride == 1) ? uColorOverride : VertexColor;
    
    vec3 finalColor = baseColor;
    if (uUseLighting == 1) {
        // Diffuse directional light (supports authentic 90s flat-faceted polygons)
        vec3 norm = (uUseFlatShading == 1)
            ? normalize(cross(dFdx(FragPos), dFdy(FragPos)))
            : normalize(Normal);
        vec3 lightDir = normalize(uLightDir);
        float diff = max(dot(norm, lightDir), 0.0);
        
        vec3 ambient = uAmbientColor * baseColor;
        vec3 diffuse = diff * uLightColor * baseColor;
        finalColor = ambient + diffuse;
    }
    
    // Distance Fog (retro arcade / Ex-Zodiac dithered depth fade)
    if (uUseFog == 1) {
        float distanceToCam = length(uCameraPos - FragPos);
        float fogFactor = clamp((distanceToCam - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
        if (uUseDithering == 1) {
            float dither = (GetBayer4() - 0.5) * 0.22;
            fogFactor = clamp(fogFactor + dither, 0.0, 1.0);
            fogFactor = floor(fogFactor * 10.0 + 0.5) / 10.0;
        }
        finalColor = mix(finalColor, uFogColor, fogFactor);
    }
    
    FragColor = vec4(finalColor, uAlpha);
}
