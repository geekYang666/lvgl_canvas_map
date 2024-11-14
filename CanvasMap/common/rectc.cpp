#include <iostream>
#include <cmath>
#include "wgs84.h"
#include "rectc.h"

using namespace std;

#define MIN_LAT deg2rad(-90.0)
#define MAX_LAT deg2rad(90.0)
#define MIN_LON deg2rad(-180.0)
#define MAX_LON deg2rad(180.0)

static inline double WLON(double lon)
{
	return remainder(lon, 360.0);
}

static inline double LLAT(double lat)
{
	return (lat < 0.0) ? max(lat, -90.0) : min(lat, 90.0);
}


RectC RectC::operator|(const RectC &r) const
{
	if (isNull())
		return r;
	if (r.isNull())
		return *this;

	double l1 = _tl.lon();
	double r1 = _tl.lon();
	if (_br.lon() < _tl.lon())
		l1 = _br.lon();
	else
		r1 = _br.lon();

	double l2 = r._tl.lon();
	double r2 = r._tl.lon();
	if (r._br.lon() < r._tl.lon())
		l2 = r._br.lon();
	else
		r2 = r._br.lon();

	double t1 = _tl.lat();
	double b1 = _tl.lat();
	if (_br.lat() > _tl.lat())
		t1 = _br.lat();
	else
		b1 = _br.lat();

	double t2 = r._tl.lat();
	double b2 = r._tl.lat();
	if (r._br.lat() > r._tl.lat())
		t2 = r._br.lat();
	else
		b2 = r._br.lat();

	return RectC(Coordinates(min(l1, l2), max(t1, t2)),
	  Coordinates(max(r1, r2), min(b1, b2)));
}

RectC RectC::operator&(const RectC &r) const
{
	if (isNull() || r.isNull())
		return RectC();

	double l1 = _tl.lon();
	double r1 = _tl.lon();
	if (_br.lon() < _tl.lon())
		l1 = _br.lon();
	else
		r1 = _br.lon();

	double l2 = r._tl.lon();
	double r2 = r._tl.lon();
	if (r._br.lon() < r._tl.lon())
		l2 = r._br.lon();
	else
		r2 = r._br.lon();

	if (l1 > r2 || l2 > r1)
		return RectC();

	double t1 = _tl.lat();
	double b1 = _tl.lat();
	if (_br.lat() > _tl.lat())
		t1 = _br.lat();
	else
		b1 = _br.lat();

	double t2 = r._tl.lat();
	double b2 = r._tl.lat();
	if (r._br.lat() > r._tl.lat())
		t2 = r._br.lat();
	else
		b2 = r._br.lat();

	if (b1 > t2 || b2 > t1)
		return RectC();

	return RectC(Coordinates(max(l1, l2), min(t1, t2)),
	  Coordinates(min(r1, r2), max(b1, b2)));
}

RectC RectC::united(const Coordinates &c) const
{
	if (c.isNull())
		return *this;
	if (isNull())
		return RectC(c, c);

	double l = _tl.lon();
	double r = _tl.lon();
	if (_br.lon() < _tl.lon())
		l = _br.lon();
	else
		r = _br.lon();

	double t = _tl.lat();
	double b = _tl.lat();
	if (_br.lat() > _tl.lat())
		t = _br.lat();
	else
		b = _br.lat();

	if (c.lon() < l)
		l = c.lon();
	if (c.lon() > r)
		r = c.lon();
	if (c.lat() < b)
		b = c.lat();
	if (c.lat() > t)
		t = c.lat();

	return RectC(Coordinates(l, t), Coordinates(r, b));
}

RectC RectC::adjusted(double lon1, double lat1, double lon2, double lat2) const
{
	return RectC(Coordinates(WLON(_tl.lon() + lon1), LLAT(_tl.lat() + lat1)),
	  Coordinates(WLON(_br.lon() + lon2), LLAT(_br.lat() + lat2)));
}
