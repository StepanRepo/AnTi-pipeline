#include<vector>
#include<iostream>

#include"../lib/int_profile.h"
#include"../lib/session_info.h"
#include"../lib/raw_profile.h"

using namespace std;

Int_profile::Int_profile(Raw_profile& raw) : session_info()
{
    session_info = raw.session_info;
    
    int obs_window = session_info.get_OBS_WINDOW();
    int channels = session_info.get_CHANELS(); 

    vector<double> chanel_delay;
    chanel_delay.reserve(channels);

    calculate_chanel_delay (chanel_delay);
    move_chanel_profiles(&raw, chanel_delay);

    profile.reserve(obs_window);

    for (int j = 0; j < obs_window; j++)
        profile[j] = 0;

    average_profiles(&raw);

    normilize_profile();
}

void Int_profile::calculate_chanel_delay(vector<double>& chanel_delay)
{
    cout << "Calculating chanel delay . . ." << endl;

    double dm = session_info.get_DM();
    double freq_max = session_info.get_FREQ_MAX();
    double freq_min = session_info.get_FREQ_MIN();
    
    int chanels = session_info.get_CHANELS();

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

    double tau = session_info.get_TAU();
    int chanels = session_info.get_CHANELS();
    int obs_window = session_info.get_OBS_WINDOW();

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

    int chanels = session_info.get_CHANELS();
    int obs_window = session_info.get_OBS_WINDOW();

    for (int j = 0; j < obs_window; j++)
    {
        for (int i = 0; i < chanels; i++)
        {
            profile[j] += raw->signal_per_chanel[i][j];
        }
    }
}

void Int_profile::normilize_profile()
{
    cout << "Normilizing integral profiles . . ." << endl;

    int obs_window = session_info.get_OBS_WINDOW();

    double min = profile[0];
    double max = 0;
    double norm_factor;

    for (int j = 0; j < obs_window; j++)
    {
        if (profile[j] < min) min = profile[j];
        if (profile[j] > max) max = profile[j];
    }

    norm_factor = max - min;

    for (int j = 0; j < obs_window; j++)
        profile[j] = (profile[j] - min)/norm_factor;


}
