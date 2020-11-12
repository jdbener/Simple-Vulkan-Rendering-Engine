#version 450
// #extension GL_KHR_vulkan_glsl : enable
// #extension GL_EXT_debug_printf : enable

// Uniforms
layout(binding = 0) uniform UBO {
    float size;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// Vertex Buffer
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 color;
// Instance Buffer
layout(location = 5) in mat4 instanceModel; // Uses locations 5-8

// Outputs
layout(location = 0) out vec3 outColor;



void main() {
    gl_Position = vec4(clamp(ubo.size, .01, 2) * position.xy, 0.0, 1.0) * instanceModel;
    outColor = color;

    // debugPrintfEXT("test");
    //https://www.reddit.com/r/vulkan/comments/gcxp0a/vulkan_shader_debugger/
    //https://vulkan.lunarg.com/issue/home?limit=10;q=;mine=false;org=false;khronos=false;lunarg=false;indie=false;status=new,open
}
