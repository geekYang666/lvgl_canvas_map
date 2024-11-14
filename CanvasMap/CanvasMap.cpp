#include "CanvasMap.h"

#define DRAW_TILE_BOUND 1
#define USE_DRAW_LINE 0
#define DRAW_VIEW_AREA 0

int16_t CanvasMap::renderTileSize = 256;
lv_draw_line_dsc_t CanvasMap::drawLineDsc;
lv_draw_label_dsc_t CanvasMap::drawLabelDsc;
lv_draw_image_dsc_t CanvasMap::drawImageDsc;

Tag waterTag;
Tag residentialTag;
Tag industrialTag;
Tag buildingTag;
Tag parkTag;

vector<lv_fpoint_t> linePoints;

zoomInfo zoomList[] = {
    {5,  500000, "500Km"},
    {6,  200000, "200km"},
    {7,  100000, "100Km"},
    {8,  50000, "50Km"},
    {9,  20000, "20Km"},
    {10, 10000, "10Km"},
    {11, 5000,  "5Km"},
    {12, 3000,  "3Km"},
    {13, 1500,  "1.5Km"},
    {14, 500,   "500m"},
    {15, 250,   "250m"},
    {16, 100,   "100m"},
    {17, 50,    "50m"},
    {18, 30,    "30m"},
};

//vector <map_type_t> mapTypeList;
vector <map_type_t> mapTypeList
    = {
        {"矢量图",  "",       "",    12, 15, false, false},
        {"谷歌卫星", "GOOGLE", "jpg", 5,  15, true,  true},
        {"高德地图",   "GAODE",  "jpg", 5,  15, true,  true},
        {"OpenCycle",   "OpenCycle",  "jpg", 5,  15, true, false},
        
    };


int32_t parentWidth;
int32_t parentHeight;

bool operator==(lv_fpoint_t &point1, lv_fpoint_t &point2) { return (point1.x == point2.x && point1.y == point2.y); }

CanvasMap::CanvasMap() {
    mapTypeIndex = 0;
    mapChanged = false;
    currentMapType = mapTypeList[mapTypeIndex];
    _valid = false;
    _zoom = 14;
    _viewWidth = 240;
    _viewHeight = 240;
    _tileSize = 256;
    for (const auto &zoom: zoomList) {
        _zoomInfoMap[zoom.level] = zoom;
    }
}

CanvasMap::~CanvasMap() = default;

