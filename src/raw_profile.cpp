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

	// set basic properties of observational session
	int total_pulses = session_info.get_TOTAL_PULSES();
	int obs_window = session_info.get_OBS_WINDOW();
	int channels = session_info.get_CHANELS();

	// declare arrays for 4-byte raw data and decoded double data
	byte32* data;
	double* signal;

	// choose which type of raw data is given
	if (session_info.get_SUMCHAN() != "adc")
	{
		// if it's basic BSA type, OBS_SIZE is just number of points in the file
		OBS_SIZE = total_pulses * obs_window * channels;

		// set the size of raw data array and fill with data from the file
		data = new byte32 [OBS_SIZE];
		read_data(file_name, data);

		// choose if the file contains (pre-processed) folded or raw profile
		if (session_info.get_SUMCHAN() == "yes")
		{
			// pre-processed file contains only one profile at every channel
			signal = new double[channels * obs_window];
			session_info.set_TOTAL_PULSES(1);
		}
		else
		{
			signal = new double[OBS_SIZE];
		}
	}
	else
	{
		// OBS_SIZE for adc (analog-to-diskrete converter) file is 
		// time of observation * 5 MiB/sec / 4
		// Division by 4 is needed to unify variable OBS_SIZE: 
		// it's further used to set the length of 4-byte array
		OBS_SIZE = session_info.get_PSR_PERIOD() * (total_pulses - 1) * 5*1024*1024/4; 

		// load observational data with length OBS_SIZE from the file
		data = new byte32 [OBS_SIZE];
		read_data(file_name, data);

		signal = new double[OBS_SIZE*2];
	}

	decode_data(data, signal);

	// declare vector of folded profile and fill it with zeros
	mean_signal_per_channel = vector (channels, vector<double>(obs_window));

	for (int i = 0; i < channels; ++i)
		for (int j = 0; j < obs_window; ++j)
			mean_signal_per_channel[i][j] = 0.0;

	// calculate folded profile for the given data
	split_data(signal);


	// ALWAYS FREE YOUR RAM
	delete[] data;
	delete[] signal;

	data = nullptr;
	signal = nullptr;
}


void Raw_profile::read_data(string file_name, byte32* data)
{
	if (cfg->verbose)
		cout << SUB << "Reading data...";


	// try to open observational file
	ifstream obs_file (file_name, ios::in | ios::binary);

	// check if it's possible
	if (!obs_file)
		throw invalid_argument (string(ERROR) + "Cann't open observational file to read data" + file_name);

	// skip header of file
	for (size_t i = 0; i < session_info.get_NUM_PARAMS(); i++)
		obs_file.ignore(40, '\n');

	// fill array data using given number of points in the file
	obs_file.read((char*) data, 4*OBS_SIZE);

	obs_file.close();

	if (cfg->verbose)
		cout  << OK << endl;
}


