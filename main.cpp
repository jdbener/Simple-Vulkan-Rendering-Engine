#include "engine/util/timer.h"

#include "engine/util/vulkan.hpp"
#include "engine/window.hpp"

std::ostream& operator <<(std::ostream& s, std::pair<int, int> size){
    s << "(" << size.first << ", " << size.second << ")";
    return s;
}

int main(){
    vk::ApplicationInfo appInfo("Project Delta", GAME_VERSION, "Delta Engine", ENGINE_VERSION, VK_API_VERSION_1_1);

    std::vector<const char*> layers = validateLayers({"VK_LAYER_KHRONOS_validation"});
    std::vector<const char*> extensions = validateExtensions(Window::requiredVulkanExtensions()
        + std::vector<const char*>{VK_EXT_DEBUG_UTILS_EXTENSION_NAME});

    vpp::Instance instance({{}, &appInfo,
        /*Layer Count*/ (uint32_t) layers.size(),
        /*Layer Names*/ layers.data(),
        /*Extension Count*/ (uint32_t) extensions.size(),
        /*Extension Names*/ extensions.data()});
    vpp::DebugMessenger debugMsg(instance);

    Window w(instance, 800, 600, str(27));
    Window w2(instance, 800, 600);




    str s = u8"This is a string which I am writing. μ";
    w.setName(s);
    w.setSize(400, 400);

    do {
        //std::cout << w.getTotalSize() << " - " << w.getFrameSize() << std::endl;
        w.swapBuffers();
        //Window::waitEvents();
        Window::pollEvents();
    // Wait for main window to be closed
    } while(!w.isClosed());

    //std::cout << std::boolalpha << w.isDestroyed() << " - " << w.isClosed() << std::endl;

}
