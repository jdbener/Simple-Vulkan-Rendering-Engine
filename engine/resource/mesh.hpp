#pragma once

#include <map>

#include "engine/vulkan/state.hpp"
#include "engine/math/math.hpp"

#include "material.hpp"


// TODO: resource type class this inherits from?
template <typename indexType = uint16_t, typename boneIndexType = uint8_t> // Mesh<uint16_t> and Mesh<uint32_t>
class _Mesh: public Resource {
public:
    struct Vertex {
        glm::vec3 position, normal, tangent;
        glm::vec2 uv;

        // TODO: create secondary data UBO and implementation
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

public:
    // TODO: Implement mechanisms for sending this binding this data
    // Struct representing the ubo which will be sent to the shader
    // template <typename boneIndexType = uint8_t>
    // struct SecondaryData {
    //     // Variable storing how many bones at max each vertex can reference
    //     uint8_t boneSpan;
    //
    //     std::vector<glm::vec3> colors = {};
    //     std::vector<glm::vec2> uv2s = {}, uv3s = {}, uv4s = {}, lightmapUVs = {};
    //
    //     // Each of these arrays are incremented by boneSpan
    //     std::vector<boneIndexType> boneIndecies = {};
    //     std::vector<float> boneWeights = {};
    // };

protected:
    // Reference to a vulkan state to pull command buffers from
    GraphicsState& state;
    // BST holding all of the data for the instances of this mesh
    std::map<Ref<class Material>, std::pair<vpp::SubBuffer, std::vector<Material::Instance>>> instances;
    // Core Buffers
    vpp::SubBuffer vertexBuffer, indexBuffer;
    // Number of indices in the index buffer
    uint64_t indexCount;

public:
    _Mesh(GraphicsState&);

    /// Function which records the commands needed to render this mesh and its instances
    ///     to the provided command buffer.s
    void rerecordCommandBuffer(vpp::CommandBuffer& renderCommandBuffer) const;

    /// Function which adds an instance buffer to the gpu
    Material::Instance& addInstance(glm::mat4, Ref<class Material>&);
    FORCE_INLINE Material::Instance& addInstance(glm::mat4 trans, Ref<class Material>&& mat) { return addInstance(trans, mat); }
    FORCE_INLINE Material::Instance& addInstance(glm::mat4 trans, const str& matName) { return addInstance(trans, ResourceManager::singleton()->get<class Material>(matName)); }
    FORCE_INLINE Material::Instance& addInstance(glm::mat4 trans, const str&& matName) { return addInstance(trans, matName); }

    /// Function which uploads the instance buffers to the GPU
    std::vector<Resource::Upload> uploadInstanceBuffers(bool wait = true);
    /// Function which uploads all of the data which this reference may need to send to the gpu
    virtual std::vector<Resource::Upload> upload(bool wait = true){ return uploadInstanceBuffers(wait); }

public:
    static Ref<_Mesh> create(GraphicsState&, const str name = "");
    static Ref<_Mesh> create(GraphicsState&, std::vector<Vertex>& vertecies, std::vector<indexType>& indecies, const str name = "");

    static Ref<_Mesh> load(std::istream& file) { throw StateNotProvidedException("A GraphicsState must be provided when creating a mesh."); }
    FORCE_INLINE static Ref<_Mesh> load(std::istream&& file) { return load(file); }
    static Ref<_Mesh> load(GraphicsState&, std::istream& file);
    FORCE_INLINE static Ref<_Mesh> load(GraphicsState&, std::istream&& file) { return load(file); }

    /// Sets up the material to use the vertex/instance infromation provided by this mesh
    template <class instanceType = Material::Instance>
    static /*GraphicsMaterial::CreateInfo*/vpp::GraphicsPipelineInfo& bindVertexBindings(/*GraphicsMaterial::CreateInfo*/vpp::GraphicsPipelineInfo& matInfo){
        static vk::VertexInputBindingDescription bindings[] = {Vertex::getBindingDescription(), instanceType::getBindingDescription()};
        static auto attributes = Vertex::getAttributeDescriptions() + instanceType::getAttributeDescriptions();

        // Describe how vertices are laid out
        matInfo.vertex.vertexBindingDescriptionCount = 2;
        matInfo.vertex.pVertexBindingDescriptions = bindings;
        matInfo.vertex.vertexAttributeDescriptionCount = attributes.size();
        matInfo.vertex.pVertexAttributeDescriptions = attributes.data();
        return matInfo;
    }
};

// Typedef the default values of a mesh
typedef _Mesh<uint16_t, uint8_t> Mesh;










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
