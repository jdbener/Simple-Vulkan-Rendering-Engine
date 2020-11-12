#pragma once

#include "engine/vulkan/state.hpp"
#include "engine/math/math.hpp"

#include "resource.hpp"

// Wrapper around a Vulkan Pipeline
class Material : public Resource {
protected:
    VulkanState& state;
    vpp::PipelineLayout layout;
    vpp::Pipeline pipeline;

public:
    Material(VulkanState& _state) : Resource(Resource::Type::Material), state(_state) {}

    const vpp::Pipeline& getPipeline() const { return pipeline; }
    //vpp::Pipeline& getPipeline() { return pipeline; }

    // TODO: Needs to be exposed?
    const vpp::PipelineLayout& getLayout() const { return layout; }

    /// Determine if the material's pipeline has been created and is valid
    bool valid() const { return pipeline.vkHandle(); }
    operator bool() const { return valid(); }

public:
    static Ref<Material> create(VulkanState&, const str name = "");
    static Ref<Material> load(const str filepath) { throw StateNotProvidedException("A GraphicsState must be provided when creating a material."); }
    static Ref<Material> load(std::istream& file) { throw StateNotProvidedException("A GraphicsState must be provided when creating a material."); }
    static Ref<Material> load(VulkanState& state, const str filepath) { std::ifstream fin(filepath); return load(state, fin); }
    static Ref<Material> load(VulkanState&, std::istream& file) { throw std::runtime_error("Base Resources can't be loaded! Load a particular type of resource!"); }
};

/// Override of material with utilities built in for creating a Graphics pipeline
class GraphicsMaterial : public Material {
public:
// Add GraphicsPipelineInfo to this namespace
using CreateInfo = vpp::GraphicsPipelineInfo;

public:
    GraphicsMaterial(GraphicsState& gstate) : Material(gstate) {};

    /// Creates the CreateInfo for this material which can then be externally modified and eventually finalized.
    ///     NOTE: When messing with the members of the CreateInfo struct, don't overwrite the whole struct,
    ///         instead modify the individual elements which need tweaking
    CreateInfo begin(vpp::ShaderProgram&& program, nytl::Span<const vk::DescriptorSetLayout> uniformLayouts = {}, nytl::Span<const vk::PushConstantRange> constantRanges = {});
    /// Binds the CreateInfo and creates the internal Pipeline object
    void finalize(CreateInfo&);
    FORCE_INLINE void finalize(CreateInfo&& info) { finalize(info); }

public:
    /// Creates an empty material, useful for the beginning of the creation process
    static Ref<GraphicsMaterial> create(GraphicsState&, const str name = "");
    /// Finializes the provided CreateInfo
    static Ref<GraphicsMaterial> create(GraphicsState& state, CreateInfo& info, const str name = ""){
        auto out = create(state, name); out->finalize(info); return out;
    }
    /// Creates a CreateInfo from the provided shaders, uniforms, and constants, and then finalizes it
    static Ref<GraphicsMaterial> create(GraphicsState& state, vpp::ShaderProgram&& program, nytl::Span<const vk::DescriptorSetLayout> uniformLayouts = {}, nytl::Span<const vk::PushConstantRange> constantRanges = {}, const str name = ""){
        auto out = create(state, name); out->finalize(out->begin(std::move(program), uniformLayouts, constantRanges)); return out;
    }

    static Ref<Material> load(std::istream& file) { throw StateNotProvidedException("A GraphicsState must be provided when creating a material."); }
    static Ref<Material> load(std::istream&& file) { return load(file); }
    static Ref<GraphicsMaterial> load(GraphicsState&, std::istream& file);
    static Ref<GraphicsMaterial> load(GraphicsState& state, std::istream&& file) { return load(state, file); }
};
