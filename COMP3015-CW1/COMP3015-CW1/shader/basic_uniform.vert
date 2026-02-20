#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexUV;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main()
{
    vec4 world = uModel * vec4(VertexPosition, 1.0);
    vWorldPos = world.xyz;

    // fine as long as you don't scale weirdly
    vNormal = mat3(uModel) * VertexNormal;
    vUV = VertexUV;

    gl_Position = uProj * uView * world;
}
