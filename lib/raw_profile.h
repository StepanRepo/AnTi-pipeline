#ifndef RAW_PROFILE_H
#define RAW_PROFILE_H

#include<string>
#include<vector>

#include"session_info.h"

typedef union byte32
{
	char as_char[4];
	int32_t as_int;
	float as_float;
} byte32;


class Raw_profile
{
    public:
        Raw_profile(std::string);

        Session_info session_info{};
        vector<vector<double>> mean_signal_per_chanel;

	double get_RAW_SIGNAL(int i);
	void print_mean_channel(string file_name);

    private:
        int OBS_SIZE;

        void read_data (std::string file_name, byte32 *data);
        void decode_data (byte32 *data, double* signal);
        void split_data (double* signal);

};

#endif
