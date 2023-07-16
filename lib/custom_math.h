#include <vector>

using namespace std;

int max_pos(vector<double>&);
double max(vector<double>&);
int min_pos(vector<double>&);
double min(vector<double>&);

int max_pos(vector<int>&);
int max(vector<int>&);
int min_pos(vector<int>&);
int min(vector<int>&);

vector<int> max_pos(vector<vector<double>>&);
double max(vector<vector<double>>&);
vector<int> min_pos(vector<vector<double>>&);
double min(vector<vector<double>>&);


void normilize_vector (vector<vector<double>>&);

// statictic characteristics of vectors
double median (vector<double>&);
double median (vector<vector<double>>&);
double median (vector<double>&, int, int);

double mean (vector<double>&);
double mean (vector<vector<double>>&);
double mean (vector<double>&, int, int);

double sum (vector<double>&);

double sigma(vector<double>&);
double sigma(vector<vector<double>>&);
double sigma(vector<double>&, int, int);


// correlation
vector<double> discrete_ccf (vector<double>& first, vector<double>& second);
vector<double> discrete_ccf (vector<vector<double>>& first, vector<vector<double>>& second);

double cycle_discrete_ccf (vector<double>& first, vector<double>& second, int delta);
double cycle_discrete_ccf (vector<vector<double>>& first, vector<vector<double>>& second, int delta);



double SNR(vector<double>&);
double SNR(vector<vector<double>>&);

// functions for work with polynomials
vector<double> interpolation4 (vector<double> f);
double horner (vector<double>& p, double x);
double find_root (vector<double> p, double left, double right);


void move_continous (vector<double>& vec, double bias);
void move_continous (vector<vector<double>>& vec, double bias);
