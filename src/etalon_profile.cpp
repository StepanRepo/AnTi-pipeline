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

Etalon_profile::Etalon_profile(vector<vector<double>> profile_ext, double tau_ext, int obs_window_ext, int channels_ext)
{
	tau = tau_ext;
	obs_window = obs_window_ext;
	channels = channels_ext;

	profile = vector (channels, vector<double>(obs_window));
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

	getline(tpl_file, header_buffer);	
		channels = stoi(header_buffer.substr(11));

	tpl_file.close();
}

void Etalon_profile::fill_profile(string file_name)
{
	ifstream tpl_file (file_name, ios::in);

	if (!tpl_file)
		throw invalid_argument (string(ERROR) + "Cann't open file with template profile " + file_name);
	
	for (int i = 0; i < 3; i ++)
		tpl_file.ignore(100, '\n');

	profile = vector (channels, vector<double>(obs_window));
	double ampl;

	for (int i = 0; i < channels; i++)
	{
		for (int j = 0; j < obs_window; j++)
		{
			tpl_file >> ampl;

			profile.at(i).at(j) = ampl;
		}
	}
}


void Etalon_profile::normilize()
{
	if (cfg->verbose)
		cout << SUB << "Normilizing of template profile . . .";

	normilize_vector(profile);

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

		return Etalon_profile(profile, tau, obs_window, channels);
	}

	double ratio = tau/tau_new;

	int obs_window_new = (int) obs_window * ratio;
	vector<vector<double>> profile_new = vector (channels, vector<double>(obs_window_new));

	int n;


	for (int i = 0; i < channels; ++i)
	{
		for (int k = 0; k < obs_window_new; ++k)
		{
			n = (int) k / ratio;

			profile_new.at(i).at(k) = profile.at(i).at(n) + (profile.at(i).at(n+1) - profile.at(i).at(n))
			       	* (double(k)*tau_new - double(n)*tau)/tau;
		}
	}

	if (cfg->verbose)
		cout << OK << endl;

	//ofstream out2 ("out/new_prf");
	//for (int i = 0; i < obs_window_new; i++)
	//{
	//	out2 << profile_new[i] << endl;
	//}
	//out2.close();

	return Etalon_profile(profile_new, tau_new, obs_window, channels);

}

Etalon_profile::Etalon_profile(vector<Int_profile>& profiles_series)
{
	if (cfg->verbose)
		cout << "Constructing template profile" << endl;

	int n = profiles_series.size();

	profile = profiles_series.at(0).profile;
	tau = profiles_series.at(0).session_info.get_TAU();
	obs_window = profiles_series.at(0).session_info.get_OBS_WINDOW();
	channels = profiles_series.at(0).session_info.get_CHANELS();

	vector<vector<double>> current_int (channels, vector<double>(obs_window));

	vector<double> ccf (2*obs_window);

	double reper_dec;
	vector<double> near_max(5), coefficients(5), derivative(4);

	int c_channels, c_tau;

	for (int i = 1; i < n; i++)
	{
		current_int = profiles_series.at(i).profile;

		c_tau = profiles_series.at(i).session_info.get_TAU();
		c_channels = profiles_series.at(i).session_info.get_CHANELS();

		if (c_channels != channels)
		{
			cout << WARNING << "Number of frequency channels must be the same for all profiles. Profile will not use" << endl;
			continue;
		}
		if (c_tau != tau)
		{
			cout << WARNING << "Tau of profiles must be the same for all profiles. Profile will not use" << endl;
			continue;
		}

		// check the size of current vector
		for (int c = 0; c < channels; ++c)
		{
			current_int.at(c).resize(obs_window, 0.0);
		}

		// correlation of profiles
		fill(ccf.begin(), ccf.end(), 0.0);

		for (int j = 0; j < 2*obs_window; j++)
			ccf.at(j) = cycle_discrete_ccf(current_int, profile, j - obs_window);

		int max_pos_ = max_pos(ccf);
		double max_ = max(ccf);


		for (int j = -2; j < 3; ++j)
		{
			if (j + max_pos_ < 0)
				near_max.at(j+2) = ccf.at((max_pos_ + j) + (2*obs_window))/max_;
			else if (j + max_pos_ > 2 * obs_window)
				near_max.at(j+2) = ccf.at((max_pos_ + j) - (2*obs_window))/max_;
			else
				near_max.at(j+2) = ccf.at(max_pos_ + j)/max_;
		}

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

		move_continous(current_int, (double) max_pos_ + reper_dec);

		for (int c = 0; c < channels; ++c)
		{
			for (int j = 0; j < obs_window; ++j)
			{
				profile.at(c).at(j) += current_int.at(i).at(j);
			}
		}
	}	


	normilize();
}

double Etalon_profile::get_SNR()
{
	return SNR(profile);
}



void Etalon_profile::print(string file_name)
{
	ofstream out (file_name);

	out << "tau = " << tau << endl;
	out << "numpointwin = " << obs_window << endl;
	out << "channels = " << channels << endl;


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

double Etalon_profile::get_TAU(){return tau;}
int Etalon_profile::get_OBS_WINDOW(){return obs_window;}
int Etalon_profile::get_CHANELS(){return channels;}
