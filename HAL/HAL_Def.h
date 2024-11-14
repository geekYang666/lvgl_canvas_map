#ifndef HAL_DEF_H
#define HAL_DEF_H

#include <cstdint>
#include <cstring>
#include <string>
#include <map>
#include "lvgl.h"

using namespace std;

/* GPS */
typedef struct {
    double longitude;
    double latitude;
    float altitude;
    float heading;
    float speed;
    uint8_t usedSatNum;
    uint8_t totalSatNum;
    uint8_t fixedType;
    uint8_t avgCno;
    bool isValid;
} GPS_Info_t;

#endif
