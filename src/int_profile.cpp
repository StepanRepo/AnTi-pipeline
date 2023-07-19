#include"../lib/int_profile.h"
#include"../lib/session_info.h"
#include"../lib/raw_profile.h"
#include"../lib/frequency_response.h"
#include"../lib/custom_math.h"
#include"../lib/configuration.h"
#include"../lib/massages.h"

#include<vector>
#include<math.h>
#include<iostream>
#include<fstream>
#include<cstdio>

using namespace std;

extern Configuration *cfg;

Int_profile::Int_profile(Raw_profile& raw, vector<double>* mask) : session_info()
{
	if (cfg->verbose)
		cout << "Making integral profile" << endl;

	session_info = raw.session_info;
	freq_comp = session_info.get_FREQ_MAX();

	int obs_window = session_info.get_OBS_WINDOW();
	int channels = session_info.get_CHANELS(); 

	if (!mask)
	{
		*mask = vector<double> (channels);
		fill (mask->begin(), mask->end(), 1.0);
	}


	profile = vector (channels, vector<double>(obs_window));
	profile = raw.mean_signal_per_chanel;

	normilize_profile();
	use_mask(*mask);

	toa = 0.0l;
	toa_error = 0.0l;

	snr = 0.0;
	snr = get_SNR();
}


Int_profile::Int_profile (string file_name) : session_info(file_name, false)
{
	if (cfg->verbose)
		cout << "Reading integral profile" << endl;

	ifstream obs_file (file_name, ios::in);

	if (!obs_file)
		throw invalid_argument (string(ERROR) + "Cann't open observational file to read data" + file_name);

	int obs_window = session_info.get_OBS_WINDOW();
	int channels = session_info.get_CHANELS();

	profile = vector (channels, vector<double>(obs_window));

	string buffer;

	// skip header of file
	for (int i = 0; i < session_info.get_NUM_PARAMS(); i++)
		getline(obs_file, buffer);	
	
	double ampl;

	for (int i = 0; i < channels; i++)
	{
		for (int j = 0; j < obs_window; j++)
		{
			obs_file >> ampl;
			profile.at(i).at(j) = ampl;
		}
	}

	normilize_profile();

	toa = 0.0l;
	toa_error = 0.0l;

	snr = 0.0;
	snr = get_SNR();
}


void Int_profile::use_mask(vector<double> mask)
{
	if (cfg->verbose)
		cout << SUB << "Using channels mask...";

	int chanels = session_info.get_CHANELS();
	int obs_window = session_info.get_OBS_WINDOW();

	for (int i = 0; i < chanels; i++)
	{
		for (int j = 0; j < obs_window; j++)
		{
			profile.at(i).at(j) = mask.at(i) * profile.at(i).at(j);
		}
	}
	
	if (cfg->verbose)
		cout << OK << endl;
}

void Int_profile::normilize_profile()
{
	if (cfg->verbose)
		cout << SUB << "Normilizing integral profile...";

	double noise;
	int chanels = session_info.get_CHANELS();
	int obs_window = session_info.get_OBS_WINDOW();

	for (int i = 0; i < chanels; i++)
	{
		noise = median(profile.at(i));

		if(noise == 0.0) noise = 1.0;

		for (int j = 0; j < obs_window; j++)
		{
			profile.at(i).at(j) /= noise;
		}
	}

	normilize_vector(profile);

	if (cfg->verbose)
		cout << OK << endl;
}


long double Int_profile::get_TOA(Etalon_profile& etalon_in)
{
	if (toa == 0.0l)
	{
		// Define etalon profile and scale it if it's nesesssery
		Etalon_profile etalon = etalon_in.scale_profile(session_info.get_TAU());


		if (cfg->verbose)
			cout << SUB << "Calculating TOA...";

		double reper_point = get_reper_point(etalon); 
		long double mjd_start = session_info.get_START_UTC().get_MJD();

		if (cfg->verbose)
			cout << OK << endl;

		toa = mjd_start + (long double) (reper_point * session_info.get_TAU())*1e-3/86400.0l;

	}

	return toa;
}


double Int_profile::get_reper_point (Etalon_profile& etalon)
{
	// Calculate discrete cross-correlation 
	// function of integral and etalon profiles
	int etalon_len = etalon.profile.at(0).size();
	vector<double> ccf1d;

	if (cfg->get_ccf)
		ccf1d = discrete_ccf(profile, etalon.profile,
			       cfg->output_dir + session_info.get_FILE_NAME() + ".ccf");
	else
		ccf1d = discrete_ccf(profile, etalon.profile);


	// find position of the maximum of ccf
	double max_ = max(ccf1d);
	int max_pos_ = max_pos(ccf1d);

	if (max_pos_ > 1.5*session_info.get_OBS_WINDOW()
	 || max_pos_ < 0.5*session_info.get_OBS_WINDOW())
	{
		vector<double> ccf1d_moved = ccf1d;
		move_continous(ccf1d_moved, session_info.get_PSR_PERIOD()*1e3/session_info.get_TAU()) ;

		for (size_t i = 0; i < ccf1d.size(); ++i)
			ccf1d.at(i) += ccf1d_moved.at(i);

		max_ = max(ccf1d);
		max_pos_ = max_pos(ccf1d);
	}


	// Clarify the position of maximum of
	// the continous ccf
	vector<double> near_max;
	near_max.reserve(5);

	for (int i = -2; i < 3; i++)
		near_max.push_back(ccf1d.at(max_pos_ + i)/max_);


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

	double reper_dec = find_root(derivative, -1.0, 1.0);

	return double(max_pos_ - etalon_len) + reper_dec;
}

double Int_profile::get_ERROR()
{
	if (cfg->verbose)
		cout << SUB << "Calculating TOA error...";

	if (toa_error == 0)
	{
		int n = session_info.get_OBS_WINDOW();
		int channels = session_info.get_CHANELS();

		vector<double> c0 (channels), mass(channels), dc(channels);

#pragma omp parallel for
		for (int c = 0; c < channels; ++c)
		{
			for (int i = 0; i < n; ++i)
			{
				c0.at(c) += profile.at(c).at(i) * ((double) i);
				mass.at(c) += profile.at(c).at(i);
			}

			c0.at(c) /= mass.at(c);
		}

		for (int c = 0; c < channels; ++c)
		{

			if (mass.at(c) == 0.0)
			{
				c0.erase(c0.begin() + c);
				mass.erase(mass.begin() + c);
				--c;
				--channels;
				continue;
			}

			for (int i = 0; i < n; ++i)
				dc.at(c) += (i - c0.at(c)) * (i - c0.at(c));

			dc.at(c) = sqrt(dc.at(c)) / mass.at(c);
		}

		toa_error = mean(dc)/get_SNR() * session_info.get_TAU()*1e-3;

	}

	if (cfg->verbose)
		cout << OK << endl;

	return toa_error;
}


double Int_profile::get_SNR()
{
	if (snr == 0.0)
		snr = SNR(profile);

	return snr;
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
				freq_comp = stod(buffer.substr(buffer.find(' ') + 1)) * 1e-3;
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

	int channels = session_info.get_CHANELS();
	int obs_window = session_info.get_OBS_WINDOW();

	ofstream out (file_name, ios_base::app);
	out.precision(8);

	for (int i = 0; i < channels; ++i)
	{
		for (int j = 0; j < obs_window; ++j)
		{
			out << profile.at(i).at(j) << " ";	
		}

		out << endl;
	}


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
