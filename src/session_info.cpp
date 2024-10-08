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
	sumchan = "";
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

	for (size_t i = 1; i < number_params; i++)
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
		{
			if (value.size() > 8)
				value = value.substr(0, 8);

			psr_name = value;
		}
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
			sumchan = value;

		else if(name == "dm")
			dm = stod(value);

		else if(name == "freq0" || name == "F0" || name == "Fmin")
			freq_min = stod(value);

		else if(name == "freq511" || name == "F511" || name == "Fmax")
			freq_max = stod(value);

		else if (name == "dt_utc")
			start_utc_s = value;
	}

	obs_file.close();

	chanels = 512;

	if (start_date_s == "")
		start_date_s = "00.00.00 00:00:00 0000000";

	if (start_utc_s == "")
		throw invalid_argument(string(ERROR) + "There is no utc time in observation file " + file_name);

	start_date = Custom_time(start_date_s);
	start_utc = Custom_time(start_utc_s);

	if (sumchan == "adc")
	{
		tau = .2048;
		obs_window = psr_period*1e3/tau;
	}
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


void strip_white(string& line)
{
	if (line == "" or line.size() == 0) return;

	size_t n = line.size();

	for (size_t i = 0; i < n; i++)
	{
		if (line[i] == ' ' or line[i] == '\t') 
		{
			line.erase(i, 1);
			i--;
			n--;
		}
	}
}



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

		// remove the last character from the line
		// if it's bad character
		size_t null = value.find('\r');
		if (null < 200)
			value = value.substr(0, null);

		null = value.find('\0');
		if (null < 200)
			value = value.substr(0, null);

		return;
	}

	while (buffer[i] != ' ')
	{
		value += buffer[i];

		if (i > 40 || i == (int) buffer.length())
			break;
		i++;
	}

	strip_white(name);
	strip_white(value);

	// remove the last character from the line
	// if it's bad character
	size_t null = value.find('\r');
	if (null < 200)
		value = value.substr(0, null);

	null = value.find('\0');
	if (null < 200)
		value = value.substr(0, null);
}

size_t Session_info::get_NUM_PARAMS() {return number_params;}

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

size_t Session_info::get_TOTAL_PULSES() {return total_pulses;}

double Session_info::get_TAU() {return tau;}
size_t Session_info::get_OBS_WINDOW() {return obs_window;}
string Session_info::get_SUMCHAN() {return sumchan;}
double Session_info::get_FREQ_MIN() {return freq_min;}
double Session_info::get_FREQ_MAX() {return freq_max;}
size_t Session_info::get_CHANELS() {return chanels;}



void Session_info::set_TOTAL_PULSES(size_t num) {total_pulses = num;}
