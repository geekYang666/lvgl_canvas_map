#ifndef MAPSFORGE_MAPDATA_H
#define MAPSFORGE_MAPDATA_H


#include "color_print/color_print.h"
#include "common/mapData.h"

#include "HAL.h"

extern const lv_img_dsc_t img_src_gps_arrow_default;
extern const lv_img_dsc_t arrow_up_12;
extern const lv_img_dsc_t icon_compass;

using namespace OSM;

typedef struct {
    string name;
    string mapPath;
    string format;
    uint8_t zoomMin;
    uint8_t zoomMax;
    bool isImage;
    bool gcj02;
} map_type_t;

extern vector <map_type_t> mapTypeList;

struct zoomInfo {
    uint8_t level;
    double distance;
    string text;
};

class SubFile;


class CanvasMap {
public:
    CanvasMap();

    ~CanvasMap();

    void Create(lv_obj_t *parentOjb);

    static int16_t renderTileSize;

    map_type_t currentMapType;
    uint16_t mapTypeIndex;
    bool mapChanged;

    void loadFile(const char *fileName);

    void toggleMap();

    void changeZoom(uint8_t num);

    void updateMap();

    void updatePosition(GPS_Info_t gpsInfo);

    void centerOn(Coordinates coord);

    void readTiles();

    void updateVectorTile(bool needRender);

    void renderVectorMap();

    void renderVectorTile();

    void renderPathText();

    void fillCanvasBG();

    void updateImageTile();

    void renderImageTile();

    void drawTileBounds();

    void drawTrackLine(lv_point_precise_t *pointPtr,uint16_t pointLen);

    static bool matchTag(Path pathItem, const Tag &tag);

    static bool findName(vector<Tag> tags, string *nameStr);

    static bool isType(Path pathItem, unsigned typeId);

    bool pathShouldRender(const Path &pathItem) const;

    void renderPolygonSingle(const Path &pathItem, bool drawName);

    void renderPathSingle(const Path &pathItem, bool drawName);

    void renderRiver(const Path &pathItem);

    void calcPathPoints(uint32_t &pointLen, const vector<Coordinates> &coords);

    bool pathHasPointVA(uint32_t pointLen) const;

//private:
    lv_fs_file_t _mapFile{};

    const lv_font_t *font_zh{};
    lv_draw_buf_t *draw_buf{};
    lv_obj_t *parent{};
    lv_obj_t *canvas{};

    lv_obj_t *iconArrow{};
    lv_obj_t *scaleRuler{};
    lv_obj_t *scaleRulerLabel{};

    lv_color_t rulerColor{};

    lv_obj_t *loadingLabel{};
    lv_obj_t *levelLabel{};
    lv_obj_t *iconCompass{};

    uint16_t headAngle{}; // 当前朝向， 单位为度*10 45° = 东北方

    lv_layer_t layer{};
    lv_vector_dsc_t *ctx{};
    lv_vector_path_t *path{};


    bool _valid;

    uint8_t _zoom;
    RectC _baseMapBounds;
    int16_t _viewWidth{};
    int16_t _viewHeight{};

    uint32_t _canvasWidth{};
    uint32_t _canvasHeight{};


    int32_t canvasOffsetX{};
    int32_t canvasOffsetY{};

    lv_fpoint_t centerPoint{};
    lv_fpoint_t startPoint{};
    lv_fpoint_t endPoint{};

    uint16_t _tileSize{};

    static lv_draw_line_dsc_t drawLineDsc;
    static lv_draw_label_dsc_t drawLabelDsc;
    static lv_draw_image_dsc_t drawImageDsc;

    SubFile currentSubFile{};

    vector<SubFileInfo> _subFiles;

    vector<TagSource> _pointTags, _pathTags;
    vector<VectorTile> _tiles;
    vector<LabelInfo> _namePoints;

    vector<tileInfoLabel> tileLabels;
    std::map<string, uint32_t> _keys;
    std::map<uint32_t, string> _keysSwap;

    std::map<uint8_t, zoomInfo> _zoomInfoMap;

    QPoint _mapCenterPixelXY;
    QPoint _baseZoomTileBounds[2];
    QPoint _viewAreaTileBounds[2];
    Coordinates _centerCoordRaw;
    Coordinates _centerCoord;
    Coordinates _firstTileTopLeft;
    Coordinates _firstTileBottomRight;

    string _errorString;

    uint32_t level(int zoom) const;


    bool readMapInfo(SubFile &hdr);

    bool readZoomInfo(SubFile &hdr);

    bool readTagInfo(SubFile &hdr);

    bool readHeaderTag(SubFile &hdr, vector<TagSource> &tags);

    bool readHeader(lv_fs_file_t file);

};

#endif // MAPSFORGE_MAPDATA_H
