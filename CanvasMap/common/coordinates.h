#ifndef COORDINATES_H
#define COORDINATES_H

#include <cmath>

#define deg2rad(d) (((d)*M_PI)/180.0)
#define rad2deg(d) (((d)*180.0)/M_PI)

class Coordinates
{
public:
	Coordinates() {_lon = NAN; _lat = NAN;}
	Coordinates(float lon, float lat) {_lon = lon; _lat = lat;}

	float &rlon() {return _lon;}
	float &rlat() {return _lat;}
	void setLon(float lon);
	void setLat(float lat);
	float lon() const {return _lon;}
	float lat() const {return _lat;}

	bool isNull() const
	  {return std::isnan(_lon) && std::isnan(_lat);}
	bool isValid() const
	  {return (_lon >= -180.0 && _lon <= 180.0
	    && _lat >= -90.0 && _lat <= 90.0);}

	float distanceTo(const Coordinates &c) const;

private:
	float _lat, _lon;
};


inline bool operator==(const Coordinates &c1, const Coordinates &c2)
  {return (c1.lat() == c2.lat() && c1.lon() == c2.lon());}
inline bool operator!=(const Coordinates &c1, const Coordinates &c2)
  {return !(c1 == c2);}
inline bool operator<(const Coordinates &c1, const Coordinates &c2)
{
	if (c1.lon() < c2.lon())
		return true;
	else if (c1.lon() > c2.lon())
		return false;
	else
		return (c1.lat() < c2.lat());
}


#endif // COORDINATES_H
