#include"../lib/etalon_profile.h"

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

void Etalon_profile::normilize(string out_file)
{
	cout << "Normilizing etalon profiles . . ." << endl;

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

	ofstream out (out_file);

	for (int j = 0; j < obs_window; j++)
		out << profile[j] << endl;
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

double Etalon_profile::get_TAU(){return tau;}
int Etalon_profile::get_OBS_WINDOW(){return obs_window;}
