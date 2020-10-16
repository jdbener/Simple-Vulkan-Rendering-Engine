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
	SPIRVShaderModule(const vpp::Device& dev, nytl::Span<const uint32_t> bytes);
    SPIRVShaderModule(const vpp::Device& dev, std::istream& sourceFile);

    vpp::ShaderProgram::StageInfo createStageInfo(vk::ShaderStageBits stage, str entryPoint = u8"main") const {
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
public:
    using SPIRVShaderModule::SPIRVShaderModule;

    GLSLShaderModule(const vpp::Device& dev, str sourceCode, vk::ShaderStageBits stage, str entryPoint = "main");
    GLSLShaderModule(const vpp::Device& dev, std::istream& sourceFile, vk::ShaderStageBits stage, str entryPoint = "main");

protected:
    void compileShaderModule(const vpp::Device& device, str sourceCode, vk::ShaderStageBits stage, str entryPoint = "main");
};
