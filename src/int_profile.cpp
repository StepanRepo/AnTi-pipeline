#include"../lib/int_profile.h"
#include"../lib/session_info.h"
#include"../lib/raw_profile.h"
#include"../lib/custom_math.h"
#include"../lib/configuration.h"
#include"../lib/massages.h"

#include<vector>
#include<math.h>
#include<iostream>
#include<fstream>
#include<cstdio>

using namespace std;

extern Configuration *cfg;


Int_profile::Int_profile(Raw_profile& raw, vector<double>* mask) : session_info()
{
// calculates integral profile from given folded profile and (optional) channel mask
	if (cfg->verbose)
		cout << "Making integral profile" << endl;

	// copy session info from raw data class
	session_info = raw.session_info;

	// declare basic properties of observational session 
	int obs_window = session_info.get_OBS_WINDOW();
	int channels = session_info.get_CHANELS(); 

	// place arrays with dispersional delay for every freq channel 
	// and with compensated for this delay profiles in RAM
	vector<double> channel_delay (channels);
	compensated_signal_per_channel = vector (channels, vector<double>(obs_window));

	// set comparsion frequency as maximal availible frequency
	freq_comp = session_info.get_FREQ_MAX();

	// calculate dispersional delay for every freq channel
	// move folded profile in every channel according to calculated delays
	calculate_channel_delay (channel_delay);
	move_channel_profiles(&raw, channel_delay);

	// place an array with inteegral profile in RAM
	profile = vector<double> (obs_window);

	for (int j = 0; j < obs_window; j++)
		profile[j] = 0.0;

	// sum profiles over all freq channels (to obtain integral profile)
	if (mask == nullptr)
		fill (mask->begin(), mask->end(), 1.0);


	average_profiles(mask);

	// normilize this profile so that its maximum equals 1 and minimum equals 0
	normilize_profile();

	// set default values to variables until they are needed
	toa = 0.0l;
	toa_error = 0.0l;

	snr = get_SNR();
}


Int_profile::Int_profile (string file_name) : session_info(file_name, false)
{
// reads integral profile from given file 

	if (cfg->verbose)
		cout << "Reading integral profile" << endl;

	// try to open input file
	ifstream obs_file (file_name, ios::in);

	// check if it's possible
	if (!obs_file)
		throw invalid_argument (string(ERROR) + "Cann't open observational file to read data" + file_name);

	size_t obs_window = session_info.get_OBS_WINDOW();

	// make an array with 2d profile but its frequency dimension equals 1
	// (the profile in file was summed over frequency channels)
	compensated_signal_per_channel = vector (1, vector<double>(obs_window));
	profile = vector<double> (obs_window);

	string buffer;

	// skip header of the file
	for (size_t i = 0; i < session_info.get_NUM_PARAMS(); i++)
		getline(obs_file, buffer);	
	
	// run all over the lines of the file
	for (size_t i = 0; i < obs_window; i++)
	{
		getline(obs_file, buffer);	

		// delete first column of input: time
		buffer = buffer.substr(buffer.find(' ') + 1);

		// change decimal comma to decimal point
		size_t pos = buffer.find(',');
		if (pos < 200)
			buffer[pos] = '.';

		// we assume that integral profile file was made automatically 
		// and contains no formatting errors
		profile.at(i) = stod(buffer);
	}

	// normilize this profile so that its maximum equals 1 and minimum equals 0
	normilize_profile();

	// set default values to variables until they are needed
	toa = 0.0l;
	toa_error = 0.0l;

	snr = 0.0;
	snr = get_SNR();
}

void Int_profile::calculate_channel_delay(vector<double>& channel_delay)
{
// calculates channel delay via formula
// \Delta t_i\  = \frac{DM}{2,41} \left(\frac{1}{f_0^2} - \frac{1}{f_i^2} \right)\cdot10^{7}\ msec,

	if (cfg->verbose)
		cout << SUB << "Calculating channel delay...";

	// here constant dispersion measure is assumed
	// setting basic info about observational session
	double dm = session_info.get_DM();
	double freq_max = session_info.get_FREQ_MAX();
	double freq_min = session_info.get_FREQ_MIN();

	size_t channels = session_info.get_CHANELS();

	double df = freq_max - freq_min;
	double freq_current;

	// calculate the frequency that corresponds to the given channel
	// and calculate its dispersion delay
	for (size_t i = 0; i < channels; ++i)
	{
		freq_current = freq_min + double(i)*df/ double(channels);
		channel_delay[i] = (1.0/(freq_current*freq_current) - 1.0/(freq_comp*freq_comp))*dm*1e7/2.41;
	}
	

	if (cfg->verbose)
		cout << OK << endl;
}

