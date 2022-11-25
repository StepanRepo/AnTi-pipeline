#ifndef ETALON_PROFILE_H
#define ETALON_PROFILE_H

#include<vector>
#include<string>

using namespace std;

class Etalon_profile
{
	public:
		Etalon_profile(string file_name);

		Etalon_profile(vector<double> profile, double tau, int obs_window);

		vector<double> profile;

		void normilize(string);

		double get_TAU();
		int get_OBS_WINDOW();

		Etalon_profile scale_profile(double tau_new);
	private:
		double tau;
		int obs_window;

		void read_header(string file_name);
		void fill_profile(string file_name);
};

#endif
