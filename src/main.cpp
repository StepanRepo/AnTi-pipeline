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

Configuration* cfg{nullptr};


Int_profile* get_int_prf(string file_name)
{
// this function is used to get integral profile from every type of 
// input file. this process is nedded in multiple places so it was 
// moved to separate function

	// declare all types of needed classes
	Raw_profile *raw = nullptr;
	Frequency_response *fr = nullptr;
	Int_profile* int_prf = nullptr;


	// fint an extension of the file
	// and process the file according to it
	string ext = file_name.substr(file_name.find('.') + 1);

	if (ext == "prf" || ext == "srez")
	{
		// this is pre-processed integral profiles
		int_prf = new Int_profile (cfg->rawdata_dir + file_name);
		int_prf->read_freq_comp(cfg->rawdata_dir + file_name);
		//int_prf->print(cfg->output_dir + cfg->files[i] + ".prf");
	}
	else if (ext == file_name)
	{
		// this is raw profiles of observational session
		
		// read raw data and construct frequency response from this data
		raw = new Raw_profile (cfg->rawdata_dir + file_name);
		fr = new Frequency_response (*raw);

		// perform a set of filtration procedures
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

		// print resulted frequency response into a file
		if (cfg->get_fr)
		{
			fr->print(cfg->output_dir + file_name + ".fr");
			fr->print_masked(cfg->output_dir + "masked_" + file_name + ".fr");
		}

		// calculate integral profile from raw data and filtration information
		int_prf = new Int_profile (*raw, &fr->mask);
		int_prf->print(cfg->output_dir + file_name + ".prf");

		// free the RAM
		delete raw;
		delete fr;

		raw = nullptr;
		fr = nullptr;
	}
	else
	{
		// throw if it's unknown file format
		throw((string) WARNING  + "Unknown file format: " + file_name);

	}

	return int_prf;
}


void read_command_line(int argc, char **argv)
{

	// section for reading configuration file name from CL
	for (int i = 1; i < argc; i++)
	{
		// try to use custom configuration for this run
		if (string(argv[i]) == "-c")
		{
			i ++;
			// check if there is anything after "-c"
			if(i < argc)
				cfg = new Configuration(argv[i]);
			else
			{
				cout << WARNING <<
				       	"Custom configuration file wasn't set. Default one will be used istead" << endl;
				cfg = new Configuration();
			}

		}

		// show help message
		else if (string(argv[i]) == "-h" || string(argv[i]) == "--help")
		{
			cout << "Usage: ./main [-c configuration_file] " << endl << endl;
			cout << "Options and arguments:" << endl << endl;;

			cout << "\t-h, --help                    : show this help message" << endl;
			cout << "\t-c file_name, --cfg file_name : use custom configuration file (file_name) instead of default one" << endl << endl;;

			cout << "Options to modificate configuration:" << endl;
			cout << "\tUse name of the parameter you want to adjust:" << endl;
			cout << "\t\t--name[=new_value]" << endl;
			cout << "\tif the parameter in configurations is boolean value then use just its name to set value equal to True" << endl;
			exit(0);
		}
		// print that there is no such CL argument
		else
		{
			cout << "Unknown command line argument: " << argv[i] << endl;
		}
	}
}




int main (int argc, char *argv[])
{
	read_command_line(argc, argv);

	// there aren't custom configuration file
	if ((argc == 1) || (cfg == nullptr))
		cfg = new Configuration();

	// modify configuration with CL arguments
	cfg->command_line(argc, argv);


	Etalon_profile *etalon_prf = nullptr;
	Int_profile *int_prf = nullptr;


	// declare lists of errors
	vector<string> error_list, error_names;

	if (cfg->do_tpl)
	{
		cout << "Do TPL mode" << endl;

		// declare a list of integral profiles
		vector <Int_profile> profiles;

		// fill the list with given integral profiles
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

			// check if profile is ok. skip if it is not.
			if (int_prf->get_SNR() > 0.0)
				profiles.push_back(*int_prf);
			else
			{
				cout << WARNING << "Low SNR. Skipping" << endl;

				error_list.push_back(string(WARNING) + "Low SNR. Skipping");
				error_names.push_back(file_name);

				continue;
			}

			// print SNR of readed profile
			cout << "SNR: " << int_prf->get_SNR() << endl;
		}

		// try to construct etalon profile from the set of integral profiles
		// and save it into a file
		try
		{
			etalon_prf = new Etalon_profile(profiles);
			etalon_prf->print(cfg->output_dir + profiles[0].session_info.get_PSR_NAME() + ".tpl");
		}
		catch (const invalid_argument &err)
		{
			if (cfg->verbose)
				cout << err.what() << endl;

			error_list.push_back(err.what());
			error_names.push_back(cfg->tplfile);

			exit(0);
		}

		// print some usefull info about new etalon profile
		cout << endl << "Template profile was made from " << profiles.size() << " integral profiles" << endl;
		cout << "SNR: " << etalon_prf->get_SNR() << endl;

		// free the RAM
		delete etalon_prf;
		etalon_prf = nullptr;
	}
	else
	{
		cout << "TOA calculating mode" << endl;


		// try to read template profile from the file
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

		// find TOA for every observational session
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


			// calculate TOA and its error
			long double toa = int_prf->get_TOA(*etalon_prf);
			double toa_error = int_prf->get_ERROR();

			// print TOA into the resulring file in ITOA format
			// (see tempo2 manual)
			int_prf->ITOA();

			// print some information about current results of processing
			if (cfg->verbose)
			{
				cout.precision(24);
				cout << endl << "TOA:  " << toa <<  " MJD" << endl;
				cout << "SNR:  " << int_prf->get_SNR() << endl;
				cout << "ERR:  " << toa_error*1e3 << " usec" << endl;
			}

			// free the RAM
			delete int_prf;
			int_prf = nullptr;
		}

		// print summary information about this run
		cout << endl << "TOA was calculated for " <<
		       	cfg->files.size() - error_list.size() << " integral profiles" << endl;

		// free the RAM
		delete etalon_prf;
		etalon_prf = nullptr;
	}

	// duplicate all warnings and errors that occured during the run
	if (error_list.size() > 0)
	{
		cout << endl << "Duplication of errors:" << endl;

		for (size_t i = 0; i < error_list.size(); i++)
		{ 
			cout << i+1 << ". In file " << error_names[i] << ": " << error_list[i] << endl;
		}
	}

	// free the RAM
	delete cfg;
	cfg = nullptr;
}
