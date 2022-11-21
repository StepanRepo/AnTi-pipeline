#include<iostream>
#include<fstream>
#include<string>
#include<ctime>

#include"../lib/raw_profile.h"
#include"../lib/int_profile.h"

using namespace std;


int main ()//(int argc, char *argv[])
{
	string file_name = "190122_0329+54_00";
	Raw_profile raw(file_name);
	
	Int_profile int_prf(raw);

	cout << endl;
	cout << raw.session_info.get_START_DATE().get_YEAR() << endl;
	cout << raw.session_info.get_START_DATE().get_MONTH() << endl;
	cout << raw.session_info.get_START_DATE().get_DAY() << endl;
	cout << raw.session_info.get_START_DATE().get_HOUR() << endl;
	cout << raw.session_info.get_START_DATE().get_MINUTE() << endl;
	cout << raw.session_info.get_START_DATE().get_SECOND() << endl;

	ofstream out ("out/chanels.prf");
	for (int i = 0; i < 570; i++)
	{
		for (int k = 0; k < 512; k++)
			out << raw.signal_per_chanel[k][i] << " ";

		out << endl;
	}
	out.close();



	ofstream out1 ("out/int.prf");
	for (int i = 0; i < 570; i++)
	{
		out1 << int_prf.profile[i] << endl;
	}
	out1.close();


}
