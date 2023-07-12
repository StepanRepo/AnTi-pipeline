#include"../lib/raw_profile.h"
#include"../lib/session_info.h"
#include"../lib/configuration.h"
#include"../lib/massages.h"

#include<cmath>
#include<string>
#include<fstream>
#include<iostream>

using namespace std;

extern Configuration* cfg;

Raw_profile::Raw_profile(string file_name) : session_info(file_name, true)
{
	if (cfg->verbose)
		cout << "Making raw profile" << endl;
	int total_pulses = session_info.get_TOTAL_PULSES();
	int obs_window = session_info.get_OBS_WINDOW();
	int chanels = session_info.get_CHANELS();

	OBS_SIZE = total_pulses * obs_window * chanels;

	byte32* data;
	data = new byte32 [OBS_SIZE];

	read_data(file_name, data);

	double* signal;

	if (session_info.get_SUMCHAN())
	{
		signal = new double[chanels * obs_window];
		session_info.set_TOTAL_PULSES(1);
	}
	else
	{
		signal = new double[OBS_SIZE];
	}

	decode_data(data, signal);

	mean_signal_per_chanel = vector (chanels, vector<double>(obs_window));

	read_data(file_name, data);
	split_data(signal);

	delete[] data;

	data = nullptr;
}


void Raw_profile::read_data(string file_name, byte32* data)
{
	if (cfg->verbose)
		cout << SUB << "Reading data...";


	ifstream obs_file (file_name, ios::in | ios::binary);

	if (!obs_file)
		throw invalid_argument (string(ERROR) + "Cann't open observational file to read data" + file_name);

	// skip header of file
	for (int i = 0; i < session_info.get_NUM_PARAMS(); i++)
		obs_file.ignore(40, '\n');


	obs_file.read((char*) data, 4*OBS_SIZE);

	obs_file.close();

	if (cfg->verbose)
		cout  << OK << endl;
}
void Raw_profile::decode_data(byte32* data, double* signal)
{
	if (cfg->verbose)
		cout << SUB << "Decoding data...";

	if (session_info.get_SUMCHAN())
	{
		int obs_window = session_info.get_OBS_WINDOW();
		int chanels = session_info.get_CHANELS();

		for (int i = 0; i < chanels*obs_window; ++i)
			signal[i] = (double) data[i].as_float;
	}
	else
	{
		#pragma omp parallel default(private) shared(data, signal) 
		{
			float spectr_t;
			int32_t exp;

			double ratio = 0.2048/session_info.get_TAU();

			vector<double> powers_of_two(200);

			powers_of_two.at(100) = 1.0;

			for (size_t i = 101; i < 200; ++i)
				powers_of_two.at(i) = powers_of_two.at(i-1) * 2.0;
			for (size_t i = 101; i > 0; --i)
				powers_of_two.at(i-1) = powers_of_two.at(i) / 2.0;


			#pragma omp for
			for (int i = 0; i < OBS_SIZE; ++i)
			{
				spectr_t = (float) (data[i].as_int & 0xFFFFFF);

				exp = (data[i].as_int & 0x7F000000) >> 24;
				exp = exp + 12;

				spectr_t = spectr_t * powers_of_two.at(exp) * ratio;

				signal[i] = spectr_t;
			}
		}
	}




	if (cfg->verbose)
		cout << OK << endl;
}


void Raw_profile::split_data (double* signal)
{
	if (cfg->verbose)
		cout << SUB << "Splitting data...";

	size_t total_pulses = session_info.get_TOTAL_PULSES();
	size_t obs_window = session_info.get_OBS_WINDOW();
	size_t chanels = session_info.get_CHANELS();

	for (int i = 0; i < 512; i++)
		fill(mean_signal_per_chanel[i].begin(), mean_signal_per_chanel[i].end(), 0.0);

	int chan_and_window = chanels*obs_window;

	for (size_t imp = 0; imp < total_pulses; ++imp)
	{
		for (size_t k = 0; k < obs_window; ++k)
		{
			for (size_t i = 0; i < chanels; ++i)
			{
				mean_signal_per_chanel[i][k] += signal[i + k*chanels + imp*chan_and_window];
			}
		}
	}


	if (cfg->verbose)
		cout << OK << endl;
}

