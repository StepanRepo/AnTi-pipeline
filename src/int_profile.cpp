#include<vector>
#include<iostream>

#include"../lib/int_profile.h"
#include"../lib/raw_profile.h"

using namespace std;

Int_profile::Int_profile(Raw_profile raw)
{
    int obs_window = 570;   // Int_profile::session_info.get_OBS_WINDOW();
    int channels = 512;     // Int_profile::sesion_info = raw.session_info;

    vector<double> chanel_delay;
    chanel_delay.reserve(channels);

    Int_profile::calculate_chanel_delay (chanel_delay);
    Int_profile::move_chanel_profiles(&raw, chanel_delay);

    Int_profile::profile.reserve(obs_window);

    for (int j = 0; j < obs_window; j++)
        Int_profile::profile[j] = 0;

    Int_profile::average_profiles(&raw);

    Int_profile::normilize_profile();
}

void Int_profile::calculate_chanel_delay(vector<double>& chanel_delay)
{
    cout << "Calculating chanel delay . . ." << endl;

    //double dm = Int_profile::session_info.get_DM();
    //double freq_max = Int_profile::session_info.get_FREQ_MAX();
    //double freq_min = Int_profile::session_info.get_FREQ_MIN();
    
    int chanels = 512; // Int_profile::session_info.get_CHANELS();

    double dm = 26.764;
    double freq_min = 109.584; // MHz
    double freq_max = 112.084; // MHz

    double df = freq_max - freq_min;
    double freq_current;

    for (int i = 0; i < chanels; i++)
    {
        freq_current = freq_min + double(i+1)*df/512.0;
        chanel_delay[i] = (1.0/(freq_current*freq_current) - 1.0/(freq_max*freq_max))*dm*1e7/2.41;
    }
}

void Int_profile::move_chanel_profiles(Raw_profile* raw, std::vector<double>& chanel_delay)
{
    cout << "Moving chanel profiles . . ." << endl;

    double tau = 1.2288;    // Int_profile::session_info.get_TAU();
    int chanels = 512;      // Int_profile::session_info.get_CHANELS();
    int obs_window = 570;   // Int_profile::session_info.get_OBS_WINDOW();

    int delta_int;
    double delta_dec;

    vector<double> temp_1, temp_2;
    temp_1.reserve(obs_window);
    temp_2.reserve(obs_window);

    for (int i = 0; i < chanels; i++)
    {
        delta_int = int(chanel_delay[i]/tau);
        delta_dec = chanel_delay[i] - tau*double(delta_int);

        for(int j = 0; j < obs_window; j++)
        {
            temp_1[j] = raw->signal_per_chanel[i][(j + delta_int) % obs_window];
            temp_2[j] = raw->signal_per_chanel[i][(j + delta_int + 1) % obs_window];
        }
        
        for(int j = 0; j < obs_window; j++)
        {
            raw->signal_per_chanel[i][j] = (1-delta_dec)*temp_1[j]+ delta_dec*temp_2[j];
        }
    }
}


void Int_profile::average_profiles(Raw_profile* raw)
{
    cout << "Averaging chanel profiles . . ." << endl;

    int chanels = 512;      // Int_profile::session_info.get_CHANELS();
    int obs_window = 570;   // Int_profile::session_info.get_OBS_WINDOW();

    for (int j = 0; j < obs_window; j++)
    {
        for (int i = 0; i < chanels; i++)
        {
            Int_profile::profile[j] += raw->signal_per_chanel[i][j];
        }
    }
}

void Int_profile::normilize_profile()
{
    int obs_window = 570;   // Int_profile::session_info.get_OBS_WINDOW();

    double min = Int_profile::profile[0];
    double max = 0;
    double norm_factor;

    for (int j = 0; j < obs_window; j++)
    {
        if (Int_profile::profile[j] < min) min = Int_profile::profile[j];
        if (Int_profile::profile[j] > max) max = Int_profile::profile[j];
    }

    norm_factor = max - min;

    for (int j = 0; j < obs_window; j++)
        Int_profile::profile[j] = (Int_profile::profile[j] - min)/norm_factor;


}
