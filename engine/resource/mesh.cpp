#ifndef __MESH_CPP__
#define __MESH_CPP__
#include "mesh.hpp"

template <typename indexType, typename bit>
Mesh<indexType, bit>::Mesh(RenderState& _state)
  : state(_state) {}

template <typename indexType, typename bit>
Mesh<indexType, bit>::Mesh(RenderState& _state, std::vector<Vertex>& vertecies, std::vector<indexType>& indecies)
  : state(_state), indexCount(indecies.size()) {
    // Create the buffers
    vpp::BufferAllocator& ba = state.device().bufferAllocator();
    vertexBuffer = {ba, vertecies.size() * sizeof(vertecies[0]), vk::BufferUsageBits::vertexBuffer | vk::BufferUsageBits::transferDst, (unsigned int) vk::MemoryPropertyBits::deviceLocal};
    indexBuffer = {ba, indecies.size() * sizeof(indecies[0]), vk::BufferUsageBits::indexBuffer | vk::BufferUsageBits::transferDst, (unsigned int) vk::MemoryPropertyBits::deviceLocal};

    // Begin copying the data into the gpu buffers
    vpp::CommandBuffer vertCB = state.commandPool.allocate(), indexCB = state.commandPool.allocate();
    uint32_t vid = state.fillStaging(vertexBuffer, vertecies, /*wait*/ false, vertCB);
    uint32_t iid = state.fillStaging(indexBuffer, indecies, /*wait*/ false, indexCB);

    // Wait for the vertex and index buffers to both have their data coppied
    state.device().queueSubmitter().wait(vid);
    state.device().queueSubmitter().wait(iid);
}

template <typename indexType, typename bit>
void Mesh<indexType, bit>::rerecordCommandBuffer(vpp::CommandBuffer& renderCommandBuffer) const {
    // // Begin recording the command buffer (possibly overwriting it)
    // vk::CommandBufferInheritanceInfo inheritanceInfo(state.renderPass, /*subPass*/ 0, VK_NULL_HANDLE, /*enableQueries*/ VK_FALSE);
    // vk::beginCommandBuffer(renderCommandBuffer, {vk::CommandBufferUsageBits::simultaneousUse, &inheritanceInfo});
    // // End recording the command buffer at the end of the function
    // defer(vk::endCommandBuffer(renderCommandBuffer);, ecb);

    // Bind the pipeline
    if(!pipeline) throw vk::VulkanError(vk::Result::errorInitializationFailed, "Can't record command buffer if no pipeline has been provided.");
    vk::cmdBindPipeline(renderCommandBuffer, vk::PipelineBindPoint::graphics, *pipeline);
    // Bind the vertex buffer
    vk::cmdBindVertexBuffers(renderCommandBuffer, /*firstBinding*/ 0, 1, vertexBuffer.buffer(), vertexBuffer.offset());
    // Bind the index buffer
    constexpr vk::IndexType vkIndexType = (sizeof(indexType) < 32 ? vk::IndexType::uint16 : vk::IndexType::uint32);
    vk::cmdBindIndexBuffer(renderCommandBuffer, indexBuffer.buffer(), indexBuffer.offset(), vkIndexType);

    // Draw
    vk::cmdDrawIndexed(renderCommandBuffer, indexCount, /*instanceCount*/ 1, /*firstIndex*/ 0, /*vertexOffset*/ 0, /*firstInstance*/ 0);
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
