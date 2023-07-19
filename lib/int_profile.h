#ifndef INT_PROFILE_H
#define INT_PROFILE_H

#include<vector>

#include"raw_profile.h"
#include"session_info.h"
#include"etalon_profile.h"

class Int_profile
{
    public:

        Int_profile(Raw_profile& raw, vector<double>* mask = nullptr);
	Int_profile(std::string file_name);

        Session_info session_info;

	std::vector<std::vector<double>> profile;


	long double get_TOA(Etalon_profile&);
	double get_ERROR();
	void ITOA();

	double get_SNR();

	double get_FREQ_COMP();
	void read_freq_comp (std::string file_name); 

	void print(string file_name);

    private:

	void use_mask(vector<double> mask);
        void normilize_profile();


	double freq_comp; 

	long double toa;
	double toa_error;

	double get_reper_point(Etalon_profile&);

	double snr;
};

#endif
