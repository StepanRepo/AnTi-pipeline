#include"../lib/custom_time.h"

#include<iostream>
#include<string>
#include <stdexcept>


using namespace std;

Custom_time::Custom_time (string time_s)
{
// string format: dd.mm.yy hh:mm:ss  sssssss

	try
	{
		day = stoi(time_s.substr(0, 2));
		month = stoi(time_s.substr(3, 2));
		year = stoi(time_s.substr(6, 4));

		hour = stoi(time_s.substr(11, 2));
		minute = stoi(time_s.substr(14, 2));

		time_s.erase(19, 1);
		time_s[19] = '.';
		second = stod(time_s.substr(17));
	}
	catch (const std::invalid_argument &error)
	{
		cout << "fuck: " << time_s << endl;
	}


	mjd = 0.0l;

}

Custom_time::Custom_time (int yr, int mnth, int d, int h, int m, long double s)
{
	year = yr;
	month = mnth;
	day = d;

	hour = h;
	minute = m;
	second = s;

	mjd = 0.0l;
}

long double Custom_time::get_MJD()
{
	if (mjd == 0.0l) calculate_mjd();

	return mjd;
}

void Custom_time::calculate_mjd()
{
	mjd = -1.0l;
}


int Custom_time::get_YEAR() {return year;}
int Custom_time::get_MONTH() {return month;}
int Custom_time::get_DAY() {return day;}
int Custom_time::get_HOUR() {return hour;}
int Custom_time::get_MINUTE() {return minute;}
long double Custom_time::get_SECOND() {return second;}

void Custom_time::set_YEAR(int yr) {year = yr;}
void Custom_time::set_MONTH(int m) {month = m;}
void Custom_time::set_DAY(int d) {day = d;}
void Custom_time::set_HOUR(int h) {hour = h;}
void Custom_time::set_MINUTE(int m) {minute = m;}
void Custom_time::set_SECOND(long double s) {second = s;}
