#include"../lib/frequency_response.h"
#include"../lib/int_profile.h"
#include"../lib/raw_profile.h"
#include"../lib/custom_math.h"
#include"../lib/configuration.h"
#include"../lib/massages.h"

#include<vector>
#include<string>
#include<iostream>
#include<fstream>

using namespace std;

extern Configuration* cfg;

Frequency_response::Frequency_response(Raw_profile& raw) : session_info()
{
// this constructor calculates frequency response from raw profile
	if (cfg->verbose)
		cout << "Making frequency response" << endl;;

	// copy info about observational session from raw profile
	session_info = raw.session_info;

	// sum all amplitudes for every freq channel
	fill_profile(raw.mean_signal_per_channel);

	// declare and fill mask with default values
	fill_mask();

}

Frequency_response::Frequency_response(Int_profile& int_prf) : session_info()
{
// this constructor calculates frequency response from integral profile
// if time-freq profile is availible

	if (cfg->verbose)
		cout << "Making frequency response" << endl;;

	// copy info about observational session from integral profile
	session_info = int_prf.session_info;

	// sum all amplitudes for every freq channel
	fill_profile(int_prf.compensated_signal_per_channel);
	
	// declare and fill mask with default values
	fill_mask();
}


void Frequency_response::fill_profile(vector<vector<double>> signal_per_channel)
{
// this function finds frequency response as sum of amplitudes of all time stamps 
// in every frequency channel

	if (cfg->verbose)
		cout << SUB << "Calculating profile of frequency response...";

	// set basic info about observational session
	size_t channels = session_info.get_CHANELS(); 
	size_t obs_window = session_info.get_OBS_WINDOW();

	// declare frequency responce profile and fill it with zeros by default
	profile = vector<double> (channels);
	fill(profile.begin(), profile.end(), 0.0);


	// perform the summation over time
	for (size_t i = 0; i < channels; i++)
		for(size_t j = 0; j < obs_window; j++)
			profile.at(i) += signal_per_channel.at(i).at(j); 

	if (cfg->verbose)
		cout << OK << endl;
}

void Frequency_response::fill_mask()
{
// declare and fill mask with default values
	size_t channels = session_info.get_CHANELS(); 

	mask = vector<double> (channels);
	fill(mask.begin(), mask.end(), 1.0);

}


void Frequency_response::throw_if_mask_is_null()
{
// check if there is any signal in the file (after the filtration)
	double mask_sum = 0;
	for (size_t i = 0; i < session_info.get_CHANELS(); ++i)
		mask_sum += mask[i]*profile[i];


	if(mask_sum == 0.0)
	{
		cout << endl;
		throw invalid_argument (string(WARNING) + "Bad frequency response in file");
	}
}


void Frequency_response::derivative_filter(double p1, size_t width)
{
// derivative filter finds a differense between two sequent points
// if this difference is higher than some threshold 
// level (set in configuration) normed by characteristic power of all 
// signal (median) then the channel is noisy
	if (cfg->verbose)
		cout << SUB << "Derivative filter...";
	if (cfg->verbose)
		cout << SUB << "Derivative filter...";

	// set characteristick power of observational session
	double m = median(profile);


	size_t channels = session_info.get_CHANELS();

	// find the difference between subsecuent points
	// and set mask to zero in bad channels
	for (size_t i = 0; i < channels - 2; ++i)
	{
		if ((profile.at(i) - profile.at(i+1))/m > p1)
		{
			mask.at(i) = 0.0;
		}
	}

	// boundaries can't be processed by the filter
	mask.at(0) = 0.0;
	mask.at(session_info.get_CHANELS()-1) = 0.0;

	// stop the function if width is zero
	if (width == 0)
	{
		// if all channels are noisy than observation is corrupted
		throw_if_mask_is_null();
		return;
	}

	size_t i = 0;

	// fill mask points around noisy channels with zeros
	// fill the beginning of the mask
	while (i < width)
	{
		if (mask.at(i) == 0.0)
		{
			for (size_t j = 0; j < i+width+1; j++)
				mask.at(j) = 0.0;
			i += width;
		}

		i++;
	}

	// fill the middle of the mask
	while (i < session_info.get_CHANELS() - width)
	{
		if (mask.at(i) == 0.0)
		{
			for (size_t j = i-width; j < i+width; j++)
				mask.at(j) = 0.0;
			i += width;
		}

		i++;
	}

	// fill the end of the mask
	while (i < session_info.get_CHANELS())
	{
		if (mask.at(i) == 0.0)
		{
			for (size_t j = i-width; j < session_info.get_CHANELS(); j++)
				mask.at(j) = 0.0;
			i += width;
		}

		i++;
	}


	// if all channels are noisy than observation is corrupted
	throw_if_mask_is_null();

	if (cfg->verbose)
		cout << OK << endl;
}


void Frequency_response::median_filter(double p2, size_t width)
{
// median filter compaires local median and local standart deviation
// if median of the sample is higher then some amount (set in configuration)
// of std than current channel is noisy

	if (cfg->verbose)
		cout << SUB << "Median filter...";

	double m, s; // vars for local median and std

	for (size_t i = 0; i < session_info.get_CHANELS(); i++)
	{
		m = median(profile, i-5, i+5);
		s = sigma(profile, i-5, i+5);

		if ((profile.at(i) - m)/s > p2)
			mask[i] = 0.0;
	}


	// stop the function if width is zero
	if (width == 0)
	{
		// if all channels are noisy than observation is corrupted
		throw_if_mask_is_null();
		return;
	}

	size_t i = 0;

	// fill mask points around noisy channels with zeros
	// fill the beginning of the mask
	while (i < width)
	{
		if (mask.at(i) == 0.0)
		{
			for (size_t j = 0; j < i+width+1; j++)
				mask.at(j) = 0.0;
			i += width;
		}

		i++;
	}

	// fill the middle of the mask
	while (i < session_info.get_CHANELS() - width)
	{
		if (mask.at(i) == 0.0)
		{
			for (size_t j = i-width; j < i+width; j++)
				mask.at(j) = 0.0;
			i += width;
		}

		i++;
	}

	// fill the end of the mask
	while (i < session_info.get_CHANELS())
	{
		if (mask.at(i) == 0.0)
		{
			for (size_t j = i-width; j < session_info.get_CHANELS(); j++)
				mask.at(j) = 0.0;
			i += width;
		}

		i++;
	}


	// if all channels are noisy than observation is corrupted
	throw_if_mask_is_null();

	if (cfg->verbose)
		cout << OK << endl;
}


void Frequency_response::print(string file_name)
{
// print frequency responce into a file in format:
// chan1 freq1 amlitude1
// ...
// chann freqn amplituden

	size_t channels = session_info.get_CHANELS();
	double freq_min = session_info.get_FREQ_MIN();
	double freq_max = session_info.get_FREQ_MAX();

	double df = (freq_max - freq_min)/double(channels);

	ofstream out(file_name);

	for (size_t i = 0; i < channels; ++i)
		out << i << " " << freq_min + df*double(i) << " " << profile[i] << endl;
}

void Frequency_response::print_masked(string file_name)
{
// print frequency responce after the filtration into a file in format:
// chan1 freq1 amlitude1
// ...
// chann freqn amplituden

	size_t channels = session_info.get_CHANELS();
	double freq_min = session_info.get_FREQ_MIN();
	double freq_max = session_info.get_FREQ_MAX();

	double df = (freq_max - freq_min)/double(channels);

	ofstream out(file_name);

	for (size_t i = 0; i < channels; i++)
		out << i << " " << freq_min + df*double(i) << " " << profile[i]*mask[i] << endl;

}
