#include"../lib/massages.h"
#include"../lib/custom_math.h"

#include <vector>
#include <algorithm>
#include <cmath>

#include<iostream>
#include<fstream>

using namespace std;

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
		deviation += (a[i] - m)*(a[i] - m);

	deviation = sqrt(deviation/double(n-1));

	return deviation;
}

double sigma(vector<double>& vec, int begin, int end)
{
	if (begin < 0) begin = 0;
	if (end >= (int) vec.size()) end = vec.size() - 1;

	vector<double> a (&vec[begin], &vec[end]);

	return sigma(a);
}



double cycle_discrete_ccf (vector<double>& first, vector<double>& second, int delta)
{
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

	bias = 0;

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


	for (int j = 0; j < n; j ++)
	{
		temp_1[j] = vec[(j + delta_int) % n];
		temp_2[j] = vec[(j + delta_int + 1) % n];
	}

	for(int j = 0; j < n; j++)
		vec[j] = (1.0 - delta_dec)*temp_1[j] + delta_dec*temp_2[j];
}
