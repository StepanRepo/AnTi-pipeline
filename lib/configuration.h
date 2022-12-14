#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include<string>
#include<vector>

using namespace std;

class Configuration 
{
	public:
		bool verbose;

		string rawdata_dir;
		string output_dir;
		string tplfile;

		bool do_filtration;
		bool get_fr;

		int num_files;
		vector<string> files;


		bool do_tpl;
		bool srez_mode;

		Configuration(string file_name = "default.cfg");

	private:
		void fill_config (string file_name);
		void remove_comments (string& line);
		void strip_white (string& line);
		void split_str (string& line, string& name, string& value);

		int my_stoi (string, int line_num = 0);

		bool fill_bool (bool& parameter, string value, int line_num);
		bool fill_directory (string& parameter, string value, int line_num);
		bool fill_file (string& parameter, string value, int line_num);

		bool is_verbose;

		bool is_rawdata_dir;
		bool is_output_dir;
		bool is_tplfile;

		bool is_do_filtration;
		bool is_get_fr;

		bool is_num_files;
		bool is_files;


		bool is_do_tpl;
		bool is_srez_mode;

};

#endif
