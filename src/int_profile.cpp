#include"../lib/int_profile.h"
#include"../lib/session_info.h"
#include"../lib/raw_profile.h"
#include"../lib/custom_math.h"
#include"../lib/configuration.h"
#include"../lib/massages.h"

#include<vector>
#include<iostream>
#include<fstream>
#include<cstdio>

using namespace std;

extern Configuration *cfg;

Int_profile::Int_profile(Raw_profile& raw) : session_info()
{
	if (cfg->verbose)
		cout << "Making integral profile" << endl;

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
	if (cfg->verbose)
		cout << "Making integral profile" << endl;

	session_info = raw.session_info;

	int obs_window = session_info.get_OBS_WINDOW();
	int channels = session_info.get_CHANELS(); 

	vector<double> chanel_delay (channels);
	compensated_signal_per_chanel = vector (channels, vector<double>(obs_window));

	calculate_chanel_delay (chanel_delay);
	move_chanel_profiles(&raw, chanel_delay);

	profile = vector<double> (obs_window);

	for (int j = 0; j < obs_window; j++)
		profile[j] = 0.0;

	average_profiles(mask);

	normilize_profile();

	toa = 0.0l;
	toa_error = 0.0l;

	snr = 0.0;
	snr = get_SNR();

	freq_comp = session_info.get_FREQ_MAX();
}


Int_profile::Int_profile (string file_name) : session_info(file_name, false)
{
	if (cfg->verbose)
		cout << "Reading integral profile" << endl;

	ifstream obs_file (file_name, ios::in);

	if (!obs_file)
		throw invalid_argument (string(ERROR) + "Cann't open observational file to read data" + file_name);

	int obs_window = session_info.get_OBS_WINDOW();

	compensated_signal_per_chanel = vector (1, vector<double>(obs_window));
	profile = vector<double> (obs_window);

	string buffer;

	// skip header of file
	for (int i = 0; i < session_info.get_NUM_PARAMS(); i++)
		getline(obs_file, buffer);	
	
	for (int i = 0; i < obs_window; i++)
	{
		getline(obs_file, buffer);	

		// delete first column of input: time
		buffer = buffer.substr(buffer.find(' ') + 1);

		// change decimal comma to decimal point
		size_t pos = buffer.find(',');
		if (pos < 200)
			buffer[pos] = '.';

		profile[i] = stod(buffer);
	}

	normilize_profile();

	toa = 0.0l;
	toa_error = 0.0l;

	snr = 0.0;
	snr = get_SNR();
}

void Int_profile::calculate_chanel_delay(vector<double>& chanel_delay)
{
	if (cfg->verbose)
		cout << SUB << "Calculating chanel delay...";

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
	

	if (cfg->verbose)
		cout << OK << endl;
}

void Int_profile::move_chanel_profiles(Raw_profile* raw, std::vector<double>& chanel_delay)
{
	if (cfg->verbose)
		cout << SUB << "Moving chanel profiles...";

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

	if (cfg->verbose)
		cout << OK << endl;
}


void Int_profile::average_profiles()
{
	if (cfg->verbose)
		cout << SUB << "Averaging chanel profiles...";

	int chanels = session_info.get_CHANELS();
	int obs_window = session_info.get_OBS_WINDOW();

	for (int j = 0; j < obs_window; j++)
	{
		for (int i = 0; i < chanels; i++)
		{
			profile[j] += compensated_signal_per_chanel[i][j];
		}
	}

	if (cfg->verbose)
		cout << OK << endl;
}

void Int_profile::average_profiles(vector<double> mask)
{
	if (cfg->verbose)
		cout << SUB << "Averaging chanel profiles whith mask...";

	int chanels = session_info.get_CHANELS();
	int obs_window = session_info.get_OBS_WINDOW();

	for (int j = 0; j < obs_window; j++)
	{
		for (int i = 0; i < chanels; i++)
		{
			profile[j] += mask[i]*compensated_signal_per_chanel[i][j];
		}
	}
	
	if (cfg->verbose)
		cout << OK << endl;
}

void Int_profile::normilize_profile()
{
	if (cfg->verbose)
		cout << SUB << "Normilizing integral profile...";

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

	if (cfg->verbose)
		cout << OK << endl;
}


long double Int_profile::get_TOA(Etalon_profile& etalon_in)
{
	if (toa != 0.0l)
	{
		return toa;
	}
	else
	{
		// Define etalon profile and scale it if it's nesesssery
		Etalon_profile etalon (etalon_in.profile, etalon_in.get_TAU(), etalon_in.get_OBS_WINDOW());
		etalon = etalon.scale_profile(session_info.get_TAU());

		if (cfg->verbose)
			cout << SUB << "Calculating TOA...";

		double reper_point = get_reper_point(etalon); 

		long double mjd_start = session_info.get_START_UTC().get_MJD();

		if (cfg->verbose)
			cout << OK << endl;

		toa = mjd_start + (long double) (reper_point*session_info.get_TAU())*1e-3/86400.0l;

		return toa;
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
		if (cfg->verbose)
			cout << SUB << "Calculating TOA error...";

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

		if (cfg->verbose)
			cout << OK << endl;

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

		double max = 2.0*median(profile);

		vector<double> noise_vec, signal_vec;
		noise_vec.reserve(obs_window);
		signal_vec.reserve(obs_window);

		for (int i = 0; i < obs_window; i++)
		{
			if(profile[i] < max)
			{
				noise_vec.push_back(profile[i]);
			}
			else
				signal_vec.push_back(profile[i]);
		}

		double signal_power = 0.0;
		for (int i = 0; i < (int) signal_vec.size(); i++)
			signal_power += signal_vec[i];
		signal_power /= (double) signal_vec.size();

		double noise = sigma(noise_vec);

		snr = signal_power/(noise);

		return snr;
	}
}

void Int_profile::read_freq_comp (string file_name)
{
	ifstream obs_file(file_name, ios::in);
	string buffer, name;

	if (!obs_file)
		throw invalid_argument (string(ERROR) + "Cann't open observational file to read header" + file_name);

	int i = 0;

	while (getline (obs_file, buffer))
	{
		name = buffer.substr(0, buffer.find(' '));

		if (name == "Fcomp" || name == "freq_comp")
		{
			try
			{
				freq_comp = stod(buffer.substr(buffer.find(' ') + 1));
			}
			catch (const invalid_argument &err)
			{
				throw invalid_argument (string(ERROR) + "Cann't read comparsion frequency from integral profile file");
			}
		}

		i++;	
		if (i == session_info.get_NUM_PARAMS())
			break;
	}
}

double Int_profile::get_FREQ_COMP() {return freq_comp;}

void Int_profile::print(string file_name)
{
	session_info.print(file_name, freq_comp);

	double tau = session_info.get_TAU();

	ofstream out (file_name, ios_base::app);
	out.precision(8);

	for (int i = 0; i < session_info.get_OBS_WINDOW(); i++)
		out << tau * (double) i << " " << profile[i] << endl;	

	out.close();
}

void Int_profile::ITOA()
{
	string file_name = cfg->output_dir + "toa";

	FILE *out = fopen (file_name.c_str(), "a+");
	
	char* format = (char*) "%-9s%18.12Lf%6.1f%11.3f%11f %3s\n";

	const char* name;
	name = session_info.get_PSR_NAME().c_str();

	fprintf(out, format, name, toa, toa_error*1e3, freq_comp, 0.0, "PO");

	fclose(out);
}
