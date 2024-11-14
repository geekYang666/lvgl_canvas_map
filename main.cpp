#define _DEFAULT_SOURCE /* needed for usleep() */
#include <unistd.h>
#include "lvgl/lvgl.h"

#include "HAL.h"
#include "CanvasMap.h"
#include "App.h"

#define main SDL_main

extern "C" int main(int argc, char *argv[]) {
    (void) argc; /*Unused*/
    (void) argv; /*Unused*/
#if WIN32
    system("chcp 65001 & cls"); //解决windows下printf中文乱码问题
#endif

    lv_init();
    HAL_Init();
    
    App app;
    app.run_map();
    
    while (1) {
        lv_timer_handler();
        usleep(10 * 1000);
    }
    
}
