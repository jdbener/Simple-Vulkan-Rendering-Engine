#include "common.hpp"

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
