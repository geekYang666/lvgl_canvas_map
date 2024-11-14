#include "App.h"
#include "src/display/lv_display_private.h"


//QPointF currentPosition(115.462528, 33.872506);
QPointF currentPosition(113.813819,22.721038) ; // /113.813819,22.721038
CanvasMap *mapDataPtr;

App::App() = default;

App::~App() = default;

void App::run_map() {
    
    //地图需要使用simple主题
    lv_display_t *disp = lv_display_get_default();
    if (lv_theme_simple_is_inited() == false) {
        disp->theme = lv_theme_simple_init(disp);
    } else {
        disp->theme = lv_theme_simple_get();
    }
    
    mapContainer = lv_obj_create(lv_screen_active());
    lv_obj_set_width(mapContainer, LCD_WIDTH);
    lv_obj_set_height(mapContainer, LCD_HEIGHT);
    lv_obj_set_style_bg_opa(mapContainer, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(mapContainer, lv_color_hex(0xeeeeee), LV_STATE_DEFAULT);
    lv_obj_clear_flag(mapContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(mapContainer);
    
    
    mapDataPtr = new CanvasMap();
    gpsInfo.longitude = currentPosition.x;
    gpsInfo.latitude = currentPosition.y;
    mapDataPtr->loadFile("china.map");
    mapDataPtr->Create(mapContainer);
    mapDataPtr->updatePosition(gpsInfo);
}


void App::mapMoveY(int32_t pixelDiff) {
    QPoint newPixelPoint = ll2PixelYX(mapDataPtr->_centerCoordRaw, mapDataPtr->_zoom, mapDataPtr->_tileSize);
    newPixelPoint.y += pixelDiff;
    printf_i("newPixelPoint:(%d,%d)", newPixelPoint.x, newPixelPoint.y);
    Coordinates newCoordinates = PixelXYToLatLong(newPixelPoint, mapDataPtr->_zoom, mapDataPtr->_tileSize);
    gpsInfo = gpsInfo;
    gpsInfo.longitude = newCoordinates.lon();
    gpsInfo.latitude = newCoordinates.lat();
    mapDataPtr->updatePosition(gpsInfo);
}

void App::mapMoveX(int32_t pixelDiff) {
    QPoint newPixelPoint = ll2PixelYX(mapDataPtr->_centerCoordRaw, mapDataPtr->_zoom, mapDataPtr->_tileSize);
    newPixelPoint.x += pixelDiff;
    printf_i("newPixelPoint:(%d,%d)", newPixelPoint.x, newPixelPoint.y);
    Coordinates newCoordinates = PixelXYToLatLong(newPixelPoint, mapDataPtr->_zoom, mapDataPtr->_tileSize);
    gpsInfo = gpsInfo;
    gpsInfo.longitude = newCoordinates.lon();
    gpsInfo.latitude = newCoordinates.lat();
    mapDataPtr->updatePosition(gpsInfo);
}

static bool flag = true;
