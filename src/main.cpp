#include<iostream>
#include<fstream>
#include<string>
#include<ctime>

#include"../lib/raw_profile.h"
#include"../lib/int_profile.h"
#include"../lib/etalon_profile.h"
#include"../lib/frequency_response.h"


using namespace std;


int main ()//(int argc, char *argv[])
{
	string file_name = "190122_0329+54_00";


	Raw_profile raw(file_name);

	Frequency_response fr(raw);

	fr.derivative_filter(2e-2, 4);
	fr.median_filter(1.5e0);

	fr.print("out/190122_0329+54_00.fr");
	fr.print_masked("out/masked_190122_0329+54_00.fr");
	
	Etalon_profile etalon_prf("0329+54.tpl");


	Int_profile int_prf(raw, fr.mask);
	long double toa = int_prf.get_TOA(etalon_prf);
	double toa_error = int_prf.get_ERROR();

	cout.precision(24);
	cout << endl << "TOA:  " << toa <<  " MJD" << endl;
	cout << "SNR:  " << int_prf.get_SNR() << endl;
	cout << "ERR:  " << toa_error*1e3 << " mcsec" << endl;



	ofstream out ("out/chanels.prf");
	for (int i = 0; i < 570; i++)
	{
		for (int k = 0; k < 512; k++)
			out << raw.mean_signal_per_chanel[k][i] << " ";

		out << endl;
	}
	out.close();

	int_prf.print("out/int.prf");
}
