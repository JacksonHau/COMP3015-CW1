#version 460

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

layout (location = 0) out vec4 FragColor;

uniform vec3 uViewPos;
uniform vec3 uLightPos;
uniform vec3 uLightColor;

uniform float uAmbientStrength;
uniform float uSpecStrength;
uniform float uShininess;

uniform vec3 uBaseColor;

uniform int uUseTexture;
uniform sampler2D uTex;

// NEW toggles
uniform int uToon;
uniform int uFog;

// Fog params
uniform vec3  uFogColor;
uniform float uFogNear;
uniform float uFogFar;

float toonify(float x, float steps)
{
    return floor(x * steps) / steps;
}

void main()
{
    vec3 base = uBaseColor;
    if (uUseTexture == 1) {
        base = texture(uTex, vUV).rgb;
    }

    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightPos - vWorldPos);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);

    vec3 ambient = uAmbientStrength * base * uLightColor;

    float diff = max(dot(N, L), 0.0);
    float spec = 0.0;

    if (diff > 0.0)
        spec = pow(max(dot(N, H), 0.0), uShininess);

    // Toon mode
    if (uToon == 1) {
        diff = toonify(diff, 5.0);
        spec = (spec > 0.5) ? 1.0 : 0.0;
    }

    vec3 diffuse  = diff * base * uLightColor;
    vec3 specular = uSpecStrength * spec * uLightColor;

    vec3 color = ambient + diffuse + specular;

    // Fog
    if (uFog == 1) {
        float d = length(uViewPos - vWorldPos);
        float fogFactor = clamp((d - uFogNear) / (uFogFar - uFogNear), 0.0, 1.0);
        color = mix(color, uFogColor, fogFactor);
    }

    FragColor = vec4(color, 1.0);
}