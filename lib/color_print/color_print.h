#ifndef COLOR_PRINT_H
#define COLOR_PRINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#if ESP_PLATFORM
    #include <esp_timer.h>
    #include <esp_heap_caps.h>
    #define sysTick (esp_timer_get_time() / 1000ULL)
#elif WIN32
    #include "SDL2/SDL.h"
    #define sysTick SDL_GetTicks()
#else
    #include "SDL2/SDL.h"
    #define sysTick SDL_GetTicks()
#endif

#ifndef __FILE_NAME__
    #define __FILE_NAME__ (__builtin_strrchr( "/" __FILE__, '/') + 1)
#endif

#define RED_TEXT(format)  "\033[0;31m" format "\033[0m"
#define GREEN_TEXT(format)  "\033[0;32m" format "\033[0m"
#define YELLOW_TEXT(format)  "\033[0;33m" format "\033[0m"
#define BLUE_TEXT(format)  "\033[0;34m" format "\033[0m"
#define PURPLE_TEXT(format)  "\033[0;35m" format "\033[0m"
#define CYAN_TEXT(format)  "\033[0;36m" format "\033[0m"

#define LIGHT_RED_TEXT(format)  "\033[1;31m" format "\033[0m"
#define LIGHT_GREEN_TEXT(format)  "\033[1;32m" format "\033[0m"
#define LIGHT_YELLOW_TEXT(format)  "\033[1;33m" format "\033[0m"
#define LIGHT_BLUE_TEXT(format)  "\033[1;34m" format "\033[0m"
#define LIGHT_PURPLE_TEXT(format)  "\033[1;35m" format "\033[0m"
#define LIGHT_CYAN_TEXT(format)  "\033[1;36m" format "\033[0m"


#define DEFAULT_LOG_FORMAT \
"[%04d.%03d," \
RED_TEXT("+%04d")              \
"]["                           \
GREEN_TEXT("%s")               \
LIGHT_YELLOW_TEXT(":%d")                          \
"]->:"


#define printf_n(...) color_print(0, __FILE_NAME__, __LINE__, __func__, __VA_ARGS__)
#define printf_i(...) color_print(1, __FILE_NAME__, __LINE__, __func__, __VA_ARGS__)
#define printf_d(...) color_print(2, __FILE_NAME__, __LINE__, __func__, __VA_ARGS__)
#define printf_w(...) color_print(3, __FILE_NAME__, __LINE__, __func__, __VA_ARGS__)
#define printf_e(...) color_print(4, __FILE_NAME__, __LINE__, __func__, __VA_ARGS__)

void color_print(uint8_t level, const char *file, int line, const char *func, const char *format, ...);

#ifdef __cplusplus
}
#endif
#endif
