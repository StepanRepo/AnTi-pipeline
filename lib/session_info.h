#ifndef SESSION_INFO_H
#define SESSION_INFO_H

#include"custom_time.h"

#include<string>

using namespace std;


class Session_info
{
public:
	Session_info();
	Session_info(string file_name);

	void add_parameter(char* buffer);


	int get_NUM_PARAMS();

	string get_PSR_NAME();
	long double get_PSR_PERIOD();
	double get_DM();

	Custom_time get_START_DATE();
	Custom_time get_START_UTC();

	int get_TOTAL_PULSES();

	double get_TAU();
	int get_OBS_WINDOW();
	bool get_SUMCHAN();
	double get_FREQ_MIN();
	double get_FREQ_MAX();
	int get_CHANELS();

	void operator=(Session_info& right);

private:
	int number_params;

	string psr_name;
	long double psr_period;
	double dm;

	string start_date_s;
	string start_utc_s;
	int total_pulses;

	Custom_time start_date;
	Custom_time start_utc;

	double tau;
	int obs_window;
	bool sumchan;
	double freq_min, freq_max;
	int chanels;
};


#endif
