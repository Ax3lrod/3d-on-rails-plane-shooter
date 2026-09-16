#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uScreenTexture;
uniform float uTime;
uniform int uCRTEnabled;
uniform float uWarpIntensity;
uniform vec2 uResolution;

// Barrel distortion / CRT monitor lens curvature
vec2 CurveUV(vec2 uv) {
    vec2 centered = uv * 2.0 - 1.0;
    vec2 offset = centered.yx / 4.2;
    centered = centered + centered * offset * offset;
    return centered * 0.5 + 0.5;
}

// Pseudo-random hash for warp star generation
float Hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

void main() {
    vec2 uv = TexCoords;
    float borderMask = 1.0;

    if (uCRTEnabled == 1) {
        vec2 curvedUV = CurveUV(uv);
        if (curvedUV.x < 0.0 || curvedUV.x > 1.0 || curvedUV.y < 0.0 || curvedUV.y > 1.0) {
            // Outside CRT curved bezel
            FragColor = vec4(0.02, 0.02, 0.03, 1.0);
            return;
        }
        uv = curvedUV;
        // Edge vignette fade
        vec2 edgeDist = uv * (1.0 - uv);
        borderMask = clamp(edgeDist.x * edgeDist.y * 35.0, 0.0, 1.0);
    }

    vec3 sceneColor = vec3(0.0);

    // 1. Hyperspace Warp Tunnel Effect
    if (uWarpIntensity > 0.01) {
        vec2 center = vec2(0.5, 0.5);
        vec2 dir = uv - center;
        float dist = length(dir);
        float angle = atan(dir.y, dir.x);

        // Radial zoom streaks
        vec3 warpAcc = vec3(0.0);
        float totalWeight = 0.0;
        int samples = 10;
        float blurScale = 0.08 * uWarpIntensity;

        for (int i = 0; i < samples; ++i) {
            float t = float(i) / float(samples - 1);
            vec2 sampleUV = center + dir * (1.0 - t * blurScale);
            float w = 1.0 - t * 0.5;
            warpAcc += texture(uScreenTexture, sampleUV).rgb * w;
            totalWeight += w;
        }
        sceneColor = warpAcc / totalWeight;

        // Procedural hyperspace star lines
        float starAngle = angle * 8.0 + uTime * 2.0;
        float streak = sin(starAngle) * cos(angle * 12.0 - uTime * 3.0);
        streak = pow(max(streak, 0.0), 6.0) * dist * 3.5;
        
        // Speed ring pulses
        float ring = sin(dist * 40.0 - uTime * 15.0);
        ring = pow(max(ring, 0.0), 4.0) * 0.4;

        vec3 warpStreakCol = mix(vec3(0.3, 0.8, 1.0), vec3(0.8, 0.4, 1.0), sin(uTime * 4.0) * 0.5 + 0.5);
        sceneColor += (streak + ring) * warpStreakCol * uWarpIntensity;
    } else {
        // Normal base texture sample with subtle chromatic aberration
        if (uCRTEnabled == 1) {
            float caStrength = 0.0022;
            float r = texture(uScreenTexture, uv + vec2(caStrength, 0.0)).r;
            float g = texture(uScreenTexture, uv).g;
            float b = texture(uScreenTexture, uv - vec2(caStrength, 0.0)).b;
            sceneColor = vec3(r, g, b);
        } else {
            sceneColor = texture(uScreenTexture, uv).rgb;
        }
    }

    // 2. Fast Multi-Tap Bloom Glow Pass
    // Sample 8 surrounding points to collect bright glow from lasers, thrusters, and explosions
    vec2 texelSize = 1.0 / uResolution;
    vec3 bloomAcc = vec3(0.0);
    vec2 offsets[8] = vec2[](
        vec2(-2.0, -2.0), vec2(0.0, -2.5), vec2(2.0, -2.0),
        vec2(-2.5,  0.0),                  vec2(2.5,  0.0),
        vec2(-2.0,  2.0), vec2(0.0,  2.5), vec2(2.0,  2.0)
    );

    for (int i = 0; i < 8; ++i) {
        vec3 tap = texture(uScreenTexture, uv + offsets[i] * texelSize * 1.5).rgb;
        float brightness = dot(tap, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > 0.65) {
            bloomAcc += (tap - vec3(0.5)) * 0.5;
        }
    }
    sceneColor += max(bloomAcc, vec3(0.0)) * 0.35;

    // 3. Retro CRT Post-Processing Pass (Scanlines, Phosphor Mask, Vignette)
    if (uCRTEnabled == 1) {
        // Horizontal Scanlines
        float scanline = sin(uv.y * uResolution.y * 3.14159265) * 0.10;
        sceneColor -= scanline;

        // RGB Phosphor Sub-pixel Mask (vertical tri-color stripes)
        float pixelX = uv.x * uResolution.x;
        int subPixel = int(mod(pixelX, 3.0));
        vec3 mask = vec3(0.85);
        if (subPixel == 0) mask = vec3(1.08, 0.88, 0.88);
        else if (subPixel == 1) mask = vec3(0.88, 1.08, 0.88);
        else mask = vec3(0.88, 0.88, 1.08);

        sceneColor *= mask;
        sceneColor *= borderMask;

        // Gentle arcade phosphor glow boost
        sceneColor *= 1.08;
    }

    FragColor = vec4(sceneColor, 1.0);
}
