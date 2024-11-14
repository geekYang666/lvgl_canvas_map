#ifndef OSM_H
#define OSM_H

#include "../common/coordinates.h"
#include "../common/rectc.h"
#include "../common/range.h"
#include "src/misc/lv_area.h"

typedef double qreal;

class QPoint {
public:
    QPoint(int x, int y) : x(x), y(y) {}
    QPoint() {
        x = 0;
        y = 0;
    }
    
    bool operator==(const lv_point_t &other) const { return (abs(other.x - x) < 80 && abs(other.y - y) < 16); }
    bool operator==(const QPoint &other) const { return (x == other.x && y == other.y); }
    bool operator!=(const QPoint &other) const { return (x != other.x || y != other.y); }
    bool operator<(const QPoint &other) const { return (x <other.x || y < other.y); }
    bool operator>(const QPoint &other) const { return (x >other.x || y > other.y); }

    int x;
    int y;
    
};

class QPointF {
public:
    QPointF(qreal x, qreal y) : x(x), y(y) {}
    
    QPointF() {
        x = 0;
        y = 0;
    }
    
    qreal x;
    qreal y;
};


namespace OSM {
    static const RectC BOUNDS(Coordinates(-180, 85.05112878),
                              Coordinates(180, -85.05112878));
    static const Range ZOOMS(0, 19);
    
    double GroundResolution(double lat, uint8_t zoom,uint16_t tileSize);
    
    void GPS_Transform(double wgLat, double wgLon, double* mgLat, double* mgLon);
    
    QPointF ll2m(const Coordinates &c);
    
    Coordinates m2ll(const QPointF &p);
    
    QPoint mercator2tile(const QPointF &m, int zoom);
    
    QPointF tile2mercator(const QPoint &p, int zoom);
    
    QPoint ll2PixelYX(const Coordinates &c, int zoom, uint16_t tileSize);
    
    Coordinates  PixelXYToLatLong(QPoint pixelPoint,int zoom ,uint16_t tileSize);
    
    qreal zoom2scale(int zoom, int tileSize);
    
    int scale2zoom(qreal scale, int tileSize);
    
    qreal resolution(const QPointF &p, int zoom, int tileSize);
    
    inline Coordinates tile2ll(const QPoint &p, int zoom) { return m2ll(tile2mercator(p, zoom)); }
    
    inline QPoint ll2tile(const Coordinates &c, int zoom) { return mercator2tile(ll2m(c), zoom); }
}

#endif // OSM_H
