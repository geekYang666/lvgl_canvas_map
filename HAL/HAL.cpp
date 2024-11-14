#include "HAL.h"

#if ESP_PLATFORM
#define PLATFORM_ROOT_DIR "/sdcard"
#elif WIN32
#define PLATFORM_ROOT_DIR "D:/esp32/SDCARD"
#else
#define PLATFORM_ROOT_DIR "/sdcard"
#endif

#define FONTS_DIR "/system/font/"

lv_font_t *fzdx_14;
GPS_Info_t gpsInfo;

void HAL_Init(){
    lv_group_set_default(lv_group_create());
    
    lv_display_t * disp = lv_sdl_window_create(LCD_WIDTH, LCD_HEIGHT);
    
    lv_indev_t * mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_display(mouse, disp);
    lv_display_set_default(disp);
    
    
    lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, disp);
    
    lv_indev_t * keyboard = lv_sdl_keyboard_create();
    lv_indev_set_display(keyboard, disp);
    lv_indev_set_group(keyboard, lv_group_get_default());
    
    fzdx_14 = lv_freetype_font_create(PLATFORM_ROOT_DIR FONTS_DIR"FZDX_GBK.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 14, LV_FREETYPE_FONT_STYLE_NORMAL);
    
    
}
