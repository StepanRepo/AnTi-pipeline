#ifndef RAW_PROFILE_H
#define RAW_PROFILE_H

#include<string>

typedef union byte32
{
	char as_char[4];
	int as_int;
} byte32;


class Raw_profile
{
    private:
        double tau;

        int total_pulses;
        int obs_window;
        int chanels;

        int OBS_SIZE;

        byte32* data;
        double* signal;
        
        void read_data (std::string file_name);
        void decode_data ();
        void split_data ();

    public:
        Raw_profile(std::string);
        //~Raw_profile();

        double** signal_per_chanel;

};

#endif