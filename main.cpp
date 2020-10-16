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
    // TEMP: Remove in release builds
    vpp::DebugMessenger debugMsg(instance);

    Window w(instance, 800, 600, str(27));

    // TODO: Image View API on Window (merged with framebuffer? renderpass?) RenderData? RenderState?

    // std::cout << "The split string: " << str("this is a set of fun words!").split(" !") << std::endl;
    // std::cout << str("       needs to be cut          ").lstrip() << "-" << str("       needs to be cut          ").rstrip() << "!" << std::endl;
    // std::cout << str("       needs to be cut          ").strip(" t") << std::endl;
    // std::cout << "Stress test: " << str("thisstringhasnosplitsinit").split(" t") << std::endl;
    // std::cout << str("this sure is a great string").replace("ing", "bob").replace("a", "b") << std::endl;

    std::ifstream vertexSource("test.vert.glsl");
    GLSLShaderModule vertex(w.device(), vertexSource, vk::ShaderStageBits::vertex);

    // TODO: Confirm that the generated headers/binary files store valid SPIR-V
    // {
    //     std::stringstream header;
    //     vertex.saveHeader(header, "vertex");
    //
    //     auto strings = str(header.str()).replace({"#pragma once\n#include <cstdint>\n\nconst uint32_t VERTEX_SIZE = 1316;\nconst uint32_t VERTEX[] = {\n", "\n};"}, "")
    //         .strip().split(", \t\n");
    //     std::vector<uint32_t> headerData;
    //     for(str& cur: strings)
    //         headerData.push_back(cur.num(16));
    //
    //     std::cout << strings << std::endl;
    //     std::cout << std::setbase(16) << headerData << std::endl;
    //     SPIRVShaderModule(w.device(), headerData);
    // }

    std::ifstream fragmentSource("test.frag.glsl");
    GLSLShaderModule fragment(w.device(), fragmentSource, vk::ShaderStageBits::fragment);

    //w.createGraphicsRenderPass({vk::ImageLayout::colorAttachmentOptimal});

    // I think this step needs to be explicit
    w.bindPipeline({ std::vector<vpp::ShaderProgram::StageInfo>{vertex.createStageInfo(vk::ShaderStageBits::vertex),
        fragment.createStageInfo(vk::ShaderStageBits::fragment)} });
    // vpp::PipelineLayout layout(w.device(), {}, {});
    // vpp::GraphicsPipelineInfo pipelineInfo(w.renderPass, layout.vkHandle(), /* Program */ });
    // w.pipeline = {w.device(), pipelineInfo.info()};



    str s = u8"This is a string which I am writing. μ";
    w.setName(s);
    w.setSize(400, 400);


    //vk::deviceWaitIdle(w.device());

    //w.rerecordCommandBuffers();

    vpp::Semaphore imgAvailable(w.device());
    vpp::Semaphore imgFinished(w.device());

    do {
        // TODO: convert to function, Window::render
        try{
            // TODO: Improve syncronization
            uint32_t i = vk::acquireNextImageKHR(w.device().vkHandle(), w.swapchain.vkHandle(), /*timeout*/ UINT64_MAX, imgAvailable.vkHandle(), {});

            vk::PipelineStageFlags waitStage = vk::PipelineStageBits::colorAttachmentOutput;
            vk::queueSubmit(w.device().presentQueue()->vkHandle(), std::vector<vk::SubmitInfo>{ {1, &imgAvailable.vkHandle(), &waitStage, 1, &w.renderBuffers[i].commandBuffer.vkHandle(), 1, &imgFinished.vkHandle()} });
            vk::queuePresentKHR(w.device().presentQueue()->vkHandle(), {1, &imgFinished.vkHandle(), 1, &w.swapchain.vkHandle(), &i, nullptr});

            //Window::waitEvents();
            std::cout << i << std::endl;
        } catch (vk::VulkanError& e){
            if (e.error == vk::Result::errorOutOfDateKHR) {
                dlg_info("Swapchain for window '" + str(w.getName()) + "' out of date, recreating");
                w.recreateSwapchain();
                w.rerecordCommandBuffers();
            }
        }
        std::cin.get();
        Window::pollEvents();
    // Wait for main window to be closed
    } while(!w.isClosed());

    std::cout << "waiting for device to idle" << std::endl;
    w.device().waitIdle();
}
