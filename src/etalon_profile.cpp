#include"../lib/etalon_profile.h"
#include"../lib/int_profile.h"
#include"../lib/massages.h"
#include"../lib/custom_math.h"

#include<iostream>
#include<fstream>
#include<vector>
#include<string>

using namespace std;

Etalon_profile::Etalon_profile(string file_name)
{
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
		throw invalid_argument (string(ERROR) + "Cann't open file with template profile" + file_name);

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
		throw invalid_argument (string(ERROR) + "Cann't open file with template profile" + file_name);
	
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
	cout << "Normilizing etalon profile . . ." << endl;

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



Etalon_profile Etalon_profile::scale_profile(double tau_new)
{
	if (tau == tau_new) 
		return Etalon_profile(profile, tau, obs_window);

	int ratio = int (tau/tau_new);

	int obs_window_new = obs_window * ratio;
	vector<double> profile_new = vector<double>(obs_window_new);

	int n;

	for (int k = 0; k < obs_window_new; k ++)
	{
		n = k * ratio;

		profile_new[k] = profile[n] + (profile[n+1] - profile[n]) * (double(k)*tau_new - double(n)*tau)/tau;
	}

	return Etalon_profile(profile_new, tau_new, obs_window);

}

Etalon_profile make_tpl (vector<vector<double>>& profiles, double tau)
{
	size_t n = profiles.size();

	vector<double> profile; 
	vector<double> mean_profile;

	mean_profile = profiles[0];
	size_t obs_window = mean_profile.size();


	vector<double> ccf (2*obs_window);

	double reper_dec;
	vector<double> near_max(5), coefficients(5), derivative (4);

	double max; 
	size_t max_pos;

	for (size_t i = 1; i < n; i++)
	{
		profile = profiles[i];

		// check the size of current vector
		if (profile.size() > obs_window)
			profile.erase(profile.begin() + obs_window+1);
		else if (profile.size() < obs_window)
		{
			profile.reserve(obs_window - profile.size());

			for (size_t i = profile.size(); i < obs_window; i ++)
			       profile.push_back(0.0);	
		}

		// correlation of profiles
		for (size_t j = 0; j < 2*obs_window; j++)
			ccf[j] = discrete_ccf(mean_profile, profile, j - obs_window);
		
		max = 0.0;
		max_pos = 0;

		for (size_t j = 0; j < 2*obs_window; j++)
		{
			if (ccf[j] > max)
			{
				max = ccf[j];
				max_pos = j;
			}
		}

		for (size_t j = -2; j < 3; j++)
			near_max[i+2] = ccf[max_pos + i]/max;

		coefficients = interpolation4(near_max);

		derivative[0] = 4.0*coefficients[0];
		derivative[1] = 3.0*coefficients[1];
		derivative[2] = 2.0*coefficients[2];
		derivative[3] = coefficients[3];

		reper_dec =  find_root(derivative, -1.0, 1.0);

		move_continous(profile, (double) max_pos + reper_dec);

		for (size_t j = 0; j < obs_window; j++)
			mean_profile[j] += profile[j];
	}	


	Etalon_profile result (mean_profile, tau, (int) obs_window);
	result.normilize();

	return result;
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
