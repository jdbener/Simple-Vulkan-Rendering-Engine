#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "engine/common.hpp"
#include "engine/util/string.hpp"

#include <fstream>
#include <nytl/nonCopyable.hpp>

class Resource: public nytl::NonMovable {
friend class Ref;
friend class ResourceManager;
public:
    // Enum which provides reflection on what kind of reference this is.
    enum Type {Null = 0, Mesh, Material};
    /// Function which converts a resource type into a str
    static str type2str(Type type){
        switch(type){
        case Null: return "Null";
        case Mesh: return "Mesh";
        case Material: return "Material";
        }
    }

public:
    template <class T = Resource>
    class Ref {
    protected:
        T* ref = nullptr;

    public:
        Ref(Resource* _ref) : ref(reinterpret_cast<T*>(_ref)) { if(ref) ref->refs++; }
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

        FORCE_INLINE T& operator* () const { return *ref; }
        FORCE_INLINE T* operator->() const { return ref; }
        FORCE_INLINE operator T() { return *ref; }
        FORCE_INLINE operator bool() { return valid(); }
    };

protected:
    size_t refs = 0;
    Type type = Null;
    str name = "";

public:
    Resource(Type _type) : type(_type) {};
    virtual ~Resource() { refs = 0; conditionalReset(); }

    /// Returns the name of the
    FORCE_INLINE const str& getName() const { return name; }

// Function which must be overriden in derived classes
public:
    /// Function which cleans up after this resource
    virtual void reset() {};

    /// Function which creates an empty version of this resource which can then be modified
    static Ref<Resource> create(const str name = "");
    /// Function which loads the resource from the specified file
    static Ref<Resource> load(const str filepath) { std::ifstream fin(filepath); return load(fin); }
    /// Function which loads the resource from the input stream
    static Ref<Resource> load(std::istream& file) { throw std::runtime_error("Base Resources can't be loaded! Load a particular type of resource!"); }

protected:
    // Function which cleans up this resource once all references to it have been destroyed
    void conditionalReset();
};

#endif // __RESOURCE_H__
