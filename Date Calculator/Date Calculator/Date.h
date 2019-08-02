#pragma once
#include <iostream>
using namespace std;

class Date
{
public:
	int GetMonthDay(int year, int month)
	{
		static int monthDays[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		if (month == 2 && (year % 400 == 0 || year % 4 == 0 && year % 100 != 0))
		{
			return 29;
		}
		else
		{
			return monthDays[month];
		}
	}

	Date(int year = 1900, int month = 1, int day = 1)
	{
		if (year >= 1900 && month > 0 && month < 13 && day > 0 && day <= GetMonthDay(year, month))
		{
			_year = year;
			_month = month;
			_day = day;
		}
		else
		{
			cout << "日期不正确" << endl;
		}
	}

	Date(const Date& d)
	{
		_year = d._year;
		_month = d._month;
		_day = d._day;
	}

	Date& operator=(const Date& d)
	{
		if (this != &d)
		{
			_year = d._year;
			_month = d._month;
			_day = d._day;
		}
		return *this;
	}

	~Date()
	{

	}

	bool operator<(const Date& d);
	bool operator>(const Date& d);
	bool operator<=(const Date& d);
	bool operator>=(const Date& d);
	bool operator==(const Date& d);
	bool operator!=(const Date& d);

	// d + 100
	Date operator+(int day);
	Date operator-(int day);
	Date operator+=(int day);
	Date operator-=(int day);
	int operator-(const Date& d);
	
	// ++d d.operator++(&d)
	Date operator++();
	// d++ d.operator++(&d, 0)
	Date operator++(int);

	// --d d.operator--(&d)
	Date operator--();
	// d-- d.operator--(&d, 0)
	Date operator--(int);
private:
	int _year;
	int _month;
	int _day;
};