void Int_profile::move_channel_profiles(Raw_profile* raw, std::vector<double>& channel_delay)
{
// moves channel profiles at freq channels according to given dispersion delay
	if (cfg->verbose)
		cout << SUB << "Moving channel profiles...";

	// set basic info about observational session
	double tau = session_info.get_TAU();
	size_t channels = session_info.get_CHANELS();
	size_t obs_window = session_info.get_OBS_WINDOW();

	long double period = session_info.get_PSR_PERIOD()*1e3;

	double delay;	// variable for current delay
	double noise;	// variable for noise level (needed for points that are getting out if window)

	// variables for current steps of delay
	int delta_int;
	double delta_dec;

	// temporal vectors for profiles
	vector<double> temp_1, temp_2;
	temp_1.reserve(obs_window);
	temp_2.reserve(obs_window);

	// this variable is new coordinate of point after moving
	double bias;

	for (size_t i = 0; i < channels; ++i)
	{
		// converting delay from time to steps in the array
		delta_int = int(channel_delay.at(i)/tau);
		delta_dec = (channel_delay.at(i) - tau*double(delta_int))/tau;

		// find some noise level for given channel
		// to set points from inside of observational window
		// (see the manual)
		noise = median(raw->mean_signal_per_channel.at(i));

		for(size_t j = 0; j < obs_window; ++j)
		{
			bias = double((int) j + delta_int) + delta_dec;

			// if a point comes from out of observational window
			// then we compensate the delay or set it to the noise level
			if (bias > double(obs_window))
			{
				// this point comes from blind zone of profile
				if (bias*tau < double(period))
				{
					temp_1.at(j) = temp_2.at(j) = noise;
					continue;
				}

				// this point crosses the blind zone of the profile
				// we compensate it
				delay = channel_delay.at(i) + floor(bias*tau/period)*(obs_window*tau - period);

				delta_int = int(delay/tau);
				delta_dec = delay - tau*double(delta_int);
			}

			// move profile on discrete number of steps
			temp_1.at(j) = raw->mean_signal_per_channel.at(i).at((j + delta_int) % obs_window);
			temp_2.at(j) = raw->mean_signal_per_channel.at(i).at((j + delta_int + 1) % obs_window);
		}

		// specify the bias on continous value (move less then on 1 step) 
		for(size_t j = 0; j < obs_window; ++j)
		{
			compensated_signal_per_channel.at(i).at(j) = 
				(1.0 - delta_dec) * temp_1.at(j) + delta_dec*temp_2.at(j);
		}
	}

	if (cfg->verbose)
		cout << OK << endl;
}



void Int_profile::average_profiles(vector<double>* mask)
{
// sum profile over freq channels (optionally) using mask

	if (cfg->verbose)
		cout << SUB << "Averaging channel profiles ...";

	// set basic info about observational session
	size_t channels = session_info.get_CHANELS();
	size_t obs_window = session_info.get_OBS_WINDOW();

	// sum profiles over the freq channels
	for (size_t j = 0; j < obs_window; ++j)
	{
		for (size_t i = 0; i < channels; ++i)
		{
			// use on don't use mask in summation process
			if (mask == nullptr)
				profile[j] += compensated_signal_per_channel.at(i).at(j);
			else
				profile[j] += mask->at(i) * compensated_signal_per_channel.at(i).at(j);
		}
	}
	
	if (cfg->verbose)
		cout << OK << endl;
}

void Int_profile::normilize_profile()
{
// normilizes profile so that its maximum equals 1 and minimum --- 0
	if (cfg->verbose)
		cout << SUB << "Normilizing integral profile...";

	int obs_window = session_info.get_OBS_WINDOW();

	double min_ = min(profile);
	double max_ = max(profile);
	double norm_factor;

	norm_factor = max_ - min_;

	for (int j = 0; j < obs_window; j++)
		profile.at(j) = (profile.at(j) - min_)/norm_factor;

	if (cfg->verbose)
		cout << OK << endl;
}


long double Int_profile::get_TOA(Etalon_profile& etalon_in)
{
// get the TOA of the profile using correlation process
// (see the manual)

	// return calculated TOA if it's already calculated
	if (toa != 0.0l)
		return toa;
	
	
	// Define new etalon profile and scale it to given time discrete
	Etalon_profile etalon (etalon_in.profile, etalon_in.get_TAU(), etalon_in.get_OBS_WINDOW());
	etalon = etalon.scale_profile(session_info.get_TAU());

	if (cfg->verbose)
		cout << SUB << "Calculating TOA...";

	// find reper point via correlation process
	double reper_point = get_reper_point(etalon); 

	// get the time of begining of the observation in MJD format
	long double mjd_start = session_info.get_START_UTC().get_MJD();

	if (cfg->verbose)
		cout << OK << endl;

	// calculate TOA as sum of begining of the session and reper point of the profile
	toa = mjd_start + (long double) (reper_point * session_info.get_TAU()) * 1e-3l/86400.0l;

	return toa;
}


