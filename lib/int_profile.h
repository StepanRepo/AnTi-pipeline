#ifndef INT_PROFILE_H
#define INT_PROFILE_H

#include<vector>

#include"raw_profile.h"
#include"session_info.h"
#include"etalon_profile.h"

class Int_profile
{
    public:

        Session_info session_info;
        std::vector<double> profile;
	std::vector<std::vector<double>> compensated_signal_per_chanel;

        Int_profile(Raw_profile& raw);
        Int_profile(Raw_profile& raw, std::vector<double> mask);

	long double get_TOA(Etalon_profile&);

    private:

        void calculate_chanel_delay (std::vector<double> & chanel_delay);
        void move_chanel_profiles(Raw_profile* raw, std::vector<double>& chanel_delay);
        void average_profiles();
        void average_profiles(std::vector<double> mask);
        void normilize_profile();

	long double toa;
	double get_reper_point(Etalon_profile&);
};

#endif
