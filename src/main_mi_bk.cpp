//***************
//	Copyright: Kyle Chen
//	Author: Kyle Chen
//	Date: 2017-06-03
//	Description: Mutual information analysis program; version 1.0
//***************
#include "../include/mi.h"
#include "../include/io.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <stdexcept>

using namespace std;
// compact function to calculate mutual information between multi-type signal
//	arguments:
//	argv[1] = mode code;
//		0: spike to spike
//		1: spike to lfp
//		2: lfp to lfp
//		3: spike to potential
//		4: potential to potential
//	argv[2] = timing step for TDMI;
//	argv[3] = delay time range;
int main(int argc, const char* argv[]) {
	if (argc != 4) {
		throw runtime_error("wrong number of args");
	}
	clock_t start, finish;
	start = clock();
	// Preparing input args;
	double dt = atof(argv[2]);
	string range = argv[3];
	string::size_type pos = range.find_first_of(',', 0 );
	int negative_time_delay = atoi(range.substr(0, pos).c_str());
	range.erase(0, pos + 1);
	int positive_time_delay = atoi(range.c_str());
	range = "";
	printf(">> dt = %f ms\n", dt);
	printf(">> Time-delay = [-%.2f, %.2f] ms\n", negative_time_delay * dt, positive_time_delay * dt);
	// Judge the running mode:
	int mode = atoi(argv[1]);
	if (mode == 0) {
		vector<double> raster_x, raster_y;
		string path;
		path = "./data/raster/raster_x.csv";
		Read1D(path, raster_x, 0, 1);
		path = "./data/raster/raster_y.csv";
		Read1D(path, raster_y, 0, 1);

		double tmax;
		if (raster_x.back() > raster_y.back()) tmax = ceil(raster_x.back());
		else tmax = ceil(raster_y.back());
		cout << ">> Calculating ordered TDMI ... " << endl;
		vector<double> tdmi;
		TDMI(raster_x, raster_y, dt, tmax, negative_time_delay, positive_time_delay, tdmi);
		//	Output data:
		ofstream data_out;
		cout << ">> Outputing data ... " << endl;
		data_out.open("./data/mi/mi_ss.csv");
		data_out << "timelag,mi" << endl;
		for (int i = 0; i < negative_time_delay + positive_time_delay + 1; i++) {
			data_out << (double)dt*(i - negative_time_delay) << ',' << (double)tdmi[1];
			if (i < positive_time_delay + negative_time_delay) data_out << endl;
		}
		data_out.close();
	} else if (mode == 1) {
		// INPUT NEURONAL DATA:
		vector<double> raster, lfp;
		vector<double> tdmi_ordered;
		vector<double> tdmi_random;
		// DATA OF PRELAYER NEURON:
		string path;
		path = "./data/raster/raster.csv";
		Read1D(path, raster, 0, 1);
		if (raster.size() == 0) {
			tdmi_ordered.resize(negative_time_delay + positive_time_delay + 1, 0);
			tdmi_random.resize(negative_time_delay + positive_time_delay + 1, 0);
		} else {
			path = "./data/lfp/lfp.csv";
			Read1D(path, lfp, 0, 1);

			double sampling_dt = 0.03125;
			cout << ">> Calculating ordered TDMI ... " << endl;
			TDMI(raster, lfp, dt, sampling_dt, negative_time_delay, positive_time_delay, tdmi_ordered, false);
			cout << ">> Calculating swapped TDMI ... " << endl;
			TDMI(raster, lfp, dt, sampling_dt, negative_time_delay, positive_time_delay, tdmi_random, true);
		}
		//	Output data:
		ofstream data_out;
		cout << ">> Outputing data ... " << endl;
		data_out.open("./data/mi/mi_sl.csv");
		data_out << "timelag,ordered,random" << endl;
		for (int i = 0; i < negative_time_delay + positive_time_delay + 1; i++) {
			data_out << (double)dt*(i - negative_time_delay) << ',' << (double)tdmi_ordered[i] << ',' << (double)tdmi_random[i];
			if (i < positive_time_delay + negative_time_delay) data_out << endl;
		}
		data_out.close();
	} else if (mode == 2) {
		// INPUT NEURONAL DATA:
		vector<double> lfp_x, lfp_y;

		// DATA OF PRELAYER NEURON:
		string path;
		path = "./data/lfp/lfp_x.csv";
		Read1D(path, lfp_x, 0, 1);
		path = "./data/lfp/lfp_y.csv";
		Read1D(path, lfp_y, 0, 1);

		double sampling_dt = 0.03125;
		// take average;
		int np = floor(dt/sampling_dt);
		int nd = floor(lfp_x.size()/np);
		vector<double> lfp_x_ave(nd, 0), lfp_y_ave(nd, 0);
		for (int i = 0; i < nd; i++) {
			for (int j = 0; j < np; j++) {
				lfp_x_ave[i] += lfp_x[i*np + j]/np;
				lfp_y_ave[i] += lfp_y[i*np + j]/np;
			}
		}
		cout << ">> Calculating ordered TDMI ... " << endl;
		vector<double> tdmi;
		TDMI_adaptive(lfp_x_ave, lfp_y_ave, negative_time_delay, positive_time_delay, tdmi);
		//	Output data:
		ofstream data_out;
		cout << ">> Outputing data ... " << endl;
		data_out.open("./data/mi/mi_ll.csv");
		data_out << "timelag,mi" << endl;
		for (int i = 0; i < negative_time_delay + positive_time_delay + 1; i++) {
			data_out << (double)dt*(i - negative_time_delay) << ',' << (double)tdmi[i];
			if (i < positive_time_delay + negative_time_delay) data_out << endl;
		}
		data_out.close();
	} else if (mode == 3) {
		// INPUT NEURONAL DATA:
		vector<double> raster, potential;

		// DATA OF PRELAYER NEURON:
		string path;
		path = "./data/raster/raster.csv";
		Read1D(path, raster, 0, 1);
		path = "./data/potential/potential.csv";
		Read1D(path, potential, 0, 1);

		double sampling_dt = 0.03125;
		cout << ">> Calculating ordered TDMI ... " << endl;
		vector<double> tdmi_ordered;
		TDMI(raster, potential, dt, sampling_dt, negative_time_delay, positive_time_delay, tdmi_ordered, false);
		cout << ">> Calculating swapped TDMI ... " << endl;
		vector<double> tdmi_random;
		TDMI(raster, potential, dt, sampling_dt, negative_time_delay, positive_time_delay, tdmi_random, true);

		//	Output data:
		ofstream data_out;
		cout << ">> Outputing data ... " << endl;
		data_out.open("./data/mi/mi_sp.csv");
		data_out << "timelag,ordered,random" << endl;
		for (int i = 0; i < negative_time_delay + positive_time_delay + 1; i++) {
			data_out << (double)dt*(i - negative_time_delay) << ',' << (double)tdmi_ordered[i] << ',' << (double)tdmi_random[i];
			if (i < positive_time_delay + negative_time_delay) data_out << endl;
		}
		data_out.close();
	} else if (mode == 4) {
		// INPUT NEURONAL DATA:
		vector<double> potential_x, potential_y;

		// DATA OF PRELAYER NEURON:
		string path;
		path = "./data/potential/potential_x.csv";
		Read1D(path, potential_x, 0, 1);
		path = "./data/potential/potential_y.csv";
		Read1D(path, potential_y, 0, 1);

		double sampling_dt = 0.03125;
		// take average;
		int np = floor(dt/sampling_dt);
		int nd = floor(potential_x.size()/np);
		vector<double> potential_x_ave(nd, 0), potential_y_ave(nd, 0);
		for (int i = 0; i < nd; i++) {
			for (int j = 0; j < np; j++) {
				potential_x_ave[i] += potential_x[i*np + j]/np;
				potential_y_ave[i] += potential_y[i*np + j]/np;
			}
		}
		cout << ">> Calculating ordered TDMI ... " << endl;
		vector<double> tdmi;
		TDMI_adaptive(potential_x_ave, potential_y_ave, negative_time_delay, positive_time_delay, tdmi);
		//	Output data:
		ofstream data_out;
		cout << ">> Outputing data ... " << endl;
		data_out.open("./data/mi/mi_pp.csv");
		data_out << "timelag,mi" << endl;
		for (int i = 0; i < negative_time_delay + positive_time_delay + 1; i++) {
			data_out << (double)dt*(i - negative_time_delay) << ',' << (double)tdmi[i];
			if (i < positive_time_delay + negative_time_delay) data_out << endl;
		}
		data_out.close();
	}

	finish = clock();
	// Time counting:
	double ToTtime;
	ToTtime = (finish - start) / CLOCKS_PER_SEC;
	cout << "It takes " << (double)ToTtime << "s" << endl;
	return 0;
}
