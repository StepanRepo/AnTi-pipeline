#ifndef FREQUENCY_RESPONSE_H
#define FREQUENCY_RESPONSE_H

#include<vector>
#include<string>

#include"raw_profile.h"
#include"int_profile.h"
#include"session_info.h"

using namespace std;

class Frequency_response
{
	public:

		Frequency_response(Raw_profile& raw);
		Frequency_response(Int_profile& int_prf);

		Session_info session_info;
		vector<double> profile;

		vector<double> mask;
	
		void derivative_filter(double p1);
		void median_filter(double p2);

		void derivative_filter(double p1, int width);
		void median_filter(double p2, int width);

		void print(string file_name);
		void print_masked(string file_name);

	private:

		void fill_profile(vector<vector<double>>);
		void fill_mask();

};

#endif
