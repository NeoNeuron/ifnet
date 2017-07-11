//******************************
//	Copyright: Kyle Chen
//	Author: Kyle Chen
//	Description: Define class Neuron, structure Spike and NeuronState;
//	Date: 2017-03-08 11:08:45
//******************************
#ifndef _MULTI_NETWORK_SIMULATION_SINGLEIF_H_
#define _MULTI_NETWORK_SIMULATION_SINGLEIF_H_

#include<string>
#include<vector>

using namespace std;

struct Spike {
	bool function; // function of spike: true for excitation, false for inhibition;
	double t; // Exact spiking time;
	bool mode; // mode of spike: true for feedforward spike, false for neuronal input spike;
};

struct NeuronalState {
	bool type; // neuronal type: true for excitatory, false for inhibitory;
	int index; // index of neuron in loop lattice;
	double membrane_potential_; // membrane potential;
	double ge; // excitatory conductance;
	double gi; // inhibitory conductance;
	double remaining_refractory_time;
};

// Class Neuron: Based on integrate and fire neuron model;
class Neuron {
private:
	// SETTINGS:

	bool type_; // neuronal type: true for excitatory, false for inhibitory;
	int index_; // index of neuron in loop lattice;
	double tau_e_; // Synaptic constant for excitatory synapses;
	double tau_i_; // Synaptic constant for inhibitory synapses;
	double g_m_; // Normalized membrane conductance;
	double tau_; // Refractory Period.
	double resting_potential_;
	double threshold_potential_;
	double excitatory_reversal_potential_;
	double inhibitory_reversal_potential_;
	double feedforward_excitatory_intensity_;
	double feedforward_inhibitory_intensity_;
	double pyramidal_synaptic_intensity_;
	double interneuronal_synaptic_intensity_;

	// DATA:

	vector<double> spike_train_; // Exact time nodes that neuron fires.
	bool driven_type_; // True for external Poisson driven, false for internal Poisson driven;
	double excitatory_poisson_rate_;
	double inhibitory_poisson_rate_;
	vector<Spike> synaptic_driven_;  // Synaptic input received by neuron, including feedforward and interneuronal spikes;
	double latest_excitatory_poisson_time_; // Used to store the last Poisson time point generated by Poisson generator for excitatory spikes;
	double latest_inhibitory_poisson_time_; // Used to store the last Poisson time point generated by Poisson generator for inhibitory spikes;

	double excitatory_conductance_; // Stored excitatory conductivity;
	double excitatory_conductance_1_; // temporal storage of excitatory conductance;
	double excitatory_conductance_2_;
	double excitatory_conductance_3_;

	double inhibitory_conductance_; // Stored inhibitory conductivity;
	double inhibitory_conductance_1_; // temporal storage of inhibitory conductance;
	double inhibitory_conductance_2_;
	double inhibitory_conductance_3_;

	double membrane_potential_;
	double membrane_potential_temp_;

	double remaining_refractory_period_; // if negative, remaining refractory period equals to zero;
	double remaining_refractory_period_temp_; // if negative, temporal remaining refractory period equals to zero;

	// FUNCTIONS:

	// Generate Poisson sequence within each time step, autosort after generatation;
	void GenerateInternalPoisson(bool function, double tmax, bool outSet);

	// Input external Poisson sequence within each time step, autosort after generatation;
	void InputExternalPoisson(bool function, double tmax, vector<double> & x);

	//	Update excitatory conductance:
	//		Mode description: true for feedforward input, false for interneuronal input;
	//		function description: functional type of inputing signal, true for excitation, false for inhibition;
	//	Return: none;
	void UpdateExcitatoryConductance(bool mode, bool function, double t, double dt);

	//	Update inhibitory conductance:
	//		Mode description: true for feedforward input, false for interneuron input;
	//		function description: functional type of inputing signal, true for excitation, false for inhibition;
	//	Return: none;
	void UpdateInhibitoryConductance(bool mode, bool function, double t, double dt);

	//	Alpha function, defined as summation of membrane conductance, excitatory and inhibitory conductance;
	//		INT option: default value = 0, 1 for t = t(n), 2 for t = t(n) + dt/2, 3 for t(n) + dt;
	//	Return: value of Alpha function;
	double Alpha(int option);

	//	Beta function, defined as summation of production between conductances and their reversal potentials;
	//		INT option: default value = 0, 1 for t = t(n), 2 for t = t(n) + dt/2, 3 for t(n) + dt;
	//	Return: value of Beta function;
	double Beta(int option);

