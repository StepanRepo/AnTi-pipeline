#include"../lib/etalon_profile.h"
#include"../lib/int_profile.h"
#include"../lib/massages.h"
#include"../lib/custom_math.h"
#include"../lib/configuration.h"

#include<iostream>
#include<fstream>
#include<vector>
#include<string>

using namespace std;

extern Configuration* cfg;

Etalon_profile::Etalon_profile(string file_name)
{
	if (cfg->verbose)
		cout << "Reading template profile" << endl;

	read_header(file_name);
	fill_profile(file_name);
}

Etalon_profile::Etalon_profile(vector<double> profile_ext, double tau_ext, int obs_window_ext)
{
	tau = tau_ext;
	obs_window = obs_window_ext;

	profile = profile_ext;
}


void Etalon_profile::read_header(string file_name)
{
	ifstream tpl_file (file_name, ios::in);

	if (!tpl_file)
		throw invalid_argument (string(ERROR) + "Cann't open file with template profile " + file_name);

	string header_buffer;

	getline(tpl_file, header_buffer);	
		tau = stod(header_buffer.substr(6));

	getline(tpl_file, header_buffer);	
		obs_window = stoi(header_buffer.substr(14));

	tpl_file.close();
}

void Etalon_profile::fill_profile(string file_name)
{
	ifstream tpl_file (file_name, ios::in);

	if (!tpl_file)
		throw invalid_argument (string(ERROR) + "Cann't open file with template profile " + file_name);
	
	for (int i = 0; i < 2; i ++)
		tpl_file.ignore(100, '\n');

	profile.reserve(obs_window);
	double ampl;

	for (int i = 0; i < obs_window; i++)
	{
		tpl_file >> ampl;
		profile.push_back(ampl);
	}
}


void Etalon_profile::normilize()
{
	if (cfg->verbose)
		cout << SUB << "Normilizing of template profile . . .";

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



Etalon_profile Etalon_profile::scale_profile(double tau_new)
{
	if (cfg->verbose)
		cout << SUB << "Scaling of template profile...";

	if (tau == tau_new) 
	{
		if (cfg->verbose)
			cout << OK << endl;

		return Etalon_profile(profile, tau, obs_window);
	}

	double ratio = tau/tau_new;

	int obs_window_new = (int) obs_window * ratio;
	vector<double> profile_new = vector<double>(obs_window_new);

	int n;

	for (int k = 0; k < obs_window_new; k ++)
	{
		n = (int) k / ratio;

		profile_new[k] = profile[n] + (profile[n+1] - profile[n]) * (double(k)*tau_new - double(n)*tau)/tau;
	}

	if (cfg->verbose)
		cout << OK << endl;

	//ofstream out2 ("out/new_prf");
	//for (int i = 0; i < obs_window_new; i++)
	//{
	//	out2 << profile_new[i] << endl;
	//}
	//out2.close();

	return Etalon_profile(profile_new, tau_new, obs_window);

}

Etalon_profile::Etalon_profile(vector<Int_profile>& profiles_series)
{
	if (cfg->verbose)
		cout << "Constructing template profile" << endl;

	int n = profiles_series.size();

	profile = profiles_series[0].profile;
	tau = profiles_series[0].session_info.get_TAU();
	obs_window = profiles_series[0].session_info.get_OBS_WINDOW();

	vector<double> current_int (obs_window);

	vector<double> ccf (2*obs_window);

	double reper_dec;
	vector<double> near_max(5), coefficients(5), derivative (4);

	double max; 
	int max_pos;

	for (int i = 1; i < n; i++)
	{
		current_int = profiles_series[i].profile;

		// check the size of current vector
		if ((int) current_int.size() > obs_window)
		{
			current_int.resize(obs_window);
		}
		else if ((int) current_int.size() < obs_window)
		{
			current_int.reserve(obs_window - current_int.size());

			for (int i = current_int.size(); i < obs_window; i ++)
			       current_int.push_back(0.0);	
		}

		// correlation of profiles
		fill(ccf.begin(), ccf.end(), 0.0);

		for (int j = 0; j < 2*obs_window; j++)
			ccf[j] = cycle_discrete_ccf(current_int, profile, j - obs_window);
		
		max = 0.0;
		max_pos = 0;

		for (int j = 0; j < 2*obs_window; j++)
		{
			if (ccf[j] > max)
			{
				max = ccf[j];
				max_pos = j;
			}
		}

		for (int j = -2; j < 3; j++)
			near_max[j+2] = ccf[max_pos + j]/max;

		coefficients = interpolation4(near_max);

		derivative[0] = 4.0*coefficients[0];
		derivative[1] = 3.0*coefficients[1];
		derivative[2] = 2.0*coefficients[2];
		derivative[3] = coefficients[3];

		try
		{
			reper_dec =  find_root(derivative, -1.0, 1.0);
		}
		catch (const invalid_argument &err)
		{
			continue;
		}

		move_continous(current_int, (double) max_pos + reper_dec);

		for (int j = 0; j < obs_window; j++)
			profile[j] += current_int[j];

	}	

	normilize();
}

double Etalon_profile::get_SNR()
{
	double snr;

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



void Etalon_profile::print(string file_name)
{
	ofstream out (file_name);

	out << "tau = " << tau << endl;
	out << "numpointwin = " << obs_window << endl;


	for (int i = 0; i < obs_window; i++)
		out << profile[i] << endl;	

	out.close();
}

double Etalon_profile::get_TAU(){return tau;}
int Etalon_profile::get_OBS_WINDOW(){return obs_window;}
