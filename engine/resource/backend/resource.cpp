#ifndef __RESOURCE_CPP__
#define __RESOURCE_CPP__

#include "resourceManager.hpp"

std::vector<Resource::Upload> Resource::waitUploads(std::vector<Resource::Upload>& runningUploads){
    for(auto& upload: runningUploads)
        // Wait for all of the data that we enqueued to be copied to the GPU
        upload.wait();

    // Return an empty vector
    return {};
}

Resource::Ref<Resource> Resource::create(const str name) {
    // Create memory for the resource
    Resource* _new = new Resource(Resource::Type::Null);
    // Add a reference to the resource's memory to the ResourceManager and return a reference
    return ResourceManager::singleton()->add<Resource>(name, *_new);
}

// Function which cleans up this resource once all references to it have been destroyed
void Resource::conditionalReset(){
    // If there are no more references to this resource
    if(!refs) {
        // Reset it
        reset();
        // Remove the Resource from the ResourceManager and delete its memory
        ResourceManager::singleton()->remove(name);
    }
}

#endif // __RESOURCE_CPP__