	//	Update the membrane potential after time-step dt;
	//		Description: 4th-order Runge Kutta integration scheme is applied;
	//		Double dt: size of time step, unit millisecond;
	//	Return: membrane potential at t = t(n) + dt;
	double UpdatePotential(double dt);

	//	Check whether the membrane potential reaches the threshold or not;
	//		Description: if membrane potential didn't reach threshold, accept prediction; else estimate the exact time that membrane potential reach the threshold by cubic interpolation, and record as spiking events;
	//		DOUBLE voltage: the membrane potential at t = t(n) + dt predicted by UpdatePotential(dt);
	//		DOUBLE t: current time point;
	//		DOUBLE dt: size of time step, unit millisecond;
	//	Return: none;
	void CheckFire(double voltage, double t, double dt);

	//	Temporarily check whether the membrane potential reaches the threshold or not, and results would be saved as temp_ version;
	//		Description: if membrane potential didn't reach threshold, accept prediction; else estimate the exact time that membrane potential reach the threshold by cubic interpolation;
	//		DOUBLE voltage: the membrane potential at t = t(n) + dt predicted by UpdatePotential(dt);
	//		DOUBLE t: current time point;
	//		DOUBLE dt: size of time step, unit millisecond;
	//	Return: if not reach return -1, else return spiking time point;
	double TempCheckFire(double voltage, double t, double dt);

	//	Off refractory period operation;
	//		Description: calculate the first non-zero membrane potential after refractory period;
	//		DOUBLE dt: size of time step, unit millisecond;
	//	Return: membrane potential at t = t(n) + dt;
	double OffRefractoryPeriod(double dt);

	//	Prime operation for updating neuronal state within single time step dt;
	//		Description: operation to update neuronal state in primary level, including updating conductances, membrane potential and checking spiking events;
  //                1 synaptic input most which arrives at the begining of time step;
  //    BOOL is_fire: true for the presence of a synaptic input at the begining of time step; false for not;
  //    BOOL mode: mode for the synaptic input;
  //    BOOL function: function for synaptic input;
  //    DOUBLE time: the begining of the time step;
  //    DOUBLE dt: size of time step, unit millisecond;
	//    BOOL temp_switch: true for temporal operation whose results would be saved in temporal memory; false for non-temporal operation;
  //	Return:
  //    for non-temp operation, return -1;
  //    for temp operation, if fires, return spiking time, else return -1;
	double PrimelyUpdateState(bool is_fire, bool mode, bool function, double t, double dt, bool temp_switch);

  //	Update conductance of fired neuron within single time step dt;
  //		Description: operation to update neuronal state in primary level, including updating conductances, membrane potential and checking spiking events;
  //                1 synaptic input most which arrives at the begining of time step;
  //    BOOL is_fire: true for the presence of a synaptic input at the begining of time step; false for not;
  //    BOOL mode: mode for the synaptic input;
  //    BOOL function: function for synaptic input;
  //    DOUBLE time: the begining of the time step;
  //    DOUBLE dt: size of time step, unit millisecond;
  //	Return: none;
	void UpdateConductanceOfFiredNeuron(bool is_fire, bool fireMode, bool fireType, double t, double dt);

public:
	// Auto initialization of parameters in Neuron;
	Neuron() {
		type_ = true;
		index_ = -1;
		tau_e_ = 2.0;
		tau_i_ = 7.0;
		g_m_ = 5e-2;
		tau_ = 2.0;
		resting_potential_ = 0;
		threshold_potential_ = 1;
		excitatory_reversal_potential_ = 14 / 3;
		inhibitory_reversal_potential_ = -2 / 3;
		feedforward_excitatory_intensity_ = 5e-3;
		feedforward_inhibitory_intensity_ = 5e-3;
		pyramidal_synaptic_intensity_ = 5e-3;
		interneuronal_synaptic_intensity_ = 5e-3;
		driven_type_ = false;
		excitatory_poisson_rate_ = 1e-20;
		inhibitory_poisson_rate_ = 1e-20;
		latest_excitatory_poisson_time_ = 0;
		latest_inhibitory_poisson_time_ = 0;
		excitatory_conductance_ = 0;
		excitatory_conductance_1_ = 0;
		excitatory_conductance_2_ = 0;
		excitatory_conductance_3_ = 0;
		inhibitory_conductance_ = 0;
		inhibitory_conductance_1_ = 0;
		inhibitory_conductance_2_ = 0;
		inhibitory_conductance_3_ = 0;
		membrane_potential_temp_ = 0;
		membrane_potential_ = 0;
		remaining_refractory_period_temp_ = -1;
		remaining_refractory_period_ = -1;
	}

