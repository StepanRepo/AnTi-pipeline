#include <vector>

using namespace std;

double median (vector<double>&);

double discrete_ccf (vector<double>& first, vector<double>& second, int delta);

// functions for work with polynomials
vector<double> interpolation4 (vector<double> f);
double horner (vector<double>& p, double x);
double find_root (vector<double> p, double left, double right);
