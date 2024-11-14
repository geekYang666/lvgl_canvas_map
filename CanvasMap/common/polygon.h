#ifndef POLYGON_H
#define POLYGON_H
#include "vector"
#include "list"
#include "array"

using namespace std;

#include <vector>
#include "rectc.h"

class Polygon
{
public:
	Polygon() = default;
	explicit Polygon(const vector<Coordinates> &path)
	{
		_paths.reserve(1);
		_paths.push_back(path);
		_boundingRect = boundingRect(path);
	}
	explicit Polygon(const RectC &rect)
	{
		vector<Coordinates> v(4);

		v[0] = Coordinates(rect.left(), rect.top());
		v[1] = Coordinates(rect.right(), rect.top());
		v[2] = Coordinates(rect.right(), rect.bottom());
		v[3] = Coordinates(rect.left(), rect.bottom());

		_paths.reserve(1);
		_paths.push_back(v);
		_boundingRect = RectC(v.at(0), v.at(2));
	}

	void append(const vector<Coordinates> &path)
	{
		_paths.push_back(path);
		_boundingRect |= boundingRect(path);
	}
	void reserve(uint32_t size) {_paths.reserve(size);}

    uint32_t size() const {return _paths.size();}
	bool isEmpty() const {return _paths.empty();}
	const vector<Coordinates> &at(int i) const {return _paths.at(i);}
	const vector<Coordinates> &first() const {return _paths.front();}
	const vector<Coordinates> &last() const {return _paths.back();}

	const RectC &boundingRect() const {return _boundingRect;}
    
    vector<vector<Coordinates> > _paths;
private:
	static RectC boundingRect(const vector<Coordinates> &path)
	{
		RectC rect;

		for (int i = 0; i < path.size(); i++)
			rect = rect.united(path.at(i));

		return rect;
	}
    
    
    RectC _boundingRect;
};


#endif // POLYGON_H
