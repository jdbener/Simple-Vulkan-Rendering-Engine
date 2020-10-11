#include "engine/util/timer.h"

#include "engine/vulkan/common.hpp"
#include "engine/window.hpp"

#include "engine/vulkan/shader.hpp"

#include <fstream>

std::ostream& operator <<(std::ostream& s, std::pair<int, int> size){
    s << "(" << size.first << ", " << size.second << ")";
    return s;
}

int main(){
    vk::ApplicationInfo appInfo("Project Delta", GAME_VERSION, "Delta Engine", ENGINE_VERSION, VK_API_VERSION_1_1);

    std::vector<const char*> layers = validateInstanceLayers({"VK_LAYER_KHRONOS_validation"});
    std::vector<const char*> extensions = validateInstanceExtensions(Window::requiredVulkanExtensions()
        + std::vector<const char*>{VK_EXT_DEBUG_UTILS_EXTENSION_NAME});

    vpp::Instance instance({{}, &appInfo,
        /*Layer Count*/ (uint32_t) layers.size(),
        /*Layer Names*/ layers.data(),
        /*Extension Count*/ (uint32_t) extensions.size(),
        /*Extension Names*/ extensions.data()});
    vpp::DebugMessenger debugMsg(instance);

    Window w(instance, 800, 600, str(27));
    Window w2(instance, 800, 600);

    // TODO: Image View API on Window (merged with framebuffer? renderpass?) RenderData? RenderState?

    std::ifstream source("test.vert.glsl");
    GLSLShaderModule module(w.device(), source, vk::ShaderStageBits::vertex);

    w.createGraphicsRenderPass({vk::ImageLayout::colorAttachmentOptimal});
    w.recreateRenderBuffers();

    str s = u8"This is a string which I am writing. μ";
    w.setName(s);
    w.setSize(400, 400);

    do {
        w.swapBuffers();
        //Window::waitEvents();
        Window::pollEvents();
    // Wait for main window to be closed
    } while(!w.isClosed());
}
