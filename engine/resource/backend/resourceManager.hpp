#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__
#include "resource.hpp"

#include "engine/vulkan/state.hpp"

// Exception thrown when trying to add a resource which already exists
struct ResourceExistsException: public std::runtime_error { using std::runtime_error::runtime_error; };

class ResourceManager: public nytl::NonMovable {
friend class Resource;
private:
    // Hashtable storing all of the currently loaded resources
    std::unordered_map<std::string, Resource*> loadedResources = {};
    // ID which is used to
    size_t creationID = 0;

protected:
    /// Constructor is private so applications can't create an instance of this class (must use singleton)
    ResourceManager() = default;

    /// Create the name of a new resource
    str createName(Resource::Type type){ return "MEMORY/" + Resource::type2str(type) + '-' + str(creationID++); }

    /// Function which clears the loadedResources
    void clear(){
        loadedResources = {};
        creationID = 0;
    }

public:
    /// Begins managing an existing resource.
    ///     NOTE: Insures that the name stored in the managed resource matches its name in the table
    template<class T>
    Resource::Ref<T> add(str name, Resource& ref) {
        if(name.size() == 0) name = createName(ref.type);

        auto [ptr, success] = loadedResources.emplace(name, &ref);
        if(!success) throw ResourceExistsException(name + " already exists.");

        ptr->second->name = name;
        return ptr->second;
    }

    /// Begins managing an existing resource
    ///     (tracks seperate instances for unique states).
    ///     NOTE: Insures that the name stored in the managed resource matches its name in the table
    template<class T>
    Resource::Ref<T> add(VulkanState& state, str name, Resource& ref) {
        // Create the resource name if nessicary
        if(name.size() == 0) name = createName(ref.type);
        // Attach the state name if nessicary
        if(!name.contains("~S")) name += "~S" + str(state.id());

        // Call the add function now that proper name has been created
        return add<T>(name, ref);
    }

    /// Determines if the specified resource has already been loaded
    FORCE_INLINE bool loaded(const str& name) const {
        return loadedResources.find(name) != loadedResources.end();
    }
    FORCE_INLINE bool loaded(const str&& name) const { return loaded(name); }

    // Removes the specified resource from the
    bool remove(const str& name){
        if(loaded(name)){
            // Save the pointer to delete
            auto toDelete = loadedResources[name];
            // Remove the Resource from the hashtable
            bool success = loadedResources.erase(name);
            // Delete the memory
            if(success && toDelete) delete toDelete;
            return success;
        }
        // If it wasn't loaded we failed to remove it
        return false;
    }
    FORCE_INLINE bool remove(const str&& name){ return remove(name); }

    /// Gets a reference to the specified resource
    template<class T>
    Resource::Ref<T> get(const str& name, bool createIfNeeded = false){
        // If the resource is not already loaded
        if(!loaded(name)){
            if(createIfNeeded) T::load(std::ifstream{name});
            else throw std::range_error(name + "not loaded.");
        }
        return loadedResources[name];
    }
    template<class T> FORCE_INLINE Resource::Ref<T> get(const str&& name, bool createIfNeeded = false){ return get<T>(name, createIfNeeded); }
    FORCE_INLINE Resource::Ref<Resource> operator[] (const str& name) { return get<Resource>(name, false); }
    FORCE_INLINE Resource::Ref<Resource> operator[] (const str&& name) { return get<Resource>(name, false); }

    /// Gets a reference to the specified resource
    ///     (Specific to the provided state)
    template<class T>
    Resource::Ref<T> get(VulkanState& state, str name, bool createIfNeeded = false){
        // Attach the state name if nessicary
        if(!name.contains("~S")) name += "~S" + str(state.id());

        // If the resource is not already loaded
        if(!loaded(name)){
            // Loading specifies the state in the load function and removes the state data from the filename
            if(createIfNeeded) T::load(state, std::ifstream{name.split<str>("~")[0]});
            else throw std::range_error(name + "not loaded.");
        }
        return loadedResources[name];
    }

    /// Upload all of the data any of the resources may need to upload to the GPU
    ///     (trys to do so in as parallelized a manor as possible)
    std::vector<Resource::Upload> upload(bool wait = true){
        std::vector<Resource::Upload> runningUploads;
        for(auto& resource: loadedResources){
            std::vector<Resource::Upload> merge = resource.second->upload(false);
            for (Resource::Upload& upload: merge)
                runningUploads.emplace_back(std::move(upload));
        }

        // If we should wait for the running uploads, do so
        if(wait) return Resource::waitUploads(runningUploads);
        // Otherwise return the references to them
        return runningUploads;
    }

    // TODO: remove...Debug
    // std::unordered_map<std::string, Resource*>& res() { return loadedResources; }

public:
    /// Function which return's the engine's ResourceManager singleton
    FORCE_INLINE static ResourceManager* singleton() { static ResourceManager singleton; return &singleton; }
};

#endif //__RESOURCE_MANAGER_H__
