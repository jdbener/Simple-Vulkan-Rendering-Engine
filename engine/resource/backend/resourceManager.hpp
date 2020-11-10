#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__
#include "resource.hpp"

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
    str createName(Resource::Type type){ return "memory/" + Resource::type2str(type) + '-' + str(creationID++); }

    /// Function which clears the loadedResources
    void clear(){
        loadedResources = {};
        creationID = 0;
    }

public:
    /// Begins managing an existing resource
    ///     NOTE: Insures that the name stored in the managed resource matches its name in the table
    template<class T>
    Resource::Ref<T> add(str name, Resource& ref) {
        if(name.size() == 0) name = createName(ref.type);

        auto [ptr, success] = loadedResources.emplace(name, &ref);
        if(!success) throw ResourceExistsException(name + " already exists.");

        ptr->second->name = name;
        return ptr->second;
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
            if(createIfNeeded) T::load(name);
            else throw std::range_error(name + "not loaded.");
        }
        return loadedResources[name];
    }
    template<class T> FORCE_INLINE Resource::Ref<T> get(const str&& name, bool createIfNeeded = false){ return get<T>(name, createIfNeeded); }
    FORCE_INLINE Resource::Ref<Resource> operator[] (const str& name) { return get<Resource>(name, false); }
    FORCE_INLINE Resource::Ref<Resource> operator[] (const str&& name) { return get<Resource>(name, false); }

    // TODO: remove...Debug
    // std::unordered_map<std::string, Resource*>& res() { return loadedResources; }

public:
    /// Function which return's the engine's ResourceManager singleton
    FORCE_INLINE static ResourceManager* singleton() { static ResourceManager singleton; return &singleton; }
};

#endif //__RESOURCE_MANAGER_H__
