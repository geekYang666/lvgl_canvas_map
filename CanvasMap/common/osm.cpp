#include <cstdint>
#include <algorithm>
#include "../common/wgs84.h"
#include "osm.h"


#define EPSILON       1e-6
#define ABS(x) (((x)>0)?(x):-(x))

static const double MinLatitude = -85.05112878;
static const double MaxLatitude = 85.05112878;
static const double MinLongitude = -180;
static const double MaxLongitude = 180;

static const double pi = 3.14159265358979324;
static const double a = 6378245.0;
static const double ee = 0.00669342162296594323;

static const double EarthRadius = 6378137;

static double transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(ABS(x));
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
    return ret;
}

static double transformLon(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(ABS(x));
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
    return ret;
}

static double index2mercator(int index, int zoom)
{
	return rad2deg(-M_PI + 2 * M_PI * ((double)index / (1<<zoom)));
}

static double Clip(double n, double minValue, double maxValue)
{
    return std::min(std::max(n, minValue), maxValue);
}

static uint32_t MapSize(uint8_t zoom, uint32_t tileSize){
    return (uint32_t)tileSize << zoom;
    
}

double OSM:: GroundResolution(double lat, uint8_t zoom,uint16_t tileSize){
    double latitude = Clip(lat, MinLatitude, MaxLatitude);
    return cos(latitude * M_PI / 180) * 2 * M_PI * EarthRadius / MapSize(zoom,tileSize);
}


void OSM::GPS_Transform(double wgLat, double wgLon, double* mgLat, double* mgLon)
{
    double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);
    double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);
    double radLat = wgLat / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
    *mgLat = wgLat + dLat;
    *mgLon = wgLon + dLon;
}


QPointF OSM::ll2m(const Coordinates &c)
{
	return {c.lon(), rad2deg(log(tan(M_PI_4 + deg2rad(c.lat())/2.0)))};
}

Coordinates OSM::m2ll(const QPointF &p)
{
	return Coordinates(p.x, rad2deg(2.0 * atan(exp(deg2rad(p.y))) - M_PI_2));
}

QPoint OSM::mercator2tile(const QPointF &m, int zoom)
{
	return {std::min((int)floor((m.x + 180.0) / 360.0 * (1<<zoom)), (1<<zoom) - 1),
                  std::min((int)floor((1.0 - (m.y / 180.0)) / 2.0 * (1<<zoom)), (1<<zoom) - 1)};
}

QPointF OSM::tile2mercator(const QPoint &p, int zoom)
{
	return {index2mercator(p.x, zoom), index2mercator(p.y, zoom)};
}

QPoint OSM::ll2PixelYX(const Coordinates&c,int zoom,uint16_t tileSize){
    uint32_t mapSize = tileSize<<zoom;

    double latitude = Clip(c.lat(), MinLatitude, MaxLatitude);
    double longitude = Clip(c.lon(), MinLongitude, MaxLongitude);

    double x = (longitude + 180) / 360;
    double sinLatitude = sin(latitude * M_PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * M_PI);

    return {(int)Clip(x * mapSize + 0.5, 0, mapSize - 1),
            (int)Clip(y * mapSize + 0.5, 0, mapSize - 1)};


}

Coordinates OSM::PixelXYToLatLong(QPoint pixelPoint, int zoom, uint16_t tileSize) {
    double mapSize = tileSize<<zoom;
    double x = (Clip(pixelPoint.x, 0, mapSize - 1) / mapSize) - 0.5;
    double y = 0.5 - (Clip(pixelPoint.y, 0, mapSize - 1) / mapSize);
    
    double  longitude = 360 * x;
    double  latitude = 90 - 360 * atan(exp(-y * 2 * M_PI)) / M_PI;
    
    return {(float)longitude,(float)latitude};
}

qreal OSM::zoom2scale(int zoom, int tileSize)
{
	return (360.0/(qreal)((1<<zoom) * tileSize));
}

int OSM::scale2zoom(qreal scale, int tileSize)
{
	return (int)(log2(360.0/(scale * (qreal)tileSize)) + EPSILON);
}

qreal OSM::resolution(const QPointF &p, int zoom, int tileSize)
{
	qreal scale = zoom2scale(zoom, tileSize);

	return (WGS84_RADIUS * 2.0 * M_PI * scale / 360.0
	  * cos(2.0 * atan(exp(deg2rad(-p.y * scale))) - M_PI/2));
}


