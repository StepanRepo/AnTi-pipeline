#include"../lib/configuration.h"
#include "../lib/session_info.h"
#include "../lib/custom_time.h"
#include"../lib/massages.h"

#include<string>
#include<iostream>
#include<fstream>
#include<exception>

using namespace std;

extern Configuration* cfg;

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

Session_info::Session_info(string file_name, bool binary) : start_date (0, 0, 0, 0, 0, 0l), start_utc (0, 0, 0, 0, 0, 0l)
{
	if (cfg->verbose)
		cout << "Reading session information" << endl;


	ifstream obs_file;
	if (binary)
		obs_file.open(file_name, ios::in | ios::binary);
	else
		obs_file.open(file_name, ios::in);


	if (!obs_file)
		throw invalid_argument (string(ERROR) + "Cann't open observational file to read header " + file_name);

	string name, value;

	char buffer_c[40];
	string buffer_s;

	// read number of lines in header
	if (binary)
	{
		obs_file.read(buffer_c, 40);
		str_split(buffer_c, name, value);
	}
	else
	{
		getline (obs_file, buffer_s);
		str_split(buffer_s, name, value);
	}

	number_params = stoi(value);

	for (int i = 1; i < number_params; i++)
	{
		if (binary)
		{
			obs_file.read(buffer_c, 40);
			str_split(buffer_c, name, value);
		}
		else
		{
			getline (obs_file, buffer_s);
			str_split(buffer_s, name, value);
		}
		
		if (name == "name")
			psr_name = value;

		else if (name == "date")
			start_date_s = value;

		else if (name == "time")
			start_date_s = start_date_s.substr(0, start_date_s.find(' ')) + value;
		
		else if (name == "period")
			psr_period = stold(value);

		if (name == "numpuls")
			total_pulses = stoi(value);

		else if (name == "tay" || name == "tau")
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

		else if(name == "freq0" || name == "F0")
			freq_min = stod(value);

		else if(name == "freq511" || name == "F511")
			freq_max = stod(value);

		else if (name == "dt_utc")
			start_utc_s = value;
	}

	obs_file.close();

	chanels = 512;

	if (start_utc_s == "")

		throw invalid_argument(string(ERROR) + "There is no utc time in observation file " + file_name);

	start_date = Custom_time(start_date_s);
	start_utc = Custom_time(start_utc_s);
}


void Session_info::print(string file_name, double freq_comp)
{
	ofstream out (file_name);
	out.precision(12);

	if (freq_comp == 0.0)
	{
		out << "numpar      " << 12 << endl;
		out << "name        " << psr_name << endl;
		out << "date        " << start_date_s << endl;
		out << "dt_utc      " << start_utc_s << endl;
		out << "period      " << psr_period << endl;
		out << "numpuls     " << total_pulses << endl;
		out << "tau         " << tau << endl;
		out << "numpointwin " << obs_window << endl;
		out << "sumchan     " << "no" << endl;
		out << "dm          " << dm << endl;
		out << "F0          " << freq_min << endl;
		out << "F511        " << freq_max << endl;
	}
	else
	{
		out << "numpar      " << 13 << endl;
		out << "name        " << psr_name << endl;
		out << "date        " << start_date_s << endl;
		out << "dt_utc      " << start_utc_s << endl;
		out << "period      " << psr_period << endl;
		out << "numpuls     " << total_pulses << endl;
		out << "tau         " << tau << endl;
		out << "numpointwin " << obs_window << endl;
		out << "sumchan     " << "yes" << endl;;
		out << "dm          " << dm << endl;
		out << "F0          " << freq_min << endl;
		out << "Fcomp       " << freq_comp << endl;
		out << "F511        " << freq_max << endl;
	}
}
/*
void Session_info::operator=(Session_info& right)
{
	number_params = right.number_params;

	psr_name = right.psr_name;
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
*/

void str_split(string buffer, string& name, string& value)
{
	int i = 0;

	name = "";
	value = "";

	size_t comma = buffer.find(',');
	if (comma < 200)
		buffer[comma] = '.';

	while (buffer[i] != ' ')
	{
		name += buffer[i];
		i++;
	}


	while (buffer[i] == ' ')
		i++;

	// костыль из-за формата записи времени в заголовочном файле
	if (name == "dt_utc" || name == "time" || name == "date") 
	{
		value = buffer.substr(i);

		// заменить конец строки на пробел
		// для избежания проблем с кодировкой
		size_t null = value.find('\r');
		if (null < 200)
			value[null] = ' ';

		null = value.find('\0');
		if (null < 200)
			value[null] = ' ';

		return;
	}

	while (buffer[i] != ' ')
	{
		value += buffer[i];

		if (i > 40 || i == (int) buffer.length())
			break;
		i++;
	}

	// заменить конец строки на пробел
	// для избежания проблем с кодировкой
	size_t null = value.find('\r');
	if (null < 200)
		value[null] = ' ';

	null = value.find('\0');
	if (null < 200)
		value[null] = ' ';
}

int Session_info::get_NUM_PARAMS() {return number_params;}

string Session_info::get_PSR_NAME() {return psr_name;}
long double Session_info::get_PSR_PERIOD() {return psr_period;}
double Session_info::get_DM() {return dm;}

Custom_time Session_info::get_START_DATE() 
{
	if (start_date_s == "") 
	{
		throw invalid_argument (string(WARNING) + "Start date hasn't been read yet");
	}

	// Нужна полная проверка всех полей
	start_date = Custom_time(start_date_s);

	return start_date;
}

Custom_time Session_info::get_START_UTC() 
{

	if (start_utc_s == "") 
	{
		throw invalid_argument (string(WARNING) + "Start UTC hasn't been read yet");
	}

	// Нужна полная проверка всех полей
	start_utc = Custom_time(start_utc_s);

	return start_utc;
}

int Session_info::get_TOTAL_PULSES() {return total_pulses;}

double Session_info::get_TAU() {return tau;}
int Session_info::get_OBS_WINDOW() {return obs_window;}
bool Session_info::get_SUMCHAN() {return sumchan;}
double Session_info::get_FREQ_MIN() {return freq_min;}
double Session_info::get_FREQ_MAX() {return freq_max;}
int Session_info::get_CHANELS() {return chanels;}



void Session_info::set_TOTAL_PULSES(int num) {total_pulses = num;}
