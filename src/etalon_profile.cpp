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
// this constructor reads template profile from a file

	if (cfg->verbose)
		cout << "Reading template profile" << endl;

	read_header(file_name);
	fill_profile(file_name);
}

Etalon_profile::Etalon_profile(vector<double> profile_ext, double tau_ext, int obs_window_ext)
{
// this constructor makes template profile with given characteristics

	tau = tau_ext;
	obs_window = obs_window_ext;

	profile = profile_ext;
}


void Etalon_profile::read_header(string file_name)
{
	// try to open file of observational session 
	ifstream tpl_file (file_name, ios::in);

	// check if file is opened
	if (!tpl_file)
		throw invalid_argument (string(ERROR) + "Cann't open file with template profile " + file_name);

	// read two lines of header
	// the first line is time interval between twe consequent points of profile (time discrete)
	// the second --- is number of points in the profile
	string header_buffer;

	getline(tpl_file, header_buffer);	
		tau = stod(header_buffer.substr(6));

	getline(tpl_file, header_buffer);	
		obs_window = stoi(header_buffer.substr(14));

	tpl_file.close();
}

void Etalon_profile::fill_profile(string file_name)
{
	// try to open file of observational session 
	ifstream tpl_file (file_name, ios::in);

	// check if file is opened
	if (!tpl_file)
		throw invalid_argument (string(ERROR) + "Cann't open file with template profile " + file_name);
	
	// skip first two lines of file (header)	
	for (int i = 0; i < 2; i ++)
		tpl_file.ignore(100, '\n');

	// place an array of profile in the RAM
	profile.reserve(obs_window);
	double ampl;

	// read profile amlitude line by line
	for (size_t i = 0; i < obs_window; i++)
	{
		tpl_file >> ampl;
		profile.push_back(ampl);
	}
}



Etalon_profile Etalon_profile::scale_profile(double tau_new)
{
// this function is nedded to re-discretize profile with higher time resolution
	if (cfg->verbose)
		cout << SUB << "Scaling of template profile...";

	// if old and new resolutions are equal then there is nithing to do
	if (tau == tau_new) 
	{
		if (cfg->verbose)
			cout << OK << endl;

		return Etalon_profile(profile, tau, obs_window);
	}

	// find two points of old profile which lie around given point of new profile
	// and perform linear interpolation between them
	double ratio = tau/tau_new;

	int obs_window_new = (int) obs_window * ratio;
	vector<double> profile_new = vector<double>(obs_window_new);

	int n;

	for (int k = 0; k < obs_window_new; k ++)
	{
		n = (int) k / ratio;

		profile_new[k] = profile[n] + 
			(profile[n+1] - profile[n]) * (double(k)*tau_new - double(n)*tau)/tau;
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
// calculate template profile by averaging a set of integral profiles

	if (cfg->verbose)
		cout << endl << BOLD << "Constructing template profile" << RESET << endl;

	// check if there is at least one integral profile 
	// to avoid segmentation fault
	if (profiles_series.size() == 0)
		throw invalid_argument 
			(string(ERROR) + "There is no intergral profiles to calculate template profile");

	size_t n = profiles_series.size();

	// set first integral profile in the list as 
	// initial approximation of template profile
	profile = profiles_series[0].profile;
	tau = profiles_series[0].session_info.get_TAU();
	obs_window = profiles_series[0].session_info.get_OBS_WINDOW();

	// declare temporal arrays for integral profile and ccf
	vector<double> current_int (obs_window);
	vector<double> ccf (2*obs_window);

	// temporal variables to establish a position of ccf maximum
	double reper_dec;
	vector<double> near_max(5), coefficients(5), derivative (4);

	for (size_t i = 1; i < n; ++i)
	{
		// get one profile from the list
		current_int = profiles_series[i].profile;

		// check the size of current vector
		// and fill it with zeros, if its size is smaller 
		// then obs_window
		if (current_int.size() > obs_window)
		{
			current_int.resize(obs_window);
		}
		else if (current_int.size() < obs_window)
		{
			current_int.reserve(obs_window - current_int.size());

			for (size_t i = current_int.size(); i < obs_window; ++i)
			       current_int.push_back(0.0);	
		}

		// correlate profiles
		fill(ccf.begin(), ccf.end(), 0.0);

		for (size_t j = 0; j < 2*obs_window; ++j)
			ccf.at(j) = cycle_discrete_ccf(current_int, profile, j - obs_window);



		// print CCF into a file to check it
		//ofstream file("out/ccf");
		//for (size_t j = 0; j < 2*obs_window; ++j)
		//	file << ccf.at(j) << endl;
			
		
		// find position of maximum of the CCF 
		// and reduce it to the convinient point
		double max_ccf = max(ccf);
		size_t max_pos_ccf = (max_pos(ccf) % obs_window) + obs_window;


		// clarify the position of maximum via interpolation
		for (int j = -2; j < 3; ++j)
			near_max[(size_t)j+2] = ccf[max_pos_ccf + (size_t) j]/max_ccf;

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


		// move integral profile to the same phase as template profile
		move_continous(current_int, (double) max_pos_ccf + reper_dec);

		// coherently sum moved integral and template profiles
		for (size_t j = 0; j < obs_window; ++j)
			profile.at(j) += current_int.at(j) / (double) n;

	}	

	if (cfg->verbose)
		cout << SUB << "Normilizing integral profile...";


	// try to normilize resulting template profile so that its maximum 
	// equals 1 and minimul equals 0
	int err = normilize_vector(profile);

	if ((cfg->verbose) & (err == 0))
			cout << OK << endl;
}

double Etalon_profile::get_SNR()
{
	return SNR(profile);
}



void Etalon_profile::print(string file_name)
{
// print template profile into a file
// in format:
// tau
// obs_window
// data 1
// ...
// data n
	ofstream out (file_name);

	out << "tau = " << tau << endl;
	out << "numpointwin = " << obs_window << endl;


	for (size_t i = 0; i < obs_window; i++)
		out << profile[i] << endl;	

	out.close();
}

double Etalon_profile::get_TAU(){return tau;}
int Etalon_profile::get_OBS_WINDOW(){return obs_window;}
