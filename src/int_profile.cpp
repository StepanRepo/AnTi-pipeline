#include<vector>
#include<iostream>
#include<fstream>

#include"../lib/int_profile.h"
#include"../lib/session_info.h"
#include"../lib/raw_profile.h"
#include"../lib/custom_math.h"

using namespace std;

Int_profile::Int_profile(Raw_profile& raw) : session_info()
{
	session_info = raw.session_info;

	int obs_window = session_info.get_OBS_WINDOW();
	int channels = session_info.get_CHANELS(); 

	vector<double> chanel_delay (channels);
	compensated_signal_per_chanel = vector (channels, vector<double>(obs_window));

	calculate_chanel_delay (chanel_delay);
	move_chanel_profiles(&raw, chanel_delay);

	profile = vector<double> (obs_window);

	for (int j = 0; j < obs_window; j++)
		profile[j] = 0;

	average_profiles();

	normilize_profile();

	toa = 0.0l;
}

Int_profile::Int_profile(Raw_profile& raw, vector<double> mask) : session_info()
{
	session_info = raw.session_info;

	int obs_window = session_info.get_OBS_WINDOW();
	int channels = session_info.get_CHANELS(); 

	vector<double> chanel_delay (channels);
	compensated_signal_per_chanel = vector (channels, vector<double>(obs_window));

	calculate_chanel_delay (chanel_delay);
	move_chanel_profiles(&raw, chanel_delay);

	profile = vector<double> (obs_window);

	for (int j = 0; j < obs_window; j++)
		profile[j] = 0;

	average_profiles(mask);

	normilize_profile();

	toa = 0.0l;
}

void Int_profile::calculate_chanel_delay(vector<double>& chanel_delay)
{
	cout << "Calculating chanel delay . . ." << endl;

	double dm = session_info.get_DM();
	double freq_max = session_info.get_FREQ_MAX();
	double freq_min = session_info.get_FREQ_MIN();

	int chanels = session_info.get_CHANELS();

	double df = freq_max - freq_min;
	double freq_current;

	for (int i = 0; i < chanels; i++)
	{
		freq_current = freq_min + double(i+1)*df/512.0;
		chanel_delay[i] = (1.0/(freq_current*freq_current) - 1.0/(freq_max*freq_max))*dm*1e7/2.41;
	}
}

void Int_profile::move_chanel_profiles(Raw_profile* raw, std::vector<double>& chanel_delay)
{
	cout << "Moving chanel profiles . . ." << endl;

	double tau = session_info.get_TAU();
	int chanels = session_info.get_CHANELS();
	int obs_window = session_info.get_OBS_WINDOW();

	long double period = session_info.get_PSR_PERIOD()*1e3;
	double delay;
	double noise;

	int delta_int;
	double delta_dec;

	vector<double> temp_1, temp_2;
	temp_1.reserve(obs_window);
	temp_2.reserve(obs_window);

	for (int i = 0; i < chanels; i++)
	{
		delta_int = int(chanel_delay[i]/tau);
		delta_dec = chanel_delay[i] - tau*double(delta_int);

		noise = median(raw->signal_per_chanel[i]);

		for(int j = 0; j < obs_window; j++)
		{
			if ((double(j + delta_int) + delta_dec) > double(obs_window))
			{
				if ((double(j + delta_int) + delta_dec)*tau < double(period))
				{
					temp_1[j] = temp_2[j] = noise;

					continue;
				}

				delay = chanel_delay[i] + obs_window*tau - period;

				delta_int = int(delay/tau);
				delta_dec = delay - tau*double(delta_int);
			}

			temp_1[j] = raw->signal_per_chanel[i][(j + delta_int) % obs_window];
			temp_2[j] = raw->signal_per_chanel[i][(j + delta_int + 1) % obs_window];
		}

		for(int j = 0; j < obs_window; j++)
		{
			compensated_signal_per_chanel[i][j] = (1 - delta_dec)*temp_1[j] + delta_dec*temp_2[j];
		}
	}
}


