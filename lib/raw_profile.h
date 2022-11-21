#ifndef RAW_PROFILE_H
#define RAW_PROFILE_H

#include<string>
#include<vector>

#include"session_info.h"

typedef union byte32
{
	char as_char[4];
	int as_int;
} byte32;


class Raw_profile
{
    public:
        Raw_profile(std::string);
        ~Raw_profile();

        Session_info session_info{};
        vector<vector<double>> signal_per_chanel;

	double get_RAW_SIGNAL(int i);

    private:
        int OBS_SIZE;

        byte32* data;
        double* signal;
        
        void fill_header (std::string file_name);
        void read_data (std::string file_name);
        void decode_data ();
        void split_data ();

};

#endif
