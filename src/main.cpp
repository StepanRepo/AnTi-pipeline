#include<iostream>
#include<fstream>
#include<string>
#include<ctime>

#include"../lib/raw_profile.h"
#include"../lib/int_profile.h"
#include"../lib/etalon_profile.h"
#include"../lib/frequency_response.h"
#include"../lib/configuration.h"
#include"../lib/massages.h"

#define RESERVATION_THRESOLD 1

using namespace std;

Configuration* cfg;

static Raw_profile* raw;
static Frequency_response* fr;
static Etalon_profile *etalon_prf;
static Int_profile *int_prf;

vector<string> error_list, error_names;

long long find_max_size()
{
	long long max = 0;
	long long file_size;

	string file_name;

	for (size_t i = 0; i < cfg->files.size(); ++i)
	{
		file_name = cfg->files[i];

		ifstream in_file(cfg->rawdata_dir + "/" + file_name, ios::binary);
		in_file.seekg(0, ios::end);

		file_size = in_file.tellg();

		if (file_size > max)
			max = file_size;
	}	

	return max;
}

int make_raw_prf (string& file_name, byte32* data, double* signal, size_t reserve_size)
{
	try
	{
		raw = new Raw_profile (cfg->rawdata_dir + file_name, data, signal, reserve_size);
	}
	catch (const invalid_argument &err)
	{
		if (cfg->verbose)
			cout << err.what() << endl;

		error_list.push_back(err.what());
		error_names.push_back(file_name);

		return 1;
	}

	fr = new Frequency_response (*raw);

	if (cfg->do_filtration)
	{
		if (cfg->is_deriv_width)
			fr->derivative_filter(cfg->deriv_threshold, cfg->deriv_width);
		else 
			fr->derivative_filter(cfg->deriv_threshold);

		if (cfg->is_median_width)
			fr->median_filter(cfg->median_threshold, cfg->median_width);
		else
			fr->median_filter(cfg->median_threshold);
	}

	if (cfg->get_fr)
	{
		fr->print(cfg->output_dir + file_name + ".fr");
		fr->print_masked(cfg->output_dir + "masked_" + file_name + ".fr");
	}

	return 0;
}




int make_int_prf (string& file_name, byte32* data, double* signal, size_t reserve_size)
{
	if (cfg->verbose)
		cout << endl << BOLD << "Processing of file: " << file_name << RESET << endl;

	string ext = file_name.substr(file_name.find('.') + 1);

	if (ext == "prf" || ext == "srez")
	{
		try
		{
			int_prf = new Int_profile (cfg->rawdata_dir + file_name);
			int_prf->read_freq_comp(cfg->rawdata_dir + file_name);
		}
		catch (const invalid_argument &err)
		{
			if (cfg->verbose)
				cout << err.what() << endl;

			error_list.push_back(err.what());
			error_names.push_back(file_name);

			return 1;
		}
	}
	else if (ext == file_name)
	{
		if (make_raw_prf(file_name, data, signal, reserve_size))
			return 1;

		int_prf = new Int_profile (*raw, fr->mask);
		int_prf->print(cfg->output_dir + file_name + ".prf");

		delete raw;
		delete fr;

		raw = nullptr;
		fr = nullptr;
	}
	else
	{
		cout << WARNING << "Unknown file format: " << file_name << endl;
		return 1;
	}

	return 0;
}




int main (int argc, char *argv[])
{
	// section for reading configuration file name from CL
	int i;

	// is exists custom configuration file?
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
			continue;
		else
		{
			cfg = new Configuration(argv[i]);
			break;
		}
	}

	// if there aren't custom configuration file
	if (i == argc)
		cfg = new Configuration();

	cfg->command_line(argc, argv);



	// reserve memory section
	long long reserve_size = find_max_size()/4;	// approximately equal number of points in raw profile


	byte32* data = nullptr;
	double* signal = nullptr;

	if (cfg->files.size() > RESERVATION_THRESOLD)
	{
		if (cfg->verbose)
			cout << "Start memory reservation" << endl;

		data = new byte32[reserve_size];
		signal = new double[reserve_size];

		if (cfg->verbose)
			cout << "End memory reservation" << endl;
	}


	// Data processing section
	if (cfg->do_tpl)
	{
		cout << "Do TPL mode" << endl;

		vector <Int_profile> profiles;

		for (size_t i = 0; i < cfg->files.size(); i++)
		{
			string file_name = cfg->files[i];

			if (make_int_prf(file_name, data, signal, reserve_size))
				continue;

			profiles.push_back(*int_prf);
		}

		etalon_prf = new Etalon_profile(profiles);
		etalon_prf->print(cfg->output_dir + profiles[0].session_info.get_PSR_NAME() + ".tpl");

		cout << endl << "Template profile was made from " << profiles.size() << " integral profiles" << endl;
		cout << "SNR: " << etalon_prf->get_SNR() << endl;

		delete etalon_prf;
		etalon_prf = nullptr;

	}
	else
	{
		cout << "TOA calculating mode" << endl;

		long double toa;
		double toa_error;

		try
		{
			etalon_prf = new Etalon_profile (cfg->tplfile);
		}
		catch (const invalid_argument &err)
		{
			if (cfg->verbose)
				cout << err.what() << endl;

			error_list.push_back(err.what());
			error_names.push_back(cfg->tplfile);

			exit(0);
		}

		for (size_t i = 0; i < cfg->files.size(); i++)
		{
			string file_name = cfg->files[i];

			if (make_int_prf(file_name, data, signal, reserve_size))
				continue;

			toa = int_prf->get_TOA(*etalon_prf);
			toa_error = int_prf->get_ERROR();

			int_prf->ITOA();

			if (cfg->verbose)
			{
				cout.precision(24);
				cout << endl << "TOA:  " << toa <<  " MJD" << endl;
				cout << "SNR:  " << (int) int_prf->get_SNR() << endl;
				cout << "ERR:  " << (int) (toa_error*1e3) << " mcsec" << endl;
			}

			int_prf->print(cfg->output_dir + cfg->files[i] + ".prf");

			delete int_prf;
			int_prf = nullptr;
		}

		cout << endl << "TOA was calculated for " << cfg->files.size() - error_list.size() << " integral profiles" << endl;

		delete etalon_prf;
		etalon_prf = nullptr;
	}

	delete[] data;
	data = nullptr;

	delete[] signal;
	signal = nullptr;


	// Error duplication section
	if (error_list.size() > 0)
	{
		cout << endl << "Duplication of errors:" << endl;

		for (size_t i = 0; i < error_list.size(); i++)
		{ 
			cout << i+1 << ". In file " << error_names[i] << ": " << error_list[i] << endl;
		}
	}

	delete cfg;
	cfg = nullptr;
}
