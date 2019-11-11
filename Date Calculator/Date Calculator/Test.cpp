#define _CRT_SECURE_NO_WARNINGS 1
#include "Date.h"

int main()
{
	Date d(2019, 8, 1);
	Date d2;
	cout << (d < d2) << endl;
	cout << (d > d2) << endl;
	cout << (d == d2) << endl;
	Date d3 = d;
	d3++;
	d3--;
	d3 += 2;
	d3 -= 2;

	return 0;
}