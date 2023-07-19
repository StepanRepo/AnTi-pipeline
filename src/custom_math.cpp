#include"../lib/massages.h"
#include"../lib/custom_math.h"
#include"../lib/configuration.h"

#include <vector>
#include <algorithm>
#include <cmath>

#include<iostream>
#include<fstream>

using namespace std;

extern Configuration *cfg;

int max_pos(vector<double>& vec)
{
	int pos = 0;
	double value = vec.at(0);

	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value < vec.at(i))
		{
			value = vec.at(i);
			pos = (int) i;
		}
	}

	return pos;
}

double max(vector<double>& vec)
{
	double value = vec.at(0);
	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value < vec.at(i))
		{
			value = vec.at(i);
		}
	}

	return value;
}

int min_pos(vector<double>& vec)
{
	int pos = 0;
	double value = vec.at(0);

	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value > vec.at(i))
		{
			value = vec.at(i);
			pos = (int) i;
		}
	}

	return pos;
}

double min(vector<double>& vec)
{
	double value = vec.at(0);
	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value > vec.at(i))
		{
			value = vec.at(i);
		}
	}

	return value;
}

int max_pos(vector<int>& vec)
{
	int pos = 0;
	int value = vec.at(0);

	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value < vec.at(i))
		{
			value = vec.at(i);
			pos = (int) i;
		}
	}

	return pos;
}

int max(vector<int>& vec)
{
	int value = vec.at(0);
	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value < vec.at(i))
		{
			value = vec.at(i);
		}
	}

	return value;
}

int min_pos(vector<int>& vec)
{
	int pos = 0;
	int value = vec.at(0);

	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value > vec.at(i))
		{
			value = vec.at(i);
			pos = (int) i;
		}
	}

	return pos;
}

int min(vector<int>& vec)
{
	int value = vec.at(0);
	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value > vec.at(i))
		{
			value = vec.at(i);
		}
	}

	return value;
}

vector<int> max_pos(vector<vector<double>>& vec)
{
	double value = vec.at(0).at(0);

	size_t n = vec.size();
	size_t m = vec.at(0).size();

	vector<int> max_pos(2);

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			if (value < vec.at(i).at(j))
			{
				value = vec.at(i).at(j);
				max_pos[0] = i;
				max_pos[1] = j;
			}
		}
	}

	return max_pos;
}

double max (vector<vector<double>>& vec)
{
	double value = vec.at(0).at(0);

	size_t n = vec.size();
	size_t m = vec.at(0).size();

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			if (value < vec.at(i).at(j))
			{
				value = vec.at(i).at(j);
			}
		}
	}

	return value;
}

vector<int> min_pos(vector<vector<double>>& vec)
{
	double value = vec.at(0).at(0);

	size_t n = vec.size();
	size_t m = vec.at(0).size();

	vector<int> min_pos(2);

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			if (value > vec.at(i).at(j))
			{
				value = vec.at(i).at(j);
				min_pos[0] = i;
				min_pos[1] = j;
			}
		}
	}

	return min_pos;
}

double min (vector<vector<double>>& vec)
{
	double value = vec.at(0).at(0);

	size_t n = vec.size();
	size_t m = vec.at(0).size();

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			if (value > vec.at(i).at(j))
			{
				value = vec.at(i).at(j);
			}
		}
	}

	return value;
}

void normilize_vector (vector<double>& vec)
{
	double min_ = min(vec);
	double max_ = max(vec);
	double norm_factor = max_ - min_;

	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
			vec.at(i) = (vec.at(i) - min_)/norm_factor;
	}
}
void normilize_vector (vector<vector<double>>& vec)
{
	double min_ = median(vec);
	double max_ = max(vec);
	double norm_factor = max_ - min_;

	size_t n = vec.size();
	size_t m = vec.at(0).size();

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			vec.at(i).at(j) = (vec.at(i).at(j) - min_)/norm_factor;
		}
	}
}






double median (vector<double>& vec)
{
	double median;
	vector<double> a = vec;

	sort(a.begin(), a.end());

	if ((a.size() / 2) != 0)
		median = a[(a.capacity() - 1)/2];
	else
		median = a[a.capacity()/2] + a[a.capacity()/2 - 1];

	return median;
}

double median (vector<vector<double>>& vec)
{
	size_t n = vec.size();
	size_t m = vec.at(0).size();

	double median;
	vector<double> a (n*m);

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			a.at(j + m*i) = vec.at(i).at(j);
		}
	}

	sort(a.begin(), a.end());

	if ((a.size() / 2) != 0)
		median = a.at((n*m - 1)/2);
	else
		median = a.at(n*m/2) + a.at(n*m/2 - 1);

	return median;
}

double median (vector<double>& vec, int begin, int end)
{
	if (begin < 0) begin = 0;
	if (end >= (int) vec.size()) end = vec.size() - 1;

	vector<double> a (&vec[begin], &vec[end]);

	return median(a);
}


