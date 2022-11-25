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

        Int_profile(Raw_profile& raw);

	long double get_TOA(Etalon_profile&);

    private:

	long double toa;

        void calculate_chanel_delay (std::vector<double> & chanel_delay);
        void move_chanel_profiles(Raw_profile* raw, std::vector<double>& chanel_delay);
        void average_profiles(Raw_profile* raw);
        void normilize_profile();

	double get_reper_point(Etalon_profile&);
};

#endif
