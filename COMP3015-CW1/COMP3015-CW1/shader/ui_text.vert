#version 460
layout(location=0) in vec2 aPos;

uniform vec2 uScreen;
uniform float uScale;

void main() {
    vec2 ndc = vec2(
        (aPos.x / uScreen.x) * 2.0 - 1.0,
        1.0 - (aPos.y / uScreen.y) * 2.0
    );
    gl_Position = vec4(ndc, 0.0, 1.0);
}