#pragma once

#include "common.hpp"

namespace vpp{
    [[nodiscard]] VPP_API vk::ShaderModule loadShaderModule(vk::Device dev,
    	std::ifstream& sourceFile);
};

class SPIRVShaderModule: public vpp::ShaderModule {
public:
    using vpp::ShaderModule::ShaderModule;

    SPIRVShaderModule(const vpp::Device& dev, std::ifstream& sourceFile);
};

class GLSLShaderModule: public vpp::ShaderModule {
public:
    using vpp::ShaderModule::ShaderModule;

    GLSLShaderModule(const vpp::Device& dev, str sourceCode, vk::ShaderStageBits stage, str entryPoint = "main");
    GLSLShaderModule(const vpp::Device& dev, std::ifstream& sourceFile, vk::ShaderStageBits stage, str entryPoint = "main");

//public: // for testing
private:
    void compileShaderModule(const vpp::Device& device, str sourceCode, vk::ShaderStageBits stage, str entryPoint = "main");
};
