#include "shader.hpp"
#include "common.hpp"

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include "engine/vendor/glslang/DirStackFileIncluder.h"

/// Loads the specified SPIR-V file and converts it into a shader module
vk::ShaderModule vpp::loadShaderModule(vk::Device dev, std::istream& sourceFile){
    std::vector<std::byte> data = readStream(sourceFile);
    // If the number of bytes of data isn't divisible by 4
    if(data.size() % 4) throw std::runtime_error("Invalid SPIR-V binary");

    uint32_t* ptr = (uint32_t*) data.data();
    return loadShaderModule(dev, {ptr, ptr + (data.size() / 4)});
}

/// Creates a shader module from the specified SPIR-V binary array
///     Saves the resuling binary array if the debugging mode is turned on
SPIRVShaderModule::SPIRVShaderModule(const vpp::Device& dev, nytl::Span<const uint32_t> _bytes)
  : vpp::ShaderModule(dev, _bytes){
#if (DEBUG_SHADER_CODE == 1)
      bytes = {_bytes.begin(), _bytes.end()};
#endif // #if (DEBUG_SHADER_CODE == 1)
}

// Helper function which converts uint8 byte data from a filestream into
//  uint32 byte data (SPIR-V)
std::vector<uint32_t> loadShaderBytes(std::istream& sourceFile){
    std::vector<std::byte> data = readStream(sourceFile);
    // If the number of bytes of data isn't divisible by 4
    if(data.size() % 4) throw std::runtime_error("Invalid SPIR-V binary");

    uint32_t* ptr = (uint32_t*) data.data();
    return {ptr, ptr + data.size() / 4};
}

/// Creates a shader module from the specified SPIR-V binary filestream
///     Saves the resuling binary array if the debugging mode is turned on
SPIRVShaderModule::SPIRVShaderModule(const vpp::Device& dev, std::istream& sourceFile)
  : vpp::ShaderModule(dev, vpp::loadShaderModule(dev, sourceFile)) {
#if (DEBUG_SHADER_CODE == 1)
        // Jump to beginning and clear EOF flag
        sourceFile.seekg(0L, std::ios::beg);
        sourceFile.clear();
        // Load in the byte code;
        bytes = loadShaderBytes(sourceFile);
#endif // #if (DEBUG_SHADER_CODE == 1)
    }

/// Saves the shader module's SPIR-V as a binary file
#if (DEBUG_SHADER_CODE == 1)
void SPIRVShaderModule::saveBinary(std::ostream& file) const {
    file.write((char*) bytes.data(), bytes.size() * sizeof(bytes[0]));
}
void SPIRVShaderModule::saveBinary(str fileName) const {
    std::ofstream file(fileName, std::ios::binary);
    saveBinary(file);
    file.close();
}

/// Saves the shader module's SPIR-V as c++ header file with the binary baked into an array.
///     This header can be used to compile the shader code into the executable
void SPIRVShaderModule::saveHeader(std::ostream& file, str variableName) const {
    // Header
    file << "#pragma once" << std::endl
        << "#include <cstdint>" << std::endl
        << std::endl
    // Namespace
        << "namespace " << variableName.replace({" ", "\t",}, "_") << " {" << std::endl
    // Size declaration
        << "\tconst uint32_t SIZE = " << bytes.size() << ";" << std::endl
    // Data declaration
        << "\tconst uint32_t DATA[] = {" << std::endl << "\t\t";

    for(size_t i = 0; i < bytes.size(); i++){
        // Write the data (it will always be 8 digits long and padded with 0s so as not to change the value)
        file << "0x" << std::setfill('0') << std::setw(8) << str::base(bytes[i], 16).toupper();
        // Print a comma after every entry (except the last one)
        if (!(i == bytes.size() - 1)) file << ", ";

        // Start a new line every 5 entries
        if(i % 5 == 4) file << std::endl << "\t\t";
    }

    // End data declaration
    file << std::endl << "\t};"
    // End namespace
        << std::endl << "};" << std::endl;
}
void SPIRVShaderModule::saveHeader(str fileName) const {
    std::ofstream file(fileName);
    saveHeader(file, fileName.split(".")[0]);
    file.close();
}
#endif // #if (DEBUG_SHADER_CODE == 1)

/*---------------------
* GLSLShaderModule
---------------------*/

// Compiles the specified GLSL into a SPIR-V based shader module
///     Saves the resuling binary array if the debugging mode is turned on
GLSLShaderModule::GLSLShaderModule(const vpp::Device& dev, str sourceCode, vk::ShaderStageBits stage, str entryPoint){
    compileShaderModule(dev, sourceCode, stage, entryPoint);
}

// Constructs a shader module from the GLSL code stored in the provided filestream
///     Saves the resuling binary array if the debugging mode is turned on
GLSLShaderModule::GLSLShaderModule(const vpp::Device& dev, std::istream& sourceFile, vk::ShaderStageBits stage, str entryPoint){
    str sourceCode = str::stream(sourceFile);
    compileShaderModule(dev, sourceCode, stage, entryPoint);
}

// Compiles the provided GLSL source code into a SPIR-V based vulkan shader module
//  Saves the resuling binary array if the debugging mode is turned on
//  NOTE: Slightly modified from: https://forestsharp.com/glslang-cpp/
void GLSLShaderModule::compileShaderModule(const vpp::Device& device, str sourceCode, vk::ShaderStageBits _stage, str entryPoint){
    dlg_warn("Be sure to compile this shader to a SPIR-V binary before release!\n(This function should not be used in release builds!)");

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
    DirStackFileIncluder includer; // #Include preprocesser
    str preproccesedCode;
    EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);
    if(!shader.preprocess(&DefaultTBuiltInResource, /*default version*/ 100, EProfile::ENoProfile, false, false, messages, &preproccesedCode, includer)){
        if(sourceCode.size() > 100) { dlg_error("Preprocessing failed for:\n" + sourceCode.substr(0, 100) + "..."); }
        else { dlg_error("Preprocessing failed for:\n" + sourceCode); }
        std::cerr << shader.getInfoLog() << std::endl;
        std::cerr << shader.getInfoDebugLog() << std::endl;
    }
    const char* preproccessedCString = preproccesedCode.c_str();
    shader.setStrings(&preproccessedCString, 1);

    // Parse the shader
    if(!shader.parse(&DefaultTBuiltInResource, 100, false, messages)){
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
#if (DEBUG_SHADER_CODE == 1)
    // Save the SPIRV for later if debug export is enabled
    std::vector<uint32_t>& spirV = bytes;
#else // #if (DEBUG_SHADER_CODE == 1)
    // Only temporarily store the data if debug export is disabled
    std::vector<uint32_t> spirV;
#endif // #if (DEBUG_SHADER_CODE == 1)
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirV);

    // Create shader module
    vpp::ShaderModule module(device, spirV);
    // Save the shader module
    swap(*this, module);
}
