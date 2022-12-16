#include<iostream>
#include<fstream>
#include<string>
#include<ctime>

#include"../lib/raw_profile.h"
#include"../lib/int_profile.h"
#include"../lib/etalon_profile.h"
#include"../lib/frequency_response.h"
#include"../lib/configuration.h"

using namespace std;

Configuration cfg;

int main ()//(int argc, char *argv[])
{

	Raw_profile *raw;
	Frequency_response *fr;
	Etalon_profile *etalon_prf;
	Int_profile *int_prf;

	long double toa;
	double toa_error;

	if (cfg.do_tpl)
	{
		cout << "Do TPL mode is currently not aviable. Please wait for next release of the programm" << endl;
	}
	else
	{

		for (size_t i = 0; i < cfg.files.size(); i++)
		{
			string file_name = cfg.files[i];

			try
			{
				raw = new Raw_profile (file_name);
			}
			catch (const invalid_argument &err)
			{
				cout << err.what() << endl;
				break;
			}

			fr = new Frequency_response (*raw);

			if (cfg.do_filtration)
			{
				if (cfg.is_deriv_width)
					fr->derivative_filter(cfg.deriv_threshold, cfg.deriv_width);
				else 
					fr->derivative_filter(cfg.deriv_threshold);

				if (cfg.is_median_width)
					fr->median_filter(cfg.median_threshold, cfg.median_width);
				else
					fr->median_filter(cfg.median_threshold);
			}

			try
			{
				etalon_prf = new Etalon_profile (cfg.tplfile);
			}
			catch (const invalid_argument &err)
			{
				cout << err.what() << endl;
				break;
			}

			int_prf = new Int_profile (*raw, fr->mask);

			toa = int_prf->get_TOA(*etalon_prf);
			toa_error = int_prf->get_ERROR();

			cout.precision(24);
			cout << endl << "TOA:  " << toa <<  " MJD" << endl;
			cout << "SNR:  " << int_prf->get_SNR() << endl;
			cout << "ERR:  " << toa_error*1e3 << " mcsec" << endl;



			fr->print(cfg.output_dir + "190122_0329+54_00.fr");
			fr->print_masked(cfg.output_dir + "masked_190122_0329+54_00.fr");

			(*raw).print_mean_channel("chanels.prf");
			int_prf->print("out/int.prf");
		}
	}
}
