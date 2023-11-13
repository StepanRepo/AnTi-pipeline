#include"../lib/raw_profile.h"
#include"../lib/session_info.h"
#include"../lib/configuration.h"
#include"../lib/massages.h"
#include"../lib/custom_math.h"


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

	byte32* data;
	double* signal;

	if (session_info.get_SUMCHAN() != "adc")
	{
		OBS_SIZE = total_pulses * obs_window * chanels;

		data = new byte32 [OBS_SIZE];
		read_data(file_name, data);

		if (session_info.get_SUMCHAN() == "yes")
		{
			signal = new double[chanels * obs_window];
			session_info.set_TOTAL_PULSES(1);
		}
		else
		{
			signal = new double[OBS_SIZE];
		}
	}
	else
	{
		OBS_SIZE = session_info.get_PSR_PERIOD() * (total_pulses - 1) * 5000000/4;

		data = new byte32 [OBS_SIZE];
		read_data(file_name, data);

		signal = new double[2*OBS_SIZE];
	}

	decode_data(data, signal);

	mean_signal_per_chanel = vector (chanels, vector<double>(obs_window));
	split_data(signal);


	delete[] data;
	delete[] signal;

	data = nullptr;
	signal = nullptr;
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

	if (session_info.get_SUMCHAN() == "yes")
	{
		int obs_window = session_info.get_OBS_WINDOW();
		int chanels = session_info.get_CHANELS();

		for (int i = 0; i < chanels*obs_window; ++i)
			signal[i] = (double) data[i].as_float;
	}
	else if (session_info.get_SUMCHAN() == "no")
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
	else if (session_info.get_SUMCHAN() == "adc")
	{
		int8_t* a;
		a = new int8_t[4];

		complex<double> time[1024], fourier[1024];
		const char* error = NULL;

		for (size_t pulse = 0; pulse < (size_t) OBS_SIZE/256ul; ++pulse)
		{
			for (size_t i = 0; i < 256; ++i)
			{
				a = (int8_t*) &data[i + pulse*256];

				for(size_t j = 0; j < 4; ++j)
					time[j + 4*i] = (double) a[j] + 0.0i;
			}

			simple_fft::IFFT(time, fourier, 1024, error);

			for (size_t i = 0; i < 512; ++i)
			{
				signal[i + 512*pulse] =
					(fourier[i] * conj(fourier[i])).real() / 1048576.0;
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
	size_t channels = session_info.get_CHANELS();

	for (int i = 0; i < 512; i++)
		fill(mean_signal_per_chanel[i].begin(), mean_signal_per_chanel[i].end(), 0.0);

	int chan_and_window = channels*obs_window;

	if (session_info.get_SUMCHAN() != "adc")
	{
		for (size_t imp = 0; imp < total_pulses; ++imp)
		{
			for (size_t k = 0; k < obs_window; ++k)
			{
				for (size_t i = 0; i < channels; ++i)
				{
					mean_signal_per_chanel[i][k] +=
					       	signal[i + k*channels + imp*chan_and_window];
				}
			}
		}
	}
	else
	{
		size_t arg;
		double dP = (double) obs_window -
		       	(double) session_info.get_PSR_PERIOD()*1e3 / session_info.get_TAU();

		for (size_t imp = 0; imp < (size_t) total_pulses - 1; ++imp)
		{
			for (size_t k = 0; k < obs_window; ++k)
			{
				for (size_t i = 0; i < channels; ++i)
				{
					arg = i + (k + obs_window*imp + int (1.5*dP*imp))*512ul; //canonical

					if (arg > (size_t) 2*OBS_SIZE)
						cout << "bad news:  " << arg << "   " << 2*OBS_SIZE << endl;

					mean_signal_per_chanel[i][k] += signal[arg];
				}
			}
		}
	}


	if (cfg->verbose)
		cout << OK << endl;
}

