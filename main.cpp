#include "engine/util/timer.h"

#include "engine/window.hpp"
#include "engine/resource/mesh.hpp"

#include "engine/vulkan/shader.hpp"

#include <fstream>

/*class debugShaderModule: public SPIRVShaderModule{
public:
    using SPIRVShaderModule::SPIRVShaderModule;

    auto getBytes(){ return bytes; }
};*/

std::ostream& operator <<(std::ostream& s, std::pair<int, int> size){
    s << "(" << size.first << ", " << size.second << ")";
    return s;
}

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription(const uint32_t binding = 0){
        return {binding, sizeof(Vertex), vk::VertexInputRate::vertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions(const uint32_t binding = 0){
        std::array<vk::VertexInputAttributeDescription, 2> out;

        out[0] = {/*location*/ 0, binding, vk::Format::r32g32Sfloat, offsetof(Vertex, position)};
        out[1] = {/*location*/ 1, binding, vk::Format::r32g32b32Sfloat, offsetof(Vertex, color)};

        return out;
    }
};

int main(){
    vk::ApplicationInfo appInfo("Project Delta", GAME_VERSION, "Delta Engine", ENGINE_VERSION, VK_API_VERSION_1_2);

    std::vector<const char*> layers = validateInstanceLayers({"VK_LAYER_KHRONOS_validation"});
    //std::vector<const char*> layers = validateInstanceLayers({});
    std::vector<const char*> extensions = validateInstanceExtensions(Window::requiredVulkanExtensions()
        + std::vector<const char*>{VK_EXT_DEBUG_UTILS_EXTENSION_NAME});

    vpp::Instance instance({{}, &appInfo,
        /*Layer Count*/ (uint32_t) layers.size(),
        /*Layer Names*/ layers.data(),
        /*Extension Count*/ (uint32_t) extensions.size(),
        /*Extension Names*/ extensions.data()});
    // TEMP: Remove in release builds
    vpp::DebugMessenger debugMsg(instance);

    Window w(instance, 800, 600);
    w.setName(str(w.getName()) + " (" + str(w.id()) + ")");

    std::ifstream vertexSource("../test.vert.glsl");
    GLSLShaderModule vertex(w.device(), vertexSource, vk::ShaderStageBits::vertex);
    std::ifstream fragmentSource("../test.frag.glsl");
    GLSLShaderModule fragment(w.device(), fragmentSource, vk::ShaderStageBits::fragment);


    auto pipelineInfo = w.createGraphicsPipelineInfo({ std::vector<vpp::ShaderProgram::StageInfo>{vertex.createStageInfo(vk::ShaderStageBits::vertex, u8"main"),
        fragment.createStageInfo(vk::ShaderStageBits::fragment, u8"main")} });

    // Describe how verticies are laid out
    {
        auto binding = MeshData::Vertex::getBindingDescription();
        auto attributes = MeshData::Vertex::getAttributeDescriptions();
        // NOTE: When messing with the members of this struct, don't overwrite the whole struct,
        //  instead modify the individual elements which need tweaking
        pipelineInfo.vertex.vertexBindingDescriptionCount = 1;
        pipelineInfo.vertex.pVertexBindingDescriptions = &binding;
        pipelineInfo.vertex.vertexAttributeDescriptionCount = attributes.size();
        pipelineInfo.vertex.pVertexAttributeDescriptions = attributes.data();
    }

    // std::vector<Vertex> vertecies {
    //     {{0.0, -0.5}, {1.0, 0.0, 0.0}},
    //     {{0.5, 0.5}, {0.0, 1.0, 0.0}},
    //     {{-0.5, 0.5}, {0.0, 0, 1.0}}
    // };
    std::vector<MeshData::Vertex> vertecies {
        {{0.0, -0.5}, {1.0, 0.0, 0.0}},
        {{0.5, 0.5}, {0.0, 1.0, 0.0}},
        {{-0.5, 0.5}, {0.0, 0, 1.0}}
    };
    // // TODO: Does setting the MemoryPropertyBits here do what I think it does?
    // vpp::SubBuffer vertBuff(w.device().bufferAllocator(), sizeof(vertecies[0]) * vertecies.size(), vk::BufferUsageBits::vertexBuffer | vk::BufferUsageBits::transferDst, (unsigned int) vk::MemoryPropertyBits::deviceLocal);
    //
    std::vector<uint16_t> indecies {
        0, 1, 2
    };
    // vpp::SubBuffer indexBuff(w.device().bufferAllocator(), sizeof(indecies[0]) * indecies.size(), vk::BufferUsageBits::indexBuffer | vk::BufferUsageBits::transferDst, (unsigned int) vk::MemoryPropertyBits::deviceLocal);
    //
    // {
    //     // Create two buffers which will be cleaned up once the data has been copied
    //     vpp::CommandBuffer vertCB = w.commandPool.allocate(), indexCB = w.commandPool.allocate();
    //
    //     // Copy the vertecies into the vertex buffer
    //     uint32_t vid = w.fillStaging(vertBuff, vertecies, /*wait*/ false, vertCB);
    //     // Copy the indecies into the index buffer
    //     uint32_t iid = w.fillStaging(indexBuff, indecies, /*wait*/ false);
    //
    //     std::cout << vid << " - " << iid << std::endl;
    //
    //     // Wait for the vertex and index buffers to both have their data coppied
    //     w.device().queueSubmitter().wait(vid);
    //     w.device().queueSubmitter().wait(iid);
    // }

    Mesh triangle(w, vertecies, indecies);


    // I think this step needs to be explicit
    w.bindPipeline(pipelineInfo);

    triangle.bindPipeline(w.pipeline);

    w.bindCustomCommandRecordingSteps([&](vpp::CommandBuffer& buffer){
        // Bind the vertex buffer
        // vk::cmdBindVertexBuffers(buffer, 0, std::vector<vk::Buffer>{vertBuff.buffer()}, std::vector<vk::DeviceSize>{vertBuff.offset()});
        // vk::cmdBindIndexBuffer(buffer, indexBuff.buffer(), indexBuff.offset(), vk::IndexType::uint16);
        //
        // vk::cmdDrawIndexed(buffer, indecies.size(), /*instances*/ 1, /*firstIndex*/ 0, /*vertexOffset*/ 0, /*firstInstance*/ 0);
        triangle.rerecordCommandBuffer(buffer);
    });
    w.rerecordCommandBuffers();


    // TODO gltf importer https://github.com/syoyo/tinygltf


    uint64_t frame = 0;
    do {
        w.mainLoop(frame);

        Window::pollEvents();
        // Increment frame count
        frame++;
    // Wait for main window to be closed
    } while(!w.isClosed());
    // Wait for the window's device to idle before terminating the program!
    w.device().waitIdle();
}
