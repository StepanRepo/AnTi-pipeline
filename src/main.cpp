#include<iostream>
#include<fstream>
#include<string>
#include<ctime>

#include"../lib/raw_profile.h"

using namespace std;


int main ()//(int argc, char *argv[])
{
	string file_name = "190122_0329+54_00";
	Raw_profile raw(file_name);

	//cout << "end of class initializing" << endl;

	ofstream out ("1.prf");
	for (int i = 0; i < 570; i++)
	{
		for (int k = 0; k < 512; k++)
			out << raw.signal_per_chanel[k][i] << " ";

		out << endl;
	}




}
