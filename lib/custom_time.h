#ifndef CUSTOM_TIME_H
#define CUSTOM_TIME_H

#include<string>

using namespace std;


class Custom_time
{
	public:
		Custom_time(string);
		Custom_time(int, int, int, int, int, long double);

		long double get_MJD();

		int get_YEAR();
		int get_MONTH();
		int get_DAY();
		int get_HOUR();
		int get_MINUTE();
		long double get_SECOND();

		void set_YEAR(int);
		void set_MONTH(int);
		void set_DAY(int);
		void set_HOUR(int);
		void set_MINUTE(int);
		void set_SECOND(long double);

	private:
		int year;
		int month;
		int day;

		int hour;
		int minute;
		long double second;

		long double mjd;

		void calculate_mjd();
};

#endif