void Int_profile::average_profiles()
{
	cout << "Averaging chanel profiles . . ." << endl;

	int chanels = session_info.get_CHANELS();
	int obs_window = session_info.get_OBS_WINDOW();

	for (int j = 0; j < obs_window; j++)
	{
		for (int i = 0; i < chanels; i++)
		{
			profile[j] += compensated_signal_per_chanel[i][j];
		}
	}
}

void Int_profile::average_profiles(vector<double> mask)
{
	cout << "Averaging chanel profiles whith mask . . ." << endl;

	int chanels = session_info.get_CHANELS();
	int obs_window = session_info.get_OBS_WINDOW();

	for (int j = 0; j < obs_window; j++)
	{
		for (int i = 0; i < chanels; i++)
		{
			profile[j] += mask[i]*compensated_signal_per_chanel[i][j];
		}
	}
}

void Int_profile::normilize_profile()
{
	cout << "Normilizing integral profiles . . ." << endl;

	int obs_window = session_info.get_OBS_WINDOW();

	double min = profile[0];
	double max = 0;
	double norm_factor;

	for (int j = 0; j < obs_window; j++)
	{
		if (profile[j] < min) min = profile[j];
		if (profile[j] > max) max = profile[j];
	}

	norm_factor = max - min;

	for (int j = 0; j < obs_window; j++)
		profile[j] = (profile[j] - min)/norm_factor;


}


long double Int_profile::get_TOA(Etalon_profile& etalon_in)
{
	if (toa != 0.0l)
	{
		return toa;
	}
	else
	{
		cout << "Calculating TOA . . ." << endl;

		// Define etalon profile and scale it if it's nesesssery
		Etalon_profile etalon (etalon_in.profile, etalon_in.get_TAU(), etalon_in.get_OBS_WINDOW()); 

		double reper_point = get_reper_point(etalon); 

		long double mjd_start = session_info.get_START_UTC().get_MJD();


		cout.precision(16);
		cout << mjd_start + (long double) (reper_point*session_info.get_TAU())*1e-3/86400.0l << endl;


		return reper_point;
	}
}


double Int_profile::get_reper_point (Etalon_profile& etalon)
{
	// Calculate discrete cross-correlation 
	// function of integral and etalon profiles
	int etalon_len = etalon.profile.capacity();
	int int_len = profile.capacity();

	vector<double> ccf;
	ccf.reserve(int_len + etalon_len);

	for (int i = -etalon_len; i < int_len; i++)
		ccf[i + etalon_len] = discrete_ccf(profile, etalon.profile, i);

	// print ccf vector to file
	//ofstream out2 ("out/ccf");
	//for (int i = -580; i < 570; i++)
	//{
	//	out2 << ccf[i+580] << endl;
	//}
	//out2.close();


	// find position of the maximum of ccf
	double max = 0.0;
	int max_pos = 0;

	for (int i = 0; i < int_len + etalon_len; i++)
	{
		if (ccf[i] > max)
		{
			max = ccf[i];
			max_pos = i;
		}
	}

	// Clarify the position of maximum of
	// the continous ccf
	vector<double> near_max;
	near_max.reserve(5);

	for (int i = -2; i < 3; i++)
		near_max.push_back(ccf[max_pos + i]/max);


	// calculate interpolation coefficients near 
	// the maximum of discrete ccf
	vector<double> coefficients;
	coefficients.reserve(5);
		//c[0]x^4 + ... + c[4]
	coefficients = interpolation4(near_max);


	vector<double> derivative;
	derivative.reserve(4);

	for (int i = 0; i < 4; i++)
		derivative.push_back(coefficients[i]);

	double reper_dec =  find_root(derivative, -1.0, 1.0);

	return double(max_pos - etalon_len) + reper_dec;
}
