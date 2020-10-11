#include "shader.hpp"
#include "common.hpp"

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include "engine/vendor/glslang/DirStackFileIncluder.h"

vk::ShaderModule vpp::loadShaderModule(vk::Device dev, std::ifstream& sourceFile){
    std::vector<std::byte> data = readStream(sourceFile);
    // If the number of bytes of data isn't divisible by 4
    if(data.size() % 4) throw std::runtime_error("Invalid SPIR-V binary");

    uint32_t* ptr = (uint32_t*) data.data();
    return loadShaderModule(dev, {ptr, ptr + (data.size() / 4)});
}

SPIRVShaderModule::SPIRVShaderModule(const vpp::Device& dev, std::ifstream& sourceFile)
    : vpp::ShaderModule(dev, vpp::loadShaderModule(dev, sourceFile)) {}

/*---------------------
* GLSLShaderModule
---------------------*/

GLSLShaderModule::GLSLShaderModule(const vpp::Device& dev, str sourceCode, vk::ShaderStageBits stage, str entryPoint){
    compileShaderModule(dev, sourceCode, stage, entryPoint);
}

// Constructs a shader module from the code stored at the provided filestream
GLSLShaderModule::GLSLShaderModule(const vpp::Device& dev, std::ifstream& sourceFile, vk::ShaderStageBits stage, str entryPoint){
    str sourceCode = str::stream(sourceFile);
    compileShaderModule(dev, sourceCode, stage, entryPoint);
}

// Compiles the provided GLSL source code into a SPIR-V based vulkan shader module
//  NOTE: Slightly modified from: https://forestsharp.com/glslang-cpp/
void GLSLShaderModule::compileShaderModule(const vpp::Device& device, str sourceCode, vk::ShaderStageBits _stage, str entryPoint){
    dlg_warn("Be sure to compile this shader to SPIRV before release!\n(This function should not be used in release builds!)");

    // TODO: Look at what all is in this monolithic beast
    TBuiltInResource DefaultTBuiltInResource = {
        /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4,
    };
    DefaultTBuiltInResource.limits = {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    };
    //TBuiltInResource DefaultTBuiltInResource = {};

    // Convert the shader stage from a vulkan stage to a glslang stage
    EShLanguage stage;
    switch(_stage){
    case vk::ShaderStageBits::vertex: stage = EShLangVertex; break;
    case vk::ShaderStageBits::tessellationControl: stage = EShLangTessControl; break;
    case vk::ShaderStageBits::tessellationEvaluation: stage = EShLangTessEvaluation; break;
    case vk::ShaderStageBits::geometry: stage = EShLangGeometry; break;
    case vk::ShaderStageBits::fragment: stage = EShLangFragment; break;
    case vk::ShaderStageBits::compute: stage = EShLangCompute; break;
    default:
        dlg_error("Unknown Shader Stage");
        stage = EShLangCount;
    }

    // Initalize glslang if it hasn't been initalized yet
    static bool glslangInitalized = false;
    if(!glslangInitalized) glslangInitalized = glslang::InitializeProcess();
    if(!glslangInitalized) throw std::runtime_error("Failed to initalize glslang");

    // Create the shader
    glslang::TShader shader(stage);
    const char* sourceCString = sourceCode;
    shader.setStrings(&sourceCString, 1);
    shader.setEnvInput(glslang::EShSource::EShSourceGlsl, stage, glslang::EShClient::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_1);
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_3);
    shader.setEntryPoint(entryPoint);

    // Preprocess the shader
    DirStackFileIncluder includer; // #Include processer
    str preproccesedCode;
    EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);
    if(!shader.preprocess(&DefaultTBuiltInResource, /*default version*/ 100, EProfile::ENoProfile, false, false, messages, &preproccesedCode, includer)){
        // TODO: check if we get error messages with an actual shader
        if(sourceCode.size() > 100) { dlg_error("Preprocessing failed for:\n" + sourceCode.substr(0, 100) + "..."); }
        else { dlg_error("Preprocessing failed for:\n" + sourceCode); }
        std::cerr << shader.getInfoLog() << std::endl;
        std::cerr << shader.getInfoDebugLog() << std::endl;
    }
    const char* preproccessedCString = preproccesedCode.c_str();
    shader.setStrings(&preproccessedCString, 1);

    // Parse the shader
    if(!shader.parse(&DefaultTBuiltInResource, 100, false, messages)){
        // TODO: check if we get error messages with an actual shader
        if(sourceCode.size() > 100) { dlg_error("GLSL Parsing failed for:\n" + sourceCode.substr(0, 100) + "..."); }
        else { dlg_error("GLSL Parsing failed for:\n" + sourceCode); }
        std::cerr << shader.getInfoLog() << std::endl;
        std::cerr << shader.getInfoDebugLog() << std::endl;
    }

    // Link the shader into a program
    glslang::TProgram program;
    program.addShader(&shader);
    if(!program.link(messages)){
        if(sourceCode.size() > 100) { dlg_error("Linking failed for:\n" + sourceCode.substr(0, 100) + "..."); }
        else { dlg_error("Linking failed for:\n" + sourceCode); }
        std::cerr << program.getInfoLog() << std::endl;
        std::cerr << program.getInfoDebugLog() << std::endl;
    }

    // Convert the program to SPIR-V
    std::vector<uint32_t> spirV;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirV);

    // Create shader module
    vpp::ShaderModule module(device, spirV);
    // Save the shader module
    swap(*this, module);
}
