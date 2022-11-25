#include<string>
#include<iostream>
#include<fstream>

#include "../lib/session_info.h"
#include "../lib/custom_time.h"

using namespace std;

void str_split(string buffer, string& name, string& value);

Session_info::Session_info() : start_date (0, 0, 0, 0, 0, 0l), start_utc (0, 0, 0, 0, 0, 0l)
{
	number_params = -1;

	psr_name = "";
	psr_period = -1.0;
	dm = -1.0;

	start_date_s = "";
	start_utc_s = "";
	total_pulses = -1;

	tau = -1.0;
	obs_window = -1;
	sumchan = -1;
	freq_min = -1.0;
	freq_max = -1.0;
	chanels = 512;
}

Session_info::Session_info(string file_name) : start_date (0, 0, 0, 0, 0, 0l), start_utc (0, 0, 0, 0, 0, 0l)
{
	cout << "Reading header . . ." << endl;


	ifstream obs_file (file_name, ios::in | ios::binary);
	char buffer[40];

	string name, value;

	// read number of lines in header
	obs_file.read(buffer, 40);
	str_split(buffer, name, value);

	number_params = stoi(value);

	for (int i = 1; i < number_params; i++)
	{
		obs_file.read(buffer, 40);
		str_split(buffer, name, value);

		if (name == "name")
			psr_name = value;

		else if (name == "date")
			start_date_s = value;

		else if (name == "time")
			start_date_s += " " + value;

		else if (name == "period")
			psr_period = stold(value);

		if (name == "numpuls")
			total_pulses = stoi(value);

		else if (name == "tay")
			tau = stod(value);

		else if (name == "numpointwin")
			obs_window = stoi(value);

		else if (name == "sumchan")
		{
			if (value == "yes") sumchan = true;
			if (value == "no") sumchan = false;
		}

		else if(name == "dm")
			dm = stod(value);

		else if(name == "freq0")
			freq_min = stod(value);

		else if(name == "freq511")
			freq_max = stod(value);

		else if (name == "dt_utc")
		{
			value = value.substr(0, 6) + "20" + value.substr(6);
			start_utc_s = value;
		}


	}

	obs_file.close();

	chanels = 512;

	start_date = Custom_time(start_date_s);
	start_utc = Custom_time(start_utc_s);
}

void Session_info::add_parameter(char* buffer_c)
{
	string buffer = buffer_c;
	string name, value;

	str_split(buffer, name, value);
	cout << name << " " << value << endl;


	if (name == "numpar")
		number_params = stoi(value);

	else if (name == "name")
		psr_name = value;

	else if (name == "date")
		start_date_s = value;

	else if (name == "time")
		start_date_s += " " + value;

	else if (name == "period")
		psr_period = stold(value);

	if (name == "numpuls")
		total_pulses = stoi(value);

	else if (name == "tay")
		tau = stod(value);

	else if (name == "numpointwin")
		obs_window = stoi(value);

	else if (name == "sumchan")
	{
		if (value == "yes") sumchan = true;
		if (value == "no") sumchan = false;
	}

	else if(name == "dm")
		dm = stod(value);

	else if(name == "freq0")
		freq_min = stod(value);

	else if(name == "freq511")
		freq_max = stod(value);

	else if (name == "dt_utc")
	{
		value = value.substr(0, 6) + "20" + value.substr(6);
		start_utc_s = value;
	}

}

void Session_info::operator=(Session_info& right)
{
	number_params = right.number_params;

	psr_name = right.number_params;
	psr_period = right.psr_period;
	dm = right.dm;

	start_date_s = right.start_date_s;
	start_utc_s = right.start_utc_s;
	total_pulses = right.total_pulses;

	start_date = right.start_date;
	start_utc = right.start_utc;

	tau = right.tau;
	obs_window = right.obs_window;
	sumchan = right.sumchan;
	freq_min = right.freq_min;
	freq_max = right.freq_max;
}

void str_split(string buffer, string& name, string& value)
{
	int i = 0;

	name = "";
	value = "";

	while (buffer[i] != ' ')
	{
		name += buffer[i];
		i++;
	}


	while (buffer[i] == ' ')
		i++;

	// костыль из-за особенности записи utc в заголовочном файле
	if (name == "dt_utc" || name == "time") 
	{
		value = buffer.substr(i);
		return;
	}

	while (buffer[i] != ' ')
	{
		value += buffer[i];

		if (i > 40) break;

		i++;
	}

}

int Session_info::get_NUM_PARAMS() {return number_params;}

string Session_info::get_PSR_NAME() {return psr_name;}
long double Session_info::get_PSR_PERIOD() {return psr_period;}
double Session_info::get_DM() {return dm;}

Custom_time Session_info::get_START_DATE() 
{
	if (start_date_s == "") 
	{
		cout << "Date hasn't been read!" << endl;
		throw;
	}

	// Нужна полная проверка всех полей
	if (start_date.get_SECOND() == 0.0l) start_date = Custom_time(start_date_s);

	return start_date;
}

Custom_time Session_info::get_START_UTC() {return start_utc;}

int Session_info::get_TOTAL_PULSES() {return total_pulses;}

double Session_info::get_TAU() {return tau;}
int Session_info::get_OBS_WINDOW() {return obs_window;}
bool Session_info::get_SUMCHAN() {return sumchan;}
double Session_info::get_FREQ_MIN() {return freq_min;}
double Session_info::get_FREQ_MAX() {return freq_max;}
int Session_info::get_CHANELS() {return chanels;}