double mean (vector<double>& vec)
{
	vector<double> a = vec;
	double mean = 0.0;

	for (int i = 0; i < (int) a.size(); i++)
		mean += a[i];

	return mean/double(a.size());
}

double mean (vector<vector<double>>& vec)
{
	size_t n = vec.size();
	size_t m = vec.at(0).size();

	double mean = 0.0;

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			mean += vec.at(i).at(j);
		}
	}

	return mean/double(n)/double(m);
}

double mean (vector<double>& vec, int begin, int end)
{
	if (begin < 0) begin = 0;
	if (end >= (int) vec.size()) end = vec.size() - 1;

	vector<double> a (&vec[begin], &vec[end]);

	return mean(a);
}


double sigma(vector<double>& vec)
{
	vector<double> a = vec;

	double deviation = 0.0;
	int n = a.size();

	double m = mean(a);

	for (int i = 0; i < n; i++)
		deviation += (a.at(i) - m)*(a.at(i) - m);

	deviation = sqrt(deviation/double(n-1));

	return deviation;
}

double sigma(vector<vector<double>>& vec)
{
	size_t n = vec.size();
	size_t m = vec.at(0).size();

	double deviation = 0.0;
	double mean_ = mean(vec);

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			deviation += (vec.at(i).at(j) - mean_) * (vec.at(i).at(j) - mean_);
		}
	}

	deviation = sqrt(deviation/double(n-1)/double(m-1));

	return deviation;
}

double sigma(vector<double>& vec, int begin, int end)
{
	if (begin < 0) begin = 0;
	if (end >= (int) vec.size()) end = vec.size() - 1;

	vector<double> a (&vec[begin], &vec[end]);

	return sigma(a);
}

double sum (vector<double>& vec)
{
	size_t n = vec.size();
	double sum = 0.0;

	for (size_t i = 0; i < n; ++i)
		sum += vec.at(i);

	return sum;
}


vector<double> discrete_ccf (vector<double>& first, vector<double>& second_in)
{
	size_t size_f = first.size();
	size_t size_s = second_in.size();

	// zero paddiong of second vector
	vector<double> second (2*(size_f-1) + size_s);

	for (size_t j = size_f; j < size_f + size_s; ++j)
		second.at(j) = second_in.at(j - size_f);

	vector<double> corr (size_f + size_s - 1);
	
	for (size_t j = 0; j < size_f + size_s - 1; ++j)
	{
		for (size_t i = 0; i < size_f; ++i)
		{
			corr.at(j) += first.at(i) * second.at(size_f + size_s - j + i - 2);
		}

		if (j >= size_f)
			corr.at(j) /= (double) (j)*1e-2;
		else
			corr.at(j) /= (double) (2*size_f - j)*1e-2;
	}

	return corr;
}

vector<double> discrete_ccf (vector<vector<double>>& first, vector<vector<double>>& second, 
		string filename)
{
	size_t c = first.size();
	size_t n = first.at(0).size();
	size_t m = second.at(0).size();

	if (c != second.size())
		throw "Vectors must have same first dimention for cross-correlation";


	vector<vector<double>> ccf;
	ccf = vector(c, vector<double>(n + m - 1));

#pragma omp parallel for
	for (size_t i = 0; i < c; ++i)
		ccf.at(i) = discrete_ccf(first.at(i), second.at(i));

	vector<double> v_sum (n + m - 1);
	vector<double> h_sum (c);

	for (size_t j = 0; j < n + m - 1; ++j)
	{
		for (size_t i = 0; i < c; ++i)
			v_sum.at(j) += ccf.at(i).at(j);

		v_sum.at(j) /= (double) c;
	}

	for (size_t i = 0; i < c; ++i)
		h_sum.at(i) = sum(ccf.at(i));


	vector<double> mask (c);
	fill(mask.begin(), mask.end(), 1.0);

	double treshold = (1 - cfg->ccf_threshold) * sum(v_sum);
	size_t width = cfg->ccf_width;


	for (size_t i = 0; i < c; ++i)
	{
		if (h_sum.at(i) < treshold)
		{
			if (i < width)
				fill(mask.begin(), mask.begin() + i + width, 0.0);
			else if (i > c - width)
				fill(mask.begin() + i - width, mask.end(), 0.0);
			else
				fill(mask.begin() + i - width, mask.begin() + i + width, 0.0);
		}
	}


	// print ccf to file if filename exists
	if (filename != "")
	{
		ofstream out(filename);
		for (size_t i = 0; i < c; ++i)
		{
			for (size_t j = 0; j < n + m - 1; ++j)
			{
				out << ccf.at(i).at(j) * mask.at(i) << "  ";
			}
			out << endl;
		}
		out.close();
	}


	for (size_t j = 0; j < n + m - 1; ++j)
	{
		for (size_t i = 0; i < c; ++i)
			v_sum.at(j) += ccf.at(i).at(j) * mask.at(i);

		v_sum.at(j) /= (double) c;
	}

	return v_sum;
}



