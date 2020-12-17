#ifndef __RESOURCE_BACKEND_H__
#define __RESOURCE_BACKEND_H__

#include "engine/vulkan/common.hpp"
#include "engine/util/string.hpp"

#include <fstream>
#include <nytl/nonCopyable.hpp>

class VulkanState;

class Resource: public nytl::NonMovable {
friend class Ref;
friend class ResourceManager;
public:
    // Struct representing data being uploaded to the cpu
    struct Upload {
        // ID in a device's submission queue representing this upload
        uint32_t waitID = UINT32_MAX;
        // The command buffer handle representing the upload
        vpp::CommandBuffer commandBuffer;
        // The queue which was used to create this upload
        vpp::QueueSubmitter& queueSubmitter;

        Upload(uint32_t id, vpp::CommandBuffer&& cb, vpp::QueueSubmitter& submit): waitID(id), commandBuffer(std::move(cb)), queueSubmitter(submit) {}
        /// Move constructor marks the other upload as invalid
        Upload(Upload&& other) : waitID(other.waitID), commandBuffer(std::move(other.commandBuffer)), queueSubmitter(other.queueSubmitter){ other.waitID = UINT32_MAX; }
        /// When this object goes out of scope, make sure upload has finished
        ~Upload() { if (waitID != UINT32_MAX) wait(); }

        /// Wait for the upload represented by this object to finish and mark
        ///     it as invalidated
        void wait() { queueSubmitter.wait(waitID); waitID = UINT32_MAX; }
    };
public:
    // Enum which provides reflection on what kind of reference this is.
    enum Type {Null = 0, Mesh, Material, GraphicsMaterial};
    /// Function which converts a resource type into a str
    static str type2str(Type type){
        switch(type){
        case Null: return "Null";
        case Mesh: return "Mesh";
        case Material: return "Material";
        case GraphicsMaterial: return "GraphicsMaterial";
        }
    }

public:
    template <class T = Resource>
    class Ref {
    // Mark all template implementations of Ref as friends
    template <class T2> friend class Ref;
    protected:
        T* ref = nullptr;

    public:
        Ref(Resource* _ref = nullptr) : ref(reinterpret_cast<T*>(_ref)) { if(ref) ref->refs++; }
        // Copy constructor increments the ref count
        template<class T2> Ref(Ref<T2>& other) : ref(reinterpret_cast<T*>(other.ref)) { if(ref) ref->refs++; }
        // Move constructor insures the other object doesn't reduce the ref count
        template<class T2> Ref(Ref<T2>&& other) : ref(reinterpret_cast<T*>(other.ref)) { other.ref = nullptr; }
        ~Ref() { deref(); }

        /// Release this function's reference to the resource and mark that it has been released
        void deref() {
            if(ref){
                ref->refs--;
                ref->conditionalReset();
                ref = nullptr;
            }
        }
        /// Return the stored reference
        FORCE_INLINE const T* get() const { return ref; }
        /// Return the stored reference cast to one of its parent types
        template <class T2> FORCE_INLINE const T2* cast() const { return reinterpret_cast<T2*>(ref); }

        /// Returns true if the stored reference is valid
        FORCE_INLINE bool refValid() { return ref; }
        /// Returns true if the stored reference is valid, and the resource manager has the reference loaded
        bool valid();

        // Returns true if this reference is pointing to a resource created in memory
        bool isMemory() { return ref->getName(false).beginsWith("MEMORY"); }
        // Returns true if this reference is pointing to a resource loaded from a file
        bool isFile() { return !ref->getName(false).beginsWith("MEMORY"); }

        FORCE_INLINE T& operator* () const { return *ref; }
        FORCE_INLINE T* operator->() const { return ref; }
        operator T() { return *ref; }
        operator bool() { return valid(); }

        // Comparison operators so that Refs can be stored in trees (maps)
        template <class T2> bool operator< (const Ref<T2>& other) const { return ref < other.ref; }
        template <class T2> bool operator< (const Ref<T2>&& other) const { return operator< (other); }
    };

protected:
    size_t refs = 0;
    Type type = Null;
    str name = "";

public:
    Resource(Type _type) : type(_type) {};
    virtual ~Resource() { refs = 0; conditionalReset(); }

    /// Returns the name of the resource
    FORCE_INLINE const str getName(bool stripState = true) const { return (stripState ? name.split<str>("~")[0] : name); }

// Function which must be overriden in derived classes
public:
    /// Function which cleans up after this resource
    virtual void reset() {};
    /// Function which uploads all of the data which this reference may need to send to the gpu
    virtual std::vector<Upload> upload(bool wait = true) { return {}; };
    /// Function which waits for any currently running uploads to finish
    static std::vector<Upload> waitUploads(std::vector<Upload>& runningUploads);

    /// Function which creates an empty version of this resource which can then be modified
    static Ref<Resource> create(const str name = "");
    /// Function which loads the resource from the input stream
    static Ref<Resource> load(std::istream&) { throw std::runtime_error("Base Resources can't be loaded! Load a particular type of resource!"); }
    FORCE_INLINE static Ref<Resource> load(std::istream&& file) { return load(file); }

protected:
    // Function which cleans up this resource once all references to it have been destroyed
    void conditionalReset();
};

#endif // __RESOURCE_BACKEND_H__