	// INPUTS:

	// Set neuronal type: true for excitatory; false for inhibitory;
	void SetNeuronType(bool x);

	//	Set neuronal index: indices of neurons in 1-D loop lattice, starting from 0 to maximum number - 1;
	void SetNeuronIndex(int x);

	//	Set driving type: true for external, false for internal;
	void SetDrivingType(bool x);

	//	Set Poisson Rate: homogeneous Poisson driving rate of internal driving type;
	//	BOOL function: type of Poisson drive, true for excitatory, false for inhibitory;
	void SetPoissonRate(bool function, double rate);

	// Define a 'neuron_file' type to store neuronal condition;
	// A ROW VECTOR:
	//	0: neuronal type;
	//	1: neuronal index;
	//	2: membrane potential;
	//	3: excitatory conductivity;
	//	4: inhibitory conductivity;
	//	5: remaining refractory period;
	void LoadNeuronalState(NeuronalState & data);

	//	Input synaptic inputs, either feedforward or interneuronal ones;
	void InSpike(Spike x);

	// Reset neuron into the condition at zero time point;
	void Reset();

	// DYNAMICS:

	// 	Update neuronal state:
	//	Description: update neuron within single time step, including its membrane potential, conductances and counter of refractory period;
	//	DOUBLE t: time point of the begining of the time step;
	//	DOUBLE dt: size of time step;
	//	VECTOR<DOUBLE> inPE: external excitatory Poisson sequence;
	//	VECTOR<DOUBLE> inPI: external inhibitory Poisson sequence;
	//	Return: membrane potential at t = t + dt;
	double UpdateNeuronalState(double t, double dt, vector<double> & inPE, vector<double> & inPI);

	//	Temporally update neuronal state;
	//	Description: update neuron state to check whether it would fire, while don't change its stored parameters, including membrane potential, conductances and counter of refractory period;
	//	DOUBLE t: time point of the begining of the time step;
	//	DOUBLE dt: size of time step;
	//	VECTOR<DOUBLE> inPE: external excitatory Poisson sequence;
	//	VECTOR<DOUBLE> inPI: external inhibitory Poisson sequence;
	//	Return: if fire, return the spiking time;
	//					if not, return -1;
	double TemporallyUpdateNeuronalState(double t, double dt, vector<double> & inPE, vector<double> & inPI);

	//	Fire: update neuronal state for neurons which fire at t = t + dt;
	void Fire(double t, double dt);

	// OUTPUTS:

	//	Get last spike: return the time point of latest spiking events;
	double GetLastSpike();

	//	Get potential: return the current value of membrane potential;
	double GetPotential();

	//	Get neuronal type: true for excitatory, false for inhibitory;
	bool GetNeuronalType();

	int GetNeuronIndex();

	//	Output spike train
	void OutSpikeTrain(vector<double> & spikes);

  //  Output Spikes before t;
	void GetNewSpikes(double t, vector<Spike> &x);

	void SetFeedforwardConductance(bool function, double F);

	// Total membrane current;
	double OutTotalCurrent();

	// Leaky current;
	double OutLeakyCurrent();

	// Excitatory or inhibitory membrane current;
	double OutSynapticCurrent(bool type);

	// True return excitatory conductance, false return inhibitory conductance;
	double GetConductance(bool x);

	//Save corrent neuronal States:
	//Define a 'neuronFile' type to store neuronal condition;
	//A ROW VECTOR:
	//	0: neuronal type;
	//	1: neuronal index;
	//	2: membrane potential;
	//	3: excitatory conductivity;
	//	4: inhibitory conductivity;
	//	5: remaining refractory period;
	void Save(NeuronalState & vals);
};

//	external Poisson generator:
//	DOUBLE rate: mean Poisson firing rate;
//	DOUBLE tmax: maximum timel'
//	int seed: random seed;
//	vecotr<double> list: memory storage for Poisson squence;
void GenerateExternalPoissonSequence(double rate, double tmax, int seed, vector<double> & list);

#endif 	// _MULTI_NETWORK_SIMULATION_SINGLEIF_H_