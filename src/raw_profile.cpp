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

	OBS_SIZE = total_pulses*obs_window*chanels;

	float* data;
	data = new float [OBS_SIZE];

	mean_signal_per_chanel = vector (chanels, vector<double>(obs_window));

	read_data(file_name, data);
	split_data(data);

	delete[] data;

	data = nullptr;
}


void Raw_profile::read_data(string file_name, float* data)
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

void Raw_profile::split_data (float* signal)
{
	if (cfg->verbose)
		cout << SUB << "Splitting data...";

	size_t total_pulses = session_info.get_TOTAL_PULSES();
	size_t obs_window = session_info.get_OBS_WINDOW();
	size_t chanels = session_info.get_CHANELS();

#pragma omp parallel default(private) shared(total_pulses, obs_window, chanels, signal, mean_signal_per_chanel)  
	{      

#pragma omp for
		for (size_t i = 0; i < 512; i++)
			fill(mean_signal_per_chanel[i].begin(), mean_signal_per_chanel[i].end(), 0.0);

		size_t chan_and_window = chanels*obs_window;

		for (size_t imp = 0; imp < total_pulses; ++imp)
		{
#pragma omp for 
			for (size_t k = 0; k < obs_window; ++k)
			{
				for (size_t i = 0; i < chanels; ++i)
				{
					#pragma omp atomic
					mean_signal_per_chanel.at(i).at(k) += 
						(double) signal[i + k*chanels + imp*chan_and_window];
				}
			}
		}
	}


	if (cfg->verbose)
		cout << OK << endl;
}

void Raw_profile::print_mean_channel(string file_name)
{
	ofstream out (file_name);

	if (!out)
		cout << WARNING << "Cann't print channel profiles: " << file_name << endl;
	for (int i = 0; i < 570; i++)
	{
		for (int k = 0; k < 512; k++)
			out << mean_signal_per_chanel[k][i] << " ";

		out << endl;
	}
	out.close();
}