void CanvasMap::Create(lv_obj_t *parentOjb) {
    parent = parentOjb;
    
    parentWidth = lv_obj_get_style_width(parent, LV_PART_MAIN);
    parentHeight = lv_obj_get_style_height(parent, LV_PART_MAIN);
    
//    _viewWidth = 240;
//    _viewHeight = 240;
    _viewWidth = parentWidth;
    _viewHeight = parentHeight;
    
    _canvasWidth = renderTileSize * (_viewWidth / renderTileSize + 2);
    _canvasHeight = renderTileSize * (_viewHeight / renderTileSize + 2);
    
    
    draw_buf = lv_draw_buf_create(_canvasWidth, _canvasHeight, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    canvas = lv_canvas_create(parent);
    lv_canvas_set_draw_buf(canvas, draw_buf);
    
    
    lv_canvas_init_layer(canvas, &layer);

//    lv_vector_dsc_set_blend_mode(ctx, LV_VECTOR_BLEND_NONE);
    
    
    lv_draw_label_dsc_init(&drawLabelDsc);
    
    lv_draw_image_dsc_init(&drawImageDsc);

    font_zh = fzdx_14;
    
    drawLabelDsc.color = lv_palette_main(LV_PALETTE_RED);
    drawLabelDsc.font = font_zh;
    drawLabelDsc.align = LV_TEXT_ALIGN_CENTER;
    drawLabelDsc.ofs_x = -80;
    drawLabelDsc.ofs_y = -7;
    drawLabelDsc.color = lv_color_hex(0xff0000);
    
    lv_draw_line_dsc_init(&drawLineDsc);
    drawLineDsc.round_start = 1;
    drawLineDsc.round_end = 1;
    
    
    //画可视区域边界
//    lv_obj_t *viewAreaObj = lv_obj_create(parent);
//
//    lv_obj_set_size(viewAreaObj, _viewWidth, _viewHeight);
//    lv_obj_set_style_border_width(viewAreaObj, 1, 0);
//    lv_obj_set_style_radius(viewAreaObj, 0, 0);
//    lv_obj_set_style_bg_opa(viewAreaObj, LV_OPA_TRANSP, 0);
//    lv_obj_set_style_border_color(viewAreaObj, lv_color_hex(0x0000ff), 0);
//    lv_obj_align(viewAreaObj, LV_ALIGN_CENTER, 0, 0);
    
    
    //比例尺
    scaleRuler = lv_obj_create(parent);
    
    rulerColor = lv_color_hex3(0x333333);
    
    lv_obj_set_size(scaleRuler, 60, 5);
    lv_obj_set_style_border_width(scaleRuler, 1, 0);
    lv_obj_set_style_radius(scaleRuler, 0, 0);
    lv_obj_set_style_bg_opa(scaleRuler, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_side(scaleRuler, lv_border_side_t (LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT), 0);
    lv_obj_align(scaleRuler, LV_ALIGN_BOTTOM_RIGHT, -5, -7);
    
    //比例尺文字
    scaleRulerLabel = lv_label_create(parent);
    lv_label_set_text(scaleRulerLabel, "500m");
    lv_obj_set_style_text_color(scaleRulerLabel, rulerColor, LV_STATE_DEFAULT);
    lv_obj_align_to(scaleRulerLabel, scaleRuler, LV_ALIGN_OUT_TOP_MID, 0, 0);
    
    //级别文字
    levelLabel = lv_label_create(parent);
    lv_obj_set_style_bg_opa(levelLabel, LV_OPA_TRANSP, 0);
    lv_obj_align(levelLabel, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    lv_obj_set_style_border_color(levelLabel, rulerColor, 0);
    lv_obj_set_style_text_color(levelLabel, rulerColor, LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(levelLabel, font_zh, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(levelLabel, "%s LV:%d", mapTypeList[mapTypeIndex].name.c_str(), _zoom);
    
    //加载文字
    
    loadingLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(loadingLabel, font_zh, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(loadingLabel, lv_color_black(), LV_STATE_DEFAULT);
    lv_label_set_text(loadingLabel, "地图加载中...");
    lv_obj_align(loadingLabel, LV_ALIGN_CENTER, 0, 0);
    
    // 地图中心点
    iconArrow = lv_image_create(parent);
    lv_image_set_src(iconArrow, &img_src_gps_arrow_default);
    lv_obj_align(iconArrow, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(iconArrow, LV_OBJ_FLAG_HIDDEN);
    
    //指北针
    iconCompass = lv_image_create(parent);
    
    lv_image_set_src(iconCompass, &icon_compass);
    lv_obj_align(iconCompass, LV_ALIGN_TOP_LEFT, 10, 20);
    
    lv_obj_center(canvas);
    
    lv_timer_handler();
    
}

void CanvasMap::updateVectorTile(bool needRender) {
    const SubFileInfo &f = _subFiles.at(level(_zoom));
    
    //计算当前zoomLayer的mapPixel
    QPoint newMapPixelPoint = OSM::ll2PixelYX(_centerCoord, _zoom, renderTileSize);
    
    QPoint viewAreaTileTopLeft(
        (newMapPixelPoint.x - _viewWidth / 2) / renderTileSize,
        (newMapPixelPoint.y - _viewHeight / 2) / renderTileSize
    );
    QPoint viewAreaTileBottomRight(
        (newMapPixelPoint.x + _viewWidth / 2) / renderTileSize,
        (newMapPixelPoint.y + _viewHeight / 2) / renderTileSize
    );

//    printf_i("viewAreaTileTopLeft:(%d,%d)", viewAreaTileTopLeft.x, viewAreaTileTopLeft.y);
//    printf_i("viewAreaTileBottomRight:(%d,%d)", viewAreaTileBottomRight.x, viewAreaTileBottomRight.y);
    
    //计算baseZoomLayer的tileXY
    
    QPoint baseZoomTileTopLeft(
        viewAreaTileTopLeft.x >> (_zoom - f.base),
        viewAreaTileTopLeft.y >> (_zoom - f.base)
    );
    QPoint baseZoomTileBottomRight(
        viewAreaTileBottomRight.x >> (_zoom - f.base),
        viewAreaTileBottomRight.y >> (_zoom - f.base)
    );
    
    bool smallZoom = _zoom - f.base < 0;
    
    if (smallZoom) {
        uint16_t realRenderSize = renderTileSize >> (f.base - _zoom);
        QPoint baseZoomPixelPoint(OSM::ll2PixelYX(_centerCoord, f.base, realRenderSize));
        
        baseZoomTileTopLeft.x = (baseZoomPixelPoint.x - _viewWidth / 2) / realRenderSize;
        baseZoomTileTopLeft.y = (baseZoomPixelPoint.y - _viewHeight / 2) / realRenderSize;
        
        baseZoomTileBottomRight.x = (baseZoomPixelPoint.x + _viewWidth / 2) / realRenderSize;
        baseZoomTileBottomRight.y = (baseZoomPixelPoint.y + _viewHeight / 2) / realRenderSize;
    }


//    printf_i("baseZoomTileTopLeft:(%d,%d)", baseZoomTileTopLeft.x, baseZoomTileTopLeft.y);
//    printf_i("baseZoomTileBottomRight:(%d,%d)", baseZoomTileBottomRight.x, baseZoomTileBottomRight.y);
    
    QPoint tileBottomRight(viewAreaTileTopLeft.x + 1, viewAreaTileTopLeft.y + 1);
    
    Coordinates firstTileTopLeftCoords(OSM::tile2ll(viewAreaTileTopLeft, _zoom));
    Coordinates firstTileBottomRightCoords(OSM::tile2ll(tileBottomRight, _zoom));
    
    firstTileTopLeftCoords = Coordinates(firstTileTopLeftCoords.lon(), -firstTileTopLeftCoords.lat());
    firstTileBottomRightCoords = Coordinates(firstTileBottomRightCoords.lon(), -firstTileBottomRightCoords.lat());
    
    
    QPoint firstTileTopLeftPixelPoint = OSM::ll2PixelYX(firstTileTopLeftCoords, _zoom, renderTileSize);
    
    centerPoint.x = float(newMapPixelPoint.x - firstTileTopLeftPixelPoint.x);
    centerPoint.y = float(newMapPixelPoint.y - firstTileTopLeftPixelPoint.y);


#if DRAW_VIEW_AREA
    startPoint.x = centerPoint.x - renderTileSize / 2;
    startPoint.y = centerPoint.y - renderTileSize / 2;
    endPoint.x = centerPoint.x + renderTileSize / 2;
    endPoint.y = centerPoint.y + renderTileSize / 2;
#else
    startPoint.x = 0;
    startPoint.y = 0;
    endPoint.x = renderTileSize * 2;
    endPoint.y = renderTileSize * 2;
#endif
    bool shouldRender = false;
    
    if (_mapCenterPixelXY != newMapPixelPoint) {

#if DRAW_VIEW_AREA
        shouldRender = true;
#endif
        _mapCenterPixelXY = newMapPixelPoint;
        
        //设置canvas偏移值
        canvasOffsetX = (parentWidth / 2 - centerPoint.x);
        canvasOffsetY = (parentHeight / 2 - centerPoint.y);
//        printf_i("_centerCoord:(%.04f,%.04f) canvas偏移:(%d,%d)", _centerCoord.lon(), _centerCoord.lat(), canvasOffsetX, canvasOffsetY);
        lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, canvasOffsetX, canvasOffsetY);
    }
    
    if(mapChanged){
        lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, canvasOffsetX, canvasOffsetY);
    }
    
    bool needReadTile = baseZoomTileTopLeft != _baseZoomTileBounds[0] || baseZoomTileBottomRight != _baseZoomTileBounds[1];
    
    if (needReadTile) {
        printf_i("读取地图:(%f,%f)", _centerCoord.lon(), _centerCoord.lat());
        _baseZoomTileBounds[0] = baseZoomTileTopLeft;
        _baseZoomTileBounds[1] = baseZoomTileBottomRight;
        readTiles();
    }
#if !DRAW_VIEW_AREA
    shouldRender = (viewAreaTileTopLeft != _viewAreaTileBounds[0] || viewAreaTileBottomRight != _viewAreaTileBounds[1] || (smallZoom && needReadTile) || mapChanged);
#endif
    
    
    if (shouldRender) {
        printf_i("绘制地图开始");
        _viewAreaTileBounds[0] = viewAreaTileTopLeft;
        _viewAreaTileBounds[1] = viewAreaTileBottomRight;
        _firstTileTopLeft = firstTileTopLeftCoords;
        _firstTileBottomRight = firstTileBottomRightCoords;
        renderVectorMap();
    }
}

void CanvasMap::readTiles() {
    
    const SubFileInfo &subFileInfo = _subFiles.at(level(_zoom));
    
    currentSubFile = SubFile(&_mapFile, subFileInfo.offset, subFileInfo.size);
    
    QPoint mapTileTopLeft(OSM::ll2tile(_baseMapBounds.topLeft(), subFileInfo.base));
    QPoint mapTileBottomRight(OSM::ll2tile(_baseMapBounds.bottomRight(), subFileInfo.base));
    
    QPoint tileTopLeft = _baseZoomTileBounds[0];
    QPoint tileBottomRight = _baseZoomTileBounds[1];
    
    //删除不在可视范围的tile
    for (auto it = _tiles.begin(); it != _tiles.end();) {
        if (*it < tileTopLeft || *it > tileBottomRight) {
            it = _tiles.erase(it);
        } else {
            ++it;
        }
    }
    
    int tileX;
    int tileY;
    
    uint32_t offsetAddr;
    uint32_t mapXTileNum = mapTileBottomRight.x - mapTileTopLeft.x + 1;
    uint32_t xTileIndex;
    uint32_t yTileIndex;
    uint32_t relativeTileIndex;

//    uint8_t readZoom = _zoom ;
    uint8_t readZoom = _zoom > 14 ? 14 : _zoom;
    
    //读取tile
    for (tileY = tileTopLeft.y; tileY <= tileBottomRight.y; tileY++) {
        for (tileX = tileTopLeft.x; tileX <= tileBottomRight.x; tileX++) {
            VectorTile currentTile(tileX, tileY);
            vector<VectorTile>::iterator it;
            it = find(_tiles.begin(), _tiles.end(), currentTile);
            if (it == _tiles.end()) {
                //tiles 中没有缓存 要读取的tile

//                判断是否超出地图边界
                currentTile.overflowBounds = currentTile < mapTileTopLeft || currentTile > mapTileBottomRight;
                
                if (!currentTile.overflowBounds) {
                    
                    xTileIndex = currentTile.x - mapTileTopLeft.x;
                    yTileIndex = currentTile.y - mapTileTopLeft.y;
//                printf_i("xTileIndex %d yTileIndex %d\n", xTileIndex, yTileIndex);
                    
                    relativeTileIndex = (yTileIndex * mapXTileNum + xTileIndex);
                    offsetAddr = relativeTileIndex * 5 + subFileInfo.offset;
//                printf_i("offsetAddr is %d\n", offsetAddr);
                    
                    
                    Coordinates currentTileTopLeft(OSM::tile2ll(QPoint((int) currentTile.x, (int) currentTile.y), subFileInfo.base));
                    currentTileTopLeft = Coordinates(currentTileTopLeft.lon(), -currentTileTopLeft.lat());
                    
                    uint64_t tileOffset;
                    bool readRes;
                    
                    lv_fs_seek(&_mapFile, offsetAddr, LV_FS_SEEK_SET);
                    currentSubFile.readTileOffset(tileOffset);

//                printf_i("tile_lookup_offset is %llu\n", tileOffset);
                    
                    currentTile.offset = tileOffset;
                    currentTile.pos = currentTileTopLeft;
                    
                    readRes = readPaths(subFileInfo, currentSubFile, &currentTile, readZoom, &currentTile.paths, _pathTags);
                    currentTile.readRes = readRes;
                    for (auto &pathItem: currentTile.paths) {
                        if (matchTag(pathItem, industrialTag)) {
                            pathItem.layer = 0;
                        }
                        if (matchTag(pathItem, residentialTag)) {
                            pathItem.layer = 1;
                        }
                        if (matchTag(pathItem, buildingTag)) {
                            pathItem.layer = 2;
                        }
                        if (matchTag(pathItem, parkTag)) {
                            pathItem.layer = 2;
                        }
                        if (matchTag(pathItem, waterTag)) {
                            pathItem.layer = 3;
                        }
                    }

//                    readRes = readPoints(subFileInfo, currentSubFile, &currentTile, _zoom, &currentTile.points, _pointTags);
                    printf_d("read Tile:(%d,%d),res=%d", tileX, tileY, readRes);
                } else {
//                    printf_i("tile:(%d,%d)不在地图范围",tileX,tileY);
                }
//                printf_i("tile:(%d,%d)不在可视范围",tileX,tileY);
                std::sort(currentTile.paths.begin(), currentTile.paths.end());
                
                _tiles.push_back(currentTile);
            } else {
//                printf_i("tile:(%d,%d)在可视范围",tileX,tileY);
            }
        }
    }
}

void CanvasMap::renderVectorMap() {
    ctx = lv_vector_dsc_create(&layer);
    path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_LOW);
    fillCanvasBG();
    renderVectorTile();
    lv_draw_vector(ctx); // submit draw
    lv_canvas_finish_layer(canvas, &layer);
#if DRAW_TILE_BOUND
    drawTileBounds();
    printf_d("绘制瓦片边框完成");
#endif
    renderPathText();
//    printf_d("绘制路径文字完成");
    lv_vector_path_delete(path);
    lv_vector_dsc_delete(ctx);
    printf_e("绘制地图完成");
}

void CanvasMap::renderVectorTile() {
    
    //清空已绘制文字坐标
    vector<LabelInfo> newPoints;
    newPoints.swap(_namePoints);
    
    //第一步画区域 水域 住宅区等
    
    lv_vector_dsc_set_stroke_width(ctx, 0);
    lv_vector_dsc_set_stroke_opa(ctx, LV_OPA_TRANSP);
    
    for (const auto &tile: _tiles) {
        uint32_t pathIndex = 0;
        if(!tile.readRes) continue;
        for (const auto &pathItem: tile.paths) {
//            if(isType(pathItem, _keys["landuse"])){
//                string tempStr;
//                for (auto &tag: pathItem.tags) {
//                    tempStr += "(" + _keysSwap[tag.key] + ":" + tag.value + "),";
//                }
//                printf_i("pathItem[%d] tags:%s", pathIndex, tempStr.c_str());
//            }
            
            if (matchTag(pathItem, waterTag)) {
                lv_vector_dsc_set_fill_color(ctx, lv_color_hex(0x5589da));
                renderPolygonSingle(pathItem, false);
            }
            if (matchTag(pathItem, industrialTag)) {
                lv_vector_dsc_set_fill_color(ctx, lv_color_hex(0xC7D6D6));
                renderPolygonSingle(pathItem, false);
            }
            if (matchTag(pathItem, residentialTag)) {
                lv_vector_dsc_set_fill_color(ctx, lv_color_hex(0xDDDDDD));
                renderPolygonSingle(pathItem, false);
            }
            if (matchTag(pathItem, buildingTag)) {
                lv_vector_dsc_set_fill_color(ctx, lv_color_hex(0xE3C6A6));
                renderPolygonSingle(pathItem, false);
            }
            if (matchTag(pathItem, parkTag)) {
                lv_vector_dsc_set_fill_color(ctx, lv_color_hex(0x83ff8d));
                renderPolygonSingle(pathItem, false);
            }
            pathIndex++;
        }
    }
//    printf_d("绘制区域完成");

//    第二步画河流
    
    lv_vector_dsc_set_fill_opa(ctx, LV_OPA_0);
    lv_vector_dsc_set_stroke_opa(ctx, LV_OPA_COVER);
    lv_vector_dsc_set_stroke_color(ctx, lv_color_hex(0x5589da));
    lv_vector_dsc_set_stroke_width(ctx, 4.0f);
    
    
    for (const auto &tile: _tiles) {
        if(!tile.readRes) continue;
        
        for (const auto &pathItem: tile.paths) {
            if (isType(pathItem, _keys["waterway"]))
                renderRiver(pathItem);
        }
    }
//    printf_d("绘制河流完成");

#if USE_DRAW_LINE
    lv_draw_vector(ctx); // submit draw
    lv_canvas_finish_layer(canvas, &layer);
#endif
    
    //    第三步画路径黑线
    for (const auto &tile: _tiles) {
        if(!tile.readRes) continue;
        
        uint16_t highWayCount = 0;
        uint16_t bridgeCount = 0;
        uint16_t pathIndex = 0;
        for (const auto &pathItem: tile.paths) {
            
            bool shouldRender = pathShouldRender(pathItem);
            
            string tagStr;
            for (auto &tag: pathItem.tags) {
                tagStr += ("(" + to_string(tag.key) + ":" + tag.value + ")");
            }
//            printf_i("pathItem[%d] shouldRender:%d tags=%s ", pathIndex,shouldRender, tagStr.c_str());
            
            if (isType(pathItem, _keys["highway"]) && shouldRender) {
                
                highWayCount++;
                drawLineDsc.color = lv_color_hex(0x333333);
                drawLineDsc.width = 5;
                lv_vector_dsc_set_stroke_width(ctx, 5.0f);
                lv_vector_dsc_set_stroke_color(ctx, lv_color_hex(0x333333));
                lv_vector_dsc_set_stroke_cap(ctx, LV_VECTOR_STROKE_CAP_ROUND);
                renderPathSingle(pathItem, false);
//
            }
            pathIndex++;
//            if (isType(pathItem, _keys["bridge"]) && shouldRender) {
//                bridgeCount++;
//                drawLineDsc.color = lv_color_hex(0x333333);
//                drawLineDsc.width = 7;
//                lv_vector_dsc_set_stroke_width(ctx, 7.0f);
//                lv_vector_dsc_set_stroke_color(ctx, lv_color_hex(0x000000));
//                lv_vector_dsc_set_stroke_cap(ctx, LV_VECTOR_STROKE_CAP_SQUARE);
//                renderPathSingle(pathItem, false);
//            }
        }
//        printf_d("tile:[%d,%d] has %d highWay %d Bridge", tile.x, tile.y, highWayCount, bridgeCount);
    }

//    printf_d("绘制路径边线完成");
    
    //    第四步画路径内部黄线
    
    lv_vector_dsc_set_stroke_color(ctx, lv_color_hex(0xf7faaa));
    lv_vector_dsc_set_stroke_width(ctx, 3.0f);
    
    drawLineDsc.color = lv_color_hex(0xf7faaa);
    drawLineDsc.width = 3;
    for (const auto &tile: _tiles) {
        if(!tile.readRes) continue;
        
        for (const auto &pathItem: tile.paths) {
            if (isType(pathItem, _keys["highway"]) && pathShouldRender(pathItem)) {
                renderPathSingle(pathItem, true);
            }
        }
    }
//    printf_d("绘制路径内线完成");

}


/*绘制路径的文字标签*/
void CanvasMap::renderPathText() {
    drawLabelDsc.align = LV_TEXT_ALIGN_CENTER;
    drawLabelDsc.ofs_x = -50;
    
    uint8_t halfWidth = arrow_up_12.header.w / 2;
    uint8_t halfHeight = arrow_up_12.header.h / 2;
    
    for (const auto &labelItem: _namePoints) {
        lv_area_t labelCoords = {labelItem.pos.x, labelItem.pos.y, labelItem.pos.x + 100, labelItem.pos.y + 16};
        drawLabelDsc.font = font_zh;
        drawLabelDsc.color = lv_color_black();
        drawLabelDsc.text = labelItem.name.c_str();
        drawLabelDsc.ofs_y = -20;
        lv_draw_label(&layer, &drawLabelDsc, &labelCoords);
        
        
        labelCoords = {labelItem.pos.x - halfWidth, labelItem.pos.y - halfHeight, labelItem.pos.x + halfWidth, labelItem.pos.y + halfHeight};
        
        drawImageDsc.src = &arrow_up_12;
        lv_draw_image(&layer, &drawImageDsc, &labelCoords);
        lv_canvas_finish_layer(canvas, &layer);
        
    }
    
}

void CanvasMap::fillCanvasBG() {
    //重新填充底色
    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);
    lv_area_t rect = {0, 0, static_cast<int32_t>(_canvasWidth), static_cast<int32_t>(_canvasHeight)};
    lv_vector_path_append_rect(path, &rect, 0, 0);
    lv_vector_dsc_set_fill_color(ctx, lv_color_hex(0xFFFFFF));
    lv_vector_dsc_set_fill_opa(ctx, LV_OPA_COVER);
    lv_vector_clear_area(ctx, &rect); // clear screen
    
    lv_vector_dsc_add_path(ctx, path); // draw a path
    lv_canvas_finish_layer(canvas, &layer);
}

void CanvasMap::renderPolygonSingle(const Path &pathItem, bool drawName) {
    uint32_t polyLen = pathItem.poly.size();
    for (int i = 0; i < polyLen; i++) {
        auto coords = pathItem.poly.at(i);
        
        uint32_t pointLen;
        calcPathPoints(pointLen, pathItem.poly._paths[i]);
        bool shouldDraw = pathHasPointVA(pointLen);
        
        if (!shouldDraw) {
            continue;
        }
//        string tempStr;
//        for (auto &tag: pathItem.tags) {
//            tempStr += ("("+ _keysSwap[tag.key]+"="+tag.value + "),");
//        }
//        printf("Polygon tags:%s pointLen=%d\n", tempStr.c_str(), pointLen);
        lv_vector_path_clear(path);
        lv_vector_dsc_identity(ctx);
        lv_vector_path_move_to(path, &linePoints[0]);
        for (int pointIndex = 1; pointIndex < pointLen; pointIndex++) {
            lv_vector_path_line_to(path, &linePoints[pointIndex]);
        }
        lv_vector_path_close(path);
        lv_vector_dsc_add_path(ctx, path);
    }
}

void CanvasMap::renderPathSingle(const Path &pathItem, bool drawName) {
    for (size_t i = 0; i < pathItem.poly._paths.size(); i++) {
        uint32_t pointLen;
        calcPathPoints(pointLen, pathItem.poly._paths[i]);
//        string tempStr;
//        for (auto &tag: pathItem.tags) {
//            tempStr += ("("+ _keysSwap[tag.key]+"="+tag.value + "),");
//        }
//        printf("path tags:%s pointLen=%d\n", tempStr.c_str(), pointLen);
        
        bool shouldDraw = pathHasPointVA(pointLen);
        
        if (!shouldDraw) {
            continue;
        }
#if USE_DRAW_LINE
        //使用draw_line方式画线
        for (int j = 0; j < pointLen - 1; j++) {
            drawLineDsc.p1.x = (int) linePoints[j].x;
            drawLineDsc.p1.y = (int) linePoints[j].y;
            drawLineDsc.p2.x = (int) linePoints[j + 1].x;
            drawLineDsc.p2.y = (int) linePoints[j + 1].y;
            lv_draw_line(&layer, &drawLineDsc);
        }
#else
        lv_vector_path_clear(path);
        lv_vector_dsc_identity(ctx);
        lv_vector_path_move_to(path, &linePoints[0]);
        for (int j = 1; j < pointLen; j++) {
            lv_vector_path_line_to(path, &linePoints[j]);
        }
        lv_vector_dsc_add_path(ctx, path); // draw a path
#endif
        
        
        if (drawName) {
            string pathName;
            
            if (findName(pathItem.tags, &pathName)) {
                
                uint16_t halfPos = 0;
                
                lv_point_t namePoint = {0, 0};
                
                if (pointLen == 2) {
                    namePoint.x = int32_t((linePoints[0].x + linePoints[1].x) / 2);
                    namePoint.y = int32_t((linePoints[0].y + linePoints[1].y) / 2);
                } else {
                    if (pointLen % 2 == 0) {
                        halfPos = pointLen / 2 - 1;
                        namePoint.x = int32_t((linePoints[halfPos].x + linePoints[halfPos + 1].x) / 2);
                        namePoint.y = int32_t((linePoints[halfPos].y + linePoints[halfPos + 1].y) / 2);
                    } else {
                        halfPos = (pointLen - 1) / 2;
                        namePoint.x = int32_t(linePoints[halfPos].x);
                        namePoint.y = int32_t(linePoints[halfPos].y);
                    }
                }
//                printf_i("pathName:%s pos:(%d,%d)", pathName.c_str(), namePoint.x, namePoint.y);
                
                lv_area_t labelCoords = {namePoint.x, namePoint.y, namePoint.x + 100, namePoint.y + 16};
                
                vector<LabelInfo>::iterator it;
                it = find(_namePoints.begin(), _namePoints.end(), namePoint);
                if (it == _namePoints.end()) {
                    
                    LabelInfo newInfo(pathName, namePoint);
                    
                    _namePoints.push_back(newInfo);
                }
            }
        }
    }
    
}

void CanvasMap::renderRiver(const Path &pathItem) {
    uint32_t polyLen = pathItem.poly.size();
    for (int i = 0; i < polyLen; i++) {
        auto coords = pathItem.poly.at(i);
        uint32_t pointLen;
        calcPathPoints(pointLen, pathItem.poly._paths[i]);
        
        bool shouldDraw = pathHasPointVA(pointLen);
        
        if (!shouldDraw) {
            continue;
        }
        
        lv_vector_path_clear(path);
        lv_vector_dsc_identity(ctx);
        lv_vector_path_move_to(path, &linePoints[0]);
        for (int j = 1; j < pointLen; j++) {
            lv_vector_path_line_to(path, &linePoints[j]);
        }
        lv_vector_dsc_add_path(ctx, path); // draw a path
    }
}


void CanvasMap::calcPathPoints(uint32_t &pointLen, const vector<Coordinates> &coords) {
    vector<lv_fpoint_t> newPoints;
    newPoints.swap(linePoints);
    pointLen = 0;
    lv_fpoint_t currPoint{};
    for (auto &coordItem: coords) {
        currPoint.x = (coordItem.lon() - _firstTileTopLeft.lon()) / (_firstTileBottomRight.lon() - _firstTileTopLeft.lon()) * float(renderTileSize);
        currPoint.y = (coordItem.lat() - _firstTileTopLeft.lat()) / (_firstTileBottomRight.lat() - _firstTileTopLeft.lat()) * float(renderTileSize);
        linePoints.push_back(currPoint);
        pointLen += 1;
    }
}

bool CanvasMap::pathHasPointVA(uint32_t pointLen) const {

//    return std::any_of(linePoints.begin(), linePoints.end(), [this](lv_fpoint_t linePoint) { return linePoint.x >= startPoint.x && linePoint.x <= endPoint.x && linePoint.y >= startPoint.y && linePoint.y <= endPoint.y; });

//    size_t pointIndex=0;
//    for (auto &linePoint: linePoints) {
//        if (linePoint.x >= startPoint.x && linePoint.x <= endPoint.x && linePoint.y >= startPoint.y && linePoint.y <= endPoint.y) {
//            return true;
//        }
////        if()
//
//    }
    return true;
}


void CanvasMap::updateImageTile() {
    //计算当前zoomLayer的mapPixel
    QPoint newMapPixelPoint = OSM::ll2PixelYX(_centerCoord, _zoom, renderTileSize);
    
    QPoint viewAreaTileTopLeft(
        (newMapPixelPoint.x - _viewWidth / 2) / renderTileSize,
        (newMapPixelPoint.y - _viewHeight / 2) / renderTileSize
    );
    QPoint viewAreaTileBottomRight(
        (newMapPixelPoint.x + _viewWidth / 2) / renderTileSize,
        (newMapPixelPoint.y + _viewHeight / 2) / renderTileSize
    );
    
    Coordinates firstTileTopLeftCoords(OSM::tile2ll(viewAreaTileTopLeft, _zoom));
    firstTileTopLeftCoords = Coordinates(firstTileTopLeftCoords.lon(), -firstTileTopLeftCoords.lat());
    QPoint firstTileTopLeftPixelPoint = OSM::ll2PixelYX(firstTileTopLeftCoords, _zoom, renderTileSize);
    
    if (viewAreaTileTopLeft != _viewAreaTileBounds[0] || viewAreaTileBottomRight != _viewAreaTileBounds[1] || mapChanged) {
        _viewAreaTileBounds[0] = viewAreaTileTopLeft;
        _viewAreaTileBounds[1] = viewAreaTileBottomRight;
        
        renderImageTile();
#if DRAW_TILE_BOUND
        printf_d("renderTileBounds start");
        ctx = lv_vector_dsc_create(&layer);
        path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_LOW);
        drawTileBounds();
#endif
    }
    
    if (_mapCenterPixelXY != newMapPixelPoint) {
        _mapCenterPixelXY = newMapPixelPoint;
        
        lv_point_t centerPoint;
        centerPoint.x = int16_t(_mapCenterPixelXY.x - firstTileTopLeftPixelPoint.x);
        centerPoint.y = int16_t(_mapCenterPixelXY.y - firstTileTopLeftPixelPoint.y);

//        printf_i("需要移动位置 canvas offset is (%d,%d)", canvasOffsetX, canvasOffsetY);
        //设置canvas偏移值
        canvasOffsetX = (parentWidth / 2 - centerPoint.x);
        canvasOffsetY = (parentHeight / 2 - centerPoint.y);
        lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, canvasOffsetX, canvasOffsetY);
    }
    
}

void CanvasMap::renderImageTile() {
    
    QPoint tileTopLeft = _viewAreaTileBounds[0];
    QPoint tileBottomRight = _viewAreaTileBounds[1];
    int tileX;
    int tileY;
    
    char imgPath[100];
    
    lv_canvas_fill_bg(canvas, lv_color_hex(0xDDDDDD), LV_OPA_COVER);
    
    printf_w("renderImageTile() start");
    for (tileY = tileTopLeft.y; tileY <= tileBottomRight.y; tileY++) {
        for (tileX = tileTopLeft.x; tileX <= tileBottomRight.x; tileX++) {
            int x = tileX - tileTopLeft.x;
            int y = tileY - tileTopLeft.y;
            
            memset(imgPath, 0, 100);
            sprintf(imgPath, "/maps/%s/%d/%d/%d.%s", currentMapType.mapPath.c_str(), _zoom, tileX, tileY, currentMapType.format.c_str());
            
            lv_area_t coords = {x * renderTileSize, y * renderTileSize, (x + 1) * renderTileSize,
                                (y + 1) * renderTileSize};
            
            drawImageDsc.src = imgPath;
            lv_draw_image(&layer, &drawImageDsc, &coords);
            lv_canvas_finish_layer(canvas, &layer);
            printf_i("加载图片:%s", imgPath);
        }
    }
    printf_w("renderImageTile() end");
    
}

void CanvasMap::drawTileBounds() {
    
    QPoint mapTileTopLeft(OSM::ll2tile(_baseMapBounds.topLeft(), _zoom));
    QPoint mapTileBottomRight(OSM::ll2tile(_baseMapBounds.bottomRight(), _zoom));
    
    QPoint tileTopLeft = _viewAreaTileBounds[0];
    QPoint tileBottomRight = _viewAreaTileBounds[1];
    
    //画地图边框
    lv_fpoint_t firstTilePoints[5] = {
        {0,                                  0},
        {static_cast<float>(renderTileSize), 0},
        {static_cast<float>(renderTileSize), static_cast<float>(renderTileSize)},
        {0,                                  static_cast<float>(renderTileSize)},
        {0,                                  0}
    };
    
    lv_fpoint_t renderTileBoundPoints[5];
    lv_area_t labelCoords;
    lv_point_t labelStartPoint;
    
    lv_vector_dsc_set_fill_opa(ctx, LV_OPA_0);
    lv_vector_dsc_set_stroke_opa(ctx, LV_OPA_COVER);
    lv_vector_dsc_set_stroke_width(ctx, 1);
    lv_vector_dsc_set_stroke_color(ctx, lv_color_hex(0xFF0000));
    lv_vector_dsc_set_stroke_cap(ctx, LV_VECTOR_STROKE_CAP_ROUND);
    
    int tileX;
    int tileY;
    
    tileLabels.clear();
    
    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = lv_color_hex(0xFF0000);
    dsc.width = 1;
    dsc.round_end = 1;
    dsc.round_start = 1;
    
    for (tileY = tileTopLeft.y; tileY <= tileBottomRight.y; tileY++) {
        for (tileX = tileTopLeft.x; tileX <= tileBottomRight.x; tileX++) {
            int x = tileX - tileTopLeft.x;
            int y = tileY - tileTopLeft.y;
            
            QPoint currentTile(tileX, tileY);
            bool overflowBounds = currentTile < mapTileTopLeft || currentTile > mapTileBottomRight;
            
            for (int i = 0; i < 5; i++) {
                renderTileBoundPoints[i].x = firstTilePoints[i].x + float(x * renderTileSize);
                renderTileBoundPoints[i].y = firstTilePoints[i].y + float(y * renderTileSize);
            }
            
            for (int j = 0; j < 4; j++) {
                dsc.p1.x = (int) renderTileBoundPoints[j].x;
                dsc.p1.y = (int) renderTileBoundPoints[j].y;
                dsc.p2.x = (int) renderTileBoundPoints[j + 1].x;
                dsc.p2.y = (int) renderTileBoundPoints[j + 1].y;
                lv_draw_line(&layer, &dsc);
            }
            
            labelStartPoint.x = (int32_t) renderTileBoundPoints[0].x;
            labelStartPoint.y = (int32_t) renderTileBoundPoints[0].y;
            labelCoords = {labelStartPoint.x, labelStartPoint.y, labelStartPoint.x + 200, labelStartPoint.y + 16};
            
            tileInfoLabel infoLabel;
            
            infoLabel.text = "(" + to_string(tileX) + "," + to_string(tileY) + ")";
            infoLabel.labelCoords = labelCoords;
            
            tileLabels.push_back(infoLabel);
            
            if (overflowBounds && !mapTypeList[mapTypeIndex].isImage) {
                infoLabel.text = "NO DATA";
                labelStartPoint.x = (int32_t) renderTileBoundPoints[0].x + renderTileSize / 2;
                labelStartPoint.y = (int32_t) renderTileBoundPoints[0].y + renderTileSize / 2;
                labelCoords = {labelStartPoint.x, labelStartPoint.y, labelStartPoint.x + 100, labelStartPoint.y + 16};
                infoLabel.labelCoords = labelCoords;
                tileLabels.push_back(infoLabel);
            }
        }
    }
    
    drawLabelDsc.ofs_x = 5;
    drawLabelDsc.ofs_y = 5;
    drawLabelDsc.align = LV_TEXT_ALIGN_LEFT;
    drawLabelDsc.font = &lv_font_montserrat_12;
    drawLabelDsc.color = lv_palette_main(LV_PALETTE_RED);
    
    for (const auto &tileInfo: tileLabels) {
        drawLabelDsc.text = tileInfo.text.c_str();
        lv_draw_label(&layer, &drawLabelDsc, &tileInfo.labelCoords);
        lv_canvas_finish_layer(canvas, &layer);
    }
}


void CanvasMap::drawTrackLine(lv_point_precise_t *pointPtr, uint16_t pointLen) {
    drawLineDsc.color = lv_color_hex(0xf70000);
    drawLineDsc.width = 3;
    for (int j = 0; j < pointLen - 1; j++) {
        drawLineDsc.p1.x = pointPtr[j].x;
        drawLineDsc.p1.y = pointPtr[j].y;
        drawLineDsc.p2.x = pointPtr[j + 1].x;
        drawLineDsc.p2.y = pointPtr[j + 1].y;
        lv_draw_line(&layer, &drawLineDsc);
    }
    lv_canvas_finish_layer(canvas, &layer);
    
}

void CanvasMap::loadFile(const char *fileName) {
    lv_fs_file_t mapFile;
    
    char fullFileNam[50];
    sprintf(fullFileNam,"/maps/%s",fileName);
    
    lv_fs_res_t openRes = lv_fs_open(&mapFile, fullFileNam, LV_FS_MODE_RD);
    
    if (openRes != LV_FS_RES_OK) {
        _errorString = "mapFile open err,res=" + to_string(openRes);
        printf_e("%s", _errorString.c_str());
        return;
    } else {
        printf_i("地图文件打开成功:%s",fileName);
    }
    _mapFile = mapFile;
    
    
    if (readHeader(mapFile)){
        _valid = true;
        printf_i("地图文件头读取成功");
        
    } else {
        _valid = false;
        printf_e("地图文件头读取失败");
    }
}


void CanvasMap::toggleMap() {
    mapTypeIndex++;
    if (mapTypeIndex >= mapTypeList.size()) mapTypeIndex = 0;
    currentMapType = mapTypeList[mapTypeIndex];
    if (_zoom > currentMapType.zoomMax) _zoom = currentMapType.zoomMax;
    if (_zoom < currentMapType.zoomMin) _zoom = currentMapType.zoomMin;
    printf_i("currentMapType is %s", currentMapType.name.c_str());
    mapChanged = true;
    centerOn(_centerCoordRaw);
    updateMap();
    mapChanged = false;
}

void CanvasMap::changeZoom(uint8_t num) {
    _zoom += num;
    if (_zoom > currentMapType.zoomMax) _zoom = currentMapType.zoomMax;
    if (_zoom < currentMapType.zoomMin) _zoom = currentMapType.zoomMin;
    updateMap();
}

void CanvasMap::updatePosition(GPS_Info_t gpsInfo) {
    Coordinates currentPosition(gpsInfo.longitude, gpsInfo.latitude);
    headAngle = gpsInfo.heading * 10;
    centerOn(currentPosition);
    updateMap();
}

void CanvasMap::updateMap() {
    
    if (mapTypeList[mapTypeIndex].name == "谷歌卫星") {
        rulerColor = lv_color_hex(0xFFFFFF);
    } else {
        rulerColor = lv_color_hex(0x333333);
    }
    
    lv_obj_set_style_border_color(scaleRuler, rulerColor, 0);
    lv_obj_set_style_text_color(levelLabel, rulerColor, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scaleRulerLabel, rulerColor, LV_STATE_DEFAULT);
//    lv_label_set_text_fmt(levelLabel, "level:%d", _zoom);
    lv_label_set_text_fmt(levelLabel, "%s LV:%d", mapTypeList[mapTypeIndex].name.c_str(), _zoom);
    
    lv_obj_remove_flag(loadingLabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(iconArrow, LV_OBJ_FLAG_HIDDEN);
    
    if (currentMapType.isImage) {
        updateImageTile();
        lv_obj_add_flag(loadingLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(iconArrow, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (_valid) {
            updateVectorTile(true);
            lv_obj_add_flag(loadingLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(iconArrow, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_label_set_text(loadingLabel, _errorString.c_str());
        }
        
    }
    
    double meterPerPx = GroundResolution(_centerCoord.lat(), _zoom, renderTileSize);
    
    auto scaleRulerLength = int32_t(_zoomInfoMap[_zoom].distance / meterPerPx);
    
    lv_obj_set_width(scaleRuler, scaleRulerLength);
    lv_obj_align(scaleRuler, LV_ALIGN_BOTTOM_RIGHT, -5, -7);
    lv_label_set_text(scaleRulerLabel, _zoomInfoMap[_zoom].text.c_str());
    lv_obj_align_to(scaleRulerLabel, scaleRuler, LV_ALIGN_OUT_TOP_MID, 0, 0);
    
    //设置中心点箭头朝向,移到最上层
    lv_image_set_rotation(iconArrow, headAngle);
    lv_obj_move_foreground(iconArrow);
    
}


void CanvasMap::centerOn(Coordinates coord) {
    _centerCoordRaw = coord;
    if (currentMapType.gcj02) {
        double newLat, newLon;
        GPS_Transform(coord.lat(), coord.lon(), &newLat, &newLon);
        _centerCoord.setLon((float) newLon);
        _centerCoord.setLat((float) newLat);
    } else {
        _centerCoord.setLon((float) coord.lon());
        _centerCoord.setLat((float) coord.lat());
    }
}


uint32_t CanvasMap::level(int zoom) const {
    for (int i = 0; i < _subFiles.size(); i++)
        if (zoom <= _subFiles.at(i).max)
            return i;
    return _subFiles.size() - 1;
}


bool CanvasMap::readMapInfo(SubFile &hdr) {
    uint64_t fileSize, date;
    uint32_t version;
    int32_t minLat, minLon, maxLat, maxLon;
    uint8_t flags;
    
    char projection[50];
    if (!(hdr.seek(4) && hdr.readUInt32(version) && hdr.readUInt64(fileSize)
          && hdr.readUInt64(date) && hdr.readInt32(minLat) && hdr.readInt32(minLon)
          && hdr.readInt32(maxLat) && hdr.readInt32(maxLon)
          && hdr.readUInt16(_tileSize) && hdr.readString(projection)
          && hdr.readByte(flags)))
        return false;
    
    if (flags & 0x40) {
        int32_t startLon, startLat;
        if (!(hdr.readInt32(startLat) && hdr.readInt32(startLon)))
            return false;
    }
    if (flags & 0x20) {
        uint8_t startZoom;
        if (!hdr.readByte(startZoom))
            return false;
    }
    if (flags & 0x10) {
        char lang[50];
        memset(lang, 0, 50);
        if (!hdr.readString(lang))
            return false;
    }
    if (flags & 0x08) {
        char comment[50];
        memset(comment, 0, 50);
        if (!hdr.readString(comment))
            return false;
    }
    if (flags & 0x04) {
        char createdBy[20];
        if (!hdr.readString(createdBy))
            return false;
    }
    
    _baseMapBounds = RectC(Coordinates(MD(minLon), MD(maxLat)),
                           Coordinates(MD(maxLon), MD(minLat))
    );
    
    return true;
}


bool CanvasMap::readZoomInfo(SubFile &hdr) {
    uint8_t zooms;
    
    if (!hdr.readByte(zooms))
        return false;
    _subFiles.resize(zooms);
    for (uint8_t i = 0; i < zooms; i++) {
        if (!(hdr.readByte(_subFiles[i].base)
              && hdr.readByte(_subFiles[i].min)
              && hdr.readByte(_subFiles[i].max)
              && hdr.readUInt64(_subFiles[i].offset)
              && hdr.readUInt64(_subFiles[i].size)))
            return false;
    }
    
    return true;
}

bool CanvasMap::readHeaderTag(SubFile &hdr, vector<TagSource> &tags) {
    uint16_t size;
    char str[100];
    
    if (!hdr.readUInt16(size))
        return false;
    tags.resize(size);
//    printf("tagSize is %d\n",size);
    for (uint16_t i = 0; i < size; i++) {
        TagSource &tag = tags[i];
        memset(str, 0, sizeof str);
        if (!hdr.readString(str))
            return false;
        tag = TagSource(str);
//        printf("tag[%d] is %s\n",i,str);
        auto matchTag = _keys.find(tag.key);
        
        if (matchTag != _keys.end()) {
            tag.id = matchTag->second;
//            _keys[tag.key] = tag.id;
        } else {
            tag.id = _keys.size() + 1;
            _keys.insert(make_pair(tag.key, tag.id));
        }
    }
    
    return true;
}

bool CanvasMap::readTagInfo(SubFile &hdr) {
    _keys.insert(make_pair<string, uint32_t>(KEY_NAME, ID_NAME));
    _keys.insert(make_pair<string, uint32_t>(KEY_HOUSE, ID_HOUSE));
    _keys.insert(make_pair<string, uint32_t>(KEY_REF, ID_REF));
    _keys.insert(make_pair<string, uint32_t>(KEY_ELE, ID_ELE));
    
    bool readPointTagRes = readHeaderTag(hdr, _pointTags);
    bool readPathTagRes = readHeaderTag(hdr, _pathTags);
    
    for (auto &_key: _keys) {
        _keysSwap[_key.second] = _key.first;
    }
//    for (auto &_key: _keysSwap) {
//        printf_i("%d:%s", _key.first, _key.second.c_str());
//    }

//    string tempStr;
//    for (const auto &pathTagItem: _pathTags) {
////        if (pathTagItem.key == "highway") {
//        tempStr = ("("+pathTagItem.key+"="+pathTagItem.value+")" );
//        cout <<  tempStr << endl;
////        }
//    }
//
//    for (const auto &pathTagItem: _pointTags) {
//        tempStr = ("("+pathTagItem.key+"="+pathTagItem.value+")" );
//        cout << tempStr << endl;
//    }
    
    waterTag = {_keys["natural"], "water"};
    residentialTag = {_keys["landuse"], "residential"};
    industrialTag = {_keys["landuse"], "industrial"};
    buildingTag = {_keys["building"], "yes"};
    parkTag = {_keys["leisure"], "park"};
    
    return (readPointTagRes && readPathTagRes);
}


bool CanvasMap::readHeader(lv_fs_file_t file) {
    char magic[MAGIC_SIZE];
    uint32_t hdrSize;
    uint32_t readBytes;
    
    lv_fs_read(&file, magic, MAGIC_SIZE, &readBytes);
    
    
    if (memcmp(magic, MAGIC, MAGIC_SIZE) != 0) {
        _errorString = "Not a Mapsforge map";
        return false;
    }
    
    lv_fs_read(&file, (void *) &hdrSize, sizeof(hdrSize), &readBytes);
    
    if (readBytes < (int64_t) sizeof(hdrSize)) {
        _errorString = "Unexpected EOF";
        return false;
    }
    
    hdrSize = __builtin_bswap32(hdrSize);
    
    SubFile hdr(&file, MAGIC_SIZE, hdrSize);
    
    if (!readMapInfo(hdr)) {
        _errorString = "Error reading map info";
        return false;
    }
    
    if (!readTagInfo(hdr)) {
        _errorString = "Error reading tags info";
        return false;
    }
    
    if (!readZoomInfo(hdr)) {
        _errorString = "Error reading zooms info";
        return false;
    }
    uint32_t index = 0;
    for (auto subFile: _subFiles) {
        printf_n("subFile[%d] min:%d,base:%d,max:%d", index, subFile.min, subFile.base, subFile.max);
        index++;
    }
    return true;
}


bool CanvasMap::matchTag(Path pathItem, const Tag &tag) {
    vector<Tag>::iterator it;
    it = find(pathItem.tags.begin(), pathItem.tags.end(), tag);
    return it != pathItem.tags.end();
}


bool CanvasMap::findName(vector<Tag> tags, string *nameStr) {
    vector<Tag>::iterator it;
    it = find(tags.begin(), tags.end(), 1);
    if (it != tags.end()) {
        *nameStr = it->value;
        return true;
    }
    return false;
}

bool CanvasMap::isType(Path pathItem, unsigned typeId) {
    vector<Tag>::iterator it;
    it = find(pathItem.tags.begin(), pathItem.tags.end(), typeId);
    return it != pathItem.tags.end();
//    return it != pathItem.tags.end()&&it->value=="service";
}

static string level12PathValue = "trunk|primary|secondary|";
//static string level14PathValue = "trunk|primary|secondary|tertiary|motorway|track|bridleway|steps|byway";
//static string level15PathValue = "trunk|primary|secondary|tertiary|motorway|track|bridleway|steps|byway";
static string level14PathValue = "unclassified|tertiary|secondary_link|primary_link|trunk_link|motorway_link|secondary|primary|residential|unclassified|service";
static string level15PathValue = "unclassified|tertiary|secondary_link|primary_link|trunk_link|motorway_link|secondary|primary|residential|unclassified|service";
static string level16PathValue = "unclassified|tertiary|residential|motorway|secondary|service|primary|trunk|motorway_link|footway|track|path|trunk_link|primary_link|construction|secondary_link|living_street|tertiary_link|steps|pedestrian|cycleway|road|services|raceway|bridleway|bus_guideway|";

bool CanvasMap::pathShouldRender(const Path &pathItem) const {
    bool res = false;
    size_t index;
    for (auto &tag: pathItem.tags) {
        if (_zoom < 14) {
            index = level12PathValue.find(tag.value);
            if (index != string::npos) {
                res = true;
                break;
            }
        }
        if (_zoom == 14) {
            index = level14PathValue.find(tag.value);
            if (index != string::npos) {
                res = true;
                break;
            }
        }
        if (_zoom == 15) {
            index = level15PathValue.find(tag.value);
            if (index != string::npos) {
                res = true;
                break;
            }
        }
        if (_zoom > 15) {
            index = level16PathValue.find(tag.value);
            if (index != string::npos) {
                res = true;
                break;
            }
        }
        
    }
    return res;
    
}