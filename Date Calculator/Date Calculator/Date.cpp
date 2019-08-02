#define _CRT_SECURE_NO_WARNINGS 1
#include "Date.h"





bool Date::operator<(const Date& d)
{
	if (_year < d._year)
		return true;
	else if (_year > d._year)
		return false;
	else
	{
		if (_month < d._month)
			return true;
		else if (_month > d._month)
			return false;
		else
		{
			if (_day < d._day)
				return true;
			else
				return false;
		}
	}
}

bool Date::operator>(const Date& d)
{
	if (_year > d._year)
		return true;
	else if (_year == d._year)
	{
		if (_month > d._month)
			return true;
		else if (_month == d._month)
		{
			if (_day > d._day)
				return true;
		}
	}
	return false;
}

bool Date::operator<=(const Date& d)
{
	if (_year <= d._year)
		return true;
	else if (_year == d._year)
	{
		if (_month <= d._month)
			return true;
		else if (_month == d._month)
		{
			if (_day <= d._day)
				return true;
		}
	}
	return false;
}

bool Date::operator>=(const Date& d)
{
	if (_year >= d._year)
		return true;
	else if (_year == d._year)
	{
		if (_month >= d._month)
			return true;
		else if (_month == d._month)
		{
			if (_day >= d._day)
				return true;
		}
	}
	return false;
}

bool Date::operator==(const Date& d)
{
	if (_year == d._year && _month == d._month && _day == d._day)
		return true;
	return false;
}

bool Date::operator!=(const Date& d)
{
	if (*this == d)
		return false;
	return true;
}

Date Date::operator+(int day)
{
	_day += day;
	while (_day > GetMonthDay(_year, _month))
	{
		_day -= GetMonthDay(_year, _month);
		_month++;

		if (_month > 12)
		{
			_year++;
			_month = 1;
		}
	}
	return *this;
}

Date Date::operator-(int day)
{
	_day -= day;
	while (_day <= 0)
	{
		_day += GetMonthDay(_year, _month);
		_month--;

		if (_month <= 0)
		{
			_year--;
			_month = 12;
		}
	}
	return *this;
}

Date Date::operator+=(int day)
{
	_day += day;
	while (_day > GetMonthDay(_year, _month))
	{
		_day -= GetMonthDay(_year, _month);
		_month++;

		if (_month > 12)
		{
			_year++;
			_month = 1;
		}
	}
	return *this;
}

Date Date::operator-=(int day)
{
	_day -= day;
	while (_day <= 0)
	{
		_day += GetMonthDay(_year, _month);
		_month--;

		if (_month <= 0)
		{
			_year--;
			_month = 12;
		}
	}
	return *this;
}

int Date::operator-(const Date& d)
{
	Date min(*this);
	Date max(d);
	int flag = -1;
	int count = 0;
	if (d < *this)
	{
		min = d;
		max = *this;
		flag = 1;
	}
	while (min < max)
	{
		min++;
		count++;
	}
	return flag * count;
}

// ++d d.operator++(&d)
Date Date::operator++()
{
	++_day;
	while (_day > GetMonthDay(_year, _month))
	{
		_day -= GetMonthDay(_year, _month);
		_month++;

		if (_month > 12)
		{
			_year++;
			_month = 1;
		}
	}
	return *this;
}

// d++ d.operator++(&d, 0)
Date Date::operator++(int)
{
	++_day;
	while (_day > GetMonthDay(_year, _month))
	{
		_day -= GetMonthDay(_year, _month);
		_month++;

		if (_month > 12)
		{
			_year++;
			_month = 1;
		}
	}
	return *this;
}

Date Date::operator--()
{
	--_day;
	if (_day == 0)
	{
		_day = GetMonthDay(_year, _month);
		_month--;
		if (_month == 0)
		{
			_year--;
			_month = 12;
		}
	}
	return *this;
}

Date Date::operator--(int)
{
	--_day;
	if (_day == 0)
	{
		_day = GetMonthDay(_year, _month);
		_month--;
		if (_month == 0)
		{
			_year--;
			_month = 12;
		}
	}
	return *this;
}
