#pragma once

#include "engine/vulkan/state.hpp"
#include "engine/math/math.hpp"

// Wrapper around a Vulkan Pipeline
class Material {
protected:
    VulkanState& state;
    vpp::PipelineLayout layout;
    vpp::Pipeline pipeline;

public:
    Material(VulkanState& _state) : state(_state) {}

    const vpp::Pipeline& getPipeline() const { return pipeline; }
    //vpp::Pipeline& getPipeline() { return pipeline; }

    // TODO: Needs to be exposed?
    const vpp::PipelineLayout& getLayout() const { return layout; }

    /// Determine if the material's pipeline has been created and is valid
    bool valid() const { return pipeline.vkHandle(); }
    operator bool() const { return valid(); }
};

/// Override of material with utilities built in for creating a Graphics pipeline
class GraphicsMaterial : public Material {
public:
// Add GraphicsPipelineInfo to this namespace
using CreateInfo = vpp::GraphicsPipelineInfo;

private:
    /// Override which makes the all in one constructer possible
    GraphicsMaterial(GraphicsState& gstate, CreateInfo&& info)
        : GraphicsMaterial(gstate, info) {}

public:
    /// Creates an empty material, usefull for then beginning
    GraphicsMaterial(GraphicsState& gstate) : Material(gstate) {};
    /// Finializes the provided CreateInfo
    GraphicsMaterial(GraphicsState& gstate, CreateInfo& info)
        : Material(gstate) { finalize(info); }
    /// Creates a CreateInfo from the provided shaders, uniforms, and constants, and then finalizes it
    GraphicsMaterial(GraphicsState& gstate, vpp::ShaderProgram&& program, nytl::Span<const vk::DescriptorSetLayout> uniformLayouts = {}, nytl::Span<const vk::PushConstantRange> constantRanges = {})
        : GraphicsMaterial(gstate, begin(std::move(program), uniformLayouts, constantRanges)) {}

    /// Creates the CreateInfo for this material which can then be externally modified and eventually finialized.
    ///     NOTE: When messing with the members of the CreateInfo struct, don't overwrite the whole struct,
    ///         instead modify the individual elements which need tweaking
    CreateInfo begin(vpp::ShaderProgram&& program, nytl::Span<const vk::DescriptorSetLayout> uniformLayouts = {}, nytl::Span<const vk::PushConstantRange> constantRanges = {});
    /// Binds the CreateInfo and creates the internal Pipeline object
    void finalize(CreateInfo&);
};
