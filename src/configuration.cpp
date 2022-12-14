#include"../lib/configuration.h"

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
			cout << "Verbose wasn't set. Defaul value will be used instead" << endl;
		if (!is_rawdata_dir)
			cout << "Input directory wasn't set. Defaul value will be used instead" << endl;
		if (!is_output_dir)
			cout << "Output directoy wasn't set. Defaul value will be used instead" << endl;
		if (!is_do_filtration)
			cout << "Filtration configuration wasn't set. Defaul value will be used instead" << endl;
		if (!is_get_fr)
			cout << "Get FR wasn't set. Defaul value will be used instead" << endl;
		if (!is_num_files)
			cout << "Number of input files wasn't set. Defaul value will be used instead" << endl;
		if (!is_do_tpl)
			cout << "Do TPL mode wasn't set. Defaul value will be used instead" << endl;
		if (!is_srez_mode)
			cout << "Srez mode wasn't set. Defaul value will be used instead" << endl;
	}


	if (!is_files)
	{
		cout << "List of files is empty. There is no default value. Exiting" << endl;
		exit (0);
	}

	if (!is_tplfile and !do_tpl)
	{
		cout << "TPL file wasn't set. There is no default value. Exiting" << endl;
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
	is_get_fr = false;
	is_num_files = false;
	is_files = false;
	is_do_tpl = false;
	is_srez_mode = false;


	// setting default values
	verbose = true;
	rawdata_dir = ".";
	output_dir = "out/";
	do_filtration = true;
	get_fr = true;
	num_files = 1;
	files = vector<string> (0);
	do_tpl = false;
	srez_mode = false;


	ifstream cfg (file_name);

	if (cfg.is_open())
	{
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
				is_tplfile = fill_file(tplfile, value, line_num);
			}
			else if (name == "do_filtration")
			{
				is_do_filtration = fill_bool(do_filtration, value, line_num);
			}
			else if (name == "get_fr")
			{
				is_get_fr = fill_bool(get_fr, value, line_num);
			}
			else if (name == "do_tpl")
			{
				is_do_tpl = fill_bool(do_tpl, value, line_num);
			}
			else if (name == "srez_mode")
			{
				is_srez_mode = fill_bool(srez_mode, value, line_num);
			}
			else if (name == "num_files")
			{
				num_files = my_stoi(value, line_num);

				if (num_files == -1)
					num_files = 1;
				else
					is_num_files = true;
			}
			else if (name == "runs:")
			{
				files.reserve(num_files);

				for (int i = 0; i < num_files; i++)
				{
// ДОБАВИТЬ EOF
					getline(cfg, line);
					files.push_back(line);
				}

				is_files = true;
			}
			else 
			{
				cout << "Unknown name of parameter: " << name << endl;
			}
		}

	}
	else
	{
		cout << "Error occured while reading configuration file: no such file " << file_name << endl;
		exit(1);
	}

	cfg.close();
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

	if (pos == 18446744073709551615u) 
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
			cout << "Unkwnown parameter value at line " << line_num << " of configuration file" << endl;
		else
			cout << "Unkwnown parameter value in configuration file" << endl;

		return -1;
	}
}


bool Configuration::fill_bool (bool& parameter, string value, int line_num)
{
	int v = my_stoi(value, line_num);

	if (v != 0 and v != 1)
	{
		cout << "error" << endl;
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
		cout << "error" << endl;
		return false;
	}
	else if(!(info.st_mode & S_IFDIR))
	{
		cout << "error " << value << " is not a directory" << endl;
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
		cout << "error" << endl;
		return false;
	}
	else if(info.st_mode & S_IFDIR)
	{
		cout << "error " << value << " is a directory" << endl;
		return false;
	}
	else
	{
		parameter = value;
		return true;
	}
}

