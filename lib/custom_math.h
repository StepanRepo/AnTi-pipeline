#include <vector>
#include<string>

#include"Simple-FFT/include/simple_fft/fft_settings.h"
#include"Simple-FFT/include/simple_fft/fft.h"


using namespace std;

int max_pos(vector<double>&);
double max(vector<double>&);
int min_pos(vector<double>&);
double min(vector<double>&);

int max_pos(vector<int>&);
int max(vector<int>&);
int min_pos(vector<int>&);
int min(vector<int>&);

void normilize_vector (vector<double>&);


// statictical characteristics of vectors
double median (vector<double>&);
double median (vector<double>&, int, int);

double mean (vector<double>&);
double mean (vector<double>&, int, int);

double sum (vector<double>&);
double sum (vector<double>&, int, int);

double sigma(vector<double>&);
double sigma(vector<double>&, int, int);


// correlations
double discrete_ccf (vector<double>& first, vector<double>& second, int delta);
double cycle_discrete_ccf (vector<double>& first, vector<double>& second, int delta);


// functions for work with polynomials
vector<double> interpolation4 (vector<double> f);
double horner (vector<double>& p, double x);
double find_root (vector<double> p, double left, double right);

double SNR(vector<double>&);

void move_continous (vector<double>& vec, double bias);
