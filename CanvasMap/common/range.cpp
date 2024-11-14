#include <iostream>
#include "range.h"


void RangeF::resize(double size)
{
	double adj = (size/2 - this->size()/2);

	_min -= adj;
	_max += adj;
}

RangeF RangeF::operator&(const RangeF &r) const
{
	if (isNull() || r.isNull())
		return RangeF();

	RangeF tmp(std::max(this->_min, r._min), std::min(this->_max, r._max));
	return tmp.isValid() ? tmp : RangeF();
}

