#include <vector>
#include<string>

#define __USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR // setting for FFT library

#include"Simple-FFT/include/simple_fft/fft_settings.h"
#include"Simple-FFT/include/simple_fft/fft.h"


using namespace std;

template<typename T> size_t max_pos(vector<T>&);
template<typename T> T max(vector<T>&);
template<typename T> size_t min_pos(vector<T>&);
template<typename T> T min(vector<T>&);


int normilize_vector (vector<double>&);


// statictical characteristics of vectors
double median (vector<double>&, size_t = 0, size_t = 0);
double mean (vector<double>&, size_t = 0, size_t = 0);
double sum (vector<double>&, size_t = 0, size_t = 0);
double sigma (vector<double>&, size_t = 0, size_t = 0);


// correlations
double discrete_ccf (vector<double>& first, vector<double>& second, int delta);
double cycle_discrete_ccf (vector<double>& first, vector<double>& second, int delta);


// functions for work with polynomials
vector<double> interpolation4 (vector<double> f);
double horner (vector<double>& p, double x);
double find_root (vector<double> p, double left, double right);


// specific functions for profiles
double SNR(vector<double>&);
void move_continous (vector<double>& vec, double bias);