double Int_profile::get_reper_point (Etalon_profile& etalon)
{
	// Calculate discrete cross-correlation 
	// function of integral and etalon profiles
	int etalon_len = etalon.profile.size();
	int int_len = profile.size();

	vector<double> ccf;
	ccf.reserve(int_len + etalon_len);

	for (int i = -etalon_len; i < int_len; i++)
		ccf.push_back(discrete_ccf(profile, etalon.profile, i));

	// print ccf vector to file
	//ofstream out2 ("out/ccf");
	//for (int i = 0; i < etalon_len+int_len; ++i)
	//{
	//	out2 << ccf[i] << endl;
	//}
	//out2.close();


	// find position of the maximum of ccf
	double max_ = max(ccf);
	int max_pos_ = max_pos(ccf);


	// Clarify the position of maximum of
	// the continous ccf
	vector<double> near_max;
	near_max.reserve(5);

	for (int i = -2; i < 3; i++)
		near_max.push_back(ccf[max_pos_ + i]/max_);


	// calculate interpolation coefficients near 
	// the maximum of discrete ccf
	vector<double> coefficients;
	coefficients.reserve(5);
	//c[0]x^4 + ... + c[4]
	coefficients = interpolation4(near_max);


	// find derivative of interpolation polynomial
	vector<double> derivative;
	derivative.reserve(4);

	derivative.push_back(4.0*coefficients[0]);
	derivative.push_back(3.0*coefficients[1]);
	derivative.push_back(2.0*coefficients[2]);
	derivative.push_back(coefficients[3]);

	// find a root of the derivative function in the range from -1 to 1
	double reper_dec =  find_root(derivative, -1.0, 1.0);

	// correct the position of the ccf to begining of the integral profile
	// (if integral profile is not biased relatively to template profile then reper point is 0)
	return double(max_pos_ - etalon_len) + reper_dec;
}

double Int_profile::get_ERROR()
{
// this function calculates TOA error in ms

	if (toa_error != 0)
		return toa_error;


	if (cfg->verbose)
		cout << SUB << "Calculating TOA error...";

	size_t obs_window = session_info.get_OBS_WINDOW();

	// positions of hwfm level
	size_t k1 = 0, k2 = 0;

	// half width at full maximum level is equal 0.5 
	double hwfm_level = 0.5;

	size_t i = 0;

	// find a position of left point of half maximum
	while (i < obs_window)
	{
		if (profile.at(i) > hwfm_level)
		{
			k1 = i;
			break;
		}
		++i;
	}

	// find a position of right point of half maximum
	while (i < obs_window)
	{
		if (profile.at(i) < hwfm_level)
		{
			k2 = i;
			break;
		}
		++i;
	}

	//toa_error = 0.3*double(k2-k1)*session_info.get_TAU()/get_SNR();	

	// error formula from "Binary and millisecond pulsars"
	toa_error = 1.0/get_SNR() /
		sqrt(session_info.get_FREQ_MAX() - session_info.get_FREQ_MIN()) /
		sqrt(session_info.get_PSR_PERIOD() * session_info.get_TOTAL_PULSES()) *
		pow((k2-k1) * session_info.get_TAU()*1e-3/session_info.get_PSR_PERIOD(), 1.5) *
		session_info.get_PSR_PERIOD() * 1e3;

	// if error is more than 1 ms than we set it to .999 ms
	// to format the output
	if (toa_error > 1.0) toa_error = 0.999;

	if (cfg->verbose)
		cout << OK << endl;

	return toa_error;
}


double Int_profile::get_SNR()
{
	if (snr != 0.0)
		return snr;


	snr = SNR(profile);
	return snr;
}

void Int_profile::read_freq_comp (string file_name)
{
// this function is nedded to find comparsion frequency in 
// a file with pre-processed integral profile file
// (srez format)

	ifstream obs_file(file_name, ios::in);
	string buffer, name;

	if (!obs_file)
		throw invalid_argument (string(ERROR) + "Cann't open observational file to read header" + file_name);

	size_t i = 0;

	// run all over the lines of the file
	while (getline (obs_file, buffer))
	{
		name = buffer.substr(0, buffer.find(' '));

		// find there a line with needed frequency
		if (name == "Fcomp" || name == "freq_comp")
		{
			try
			{
				freq_comp = stod(buffer.substr(buffer.find(' ') + 1)) * 1e-3;
			}
			catch (const invalid_argument &err)
			{
				throw invalid_argument 
					(string(ERROR) + "Cann't read comparsion frequency from integral profile file");
			}
		}

		i++;	

		// we reach the end of the header 
		if (i == session_info.get_NUM_PARAMS())
			break;
	}
}

double Int_profile::get_FREQ_COMP() {return freq_comp;}

void Int_profile::print(string file_name)
{
// print calculated profile into a file

	// print session info into a file
	session_info.print(file_name, freq_comp);

	double tau = session_info.get_TAU();

	// adjust output format to print double numbers
	ofstream out (file_name, ios_base::app);
	out.precision(8);

	// print integral profile into a file
	for (size_t i = 0; i < session_info.get_OBS_WINDOW(); i++)
		out << tau * (double) i << " " << profile.at(i) << endl;	

	out.close();
}

void Int_profile::ITOA()
{
// print calculated TOA in ITOA format
// (see tempo2 documentation)


	// set format and print in c format
	string file_name = cfg->output_dir + "toa";

	FILE *out = fopen (file_name.c_str(), "a+");
	
	char* format = (char*) "%-9s%18.12Lf%6.1f%11.3f%11f %3s\n";

	const char* name;
	name = session_info.get_PSR_NAME().c_str();

	fprintf(out, format, name, toa, toa_error*1e3, freq_comp, 0.0, "PO");

	fclose(out);
}
