//***************
//	Copyright: Kyle Chen
//	Author: Kyle Chen
//	Date: 2018-01-26
//	Description: Mutual information analysis program, between spikes and spikes;
//***************
#include "../include/mi_uniform.h"
#include "../include/io.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <stdexcept>

using namespace std;
// compact function to calculate mutual information between multi-type signal
//	arguments:
//	argv[1] = path for first spike train;
//	argv[2] = path for second spike train;
//	argv[3] = path for output data file;
//	argv[4] = range of timelag;
int main(int argc, const char* argv[]) {
	if (argc != 5) throw runtime_error("wrong number of args");
	clock_t start, finish;
	start = clock();
	// INPUT NEURONAL DATA:
	vector<bool> x, y;
	Read1DBin(argv[1], x, 0, 0);
	Read1DBin(argv[2], y, 0, 0);
	// Set time range;
	size_t range[2];
	istringstream range_in(argv[4]);
	string buffer;
	getline(range_in, buffer, ',');
	int ntd = atoi(buffer.c_str());
	getline(range_in, buffer, ',');
	int ptd = atoi(buffer.c_str());
  range[0] = ntd;
  range[1] = ptd;

	vector<double> tdmi;
	TDMI(x, y, tdmi, range);

	//	Output data:
	ofstream data_out;
	data_out.open(argv[3]);
	data_out << "#timelag,mi" << endl;
	for (int i = 0; i < ntd + ptd + 1; i++) {
		data_out << i - ntd << ',' << setprecision(15) << (double)tdmi[i] << '\n';
	}
	data_out.close();

	finish = clock();
	// Time counting:
	cout << "It takes " << (finish - start)*1.0 / CLOCKS_PER_SEC << "s" << endl;
	return 0;
}
