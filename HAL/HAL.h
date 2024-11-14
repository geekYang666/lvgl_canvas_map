#ifndef HAL_H
#define HAL_H
#include "HAL_Def.h"

#define LCD_WIDTH 320
#define LCD_HEIGHT 480

extern lv_font_t *fzdx_14;
extern GPS_Info_t gpsInfo;

void HAL_Init();


#endif //HAL_H
