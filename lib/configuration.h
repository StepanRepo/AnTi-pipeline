#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include<string>
#include<vector>

using namespace std;

class Configuration 
{
	public:
		Configuration(string file_name = "default.cfg");
		void command_line(int argc, char** argv);

		bool verbose;

		string rawdata_dir;
		string output_dir;
		string tplfile;

		bool do_filtration;
		bool get_fr;

		double deriv_threshold, median_threshold;
		int deriv_width, median_width;

		int num_files;
		vector<string> files;


		bool do_tpl;

		bool is_verbose;

		bool is_rawdata_dir;
		bool is_output_dir;
		bool is_tplfile;


		bool is_do_filtration;

		bool is_deriv_threshold;
		bool is_median_threshold;
		bool is_deriv_width;
		bool is_median_width;

		bool is_get_fr;

		bool is_num_files;
		bool is_files;


		bool is_do_tpl;

	private:
		void fill_config (string file_name);
		void remove_comments (string& line);
		void strip_white (string& line);
		void split_str (string& line, string& name, string& value);

		int my_stoi (string, int line_num = 0);
		double my_stod (string, int line_num = 0);

		bool fill_bool (bool& parameter, string value, int line_num);
		bool fill_directory (string& parameter, string value, int line_num);
		bool fill_file (string& parameter, string value, int line_num);

};

#endif
