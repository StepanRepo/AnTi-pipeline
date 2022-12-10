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
	toa_error = 0.0l;

	snr = 0.0;
	snr = get_SNR();

	freq_comp = session_info.get_FREQ_MAX();
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
	toa_error = 0.0l;

	snr = 0.0;
	snr = get_SNR();

	freq_comp = session_info.get_FREQ_MAX();
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
		delta_dec = (chanel_delay[i] - tau*double(delta_int))/tau;

		noise = median(raw->mean_signal_per_chanel[i]);

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

			temp_1[j] = raw->mean_signal_per_chanel[i][(j + delta_int) % obs_window];
			temp_2[j] = raw->mean_signal_per_chanel[i][(j + delta_int + 1) % obs_window];
		}

		for(int j = 0; j < obs_window; j++)
		{
			compensated_signal_per_chanel[i][j] = (1.0 - delta_dec)*temp_1[j] + delta_dec*temp_2[j];
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
		etalon = etalon.scale_profile(session_info.get_TAU());

		double reper_point = get_reper_point(etalon); 

		long double mjd_start = session_info.get_START_UTC().get_MJD();

		return mjd_start + (long double) (reper_point*session_info.get_TAU())*1e-3/86400.0l;
	}
}


double Int_profile::get_reper_point (Etalon_profile& etalon)
{
	// Calculate discrete cross-correlation 
	// function of integral and etalon profiles
	int etalon_len = etalon.profile.size();
	int int_len = profile.size();

	vector<double> ccf;
	ccf.reserve(int_len + etalon_len);

	for (int i = -etalon_len; i < int_len; i++)
		ccf.push_back(discrete_ccf(profile, etalon.profile, i));

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

	//for (int )

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

	derivative.push_back(4.0*coefficients[0]);
	derivative.push_back(3.0*coefficients[1]);
	derivative.push_back(2.0*coefficients[2]);
	derivative.push_back(coefficients[3]);

	double reper_dec =  find_root(derivative, -1.0, 1.0);

	return double(max_pos - etalon_len) + reper_dec;
}

double Int_profile::get_ERROR()
{
	if (toa_error != 0)
	{
		return toa_error;
	}
	else
	{
		cout << "Calculating TOA error . . ." << endl;

		int obs_window = session_info.get_OBS_WINDOW();
		int k1 = 0, k2 = 0;

		// half width full maximum level is
		// equal 0.5 
		double hwfm_level = 0.5;
		//hwfm_level = 0.0;
		//for (int i = 0; i < session_info.get_OBS_WINDOW(); i++)
		//	if (hwfm_level < profile[i])
		//		hwfm_level = profile[i];
		//hwfm_level /= 2.0;

		int i = 0;

		while (i < obs_window)
		{
			if (profile[i] > hwfm_level)
			{
				k1 = i;
				break;
			}
			i++;
		}

		while (i < obs_window)
		{
			if (profile[i] < hwfm_level)
			{
				k2 = i;
				break;
			}
			i++;
		}

		toa_error = 0.3*double(k2-k1)*session_info.get_TAU()/get_SNR();	

		return toa_error;
	}
}


double Int_profile::get_SNR()
{
	if (snr != 0.0)
	{
		return snr;
	}
	else
	{
		int obs_window = session_info.get_OBS_WINDOW();

		// find level of noise as
		// 1.0% of maximum of signal

		// max is equal 1.0
		//for (int i = 0; i < obs_window; i++)
		//	if (max < profile[i]) max = profile[i];

		double max = 1e-1;

		vector<double> noise_vec;
		noise_vec.reserve(obs_window);

		for (int i = 0; i < obs_window; i++)
		{
			if(profile[i] < max)
			{
				noise_vec.push_back(profile[i]);
			}
		}

		double noise = sigma(noise_vec);

		snr = 1.0/(noise);

		return snr;
	}
}

void Int_profile::print(string file_name)
{
	session_info.print(file_name, freq_comp);

	ofstream out (file_name, ios_base::app);

	for (int i = 0; i < session_info.get_OBS_WINDOW(); i++)
		out << profile[i] << endl;	

	out.close();
}
