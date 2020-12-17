#ifndef __MESH_CPP__
#define __MESH_CPP__
#include "mesh.hpp"

#include "material.hpp"

template <typename it, typename bit>
_Mesh<it, bit>::_Mesh(GraphicsState& _state)
  : Resource(Resource::Type::Mesh), state(_state) {}

template <typename it, typename bit>
Resource::Ref<_Mesh<it, bit>> _Mesh<it, bit>::create(GraphicsState& state, str name){
    // Create memory for the resource
    _Mesh* _new = new _Mesh(state);
    // Add a reference to the resource's memory to the ResourceManager and return a reference
    return ResourceManager::singleton()->add<_Mesh<it, bit>>(state, name, *_new);
}

template <typename indexType, typename bit>
Resource::Ref<_Mesh<indexType, bit>> _Mesh<indexType, bit>::create(GraphicsState& state, std::vector<Vertex>& vertices, std::vector<indexType>& indices, str name){
    // Allocate memory for a new mesh
    auto out = create(state, name);

    // Set the number of indecies
    out->indexCount = indices.size();

    // Create the buffers
    vpp::BufferAllocator& ba = state.device().bufferAllocator();
    out->vertexBuffer = {ba, vertices.size() * sizeof(vertices[0]), vk::BufferUsageBits::vertexBuffer | vk::BufferUsageBits::transferDst, (unsigned int) vk::MemoryPropertyBits::deviceLocal};
    out->indexBuffer = {ba, indices.size() * sizeof(indices[0]), vk::BufferUsageBits::indexBuffer | vk::BufferUsageBits::transferDst, (unsigned int) vk::MemoryPropertyBits::deviceLocal};

    // Begin copying the data into the gpu buffers
    vpp::CommandBuffer vertCB = state.commandPool.allocate(), indexCB = state.commandPool.allocate();
    uint32_t vid = state.fillStaging(out->vertexBuffer, vertices, /*wait*/ false, vertCB);
    uint32_t iid = state.fillStaging(out->indexBuffer, indices, /*wait*/ false, indexCB);

    // Wait for the vertex and index buffers to both have their data copied
    state.device().queueSubmitter().wait(vid);
    state.device().queueSubmitter().wait(iid);

    return out;
}

template <typename it, typename bit>
typename Material::Instance& _Mesh<it, bit>::addInstance(glm::mat4 transform, Ref<class Material>& material){
    // If the material is not in the map...
    if(instances.find(material) == instances.end())
        // Add an empty vector
        instances.emplace(material, std::make_pair(vpp::SubBuffer{}, std::vector<Material::Instance>{}));

    // References to the stored data elements
    std::vector<Material::Instance>& insts = instances[material].second;

    // Add the instance to the array
    insts.emplace_back(transform);
    return insts.back();
}

template <typename it, typename bit>
std::vector<Resource::Upload> _Mesh<it, bit>::uploadInstanceBuffers(bool wait){
    std::vector<Resource::Upload> runningUploads;
    runningUploads.reserve(instances.size());

    // For each unique material in instances map
    for(std::pair<const Ref<class Material>, std::pair<vpp::SubBuffer, std::vector<Material::Instance>>>& instanceData: instances){
        // Reference the stored data elements
        std::vector<Material::Instance>& insts = instanceData.second.second;
        vpp::SubBuffer& buffer = instanceData.second.first;

        // Create a buffer large enouph to hold all of the instances for this material
        buffer = {state.device().bufferAllocator(), insts.size() * insts[0].byteSize(), vk::BufferUsageBits::vertexBuffer | vk::BufferUsageBits::transferDst, (unsigned int) vk::MemoryPropertyBits::deviceLocal};

        // Begin uploading the data to the buffer and add it to the list of running uploads
        vpp::CommandBuffer cb = state.commandPool.allocate();
        uint32_t waitID = state.fillStaging(buffer, insts, false, cb);
        runningUploads.emplace_back(waitID, std::move(cb), state.device().queueSubmitter());
    }

    // If we should wait for this upload to finish, do so
    if(wait) return Resource::waitUploads(runningUploads);
    // If we shouldn't wait for the uploads, return references to each of them
    return runningUploads;
}

template <typename indexType, typename bit>
void _Mesh<indexType, bit>::rerecordCommandBuffer(vpp::CommandBuffer& renderCommandBuffer) const {
    for(auto it = instances.begin(); it != instances.end(); ++it) {
        // Reference the stored data elements
        const Ref<class Material>& material = it->first;
        const vpp::SubBuffer& instanceBuffer = it->second.first;
        uint64_t instanceCount = it->second.second.size();

        //Bind the material's pipeline
        if(!material->valid()) throw vk::VulkanError(vk::Result::errorInitializationFailed, "Can't record command buffer material: '" + material->getName() + "' is invalid.");
        vk::cmdBindPipeline(renderCommandBuffer, vk::PipelineBindPoint::graphics, material->getPipeline());

        // Bind the vertex buffer
        vk::cmdBindVertexBuffers(renderCommandBuffer, /*firstBinding*/ 0, 1, vertexBuffer.buffer(), vertexBuffer.offset());

        // Bind the instance buffer
        vk::cmdBindVertexBuffers(renderCommandBuffer, /*firstBinding*/ 1, 1, instanceBuffer.buffer(), instanceBuffer.offset());

        // Bind the index buffer
        constexpr vk::IndexType vkIndexType = (sizeof(indexType) < 32 ? vk::IndexType::uint16 : vk::IndexType::uint32);
        vk::cmdBindIndexBuffer(renderCommandBuffer, indexBuffer.buffer(), indexBuffer.offset(), vkIndexType);

        // Draw
        vk::cmdDrawIndexed(renderCommandBuffer, indexCount, /*instanceCount*/ instanceCount, /*firstIndex*/ 0, /*firstVertex*/ 0, /*firstInstance*/ 0);
    }
}

template <typename it, typename bit>
Resource::Ref<_Mesh<it, bit>> _Mesh<it, bit>::load(GraphicsState& state, std::istream& file) {
    // TODO
}








// template <typename it, typename bit>
// BufferedMesh<it, bit>::~BufferedMesh(){
//     clearSecondaryBuffers();
// }
//
// template <typename it, typename bit>
// void BufferedMesh<it, bit>::clearSecondaryBuffers(){
//     #define deleteFunc(var) if(var){\
//         delete var;\
//         var = nullptr;\
//     }
//
//     // Reset secondary arrays
//     deleteFunc(colors);
//     deleteFunc(uv2);
//     deleteFunc(uv3);
//     deleteFunc(uv4);
//     deleteFunc(lightmapUV);
//     deleteFunc(bones);
//     deleteFunc(boneWeights);
//
//     // Reset the bone count
//     maxBoneCount = 0;
// }

#endif //__MESH_CPP__
