#include"../lib/raw_profile.h"
#include"../lib/session_info.h"

#include<string>
#include<fstream>
#include <iostream>

using namespace std;

Raw_profile::Raw_profile(string file_name) : session_info(file_name)
{
	int total_pulses = session_info.get_TOTAL_PULSES();
	int obs_window = session_info.get_OBS_WINDOW();
	int chanels = session_info.get_CHANELS();

	OBS_SIZE = total_pulses*obs_window*chanels;

	byte32* data;
	data = new byte32 [OBS_SIZE];

	read_data(file_name, data);

	double* signal;
	signal = new double[OBS_SIZE];
	decode_data(data, signal);

	mean_signal_per_chanel = vector (chanels, vector<double>(obs_window));

	split_data(signal);
}



void Raw_profile::read_data(string file_name, byte32* data)
{
	cout << "Reading data . . ." << endl;

	ifstream obs_file (file_name, ios::in | ios::binary);

	// skip header of file
	for (int i = 0; i < session_info.get_NUM_PARAMS(); i++)
		obs_file.ignore(40, '\n');


	obs_file.read((data[0].as_char), 4*OBS_SIZE);

	obs_file.close();
}

void Raw_profile::decode_data(byte32* data, double* signal)
{
	cout << "Decoding data . . ." << endl;

	#pragma omp parallel default(private) shared(data, signal) 
	{      
		double exp, spectr_t;
		double ratio = 0.2048/session_info.get_TAU();


		#pragma omp for
		for (int i = 0; i < OBS_SIZE; i++)
		{
			spectr_t = double (data[i].as_int & 0xFFFFFF);

			exp = double ( (data[i].as_int & 0x7F000000) >> 24 );
			exp -= 64.0;

			exp = double(1llu << (unsigned long long) exp);

			spectr_t = spectr_t*exp*ratio;
			spectr_t = spectr_t*1.3565771745707199e-14;

			signal[i] = spectr_t;
		}
	}
}

void Raw_profile::split_data (double* signal)
{
	cout << "Splitting data . . ." << endl;

	int total_pulses = session_info.get_TOTAL_PULSES();
	int obs_window = session_info.get_OBS_WINDOW();
	int chanels = session_info.get_CHANELS();

	for (int i = 0; i < 512; i++)
		fill(mean_signal_per_chanel[i].begin(), mean_signal_per_chanel[i].end(), 0.0);

	int chan_and_window = chanels*obs_window;

	for (int imp = 0; imp < total_pulses; imp ++)
	{
		for (int k = 0; k < obs_window; k ++)
		{
			for (int i = 0; i < chanels; i++)
			{
				mean_signal_per_chanel[i][k] += signal[i + k*chanels + imp*chan_and_window];
			}
		}
	}
}

