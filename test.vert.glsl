#version 450
#extension GL_ARB_separate_shader_objects : enable

// layout(location = 0) in vec2 position;
// layout(location = 1) in vec3 color;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 color;

layout(location = 0) out vec3 outColor;


void main() {
    gl_Position = vec4(position.xy, 0.0, 1.0);
    outColor = color;
}
