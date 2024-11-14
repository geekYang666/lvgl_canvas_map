#ifndef RANGE_H
#define RANGE_H


#ifdef _min
#undef _min
#undef _max
#endif

class Range
{
public:
	Range() {_min = 0; _max = 0;}
	Range(int min, int max) : _min(min), _max(max) {}

	bool operator==(const Range &other) const
	  {return _min == other._min && _max == other._max;}
	bool operator!=(const Range &other) const
	  {return _min != other._min || _max != other._max;}

	int size() const {return (_max - _min);}
	int min() const {return _min;}
	int max() const {return _max;}

	bool isValid() const {return size() >= 0;}
	bool isNull() const {return _min == 0 && _max == 0;}

	void setMin(int min) {_min = min;}
	void setMax(int max) {_max = max;}

	bool contains(int val) const {return (val >= _min && val <= _max);}

private:
	int _min, _max;
};

class RangeF
{
public:
	RangeF() {_min = 0; _max = 0;}
	RangeF(double min, double max) : _min(min), _max(max) {}

	RangeF operator&(const RangeF &r) const;
	RangeF &operator&=(const RangeF &r) {*this = *this & r; return *this;}

	double min() const {return _min;}
	double max() const {return _max;}
	double size() const {return (_max - _min);}

	bool isNull() const {return _min == 0 && _max == 0;}
	bool isValid() const {return size() >= 0;}

	void setMin(double min) {_min = min;}
	void setMax(double max) {_max = max;}

	void resize(double size);

private:
	double _min, _max;
};


#endif // RANGE_H
