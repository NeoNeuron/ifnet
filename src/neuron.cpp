//******************************
//	Copyright: Kyle Chen
//	Author: Kyle Chen
//	Description: Define class Neuron, structure Spike and NeuronState;
//	Date: 2018-05-30
//******************************
#include<iostream>
#include<cmath>
#include<ctime>
#include<algorithm>
#include "../include/neuron.h"
#include "../include/math_helper.h"
#include "../include/fmath.hpp"
#define exp(x) fmath::expd(x)
using namespace std;

bool compSpike(const Spike &x, const Spike &y) { return x.t < y.t; }

void NeuronSim::GenerateInternalPoisson(bool function, double tmax, bool outSet) {
	double temp, rate, strength;
	if (function) {
		temp = latest_excitatory_poisson_time_;
		rate = excitatory_poisson_rate_;
		strength = feedforward_excitatory_intensity_;
	} else {
		temp = latest_inhibitory_poisson_time_;
		rate = inhibitory_poisson_rate_;
		strength = feedforward_inhibitory_intensity_;
	}
	Spike ADD;
	ADD.t = temp;
	ADD.function = function;
	ADD.s = strength;
	double x, tLast;
	if (rate > 1e-18) {
		if (temp < 1e-18) {
			synaptic_driven_.push_back(ADD);
			if (outSet) cout << ADD.t << '\t';
		}
		tLast = temp;
		while (tLast < tmax) {
			x = (rand() + 1.0) / (RAND_MAX + 1.0);
			tLast -= log(x) / rate;
			ADD.t = tLast;
			synaptic_driven_.push_back(ADD);
			if (outSet) cout << ADD.t << '\t';
		}
		if (function) {
			latest_excitatory_poisson_time_ = tLast;
		} else {
			latest_inhibitory_poisson_time_ = tLast;
		}
	}
	sort(synaptic_driven_.begin(), synaptic_driven_.end(), compSpike);
}

void NeuronSim::InputExternalPoisson(double tmax, vector<Spike>& x) {
	if (!x.empty()) {
		vector<Spike>::iterator it = x.begin();
		while (it->t < tmax) {
			synaptic_driven_.push_back(*it);
			it = x.erase(it);
			if (x.empty()) break;
		}
	}
	sort(synaptic_driven_.begin(), synaptic_driven_.end(), compSpike);
}

void NeuronSim::SetFeedforwardStrength(bool function, double F) {
	if (function)	feedforward_excitatory_intensity_ = F;
	else feedforward_inhibitory_intensity_ = F;
}

void NeuronSim::SetPoissonRate(bool function, double rate) {
	if (function) {
		excitatory_poisson_rate_ = rate;
	} else {
		inhibitory_poisson_rate_ = rate;
	}
}

void NeuronSim::Reset(double *dym_val) {
	synaptic_driven_.clear();
	spike_train_.clear();
	driven_type_ = false;
	latest_excitatory_poisson_time_ = 0;
	latest_inhibitory_poisson_time_ = 0;
	excitatory_poisson_rate_ = 0;
	inhibitory_poisson_rate_ = 0;
	cycle_ = 0;
	// reset dynamic variables;
	p_neuron_ -> SetDefaultDymVal(dym_val);
}

void NeuronSim::OutSpikeTrain(vector<double> & spikes) {
	spikes.clear();
	spikes = spike_train_;
}

void NeuronSim::GetNewSpikes(double t, vector<Spike>& x) {
	Spike add_spike;
	add_spike.s = 0.0;
	add_spike.function = type_;
	x.clear();
	for (vector<double>::reverse_iterator iter = spike_train_.rbegin(); iter != spike_train_.rend(); iter++) {
		if (*iter >= t) {
			add_spike.t = *iter;
			x.push_back(add_spike);
		} else break;
	}
}

