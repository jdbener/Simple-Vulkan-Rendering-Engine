#include "common.hpp"
#include "../window.hpp"

vpp::Instance createInstance(str appName, uint32_t appVersion, std::vector<const char*> extraExtensions, std::vector<const char*> extraValidationLayers, const void* pNext){
    vk::ApplicationInfo appInfo(appName, appVersion, "Delta Engine", ENGINE_VERSION, VK_API_VERSION_1_2);

    std::vector<const char*> layers = validateInstanceLayers(extraValidationLayers);
    std::vector<const char*> extensions = validateInstanceExtensions(Window::requiredVulkanExtensions()
        + extraExtensions);

    vk::InstanceCreateInfo info = {{}, &appInfo,
        /*Layer Count*/ (uint32_t) layers.size(),
        /*Layer Names*/ layers.data(),
        /*Extension Count*/ (uint32_t) extensions.size(),
        /*Extension Names*/ extensions.data()};
    info.pNext = pNext;

    return { info };
}

vpp::Instance createDebugInstance(str appName, uint32_t appVersion, std::vector<const char*> extraExtensions, std::vector<const char*> extraValidationLayers, const void* pNext){
    return createInstance(appName, appVersion,
        extraExtensions + std::vector<const char*>{VK_EXT_DEBUG_UTILS_EXTENSION_NAME},
        extraValidationLayers + std::vector<const char*>{"VK_LAYER_KHRONOS_validation"}, pNext);
}

std::vector<const char*> validateInstanceLayers(std::vector<const char*> layers){
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
    for(size_t i = 0; i < layers.size(); i++){
        bool found = false;
        for(vk::LayerProperties& haystack: availableLayers)
            if(str(layers[i]) == str(haystack.layerName.data())){
                found = true;
                break;
            }
        if(!found){
            dlg_fatal("Failed to find Validation Layer: " + str(layers[i]));
            layers.erase(layers.begin() + i);
            i--;
        }
    }
    return layers;
}

std::vector<const char*> validateInstanceExtensions(std::vector<const char*> extensions) {
    std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties(nullptr);
    for(size_t i = 0; i < extensions.size(); i++){
        bool found = false;
        for(vk::ExtensionProperties& haystack: availableExtensions)
            if(str(extensions[i]) == str(haystack.extensionName.data())){
                found = true;
                break;
            }
        if(!found){
            dlg_fatal("Failed to find Instance Extension: " + str(extensions[i]));
            extensions.erase(extensions.begin() + i);
            i--;
        }
    }
    return extensions;
}
