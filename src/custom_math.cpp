#include"../lib/massages.h"
#include"../lib/custom_math.h"

#include <vector>
#include <algorithm>
#include <cmath>

#include<iostream>
#include<fstream>

using namespace std;

template<typename T> size_t max_pos(vector<T>& vec)
{
// find the position of maximum of the given vector

	size_t pos = 0;
	T value = vec.at(0);

	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value < vec.at(i))
		{
			value = vec.at(i);
			pos = i;
		}
	}

	return pos;
}

template<typename T> T max(vector<T>& vec)
{
// find the value of maximum of the given vector

	T value = vec.at(0);
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

template<typename T> size_t min_pos(vector<T>& vec)
{
// find the position of minimum of the given vector

	size_t pos = 0;
	double value = vec.at(0);

	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
		if (value > vec.at(i))
		{
			value = vec.at(i);
			pos = i;
		}
	}

	return pos;
}

template<typename T> T min(vector<T>& vec)
{
// find the value of minimum of the given vector

	T value = vec.at(0);
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



int normilize_vector (vector<double>& vec)
{
// normilize vector so that its maximum equals 1 and minimum equals 0

	double min_ = min(vec);
	double max_ = max(vec);
	double norm_factor = max_ - min_;

	size_t n = vec.size();

	for (size_t i = 0; i < n; ++i)
	{
			vec.at(i) = (vec.at(i) - min_)/norm_factor;
	}

	return 0;
}




double median (vector<double>& vec, size_t begin, size_t end)
{
// find median of given array

	if (end >= vec.size() || end == 0) 
		end = vec.size() - 1;

	vector<double> a (&vec.at(begin), &vec.at(end));
	double median;

	sort(a.begin(), a.end());

	if ((a.size() / 2) != 0)
		median = a[(a.capacity() - 1)/2];
	else
		median = a[a.capacity()/2] + a[a.capacity()/2 - 1];

	return median;
}


double mean (vector<double>& vec, size_t begin, size_t end)
{
// find mean value of given array

	if (end >= vec.size() || end == 0) 
		end = vec.size() - 1;

	vector<double> a (&vec[begin], &vec[end]);
	double mean = 0.0;

	for (size_t i = 0; i < a.size(); i++)
		mean += a[i];

	return mean/double(a.size());
}


double sum (vector<double>& vec, size_t begin, size_t end)
{
// find sum of given array

	if (end >= vec.size() || end == 0) 
		end = vec.size() - 1;

	vector<double> a (&vec[begin], &vec[end]);
	double sum = 0.0;

	for (size_t i = 0; i < vec.size(); ++i)
		sum += vec.at(i);

	return sum;
}


double sigma(vector<double>& vec, size_t begin, size_t end)
{
// find standart deviation of given array

	if (end >= vec.size() || end == 0) 
		end = vec.size() - 1;

	vector<double> a (&vec[begin], &vec[end]);

	double deviation = 0.0;
	int n = a.size();

	double m = mean(a);

	for (int i = 0; i < n; i++)
		deviation += (a.at(i) - m)*(a.at(i) - m);

	deviation = sqrt(deviation/double(n-1));

	return deviation;
}





double cycle_discrete_ccf (vector<double>& first, vector<double>& second, int delta)
{
// find ccf of two given array with cycle transit across the obsevational window

	int size_f = first.capacity();
	int size_s = second.capacity();

	if (size_f > size_s)
	{
		for (int i = 0; i < size_f - size_s; ++i)
			second.push_back(0.0);
	}

	double sum = 0.0;

	if (delta >= 0)
	{
		for (int i = 0; i < size_f; i++)
			sum += first[(i + delta)%size_f] * second[i]; 
	}else
	{
		for (int i = 0; i < size_f; i++)
			sum += first[i] * second[(i - delta)%size_s]; 
	}

	return sum;
}


double discrete_ccf (vector<double>& first_in, vector<double>& second, int delta)
{
// find ccf of two given array with partical crossing of them

	int size_f = first_in.capacity();
	int size_s = second.capacity();

	double sum = 0;
	int end;

	if (delta > size_f || delta < -size_s)
		return 0.0;


	int max_pos = 0;
	double max = 0;

	for (int i = 0; i < size_f; ++i)
	{
		if (first_in[i] > max)
		{
			max = first_in[i];
			max_pos = i;
		}
	}

	vector<double> first = first_in;

	int bias = max_pos - size_f/2;

	move_continous(first, bias);

	delta -= (double) bias;

	if (delta >= 0)
	{
		// filnd end btw ends of first and second vectors
		end = size_f - delta;
		if (end > size_s) end = size_s;

		for (int i = 0; i < end; i++)
			sum += first[i + delta] * second[i]; 
	}else
	{
		// filnd end btw ends of first and second vectors
		end = size_s + delta;
		if (end > size_f) end = size_f;

		for (int i = 0; i < end; i++)
			sum += first[i] * second[i - delta]; 
	}

	sum /= double(end);



	return sum;
}

vector<double> interpolation4 (vector<double> f)
{
// calculate coefficients of interpolation polynomial of 4th degree

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
// find value of polynomial at given point
// using horner algorithm (see wiki)

	double sum = 0.0;
	int n = p.size();

	for (int i = 0; i < n-1; i++)
		sum = (sum + p[i])*x;
	
	sum += p[n-1];

	return sum;
}

double find_root (vector<double> p, double left, double right)
{
// find a root of polynomial in the given range using bisection algorithm

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

double SNR(vector<double>& vec)
{
	// ATTENTION!!!
	// the function apropriate only for
	// normalized vectors
	// i.e. max(vec) = 1 
	//
	// we consider all points above level of 0.1 as signal 
	// and under as noise
	size_t n = vec.size();

	vector<double> noise, signal_sq;
	noise.reserve(n);
	signal_sq.reserve(n);


	// split the series to signal and noise arrays
	// and write squares of this points
	for (size_t i = 0; i < n; ++i)
	{
		if (vec.at(i) < .1)
			noise.push_back(vec.at(i));
		else
			signal_sq.push_back(vec.at(i) * vec.at(i));
	}

	if (signal_sq.size() == 0l || noise.size() < 5l)
		return 1e-5;

	double signal_power = sqrt(sum(signal_sq));

	return signal_power/sigma (noise);
}

void move_continous(vector<double>& vec, double bias)
{
// continually move the array using cycle moving between sequential profiles

	if (bias < 0.0)
		bias += (double) vec.size();

	// find integer and fraction part of needed bias
	int delta_int = int(bias);
	double delta_dec = bias - double(delta_int);

	// declare temporal vectors for profiles
	int n = vec.size();
	vector<double> temp_1 (n), temp_2(n);


	// move profile on discrete number of steps
	for (int j = 0; j < n; j ++)
	{
		temp_1.at(j) = vec.at((j + delta_int) % n);
		temp_2.at(j) = vec.at((j + delta_int + 1) % n);
	}

	// specify the bias on continous value (move less then on 1 step) 
	for(int j = 0; j < n; j++)
		vec[j] = (1.0 - delta_dec)*temp_1[j] + delta_dec*temp_2[j];
}



// declare template functions for all needed formats
template size_t max_pos(vector<double>& vec);
template size_t max_pos(vector<long double>& vec);
template size_t max_pos(vector<int>& vec);
template size_t max_pos(vector<size_t>& vec);

template size_t min_pos(vector<double>& vec);
template size_t min_pos(vector<long double>& vec);
template size_t min_pos(vector<int>& vec);
template size_t min_pos(vector<size_t>& vec);

template double max(vector<double>& vec);
template long double max(vector<long double>& vec);
template int max(vector<int>& vec);
template size_t max(vector<size_t>& vec);

template double min(vector<double>& vec);
template long double min(vector<long double>& vec);
template int min(vector<int>& vec);
template size_t min(vector<size_t>& vec);