vector<vector<double>> discrete_ccf (vector<vector<double>>& first, vector<vector<double>>& second, 
		vector<double>& mask, string filename)
{
	size_t c = first.size();
	size_t n = first.at(0).size();
	size_t m = second.at(0).size();

	if (c != second.size())
		throw "Vectors must have same first dimention for cross-correlation";

	fill(mask.begin(), mask.end(), 1.0);

	vector<vector<double>> ccf;
	ccf = vector(c, vector<double>(n + m - 1));

#pragma omp parallel for
	for (size_t i = 0; i < c; ++i)
		ccf.at(i) = discrete_ccf(first.at(i), second.at(i));


	vector<double> v_sum (n + m - 1);
	vector<double> h_sum (c);

	for (size_t j = 0; j < n + m - 1; ++j)
	{
		for (size_t i = 0; i < c; ++i)
			v_sum.at(j) += ccf.at(i).at(j);

		v_sum.at(j) /= (double) c;
	}

	for (size_t i = 0; i < c; ++i)
		h_sum.at(i) = sum(ccf.at(i));

	double treshold = (1 - cfg->ccf_threshold) * sum(v_sum);
	size_t width = cfg->ccf_width;

	for (size_t i = 0; i < c; ++i)
	{
		if (h_sum.at(i) < treshold)
		{
			if (i < width)
				fill(mask.begin(), mask.begin() + i + width, 0.0);
			else if (i > c - width)
				fill(mask.begin() + i - width, mask.end(), 0.0);
			else
				fill(mask.begin() + i - width, mask.begin() + i + width, 0.0);
		}
	}

	// print ccf to file if filename exists
	if (filename != "")
	{
		ofstream out(filename);
		for (size_t i = 0; i < c; ++i)
		{
			for (size_t j = 0; j < n + m - 1; ++j)
			{
				out << ccf.at(i).at(j) * mask.at(i) << "  ";
			}
			out << endl;
		}
		out.close();
	}

	return ccf;
}

vector<double> interpolation4 (vector<double> f)
{
	if (f.size() != 5)
		cout << "INTERPOLATION ERROR " << f.size() << endl;

	vector<double> coefficients;
	coefficients.reserve(5);

	coefficients.push_back((f[0] + f[4])/24.0 - (f[1] + f[3])/6.0 + f[2]/4.0);
	coefficients.push_back((f[4] - f[0])/12.0 + (f[1] - f[3])/6.0);
	coefficients.push_back(2.0*(f[1] + f[3])/3.0 - (f[0] + f[4])/24.0 - 5.0*f[2]/4.0);
	coefficients.push_back((f[0] - f[4] - 8.0*(f[1] - f[3]))/12.0);
	coefficients.push_back(f[2]);

	return coefficients;
}

double horner (vector<double>& p, double x)
{
	double sum = 0.0;
	int n = p.size();

	for (int i = 0; i < n-1; i++)
		sum = (sum + p[i])*x;
	
	sum += p[n-1];

	return sum;
}

double find_root (vector<double> p, double left, double right)
{
	if (horner(p, left) * horner(p, right) > 0)
		throw invalid_argument (string(WARNING) + "Cann't find root");

	double eps = 1e-16;
	double root = 0.0;

	double dist = right - left;

	while (dist > eps)
	{
		dist /= 2.0;

		root = left + dist;

		if (horner(p, left)*horner(p, root) < 0)
			right = root;
		else
			left = root;
	}

	return root;
}

void move_continous(vector<double>& vec, double bias)
{
	if (bias < 0.0)
		bias += (double) vec.size();

	int delta_int = int(bias);
	double delta_dec = bias - double(delta_int);

	int n = vec.size();
	vector<double> temp_1 (n), temp_2(n);


	for (int j = 0; j < n; ++j)
	{
		temp_1[j] = vec[(j + delta_int) % n];
		temp_2[j] = vec[(j + delta_int + 1) % n];
	}

	for(int j = 0; j < n; ++j)
		vec[j] = (1.0 - delta_dec)*temp_1[j] + delta_dec*temp_2[j];
}

void move_continous(vector<vector<double>>& vec, double bias)
{
	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
		move_continous(vec.at(i), bias);
}


double SNR(vector<double>& vec)
{
	size_t n = vec.size();

	vector<double> noise;
	noise.reserve(n);


	for (size_t i = 0; i < n; ++i)
	{
		if (vec.at(i) < .1)
		{
			noise.push_back(vec.at(i));
		}
	}

	return 1.0/sigma (noise);
}

double SNR(vector<vector<double>>& vec)
{
	size_t n = vec.size();
	size_t m = vec.at(0).size();

	vector<double> noise;
	noise.reserve(n*m);

	for (size_t i = 0; i < n; ++i)
	{
		for (size_t j = 0; j < m; ++j)
		{
			if (vec.at(i).at(j) < .1)
				noise.push_back(vec.at(i).at(j));
		}
	}

	return 1.0/sigma (noise);
}
