#include "wgs84.h"
#include "coordinates.h"

float Coordinates::distanceTo(const Coordinates &c) const
{
	float dLat = deg2rad(c.lat() - _lat);
	float dLon = deg2rad(c.lon() - _lon);
	float a = pow(sin(dLat / 2.0), 2.0)
	  + cos(deg2rad(_lat)) * cos(deg2rad(c.lat())) * pow(sin(dLon / 2.0), 2.0);

	return (WGS84_RADIUS * (2.0 * atan2(sqrt(a), sqrt(1.0 - a))));
}

void Coordinates::setLon(float lon) {_lon = lon;}
void Coordinates::setLat(float lat) {_lat = lat;}
