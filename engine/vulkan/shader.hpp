#pragma once

#include "common.hpp"

class GLSLShaderModule: public vpp::ShaderModule {
public:
    using vpp::ShaderModule::ShaderModule;

    GLSLShaderModule(const vpp::Device& dev, str sourceCode, vk::ShaderStageBits stage, str entryPoint = "main");
    GLSLShaderModule(const vpp::Device& dev, std::ifstream& sourceCode, vk::ShaderStageBits stage, str entryPoint = "main");

//public: // for testing
private:
    void compileShaderModule(const vpp::Device& device, str sourceCode, vk::ShaderStageBits stage, str entryPoint = "main");
};
