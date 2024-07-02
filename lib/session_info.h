#ifndef SESSION_INFO_H
#define SESSION_INFO_H

#include"custom_time.h"

#include<string>

using namespace std;


class Session_info
{
public:
	Session_info();
	Session_info(string file_name, bool binary);

	void add_parameter(char* buffer);

	void print(string file_name, double freq_comp = 0.0);


	size_t get_NUM_PARAMS();

	string get_PSR_NAME();
	long double get_PSR_PERIOD();
	double get_DM();

	Custom_time get_START_DATE();
	Custom_time get_START_UTC();

	size_t get_TOTAL_PULSES();

	double get_TAU();
	size_t get_OBS_WINDOW();
	string get_SUMCHAN();
	double get_FREQ_MIN();
	double get_FREQ_MAX();
	size_t get_CHANELS();

	void set_TOTAL_PULSES(size_t);
	//void operator=(Session_info& right);

private:
	size_t number_params;

	string psr_name;
	long double psr_period;
	double dm;

	string start_date_s;
	string start_utc_s;
	size_t total_pulses;

	Custom_time start_date;
	Custom_time start_utc;

	double tau;
	size_t obs_window;
	string sumchan;
	double freq_min, freq_max;
	size_t chanels;
};


#endif
