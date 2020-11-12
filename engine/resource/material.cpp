#include "material.hpp"

Resource::Ref<Material> Material::create(VulkanState& state, const str name){
    // Create memory for the resource
    Material* _new = new Material(state);
    // Add a reference to the resource's memory to the ResourceManager and return a reference
    return ResourceManager::singleton()->add<Material>(name, *_new);
}

Resource::Ref<GraphicsMaterial> GraphicsMaterial::create(GraphicsState& state, const str name){
    // Create memory for the resource
    GraphicsMaterial* _new = new GraphicsMaterial(state);
    // Add a reference to the resource's memory to the ResourceManager and return a reference
    return ResourceManager::singleton()->add<GraphicsMaterial>(name, *_new);
}

/// Creates the pipeline create info for this material which can then be modified and eventually finalized
GraphicsMaterial::CreateInfo GraphicsMaterial::begin(vpp::ShaderProgram&& program, nytl::Span<const vk::DescriptorSetLayout> uniforms, nytl::Span<const vk::PushConstantRange> constants) {
    GraphicsState& gState = reinterpret_cast<GraphicsState&>(state);

    layout = {gState.device(), uniforms, constants};
    return {gState.renderPass, layout, std::move(program)};
}

/// Binds the Provided Graphics Pipeline Info and creates the internal Pipeline
void GraphicsMaterial::finalize(GraphicsMaterial::CreateInfo& info){
    // Determines if the internal layout is different from the one provided
    bool rebindLayout = info.info().layout != layout.vkHandle();

    // TODO: caching
    // Creates the pipeline
    pipeline = {state.device(), info.info()};

    // Stores the provided layout (if necessary)
    if(rebindLayout) layout = {state.device(), info.info().layout};
}
