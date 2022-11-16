#include"../lib/raw_profile.h"
#include"string"
#include<fstream>
#include <iostream>

using namespace std;

Raw_profile::Raw_profile(string file_name)
{
    //from header

    Raw_profile::tau = 1.2288;
    Raw_profile::total_pulses = 464;
    Raw_profile::obs_window = 570;
    Raw_profile::chanels = 512;
    
    Raw_profile::OBS_SIZE = total_pulses*obs_window*chanels;

    Raw_profile::data = new byte32 [OBS_SIZE];
    Raw_profile::read_data(file_name);

    Raw_profile::signal = new double [OBS_SIZE];
    Raw_profile::decode_data();
    
    //delete[] Raw_profile::data;
    //delete[] Raw_profile::signal;
    
    Raw_profile::signal_per_chanel = new double* [chanels];

    for (int i = 0; i < chanels; i++)
    {
        Raw_profile::signal_per_chanel[i] = new double [obs_window];
    }

    Raw_profile::split_data();
}



void Raw_profile::read_data(string file_name)
{
    cout << "Reading header . . ." << endl;

	ifstream obs_file (file_name, ios::in | ios::binary);
	char header_buffer[40];

	for (int i = 0; i < 13; i++)
	{
		obs_file.read(header_buffer, 40);
		cout << header_buffer << endl;
	}	

	int i = 0;

    cout << "Reading data . . ." << endl;

	while(obs_file.good())
	{
		obs_file.read((Raw_profile::data[i].as_char), 4);
		i++;
	}
	obs_file.close();

    if (i-1 != Raw_profile::OBS_SIZE)
        cout << "ERROR WHILE READING DATA" << endl;
}

void Raw_profile::decode_data()
{
    cout << "Decoding data . . ." << endl;


	double exp, spectr_t;
	double ratio = tau/0.2048;

	#pragma clang loop vectorize(assume_safety)
	for (int i = 0; i < OBS_SIZE; i++)
	{
		spectr_t = double (Raw_profile::data[i].as_int & 0xFFFFFF);

		exp = double ( (Raw_profile::data[i].as_int & 0x7F000000) >> 24 );
		exp -= 64.0;

		exp = double(2 << (int) exp);

		spectr_t = spectr_t*exp/ratio;
		spectr_t = spectr_t*1.3565771745707199e-14;

		Raw_profile::signal[i] = spectr_t;
	}
}

void Raw_profile::split_data ()
{
    cout << "Splitting data . . ." << endl;

	for (int i = 0; i < 512; i++)
	{
		for (int k = 0; k < 570; k ++)
		{
			Raw_profile::signal_per_chanel[i][k] = 0.0;
		}
	}

    int chan_and_window = chanels*obs_window;

	for (int imp = 0; imp < total_pulses; imp ++)
	{
		for (int k = 0; k < obs_window; k ++)
		{
			for (int i = 0; i < chanels; i++)
			{
				Raw_profile::signal_per_chanel[i][k] += Raw_profile::signal[i + k*chanels + imp*chan_and_window];
			}
		}
	}
}

