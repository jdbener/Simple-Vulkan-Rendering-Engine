#include "engine/util/timer.h"

#include "engine/window.hpp"
#include "engine/resource/mesh.hpp"
#include "engine/resource/material.hpp"

#include "engine/vulkan/shader.hpp"

#include "vpp/trackedDescriptor.hpp"

#include <fstream>

std::ostream& operator <<(std::ostream& s, std::pair<int, int> size){
    s << "(" << size.first << ", " << size.second << ")";
    return s;
}

struct UBO {
    float size;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    static vk::DescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding = 0) {
        return {binding, vk::DescriptorType::uniformBuffer, 1, vk::ShaderStageBits::vertex, nullptr};
    }
};

void updateUniformBuffer(vpp::SubBuffer& buffer, std::variant<std::pair<int, int>, float> dimensionsAspect){
    static auto startTime = std::chrono::high_resolution_clock::now();
    //std::cout << dimensionsAspect << std::endl;

    float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();
    UBO ubo;
    //ubo.model = glm::rotate<float>(glm::mat4(1.0), time * glm::radians(90.0), {0, 0, 1});
    //ubo.view = glm::lookAt<float>(glm::vec3{2, 2, 2}, {0, 0, 0}, {0, 0, 1});
    //if(dimensionsAspect.index() == 0) ubo.projection = glm::perspective<float>(glm::radians(45.0), std::get<std::pair<int, int>>(dimensionsAspect).first / (float) std::get<std::pair<int, int>>(dimensionsAspect).second, .1, 10);
    //else ubo.projection = glm::perspective<float>(glm::radians(45.0), std::get<float>(dimensionsAspect), .1, 10);

    // If they are all identity matrices nothing should change
    float size = cos(time) + 1;
    //std::cout << size << std::endl;
    ubo.size = size;


    vpp::MemoryMapView map = buffer.memoryMap();
    memcpy(map.ptr(), &ubo, sizeof(ubo));
}

int main(){
    // Terminate all connections to the window at the end of the program
    // Termination will still occur even if an exception is thrown!
    defer(Window::terminate();, WTERM);

#ifndef DEBUG
#define DEBUG 1
#endif
#if DEBUG == 1
    vpp::Instance instance = createDebugInstance("Project Delta", GAME_VERSION);
    vpp::DebugMessenger debugMsg(instance);
#else
    vpp::Instance instance = createInstance("Project Delta", GAME_VERSION);
#endif //DEBUG == 1


    Window w(instance, 800, 600);
    w.setName(str(w.getName()) + " (" + str(w.id()) + ")");

    std::vector<MeshData::Vertex> vertices {
        {{0.0, -0.5}, {1.0, 0.0, 0.0}},
        {{0.5, 0.5}, {0.0, 1.0, 0.0}},
        {{-0.5, 0.5}, {0.0, 0, 1.0}}
    };
    std::vector<uint16_t> indices {
        0, 1, 2
    };
    Mesh triangle(w, vertices, indices);


    vpp::SubBuffer uniformBuffers [w.renderBuffers.size()];
    vpp::TrDsLayout uboDescriptorLayout(w.device(), {UBO::createDescriptorSetLayoutBinding()});
    vpp::TrDs uboDescriptorSets [w.renderBuffers.size()];
    vk::WriteDescriptorSet writes [w.renderBuffers.size()];
    repeat(w.renderBuffers.size(), i) {
        // Buffers are host visible/coherent since we will often be copying new data into it and a staging buffer makes that unnecessarily complicated
        uniformBuffers[i] = {w.device().bufferAllocator(), sizeof(UBO), vk::BufferUsageBits::uniformBuffer, vk::MemoryPropertyBits::hostVisible | vk::MemoryPropertyBits::hostCoherent};
        uboDescriptorSets[i] = w.device().descriptorAllocator().alloc(uboDescriptorLayout);

        vk::DescriptorBufferInfo bufferInfo{uniformBuffers[i].buffer(), uniformBuffers[i].offset(), uniformBuffers[i].size()};
        writes[i] = {uboDescriptorSets[i], /*binding*/ 0, /*firstArrayElem*/ 0, 1, vk::DescriptorType::uniformBuffer, /*imgInfo*/ nullptr, &bufferInfo};
    }
    // Bind the created buffers to their descriptor sets
    vk::updateDescriptorSets(w.device(), {writes, w.renderBuffers.size()}, {});




    GraphicsMaterial triangleMat(w);
    {
        // Load the shaders for the material
        std::ifstream vertexSource("../test.vert.glsl");
        GLSLShaderModule vertex(w.device(), vertexSource, vk::ShaderStageBits::vertex);
        std::ifstream fragmentSource("../test.frag.glsl");
        GLSLShaderModule fragment(w.device(), fragmentSource, vk::ShaderStageBits::fragment);

        // Create the setup structure for the material
        // NOTE: When messing with the members of this struct, don't overwrite the whole struct,
        //  instead modify the individual elements which need tweaking
        GraphicsMaterial::CreateInfo matInfo = triangleMat.begin({ std::vector<vpp::ShaderProgram::StageInfo>{
            vertex.createStageInfo(),
            fragment.createStageInfo()
        } }, nytl::make_span(uboDescriptorLayout.vkHandle()) );

        // Describe how vertices are laid out
        auto binding = MeshData::Vertex::getBindingDescription();
        auto attributes = MeshData::Vertex::getAttributeDescriptions();
        matInfo.vertex.vertexBindingDescriptionCount = 1;
        matInfo.vertex.pVertexBindingDescriptions = &binding;
        matInfo.vertex.vertexAttributeDescriptionCount = attributes.size();
        matInfo.vertex.pVertexAttributeDescriptions = attributes.data();

        // Finalize the material and create all of the internal vulkan objects
        triangleMat.finalize(matInfo);
    }

    triangle.bindMaterial(triangleMat);

    w.bindCustomCommandRecordingSteps([&](vpp::CommandBuffer& buffer, uint8_t i){
        vk::cmdBindDescriptorSets(buffer, vk::PipelineBindPoint::graphics, triangleMat.getLayout(), 0, nytl::make_span(uboDescriptorSets[i].vkHandle()), /*dynamicOffsets*/ {});

        triangle.rerecordCommandBuffer(buffer);
    });
    w.rerecordCommandBuffers();

    w.bindCustomMainLoopSteps([&](VulkanState& state, uint32_t i){
        updateUniformBuffer(uniformBuffers[i], w.getTotalSize());
    });


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