void Raw_profile::decode_data(byte32* data, double* signal)
{
	if (cfg->verbose)
		cout << SUB << "Decoding data...";

	// check if given file was folded earlier
	if (session_info.get_SUMCHAN() == "yes")
	{
		// for this type of files data is storaged in basic float format
		// there is no need to decode it
		int obs_window = session_info.get_OBS_WINDOW();
		int channels = session_info.get_CHANELS();

		for (int i = 0; i < channels*obs_window; ++i)
			signal[i] = (double) data[i].as_float;
	}
	else if (session_info.get_SUMCHAN() == "no")
	{
		#pragma omp parallel default(private) shared(data, signal) 
		{
			// this format of observations suggests decoding of data
			// (an antenna equipment can't keep up with the flow of data)
			// the number is stored in exponent format (see the manual for more details of decoding)
			float spectr_t;
			int32_t exp;

			double ratio = 0.2048/session_info.get_TAU();

			vector<double> powers_of_two(200);

			// pre-computing of powers of two for speeding up the decoding
			powers_of_two.at(100) = 1.0;

			for (size_t i = 101; i < 200; ++i)
				powers_of_two.at(i) = powers_of_two.at(i-1) * 2.0;
			for (size_t i = 101; i > 0; --i)
				powers_of_two.at(i-1) = powers_of_two.at(i) / 2.0;


			#pragma omp for
			// decode all data points to usual double numbers
			for (int i = 0; i < OBS_SIZE; ++i)
			{
				spectr_t = (float) (data[i].as_int & 0xFFFFFF);

				// there is no physical sence here
				// number 12 is founded to not to overload computer bit grid
				exp = (data[i].as_int & 0x7F000000) >> 24;
				exp = exp + 12;

				spectr_t = spectr_t * powers_of_two.at(exp) * ratio;

				signal[i] = spectr_t;
			}
		}
	}
	else if (session_info.get_SUMCHAN() == "adc")
	{
		// ADC format are files that are directly writen from adc of the antenna
		// to get frequency-time profile one needs to perform inverse fourier transrorm
		// (see the manual for details)

		// numbers in adc files are stored as signed 8-bit integers
		// we declare the array a[4] to read 4 numbers from 32-bit raw data type
		// (this inconvinience came from unification of formats)
		int8_t* a;
		a = new int8_t[4];

		// we perform ifft for 1024 points of raw data to get the profile with 512 frequency channels
		complex<double> time[1024], fourier[1024];
		const char* error = NULL;

		// every point in raw data contains 4 numbers
		// we need to devide this ponts to sets of 1024 points:
		// OBS_SIZE * 4 / 1024 
		for (size_t pulse = 0; pulse < (size_t) OBS_SIZE/256ul; ++pulse)
		{
			// fill 1024 points in freq domain
			for (size_t i = 0; i < 256; ++i)
			{
				a = (int8_t*) &data[i + pulse*256];

				for(size_t j = 0; j < 4; ++j)
					fourier[j + 4*i] = (double) a[j] + 0.0i;
			}

			// convert it to time domain
			simple_fft::IFFT(fourier, time, 1024, error);

			// fill array signal with decoded data
			for (size_t i = 0; i < 512; ++i)
			{
				// 1048576 = 1024^2 -- normalization of FFT
				signal[i + 512*pulse] =
					(time[i] * conj(time[i])).real() / 1048576.0;
			}
		}
	}


	if (cfg->verbose)
		cout << OK << endl;
}


void Raw_profile::split_data (double* signal)
{
// This function splits 1d arrays of decoded data to 2d freq-time profile
// and folds it to get summed profile in every channel


	if (cfg->verbose)
		cout << SUB << "Splitting data...";

	// set basic propeties of observational session
	size_t total_pulses = session_info.get_TOTAL_PULSES();
	size_t obs_window = session_info.get_OBS_WINDOW();
	size_t channels = session_info.get_CHANELS();

	// fill resulting profile with zeros
	for (int i = 0; i < 512; i++)
		fill(mean_signal_per_channel[i].begin(), mean_signal_per_channel[i].end(), 0.0);

	int chan_and_window = channels*obs_window;


	// see the manual to understand how the data is stored in the 1d data array
	if (session_info.get_SUMCHAN() != "adc")
	{
		for (size_t imp = 0; imp < total_pulses; ++imp)
		{
			for (size_t k = 0; k < obs_window; ++k)
			{
				for (size_t i = 0; i < channels; ++i)
				{
					mean_signal_per_channel[i][k] +=
					       	signal[i + k*channels + imp*chan_and_window];
				}
			}
		}
	}
	else
	{
		// this option performs the same process of folding
		// but number of points in one profile is not integer
		// to split the data there is need to ajust some steps 
		// length between different pulses
		size_t arg;
		double dP = (double) session_info.get_PSR_PERIOD()*1e3 / session_info.get_TAU()
			- (double) obs_window;

		for (size_t imp = 0; imp < (size_t) total_pulses - 1; ++imp)
		{

			for (size_t k = 0; k < obs_window; ++k)
			{
				for (size_t i = 0; i < channels; ++i)
				{
					arg = i + (k + obs_window*imp + int (.5+dP*imp))*512ul; 

					// check if smth wrong with the length of the file
					if (arg > (size_t) OBS_SIZE*2)
						cout << "bad news:  " << arg << "   " << OBS_SIZE*2 << endl;

					mean_signal_per_channel[i][k] += signal[arg];
				}
			}
		}
	}



	if (cfg->verbose)
		cout << OK << endl;
}

