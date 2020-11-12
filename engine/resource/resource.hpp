#pragma once
#ifndef __RESOURCE_H__
#define __RESOURCE_H__

// Data merging file which combines all of the needed template files of the backend
#include "backend/resource.hpp"
#include "backend/resourceManager.hpp"

// Templated implementation of Ref's valid function
//  NOTE: can't be included in backend's resource since it depends on the Resource-
//      Manager and creates a cyclic inclusion chain
template<class T>
bool Resource::Ref<T>::valid(){
    if(refValid())
        return ResourceManager::singleton()->loaded(ref->getName());
    return false;
}

#endif // __RESOURCE_H__
