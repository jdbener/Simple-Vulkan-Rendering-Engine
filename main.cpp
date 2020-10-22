#include "engine/util/timer.h"

#include "engine/vulkan/common.hpp"
#include "engine/window.hpp"

#include "engine/vulkan/shader.hpp"

#include <fstream>

#include "vertex.h"

class debugShaderModule: public SPIRVShaderModule{
public:
    using SPIRVShaderModule::SPIRVShaderModule;

    auto getBytes(){ return bytes; }
};

std::ostream& operator <<(std::ostream& s, std::pair<int, int> size){
    s << "(" << size.first << ", " << size.second << ")";
    return s;
}

int main(){
    vk::ApplicationInfo appInfo("Project Delta", GAME_VERSION, "Delta Engine", ENGINE_VERSION, VK_API_VERSION_1_1);

    //std::vector<const char*> layers = validateInstanceLayers({"VK_LAYER_KHRONOS_validation"});
    std::vector<const char*> layers = validateInstanceLayers({});
    std::vector<const char*> extensions = validateInstanceExtensions(Window::requiredVulkanExtensions()
        + std::vector<const char*>{VK_EXT_DEBUG_UTILS_EXTENSION_NAME});

    vpp::Instance instance({{}, &appInfo,
        /*Layer Count*/ (uint32_t) layers.size(),
        /*Layer Names*/ layers.data(),
        /*Extension Count*/ (uint32_t) extensions.size(),
        /*Extension Names*/ extensions.data()});
    // TEMP: Remove in release builds
    vpp::DebugMessenger debugMsg(instance);

    Window w(instance, 800, 600), w2(instance, 800, 600);
    w.setName(str(w.getName()) + " (" + str(w.id()) + ")");
    w2.setName(str(w2.getName()) + " (" + str(w2.id()) + ")");


    std::ifstream vertexSource("test.vert.glsl");
    GLSLShaderModule vertex(w.device(), vertexSource, vk::ShaderStageBits::vertex);

    std::ifstream fragmentSource("test.frag.glsl");
    GLSLShaderModule fragment(w.device(), fragmentSource, vk::ShaderStageBits::fragment);

    SPIRVShaderModule headerVert(w2.device(), {vertex::DATA, vertex::SIZE});

    // I think this step needs to be explicit
    w.bindPipeline({ std::vector<vpp::ShaderProgram::StageInfo>{vertex.createStageInfo(vk::ShaderStageBits::vertex),
        fragment.createStageInfo(vk::ShaderStageBits::fragment)} });
    w2.bindPipeline({ std::vector<vpp::ShaderProgram::StageInfo>{headerVert.createStageInfo(vk::ShaderStageBits::vertex),
        fragment.createStageInfo(vk::ShaderStageBits::fragment)} });


    //str s = u8"This is a string which I am writing. μ";
    //w.setName(s);
    w.setSize(400, 400);

    uint64_t frame = 0;
    do {
        w.mainLoop(frame);
        w2.mainLoop(frame);
        Window::pollEvents();
        // Increment frame count
        frame++;
    // Wait for main window to be closed
    } while(!w.isClosed());
    // Wait for the window's device to idle before terminating the program!
    w.device().waitIdle();
}
