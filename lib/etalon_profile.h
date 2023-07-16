#ifndef ETALON_PROFILE_H
#define ETALON_PROFILE_H

#include<vector>
#include<string>

using namespace std;

class Int_profile;

class Etalon_profile
{
	public:
		Etalon_profile(string file_name);
		Etalon_profile(vector<Int_profile> &);



		vector<vector<double>> profile;

		void normilize();
		double get_SNR();

		double get_TAU();
		int get_OBS_WINDOW();
		int get_CHANELS();

		Etalon_profile scale_profile(double tau_new);

		void print(string file_name);
	private:
		Etalon_profile(vector<vector<double>> profile, double tau, int obs_window, int channels);

		double tau;
		int obs_window;
		int channels;

		void read_header(string file_name);
		void fill_profile(string file_name);
};

#endif
