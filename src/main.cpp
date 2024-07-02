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


using namespace std;

Configuration* cfg;

Int_profile* get_int_prf(string file_name)
{
	Raw_profile *raw = nullptr;
	Frequency_response *fr = nullptr;
	Int_profile* int_prf = nullptr;


	string ext = file_name.substr(file_name.find('.') + 1);

	if (ext == "prf" || ext == "srez")
	{
		int_prf = new Int_profile (cfg->rawdata_dir + file_name);
		int_prf->read_freq_comp(cfg->rawdata_dir + file_name);
		//int_prf->print(cfg->output_dir + cfg->files[i] + ".prf");
	}
	else if (ext == file_name)
	{
		raw = new Raw_profile (cfg->rawdata_dir + file_name);
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

		int_prf = new Int_profile (*raw, &fr->mask);
		int_prf->print(cfg->output_dir + file_name + ".prf");


		delete raw;
		delete fr;

		raw = nullptr;
		fr = nullptr;
	}
	else
	{
		throw((string) WARNING  + "Unknown file format: " + file_name);

	}

	return int_prf;
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

	// there aren't custom configuration file
	if (i == argc)
		cfg = new Configuration();

	cfg->command_line(argc, argv);


	Etalon_profile *etalon_prf = nullptr;
	Int_profile *int_prf = nullptr;

	long double toa;
	double toa_error;

	string ext;

	vector<string> error_list, error_names;

	if (cfg->do_tpl)
	{
		cout << "Do TPL mode" << endl;

		vector <Int_profile> profiles;

		for (size_t i = 0; i < cfg->files.size(); i++)
		{
			string file_name = cfg->files[i];

			if (cfg->verbose)
				cout << endl << BOLD << "Processing of file: " << file_name << RESET << endl;

			// try to get integral profile from file file_name
			// and skip this file if smth gone wrong
			try
			{
				int_prf = get_int_prf(file_name);
			}
			catch (const invalid_argument &err)
			{
				if (cfg->verbose)
					cout << err.what() << endl;

				error_list.push_back(err.what());
				error_names.push_back(file_name);

				continue;
			}

			// check if profile is ok
			if (int_prf->get_SNR() > 0.0)
				profiles.push_back(*int_prf);
			else
			{
				cout << WARNING << "Low SNR. Skipping" << endl;
				error_list.push_back(string(WARNING) + "Low SNR. Skipping");
				error_names.push_back(file_name);
			}

			// print SNR of readed profile
			cout << "SNR: " << int_prf->get_SNR() << endl;
		}

		// construct etalon profile from the set of integral profiles
		// and save it into a file
		etalon_prf = new Etalon_profile(profiles);
		etalon_prf->print(cfg->output_dir + profiles[0].session_info.get_PSR_NAME() + ".tpl");

		// print some usefull info about new etalon profile
		cout << endl << "Template profile was made from " << profiles.size() << " integral profiles" << endl;
		cout << "SNR: " << etalon_prf->get_SNR() << endl;

		// free RAM
		delete etalon_prf;
		etalon_prf = nullptr;
	}
	else
	{
		cout << "TOA calculating mode" << endl;

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

			if (cfg->verbose)
				cout << endl << BOLD << "Processing of file: " << file_name << RESET << endl;

			// try to get integral profile from file file_name
			// and skip this file if smth gone wrong
			try
			{
				int_prf = get_int_prf(file_name);
			}
			catch (const invalid_argument &err)
			{
				if (cfg->verbose)
					cout << err.what() << endl;

				error_list.push_back(err.what());
				error_names.push_back(file_name);

				continue;
			}


			toa = int_prf->get_TOA(*etalon_prf);
			toa_error = int_prf->get_ERROR();

			int_prf->ITOA();

			if (cfg->verbose)
			{
				cout.precision(24);
				cout << endl << "TOA:  " << toa <<  " MJD" << endl;
				cout << "SNR:  " << int_prf->get_SNR() << endl;
				cout << "ERR:  " << toa_error*1e3 << " usec" << endl;
			}

			delete int_prf;
			int_prf = nullptr;
		}

		cout << endl << "TOA was calculated for " <<
		       	cfg->files.size() - error_list.size() << " integral profiles" << endl;

		delete etalon_prf;
		etalon_prf = nullptr;
	}

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
