#include"../lib/custom_time.h"
#include"../lib/massages.h"

#include<iostream>
#include<string>
#include<algorithm>
#include <stdexcept>


using namespace std;

Custom_time::Custom_time (string time_s)
{
// string format: dd.mm.yy hh:mm:ss  sssssss

	time_s.erase(std::remove_if(time_s.begin(), time_s.end(), ::isspace), time_s.end());

	try
	{
		day = stoi(time_s.substr(0, 2));
		month = stoi(time_s.substr(3, 2));
		year = stoi(time_s.substr(6, 4));

		hour = stoi(time_s.substr(10, 2));
		minute = stoi(time_s.substr(13, 2));

		second = stod(time_s.substr(16,2) + "." + time_s.substr(18));
	}
	catch (const invalid_argument &err)
	{
		throw invalid_argument (string(ERROR) + "Cann't read time string value");
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
	int y = (long double) year;
	int m = (long double) month;
	int d = (long double) day;
	long double h = (long double) hour;
	long double min = (long double) minute;
	long double s = (long double) second;

	mjd = (long double) (367*y - 7*(y + (m + 9)/12)/4 - 3*(1 + (y + (m - 9)/7)/100)/4 + 275*m/9 + d);
	mjd += 1721028.0l - 2400000.0l + h/24.0l + min/1440.0l + s/86400.0l;
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
