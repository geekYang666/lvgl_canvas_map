#ifndef MAP_DATA_H
#define MAP_DATA_H

#include <cstring>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>

#include "rectc.h"
#include "range.h"
#include "polygon.h"
#include "osm.h"
#include "coordinates.h"
#include "osm.h"
#include "polygon.h"
#include "SubFile.h"

#define MAGIC "mapsforge binary OSM"
#define MAGIC_SIZE (sizeof(MAGIC) - 1)
#define MD(val) ((val) / 1e6)
#define OFFSET_MASK 0x7FFFFFFFFFL


#define ID_NAME   1
#define ID_HOUSE  2
#define ID_REF    3
#define ID_ELE    4

#define KEY_NAME  "name"
#define KEY_HOUSE "addr:housenumber"
#define KEY_REF   "ref"
#define KEY_ELE   "ele"


using namespace std;

struct Tag {
    Tag() = default;

    Tag(unsigned key, string value) : key(key), value(std::move(value)) {}

    bool operator==(const Tag &other) const { return (key == other.key && value == other.value); }

    bool operator==(const unsigned &x) const { return key == x; }

    unsigned key{};
    string value;
};

struct LabelInfo {
    LabelInfo(string name, lv_point_t pos) : name(name), pos(pos) {};
    string name;
    lv_point_t pos;

    bool operator==(const lv_point_t &other) const { return (abs(other.x - pos.x) < 100 && abs(other.y - pos.y) < 20); }
};

struct SubFileInfo {
    uint8_t base;
    uint8_t min;
    uint8_t max;
    uint64_t offset;
    uint64_t size;
};


struct TagSource {
    TagSource() = default;

    explicit TagSource(const string &str) {
        size_t pos = str.find('=');
        if (pos != string::npos) {
            key = str.substr(0, pos);
            value = str.substr(pos + 1);
        }
    }

    string key;
    string value;
    unsigned id{};
};

struct Point {
    explicit Point(const Coordinates &c) : coordinates(c) {}

    uint64_t id{};
    Coordinates coordinates;
    vector <Tag> tags;
    int layer{};
};

struct Path {
    class Polygon poly;

    vector <Tag> tags;
    Coordinates labelPos;
    int layer;
    bool closed;

    bool operator<(const Path &other) const { return layer < other.layer; }
};

struct tileInfoLabel {
    string text;
    lv_area_t labelCoords;
};

struct VectorTile {
    VectorTile(uint32_t x, uint32_t y)
            : x(x), y(y) {}

    size_t offset{};
    Coordinates pos;
    uint32_t x;
    uint32_t y;
    vector <Path> paths;
    vector <Point> points;
    bool overflowBounds{};
    
    bool readRes;

    bool operator==(const VectorTile &other) const { return (x == other.x && y == other.y); }

    bool operator<(const QPoint &other) const { return (x < other.x || y < other.y); }

    bool operator>(const QPoint &other) const { return (x > other.x || y > other.y); }
};



static double getDistance(const Coordinates &c1, const Coordinates &c2) {
    return hypot(c1.lon() - c2.lon(), c1.lat() - c2.lat());
}

static bool isClosed(const vector<Coordinates> &poly) {
    return (getDistance(poly.front(), poly.back()) < 0.000000001);
}

static bool readSingleDelta(SubFile &subFile, const Coordinates &c, uint32_t count, vector<Coordinates> &nodes) {
    int32_t mdLat, mdLon;

    if (!(subFile.readVInt32(mdLat) && subFile.readVInt32(mdLon)))
        return false;

    double lat = c.lat() + MD(mdLat);
    double lon = c.lon() + MD(mdLon);

    nodes.reserve(count);
    nodes.emplace_back(lon, lat);

    for (int i = 1; i < count; i++) {
        if (!(subFile.readVInt32(mdLat) && subFile.readVInt32(mdLon)))
            return false;

        lat = lat + MD(mdLat);
        lon = lon + MD(mdLon);

        nodes.emplace_back(lon, lat);
    }

    return true;
}

static bool readDoubleDelta(SubFile &subFile, const Coordinates &c, uint32_t count, vector<Coordinates> &nodes) {
    int32_t mdLat, mdLon;

    if (!(subFile.readVInt32(mdLat) && subFile.readVInt32(mdLon)))
        return false;
//    printf("Origin Pos:(%d,%d)\n",mdLat,mdLon);

    double lat = c.lat() + MD(mdLat);
    double lon = c.lon() + MD(mdLon);
    double prevLat = 0;
    double prevLon = 0;

    nodes.reserve(count);
    nodes.emplace_back(lon, lat);

    for (int i = 1; i < count; i++) {
        if (!(subFile.readVInt32(mdLat) && subFile.readVInt32(mdLon)))
            return false;

        double singleLat = MD(mdLat) + prevLat;
        double singleLon = MD(mdLon) + prevLon;

        lat += singleLat;
        lon += singleLon;

        nodes.emplace_back(lon, lat);

        prevLat = singleLat;
        prevLon = singleLon;
    }

    return true;
}

static bool readPolygonPath(SubFile &subFile, const Coordinates &c, bool doubleDelta, class Polygon &polygon) {
    uint32_t blocks, nodes;

    if (!subFile.readVUInt32(blocks))
        return false;

    polygon.reserve(polygon.size() + blocks);
    for (uint32_t i = 0; i < blocks; i++) {
        if (!subFile.readVUInt32(nodes) || !nodes)
            return false;

        vector<Coordinates> path;

        if (doubleDelta) {
            if (!readDoubleDelta(subFile, c, nodes, path))
                return false;

        } else {
            if (!readSingleDelta(subFile, c, nodes, path))
                return false;
        }

        polygon.append(path);
    }

    return true;
}

bool readTags(SubFile &subFile, int count, const vector<TagSource> &tags, vector<Tag> &list);

bool readPaths(SubFileInfo info,SubFile subFile, const VectorTile *tile, int zoom, vector <Path> *list,const vector <TagSource>&pointTags );

bool readPoints(SubFileInfo info,SubFile subFile, const VectorTile *tile, int zoom, vector <Point> *list,const vector <TagSource>&pointTags );


#endif //MAP_DATA_H
