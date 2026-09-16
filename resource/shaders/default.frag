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
uniform int uUseLighting;       // 1 for shaded, 0 for pure emissive/unlit (lasers, reticle)
uniform float uAlpha;           // Transparency factor

void main() {
    vec3 baseColor = VertexColor;
    
    vec3 finalColor = baseColor;
    if (uUseLighting == 1) {
        // Diffuse directional light
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(uLightDir);
        float diff = max(dot(norm, lightDir), 0.0);
        
        vec3 ambient = uAmbientColor * baseColor;
        vec3 diffuse = diff * uLightColor * baseColor;
        finalColor = ambient + diffuse;
    }
    
    // Distance Fog (retro arcade / N64 depth fade)
    float distanceToCam = length(uCameraPos - FragPos);
    float fogFactor = clamp((distanceToCam - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
    finalColor = mix(finalColor, uFogColor, fogFactor);
    
    FragColor = vec4(finalColor, uAlpha);
}
