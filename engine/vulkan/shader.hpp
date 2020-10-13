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

#if (DEBUG_SHADER_CODE == 1)
    void saveBinary(std::ostream& file);
    void saveBinary(str fileName);

    void saveHeader(std::ostream& file, str variableName);
    void saveHeader(str fileName);
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
