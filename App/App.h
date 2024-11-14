#ifndef APP_H
#define APP_H

#include "CanvasMap/CanvasMap.h"

class App {
public:
    App();
    
    ~App();
    
    void run_map();
    
    static void mapMoveX(int32_t pixelDiff);
    
    static void mapMoveY(int32_t pixelDiff);
    
    lv_obj_t *mapContainer{};
    
};


#endif //APP_H
