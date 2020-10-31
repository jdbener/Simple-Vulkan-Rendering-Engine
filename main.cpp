#include "engine/util/timer.h"

#include "engine/window.hpp"
#include "engine/resource/mesh.hpp"
#include "engine/resource/material.hpp"

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

    std::vector<MeshData::Vertex> vertecies {
        {{0.0, -0.5}, {1.0, 0.0, 0.0}},
        {{0.5, 0.5}, {0.0, 1.0, 0.0}},
        {{-0.5, 0.5}, {0.0, 0, 1.0}}
    };
    std::vector<uint16_t> indecies {
        0, 1, 2
    };
    Mesh triangle(w, vertecies, indecies);

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
        } });

        // Describe how vertecies are laid out
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

    w.bindCustomCommandRecordingSteps([&](vpp::CommandBuffer& buffer){
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
