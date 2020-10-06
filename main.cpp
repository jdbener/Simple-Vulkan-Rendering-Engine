#include "engine/common.hpp"

#include <vpp/vpp.hpp>

#include <GLFW/glfw3.h>

#include "engine/util/timer.h"

int main(){

    std::cout << "Hello World!" << std::endl;
    vk::ApplicationInfo appInfo("Project Delta", GAME_VERSION, "Delta Engine", ENGINE_VERSION, VK_API_VERSION_1_1);
    //vk::Instance i = vk::createInstance({{}, &appInfo, /*Layer Count*/ 0, /*Layer Names*/ nullptr, /*Extension Count*/ 0, /*Extension Names*/ nullptr});*/

    vpp::Instance instance({{}, &appInfo, /*Layer Count*/ 0, /*Layer Names*/ nullptr, /*Extension Count*/ 0, /*Extension Names*/ nullptr});
    dlg_info("Crash?");
    dlg_warn("Crash?");
    dlg_error("Crash?");
    dlg_trace("Crash");
    dlg_debug("Crash?");
    dlg_fatal("Crash?");

    repeat(5){
        Timer t;
        std::cout << _i << std::endl;
        dlg_trace(str(instance));
        dlg_info(str(t.stop(true)));
    }
}