double NeuronSim::UpdateNeuronalState(double *dym_val, double t, double dt, vector<Spike>& extPoisson, vector<double>& new_spikes) {
	new_spikes.clear();
	double tmax = t + dt;
	if (driven_type_) {
		InputExternalPoisson(tmax, extPoisson);
	} else {
		GenerateInternalPoisson(true, tmax, false);
		//GenerateInternalPoisson(false, tmax, false);
	}
	double t_spike;
	vector<Spike>::iterator s_begin = synaptic_driven_.begin();
	if (s_begin == synaptic_driven_.end() || tmax <= s_begin->t) {
		t_spike = p_neuron_ -> DymCore(dym_val, dt);
		cycle_ ++;
		if (t_spike >= 0) new_spikes.push_back(t_spike);
	} else {
		if (t != s_begin->t) {
			t_spike = p_neuron_ -> DymCore(dym_val, s_begin->t - t);
			cycle_ ++;
			if (t_spike >= 0) new_spikes.push_back(t_spike);
		}
		for (vector<Spike>::iterator iter = s_begin; iter != synaptic_driven_.end(); iter++) {
			// Update conductance due to the synaptic inputs;
			if (iter -> s) dym_val[ p_neuron_ -> GetGEID() ] += iter -> s;
			else dym_val[ p_neuron_ -> GetGIID() ] += iter -> s;
			if (iter + 1 == synaptic_driven_.end() || (iter + 1)->t >= tmax) {
				t_spike = p_neuron_ -> DymCore(dym_val, tmax - iter->t);
				cycle_ ++;
				if (t_spike >= 0) new_spikes.push_back(t_spike);
				break;
			} else {
				t_spike = p_neuron_ -> DymCore(dym_val, (iter + 1)->t - iter->t);
				cycle_ ++;
				if (t_spike >= 0) new_spikes.push_back(t_spike);
			}
		}
	}
	return dym_val[p_neuron_ -> GetVID()];
}

double NeuronSim::CleanUsedInputs(double *dym_val, double *dym_val_new, double tmax) {
	// Update dym_val with dym_val_new;
	for (int i = 0; i < p_neuron_ -> GetDymNum(); i ++) dym_val[i] = dym_val_new[i];
	// clean old synaptic driven;
	int slen = synaptic_driven_.size();
	if (slen != 0) {
		int i = 0;
		for (; i < slen; i ++) {
			if (synaptic_driven_[i].t >= tmax) break;
		}
		synaptic_driven_.erase(synaptic_driven_.begin(), synaptic_driven_.begin() + i);
	}
	return dym_val[p_neuron_ -> GetVID()];
}

void NeuronSim::UpdateSource(double *dym_val, double t, double dt) {
	double tmax = t + dt;
	if (synaptic_driven_.empty() || tmax <= synaptic_driven_.begin()->t) {
		p_neuron_ -> UpdateSource(dym_val, dt);
	} else {
		if (t != synaptic_driven_.begin()->t) {
			p_neuron_ -> UpdateSource(dym_val, synaptic_driven_.begin()->t - t);
		}
		for (vector<Spike>::iterator iter = synaptic_driven_.begin(); iter != synaptic_driven_.end(); iter++) {
			if (iter -> s) dym_val[ p_neuron_ -> GetGEID() ] += iter -> s;
			else dym_val[ p_neuron_ -> GetGIID() ] += iter -> s;
			if (iter + 1 == synaptic_driven_.end() || (iter + 1)->t >= tmax) {
				p_neuron_ -> UpdateSource(dym_val, tmax - iter->t);
				break;
			} else {
				p_neuron_ -> UpdateSource(dym_val, (iter + 1)->t - iter->t);
			}
		}
	}
}

void NeuronSim::Fire(double t, double spike_time) {
		spike_train_.push_back(t + spike_time);
}

void NeuronSim::Fire(double t, vector<double>& spike_times) {
	for (vector<double>::iterator it = spike_times.begin(); it != spike_times.end(); it ++) {
		spike_train_.push_back(t + *it);
	}
}	

void NeuronSim::InSpike(Spike x) {
	if ((synaptic_driven_.back()).t < x.t) {
		synaptic_driven_.push_back(x);
	} else {
		synaptic_driven_.push_back(x);
		sort(synaptic_driven_.begin(),synaptic_driven_.end(),compSpike);
	}
}

void GenerateExternalPoissonSequence(double rate, double tmax, int seed, vector<double> & list) {
	srand(seed);
	list.push_back(0);
	double x, tLast = 0;
	while (tLast < tmax) {
		x = (rand() + 1.0) / (RAND_MAX + 1.0);
		tLast -= log(x) / rate;
		list.push_back(tLast);
	}
}
