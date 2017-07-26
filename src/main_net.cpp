//*************************
//	Copyright: Kyle Chen
//	Author: Kyle Chen
//	Date: 2017-03-13 15:07:52
//	Description: test program for multi-network simulation;
//*************************

#include "../include/group.h"
#include "../include/get-config.h"
#include "../include/io.h"
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <sstream>
#include <stdexcept>
using namespace std;

//	Simulation program for single network system;
//	arguments:
//	argv[1] = Outputing directory for neural data;
int main(int argc, const char* argv[]) {
	if (argc != 2) {
		throw runtime_error("wrong number of args");
	}
	clock_t start, finish;
	start = clock();
	// 	Setup directory for output files;
	//	it must be existing dir;
	string dir;
	dir = argv[1];

	// Loading config.ini:
	string net_config_path = "./doc/config_net.ini";
  map<string, string> m_map_config;
  ReadConfig(net_config_path,m_map_config);
  cout << ">> [Config.ini]:" << endl;
	PrintConfig(m_map_config);
	cout << endl;
	// load neuron number;
	int neuron_number = atoi(m_map_config["NeuronNumber"].c_str());
	NeuronalNetwork net(neuron_number);
	// load connecting mode;
	int connecting_mode = atoi(m_map_config["ConnectingMode"].c_str());
	if (connecting_mode == 0) { // External connectivity matrix;
		net.LoadConnectivityMat("./data/sampleMat.csv");
	} else {
		int connecting_density = atoi(m_map_config["ConnectingDensity"].c_str());
		net.SetConnectingDensity(connecting_density);
		int rewiring_probability = atof(m_map_config["RewiringProbability"].c_str());
		int rewiring_seed = atoi(m_map_config["RewiringSeed"].c_str());
		// Generate networks;
		net.Rewire(rewiring_probability, rewiring_seed, true);
	}
	// initialize the network;
	net.InitializeNeuronalType(atof(m_map_config["TypeProbability"].c_str()), atoi(m_map_config["TypeSeed"].c_str()));
	cout << "in the network." << endl;

	double maximum_time = atof(m_map_config["MaximumTime"].c_str());
	bool type;
	istringstream(m_map_config["DrivingType"]) >> boolalpha >> type;
	net.SetDrivingType(type);
	net.InitializeExternalPoissonProcess(true, atof(m_map_config["DrivingRateExcitatory"].c_str()), atof(m_map_config["DrivingRateInhibitory"].c_str()), maximum_time, atoi(m_map_config["ExternalDrivingSeed"].c_str()));
	net.InitializeExternalPoissonProcess(false, 0, 0, maximum_time, 0);

	// SETUP DYNAMICS:
	double t = 0, dt = atof(m_map_config["TimingStep"].c_str()), tmax = maximum_time;
	// Define file path for output data;
	string V_path = dir + "V.csv";
	string I_path = dir + "I.csv";
	// Initialize files:
	ofstream V, I;
	V.open(V_path.c_str());
	V.close();

	I.open(I_path.c_str());
	I.close();

	char cr = (char)13;
	double progress;
	while (t < tmax) {
		net.UpdateNetworkState(t, dt);
		t += dt;
		// Output temporal data;
		net.OutPotential(V_path);
		net.OutCurrent(I_path);

		progress = t * 100.0 / tmax;
		cout << cr;
		printf(">> Processing ... %6.2f", progress);
		cout << "%";
	}
	cout << endl;

	string neuron_path, mat_path;
	neuron_path = dir + "neuron.csv";
	mat_path = dir + "mat.csv";
	net.Save(neuron_path, mat_path);

	// OUTPUTS:
	string raster_path = dir + "raster.csv";
	net.OutSpikeTrains(raster_path);

	finish = clock();

	// COUNTS:
	double ToTtime;
	ToTtime = (finish - start) / CLOCKS_PER_SEC;
	printf(">> It takes %.2fs\n", ToTtime);
	return 0;
}