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
	string file_name = cfg.files[0];

	Raw_profile raw(file_name);

	Frequency_response fr(raw);

	fr.derivative_filter(2e-2, 4);
	fr.median_filter(1.5e0);

	fr.print(cfg.output_dir + "190122_0329+54_00.fr");
	fr.print_masked(cfg.output_dir + "masked_190122_0329+54_00.fr");
	
	Etalon_profile etalon_prf(cfg.tplfile);


	Int_profile int_prf(raw, fr.mask);
	long double toa = int_prf.get_TOA(etalon_prf);
	double toa_error = int_prf.get_ERROR();

	cout.precision(24);
	cout << endl << "TOA:  " << toa <<  " MJD" << endl;
	cout << "SNR:  " << int_prf.get_SNR() << endl;
	cout << "ERR:  " << toa_error*1e3 << " mcsec" << endl;

	raw.print_mean_channel("chanels.prf");
	int_prf.print("out/int.prf");
}
