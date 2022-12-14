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

extern Configuration cfg;

Frequency_response::Frequency_response(Raw_profile& raw) : session_info()
{
	if (cfg.verbose)
		cout << "Making frequency response" << endl;;

	session_info = raw.session_info;

	fill_profile(raw.mean_signal_per_chanel);
	fill_mask();

}

Frequency_response::Frequency_response(Int_profile& int_prf) : session_info()
{
	if (cfg.verbose)
		cout << "Making frequency response" << endl;;

	session_info = int_prf.session_info;

	fill_profile(int_prf.compensated_signal_per_chanel);
	fill_mask();
}


void Frequency_response::fill_profile(vector<vector<double>> signal_per_chanel)
{
	if (cfg.verbose)
		cout << SUB << "Calculating profile of frequency response...";

	int channels = session_info.get_CHANELS(); 
	int obs_window = session_info.get_OBS_WINDOW();

	profile = vector<double> (channels);
	fill(profile.begin(), profile.end(), 0.0);


	for (int i = 0; i < channels; i++)
		for(int j = 0; j < obs_window; j++)
			profile[i] += signal_per_chanel[i][j]; 

	if (cfg.verbose)
		cout << OK << endl;
}

void Frequency_response::fill_mask()
{
	int channels = session_info.get_CHANELS(); 

	mask = vector<double> (channels);

	fill(mask.begin(), mask.end(), 1.0);

}

void Frequency_response::derivative_filter(double p1)
{
	if (cfg.verbose)
		cout << SUB << "Derivative filter...";

	double m = median(profile);

	for (int i = 0; i < session_info.get_CHANELS() - 1; i++)
	{
		if ((profile[i] - profile[i+1])/m > p1)
		{
			mask[i] = 0.0;
		}
	}

	if (cfg.verbose)
		cout << OK << endl;
}

void Frequency_response::derivative_filter(double p1, int width)
{
	if (cfg.verbose)
		cout << SUB << "Derivative filter...";

	double m = median(profile);
	int hw = width/2;

	int channels = session_info.get_CHANELS();

	for (int i = 0; i < hw; i++)
	{
		if ((profile[i] - profile[i+1])/m > p1)
		{
			for (int j = i; j < i+hw+1; j++)
				mask[j] = 0.0;
		}
	}

	for (int i = hw; i < channels - hw - 1; i++)
	{
		if (abs(profile[i] - profile[i+1])/m > p1)
		{
			for (int j = i-hw; j < i+hw+1; j++)
				mask[j] = 0.0;
		}
	}

	for (int i = channels - hw - 1; i < channels - 1; i++)
	{
		if ((profile[i] - profile[i+1])/m > p1)
		{
			for (int j = i-hw; j < i+1; j++)
				mask[j] = 0.0;
		}
	}

	if (cfg.verbose)
		cout << OK << endl;
}

void Frequency_response::median_filter(double p2)
{
	if (cfg.verbose)
		cout << SUB << "Median filter...";

	double m;
	double s;

	for (int i = 0; i < session_info.get_CHANELS(); i++)
	{
		m = median(profile, i-5, i+5);
		s = sigma(profile, i-5, i+5);

		if ((profile[i] - m)/s > p2)
		{
			mask[i] = 0.0;
		}
	}

	if (cfg.verbose)
		cout << OK << endl;
}

void Frequency_response::print(string file_name)
{
	int chanels = session_info.get_CHANELS();
	double freq_min = session_info.get_FREQ_MIN();
	double freq_max = session_info.get_FREQ_MAX();

	double df = (freq_max - freq_min)/double(chanels);

	ofstream out(file_name);

	for (int i = 0; i < chanels; i++)
		out << i << " " << freq_min + df*double(i) << " " << profile[i] << endl;
}

void Frequency_response::print_masked(string file_name)
{
	int chanels = session_info.get_CHANELS();
	double freq_min = session_info.get_FREQ_MIN();
	double freq_max = session_info.get_FREQ_MAX();

	double df = (freq_max - freq_min)/double(chanels);

	ofstream out(file_name);

	for (int i = 0; i < chanels; i++)
		out << i << " " << freq_min + df*double(i) << " " << profile[i]*mask[i] << endl;

}
