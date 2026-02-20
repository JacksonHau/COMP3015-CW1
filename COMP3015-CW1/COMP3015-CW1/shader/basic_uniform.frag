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

// Texture controls (for ground)
uniform int uUseTexture;      // 0 or 1
uniform sampler2D uTex;       // bound to texture unit 0

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
    vec3 diffuse = diff * base * uLightColor;

    float spec = pow(max(dot(N, H), 0.0), uShininess);
    vec3 specular = uSpecStrength * spec * uLightColor;

    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);
}