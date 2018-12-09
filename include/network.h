//***************
//	Copyright: Kyle Chen
//	Author: Kyle Chen
//	Date: 2018-05-31
//	Description: define Struct SpikeElement and Class NeuronalNetwork;
//***************
#ifndef _IFNET_NETWORK_H_
#define _IFNET_NETWORK_H_

#include "../include/io.h"
#include "../include/neuron.h"
#include "../include/get-config.h"
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

struct SpikeElement {
	int index;	// The sequence order of spikes within single time interval;
	double t;	// exact spiking time;
	bool type;	// The type of neuron that fired;
};

bool compSpikeElement(const SpikeElement &x, const SpikeElement &y);

class NeuronalNetwork {
private:
	// Neuron Simulators:
	vector<NeuronSim> neurons_;

	// Network Parameters:
	int neuron_number_;	// number of the neurons in the group;
	vector<double*> dym_vals_; // dynamic variables of neurons;
	vector<double*> dym_vals_new_; // temporal dynamic variables of neurons;
	vector<bool> types_; // vector to store types of neurons;

	// Network Structure:
	vector<vector<bool> > con_mat_; // built-in matrix for neuronal connectivity;
	bool is_con_;
	vector<vector<double> > s_mat_; // matrix of inter-neuronal interacting strength;
	vector<vector<double> > delay_mat_;

	// Network Inputs:
	vector<vector<Spike> > ext_inputs_; // temp storage of external Poisson input;
	vector<vector<Spike>::iterator > ext_iters_; // iterator for current used external Poisson Drives;


	// Functions:
	//
  // Set interaction delay between neurons;
	void SetDelay(vector<vector<double> > &coordinates, double speed);

	// Sort spikes within single time interval, and return the time of first spike;
	double SortSpikes(vector<int>& update_list, vector<int>& fired_list, double t, double dt, vector<SpikeElement> &T);

public:
	//	Neuronal network initialization:
	NeuronalNetwork(string neuron_type, int neuron_number) {
		// Network Parameters:
		neuron_number_ = neuron_number;
		dym_vals_.resize(neuron_number_, NULL);
		dym_vals_new_.resize(neuron_number_, NULL);
		for (int i = 0; i < neuron_number_; i++) {
			neurons_.push_back(NeuronSim(neuron_type, dym_vals_[i]));
			//TODO: allocate memory for dynamic variable for different neuronal model;
			//dym_vals_new[i] = new double[GetLen(dym_vals_[i])];
			dym_vals_new_[i] = new double[4];
		}
		types_.resize(neuron_number_, false);
		// Network structure:
		con_mat_.resize(neuron_number_, vector<bool>(neuron_number_, false));
		is_con_ = false;
		s_mat_.resize(neuron_number_, vector<double>(neuron_number_, 0.0));
		delay_mat_.resize(neuron_number_, vector<double>(neuron_number_, 0.0));
	}
	
	~NeuronalNetwork() {
		for (int i = 0; i < neuron_number_; i++) {
			delete dym_vals_[i];
			delete dym_vals_new_[i];
		}
	}
	// Initialize network connectivity matrix:
	// Three options:
	// 0. given pre-defined network connectivity matrix;
	// 1. small-world network, defined by connectivity density and rewiring;
	// 2. randomly connected network;
	void InitializeConnectivity(map<string, string> &m_config);
	
	// INPUTS:
	// Set interneuronal coupling strength;
	void InitializeSynapticStrength(map<string, string> &m_config);
	// Initialize the delay of synaptic interaction;
	void InitializeSynapticDelay(map<string, string> &m_config);
	
	// Set time period of refractory:
	void SetRef(double t_ref);

	// 	Initialize neuronal types in the network;
	//	p: the probability of the presence of excitatory neuron;
	//	seed: seed for random number generator;
	void InitializeNeuronalType(double p, int seed);

	//	Set driving type: true for external Poisson driven, false for internal ones;
	void SetDrivingType(bool driving_type);

	//	Initialize internal homogeneous feedforward Poisson rate;
	//	driving_setting: 
	//		[:,0] excitatory Poisson rate;
	//		[:,1] inhibitory Poisson rate;
	//		[:,2] excitatory Poisson strength;
	//		[:,3] inhibitory Poisson strength;
	void InitializeInternalPoissonRate(vector<vector<double> >& driving_setting);

	//	Initialize external homogeneous feedforward Poisson process;
	//	driving_setting: 
	//		[:,0] excitatory Poisson rate;
	//		[:,1] inhibitory Poisson rate;
	//		[:,2] excitatory Poisson strength;
	//		[:,3] inhibitory Poisson strength;
	//	tmax: maximum time range for Poisson process;
	//	seed: seed for built-in random generator;
	void InitializeExternalPoissonProcess(vector<vector<double> >& driving_setting, double tmax, int seed);

	// 	Input new spikes for neurons all together;
	void InNewSpikes(vector<vector<Spike> > &data);

	// DYNAMICS:

	//	Restore neuronal state for all neurons, including neuronal potential, conductances, refractory periods and external network drive;
	void RestoreNeurons();

	//	Update network state:
	void UpdateNetworkState(double t, double dt);

	// OUTPUTS:

	// Print cycle:
	void PrintCycle();

	//	Output potential to *.csv file;
	void OutPotential(FILEWRITE& file);

  //	Output synaptic conductance to *.csv files:
  //		BOOL function: function of synaptic conductance, true for excitation, false for inhibition;
  void OutConductance(FILEWRITE& file, bool function);

	//	Output current to *.csv file;
	void OutCurrent(FILEWRITE& file);

	// Save neuronal type vector;
	void SaveNeuronType(string neuron_type_file);

	// Save connectivity matrix
	void SaveConMat(string connecting_matrix_file);

	// Output spike trains of the network to spike_trains;
	// return: the total number of spikes in the network during the simulation;
	int OutSpikeTrains(vector<vector<double> >& spike_trains);

  //  Output spikes before t, including their functions;
	void GetNewSpikes(double t, vector<vector<Spike> >& data);

	int GetNeuronNumber();

	void GetConductance(int i, bool function);
};

#endif // _IFNET_NETWORK_H_
