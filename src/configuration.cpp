#include"../lib/configuration.h"
#include"../lib/massages.h"

#include<fstream>
#include<iostream>
#include<string>
#include<vector>
#include <sys/stat.h>


Configuration::Configuration(string file_name)
{
	fill_config(file_name);

	if (verbose)
	{
		if (!is_verbose)
			cout << WARNING << "Verbose wasn't set. Defaul value will be used instead" << endl;
		if (!is_rawdata_dir)
			cout << WARNING << "Input directory wasn't set. Defaul value will be used instead" << endl;
		if (!is_output_dir)
			cout << WARNING << "Output directoy wasn't set. Defaul value will be used instead" << endl;
		if (!is_do_filtration)
			cout << WARNING << "Filtration configuration wasn't set. Defaul value will be used instead" << endl;
		if (!is_deriv_threshold)
			cout << WARNING << "Derivative filter thresold wasn't set. Defaul value will be used instead" << endl;
		if (!is_deriv_width)
			cout << WARNING << "Derivative filter width wasn't set. Defaul value will be used instead" << endl;
		if (!is_median_threshold)
			cout << WARNING << "Median filter thresold wasn't set. Defaul value will be used instead" << endl;
		if (!median_width)
			cout << WARNING << "Median filter width wasn't set. Defaul value will be used instead" << endl;
		if (!is_get_fr)
			cout << WARNING << "Get FR wasn't set. Defaul value will be used instead" << endl;
		if (!is_num_files)
			cout << WARNING << "Number of input files wasn't set. Defaul value will be used instead" << endl;
		if (!is_do_tpl)
			cout << WARNING << "Do TPL mode wasn't set. Defaul value will be used instead" << endl;
	}


	if (!is_files)
	{
		cout << ERROR << "List of files is empty. There is no default value. Exiting" << endl;
		exit (0);
	}

	if (!is_tplfile and !do_tpl)
	{
		cout << ERROR << "TPL file wasn't set. There is no default value. Exiting" << endl;
		exit (0);
	}
}

void Configuration::fill_config(string file_name)
{
	is_verbose = true;
	is_rawdata_dir = false;
	is_output_dir = false;
	is_tplfile = false;
	is_do_filtration = false;
	is_deriv_threshold = false;
	is_median_threshold = false;
	is_deriv_width = false;
	is_median_width = false;
	is_get_fr = false;
	is_num_files = false;
	is_files = false;
	is_do_tpl = false;


	// setting default values
	verbose = true;
	rawdata_dir = ".";
	output_dir = "out/";
	do_filtration = true;
	deriv_threshold = 2e-2;
	deriv_width = 2;
	median_threshold = 1.5;
	median_width = 1;
	get_fr = true;
	num_files = 0;
	files = vector<string> (0);
	do_tpl = false;


	ifstream cfg (file_name);

	if (!cfg.is_open())
	{
		cout << ERROR << "Error occured while reading configuration file: no such file " << file_name << endl;
		exit(0);
	}

	string line, name, value;
	int line_num = 0;

	while (getline(cfg, line))
	{
		line_num ++;

		remove_comments (line);
		strip_white (line);

		if (line == "") continue;

		split_str (line, name, value);

		if (name == "verbose")
		{
			is_verbose = fill_bool(verbose, value, line_num);
		}
		else if (name == "rawdata_dir")
		{
			is_rawdata_dir = fill_directory(rawdata_dir, value, line_num);
		}
		else if (name == "output_dir")
		{
			is_output_dir = fill_directory(output_dir, value, line_num);
		}
		else if (name == "tplfile")
		{
			is_tplfile = fill_file(tplfile, rawdata_dir + value, line_num);
		}
		else if (name == "do_filtration")
		{
			is_do_filtration = fill_bool(do_filtration, value, line_num);
		}
		else if (name == "deriv_threshold")
		{
			deriv_threshold = my_stod(value, line_num);

			if (deriv_threshold == -1.0)
				deriv_threshold = 2e-2;
			else
				is_deriv_threshold = true;
		}
		else if (name == "deriv_width")
		{
			deriv_width = my_stoi(value, line_num);

			if (deriv_width == -1)
				deriv_threshold = 2;
			else
				is_deriv_width = true;
		}
		else if (name == "median_threshold")
		{
			median_threshold = my_stod(value, line_num);

			if (median_threshold == -1.0)
				deriv_threshold = 1.5;
			else
				is_median_threshold = true;
		}
		else if (name == "median_width")
		{
			median_width = my_stoi(value, line_num);

			if (median_width == -1)
				median_width = 1;
			else
				is_median_width = true;
		}
		else if (name == "get_fr")
		{
			is_get_fr = fill_bool(get_fr, value, line_num);
		}
		else if (name == "do_tpl")
		{
			is_do_tpl = fill_bool(do_tpl, value, line_num);
		}
		else if (name == "num_files")
		{
			num_files = my_stoi(value, line_num);

			if (num_files == -1)
				num_files = 0;
			else
				is_num_files = true;
		}
		else if (name == "runs:")
		{
			files.reserve(num_files);

			int i = 0;
			while (getline(cfg, line))
			{
				files.push_back(line);

				i++;
				if (i == num_files)
					break;
			}

			is_files = true;
			break;
		}
		else 
		{
			cout << WARNING << "Unknown name of parameter at line " << line_num << " of configuration file" << endl;
		}
	}

	cfg.close();
}

