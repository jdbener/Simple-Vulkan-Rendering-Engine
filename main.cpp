#include "engine/util/timer.h"

#include "engine/window.hpp"

std::ostream& operator <<(std::ostream& s, std::pair<int, int> size){
    s << "(" << size.first << ", " << size.second << ")";
    return s;
}

int main(){

    vk::ApplicationInfo appInfo("Project Delta", GAME_VERSION, "Delta Engine", ENGINE_VERSION, VK_API_VERSION_1_1);
    //vk::Instance i = vk::createInstance({{}, &appInfo, /*Layer Count*/ 0, /*Layer Names*/ nullptr, /*Extension Count*/ 0, /*Extension Names*/ nullptr});*/

    vpp::Instance instance({{}, &appInfo, /*Layer Count*/ 0, /*Layer Names*/ nullptr, /*Extension Count*/ 0, /*Extension Names*/ nullptr});

    Window w(instance, 800, 600, str(27));

    std::vector<const char*> ext = Window::vulkanExtensions();

    std::cout << (str("0F2a") + " - " + str::precision(7, 5) + " - " +  str::base(32, 16)) << std::endl;
    str s = u8"This is a string which I am writing. μ";

    do {
        std::cout << w.getTotalSize() << " - " << w.getFrameSize() << std::endl;
        Window::waitEvents();
        //Window::pollEvents();
    } while(!Window::allWindowsClosed());

    //std::cout << std::boolalpha << w.isDestroyed() << " - " << w.isClosed() << std::endl;

}
