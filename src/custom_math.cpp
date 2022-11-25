#include <vector>
#include <algorithm>

#include<iostream>

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

double discrete_ccf (vector<double>& first, vector<double>& second, int delta)
{
	int size_f = first.capacity();
	int size_s = second.capacity();

	double sum = 0;
	int end;

	if (delta > size_f || delta < -size_s)
		return 0.0;

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
	{
		cout << "ERROR WHILE FINDING ROOT" << endl;
		return 0.0;
	}

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
