#pragma once

#include "engine/vulkan/state.hpp"
#include "engine/math/math.hpp"

class Material;

namespace MeshData {
    struct Vertex {
        glm::vec3 position, normal, tangent;
        glm::vec2 uv;

        // TODO: remove
        glm::vec3 color;

        Vertex() = default;
        Vertex(glm::vec2 pos, glm::vec3 col) : position({pos.x, pos.y, 0}), color(col) {}

        static vk::VertexInputBindingDescription getBindingDescription(const uint32_t binding = 0){
            return {binding, sizeof(Vertex), vk::VertexInputRate::vertex};
        }

        static std::array<vk::VertexInputAttributeDescription, 5> getAttributeDescriptions(const uint32_t binding = 0){
            std::array<vk::VertexInputAttributeDescription, 5> out;

            out[0] = {/*location*/ 0, binding, vk::Format::r32g32b32Sfloat, offsetof(Vertex, position)};
            out[1] = {/*location*/ 1, binding, vk::Format::r32g32b32Sfloat, offsetof(Vertex, normal)};
            out[2] = {/*location*/ 2, binding, vk::Format::r32g32b32Sfloat, offsetof(Vertex, tangent)};
            out[3] = {/*location*/ 3, binding, vk::Format::r32g32Sfloat, offsetof(Vertex, uv)};
            out[4] = {/*location*/ 4, binding, vk::Format::r32g32b32Sfloat, offsetof(Vertex, color)};

            return out;
        }
    };

    // TODO: Implement mechanisms for sending this binding this data
    // Struct representing the ubo which will be sent to the shader
    template <typename boneIndexType = uint8_t>
    struct SecondaryData {
        // Variable storing how many bones at max each vertex can reference
        uint8_t boneSpan;

        std::vector<glm::vec3> colors = {};
        std::vector<glm::vec2> uv2s = {}, uv3s = {}, uv4s = {}, lightmapUVs = {};

        // Each of these arrays are incremented by boneSpan
        std::vector<boneIndexType> boneIndecies = {};
        std::vector<float> boneWeights = {};
    };
}

// TODO: resource type class this inherits from?
template <typename indexType = uint16_t, typename boneIndexType = uint8_t> // Mesh<uint16_t> and Mesh<uint32_t>
class Mesh {
using Vertex = MeshData::Vertex;
using SecondaryData = MeshData::SecondaryData<boneIndexType>;

protected:
    // Reference to a vulkan state to pull command buffers from
    GraphicsState& state;

    // Pipeline for the material of this object
    // TODO: replace with material system
    Material* material = nullptr;
    // Core Buffers
    vpp::SubBuffer vertexBuffer, indexBuffer;
    // Number of indices in the index buffer
    uint64_t indexCount;


public:
    Mesh() = default;
    Mesh(GraphicsState&);
    Mesh(GraphicsState&, std::vector<Vertex>&, std::vector<indexType>&);
    virtual ~Mesh() {}

    void rerecordCommandBuffer(vpp::CommandBuffer& renderCommandBuffer) const;

    //TODO:: replace the single material with instance data
    Mesh& bindMaterial(Material& mat) { material = &mat; return *this;}
    //Mesh& bindVertexColors();
};














// Mesh which saves all of the provided buffers for future use
// template <typename indexType = uint16_t, typename boneIndexType = uint8_t> // Mesh<uint16_t> and Mesh<uint32_t>
// class BufferedMesh: Mesh<indexType, boneIndexType> {
// protected:
//     // ------------------------------
//     // ---- Core Mesh Properties ----
//     // ------------------------------
//     // Array of core elements of a vertex
//     std::vector<typename Mesh<indexType, boneIndexType>::Vertex> vertecies;
//     // Array of indecies referencing the other arrays
//     std::vector<indexType> indecies;
//
// protected:
//     // -----------------------------------
//     // ---- Secondary Mesh Properties ----
//     // -----------------------------------
//     // These are all stored in null defaulted pointers and only those which are used are added
//     // Vertex colors
//     std::vector<glm::vec3>* colors = nullptr;
//
//     // Three extra UV channels
//     std::vector<glm::vec2>* uv2 = nullptr;
//     std::vector<glm::vec2>* uv3 = nullptr;
//     std::vector<glm::vec2>* uv4 = nullptr;
//     // Shadowmap uv channel (nessicary? core?)
//     std::vector<glm::vec2>* lightmapUV = nullptr;
//
//     // The number of bones each vertex references (is equal to the maximum number of bones any vertex references)
//     uint8_t maxBoneCount = 0;
//     // Bone indecies (increment by maxBoneCount)
//     std::vector<boneIndexType>* bones = nullptr;
//     // Bone weights (increment by maxBoneCount)
//     std::vector<float>* boneWeights = nullptr;
//
// public:
//     virtual ~BufferedMesh();
//
// protected:
//     void clearSecondaryBuffers();
// };

#include "mesh.cpp"
