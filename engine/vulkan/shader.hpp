#pragma once

// Disable this define or redefine this to 0 to remove extra code for saving shader binaries
#define DEBUG_SHADER_CODE 1

#include "common.hpp"

namespace vpp{
    [[nodiscard]] VPP_API vk::ShaderModule loadShaderModule(vk::Device dev,
    	std::istream& sourceFile);
};

class SPIRVShaderModule: public vpp::ShaderModule {
protected:
#if (DEBUG_SHADER_CODE == 1)
    std::vector<uint32_t> bytes;
#endif // #if (DEBUG_SHADER_CODE == 1)

public:
    SPIRVShaderModule() = default;
    /// Creates a shader module from the specified binary
	SPIRVShaderModule(const vpp::Device& dev, nytl::Span<const uint32_t> bytes);
    /// Creates a shader module from the specified stream
    SPIRVShaderModule(const vpp::Device& dev, std::istream& sourceFile);
    SPIRVShaderModule(const vpp::Device& dev, std::istream&& sourceFile) : SPIRVShaderModule(dev, sourceFile) {}

    vpp::ShaderProgram::StageInfo createStageInfo(vk::ShaderStageBits stage, const str& entryPoint = u8"main") const {
        return {vkHandle(), stage, nullptr, entryPoint, {}};
    }

#if (DEBUG_SHADER_CODE == 1)
    void saveBinary(std::ostream& file) const;
    void saveBinary(str fileName) const;

    void saveHeader(std::ostream& file, str variableName) const;
    void saveHeader(str fileName) const;
#endif // DEBUG_SHADER_CODE == 1
};

class GLSLShaderModule: public SPIRVShaderModule {
private:
    vk::ShaderStageBits stage;
    str entryPoint;

public:
    using SPIRVShaderModule::SPIRVShaderModule;

    GLSLShaderModule(const vpp::Device& dev, str sourceCode, vk::ShaderStageBits _stage, str entryPoint = u8"main");
    GLSLShaderModule(const vpp::Device& dev, std::istream& sourceFile, vk::ShaderStageBits _stage, str entryPoint = u8"main");
    GLSLShaderModule(const vpp::Device& dev, std::istream&& sourceFile, vk::ShaderStageBits _stage, str entryPoint = u8"main") : GLSLShaderModule(dev, sourceFile, _stage, entryPoint) {}

    vpp::ShaderProgram::StageInfo createStageInfo() const { return SPIRVShaderModule::createStageInfo(stage, entryPoint); }

protected:
    void compileShaderModule(const vpp::Device& device, str sourceCode, str entryPoint = "main");
};