void Configuration::command_line(int argc, char** argv)
{
	string buffer, name, value;

	for (int i = 1; i < argc; i++)
	{
		buffer = argv[i];

		if (buffer[0] == '-')
		{
			if (buffer[1] == '-')
				buffer = buffer.substr(2);
			else
				buffer = buffer.substr(1);
		}

		split_str (buffer, name, value);


		if (name == "verbose" || name == "v")
		{
			verbose = true;
			is_verbose = true;
		}
		else if (name == "rawdata_dir")
		{
			is_rawdata_dir = fill_directory(rawdata_dir, value, i);
		}
		else if (name == "output_dir")
		{
			is_output_dir = fill_directory(output_dir, value, i);
		}
		else if (name == "tplfile")
		{
			is_tplfile = fill_file(tplfile, rawdata_dir + value, i);
		}
		else if (name == "do_filtration")
		{
			do_filtration = true;
			is_do_filtration = true;
		}
		else if (name == "deriv_threshold")
		{
			deriv_threshold = my_stod(value, i);

			if (deriv_threshold == -1.0)
				deriv_threshold = 2e-2;
			else
				is_deriv_threshold = true;
		}
		else if (name == "deriv_width")
		{
			deriv_width = my_stoi(value, i);

			if (deriv_width == -1)
				deriv_threshold = 2;
			else
				is_deriv_width = true;
		}
		else if (name == "median_threshold")
		{
			median_threshold = my_stod(value, i);

			if (median_threshold == -1.0)
				deriv_threshold = 1.5;
			else
				is_median_threshold = true;
		}
		else if (name == "median_width")
		{
			median_width = my_stoi(value, i);

			if (median_width == -1)
				median_width = 1;
			else
				is_median_width = true;
		}
		else if (name == "get_fr")
		{
			get_fr = true;
			is_get_fr = true;
		}
		else if (name == "do_tpl")
		{
			do_tpl = true;
			is_do_tpl = true; 
		}
		else 
		{
			cout << WARNING << "Unknown argument: " << buffer << endl;
		}
	}
}

void Configuration::remove_comments(string& line)
{
	size_t n = line.find("#");
	line = line.substr(0, n);
}

void Configuration::strip_white(string& line)
{
	if (line == "" or line.size() == 0) return;

	size_t n = line.size();

	for (size_t i = 0; i < n; i++)
	{
		if (line[i] == ' ' or line[i] == '\t') 
		{
			line.erase(i, 1);
			i--;
			n--;
		}
	}
}

void Configuration::split_str (string& line, string& name, string& value)
{
	size_t pos = line.find('=');

	if (pos > 200) 
	{
		name = line;
		return;
	}

	name = line.substr(0, pos);
	value = line.substr(pos+1);
}

int Configuration::my_stoi(string str, int line_num)
{
	try
	{
		return stoi(str);
	}
	catch(std::invalid_argument const&)
	{
		if (line_num != 0)
			cout << WARNING << "Unkwnown parameter value at line " << line_num << " of configuration file" << endl;
		else
			cout << WARNING << "Unkwnown parameter value in configuration file" << endl;

		return -1.0;
	}
}


double Configuration::my_stod(string str, int line_num)
{
	try
	{
		return stod(str);
	}
	catch(std::invalid_argument const&)
	{
		if (line_num != 0)
			cout << WARNING << "Unkwnown parameter value at line " << line_num << " of configuration file" << endl;
		else
			cout << WARNING << "Unkwnown parameter value in configuration file" << endl;

		return -1;
	}
}

bool Configuration::fill_bool (bool& parameter, string value, int line_num)
{
	int v = my_stoi(value, line_num);

	if (v != 0 and v != 1)
	{
		cout << WARNING << "Unkwnown parameter value at line " << line_num << " of configuration file" << endl;
		return false;
	}

	else if (v == 1)
		parameter = true;
	else if (v == 0)
		parameter = false;

	return true;
}

bool Configuration::fill_directory (string& parameter, string value, int line_num)
{
	struct stat info;

	if(stat(value.c_str(), &info) != 0 )
	{
		cout << WARNING << "Unkwnown parameter value at line " << line_num << " of configuration file" << endl;
		return false;
	}
	else if(!(info.st_mode & S_IFDIR))
	{
		cout << WARNING << value << " is not a directory. Parameter wasn't set" << endl;
		return false;
	}
	else
	{
		parameter = value;
		return true;
	}
}

bool Configuration::fill_file (string& parameter, string value, int line_num)
{
	struct stat info;

	if(stat(value.c_str(), &info) != 0 )
	{
		cout << WARNING << "Unkwnown parameter value at line " << line_num << " of configuration file" << endl;
		return false;
	}
	else if(info.st_mode & S_IFDIR)
	{
		cout << WARNING << value << " is a directory. Parameter wasn't set" << endl;
		return false;
	}
	else
	{
		parameter = value;
		return true;
	}
}

