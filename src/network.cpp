//******************************
//	Copyright: Kyle Chen
//	Author: Kyle Chen
//	Description: Define class Neuron, structure Spike and NeuronState;
//	Date: 2017-02-21 16:06:30
//******************************
#include "../include/network.h"

using namespace std;

void Scan(vector<bool> & mat, int target_value, vector<int> &output_indices) {
	output_indices.clear();
	for (int s = 0; s < mat.size(); s++) {
		if (mat[s] == target_value) output_indices.push_back(s);
	}
}

bool compSpikeElement(const SpikeElement &x, const SpikeElement &y) { return x.t < y.t; }

inline double L2(vector<double> &x, vector<double> &y) {	
	return sqrt((x[0] - y[0])*(x[0] - y[0]) + (x[1] - y[1])*(x[1] - y[1]));
}

void NeuronalNetwork::InitializeConnectivity(map<string, string> &m_config) {
	int connecting_mode = atoi(m_config["ConnectingMode"].c_str());
	if (connecting_mode == 0) { // External connectivity matrix;
		vector<vector<int> > connecting_matrix;
		Read2D(m_config["MatPath"], connecting_matrix);
		if (connecting_matrix.size() != neuron_number_ || connecting_matrix[0].size() != neuron_number_) {
			throw runtime_error("wrong size of connectivity matrix");
		} else {
			for (size_t i = 0; i < neuron_number_; i ++) {
				for (size_t j = 0; j < neuron_number_; j ++) {
					if (connecting_matrix[i][j]) {
						con_mat_[i][j] = true;
						if (!is_con_) is_con_ = true;
					}
				}
			}
		}
	} else if (connecting_mode == 1) {
		int con_density = atoi(m_config["ConnectingDensity"].c_str());
		for (int i = 0; i < neuron_number_; i++)  {
			for (int j = 0; j < neuron_number_; j++) {
				if (i != j) {
					if (abs(i - j) <= con_density or neuron_number_ - abs(i - j) <= con_density) {
					con_mat_[i][j] = true;
					if (!is_con_) is_con_ = true;
					}
				}
			}
		}
		double rewiring_probability = atof(m_config["RewiringProbability"].c_str());
		bool output_option;
		istringstream(m_config["PrintRewireResult"]) >> boolalpha >> output_option;
		// Generate networks;
		cout << 2 * neuron_number_ * con_density << " connections total with ";
		double x; // random variable;
		int ind, empty_connection, count = 0;
		vector<int> ones, zeros;
		for (int i = 0; i < neuron_number_; i++) {
			Scan(con_mat_[i], 1, ones);
			for (int j = 0; j < ones.size(); j++) {
				x = rand_distribution(rand_gen);
				if (x <= rewiring_probability) {
					Scan(con_mat_[i], 0, zeros);
					for (vector<int>::iterator it = zeros.begin(); it != zeros.end(); it++) {
						if (*it == i) {
							zeros.erase(it);
							break;
						}
					}
					empty_connection = zeros.size();
					ind = rand_gen() % empty_connection;
					con_mat_[i][zeros[ind]] = true;
					con_mat_[i][ones[j]] = false;
					count += 1;
				}
			}
		}
		cout << count << " rewirings." << endl;
	} else if (connecting_mode == 2) {
		double con_prob_ee = atof(m_config["ConnectingProbabilityEE"].c_str());
		double con_prob_ei = atof(m_config["ConnectingProbabilityEI"].c_str());
		double con_prob_ie = atof(m_config["ConnectingProbabilityIE"].c_str());
		double con_prob_ii = atof(m_config["ConnectingProbabilityII"].c_str());
		double x;
		for (size_t i = 0; i < neuron_number_; i ++) {
			for (size_t j = 0; j < neuron_number_; j ++) {
				x = rand_distribution(rand_gen);
				if (types_[i]) {
					if (types_[j]) {
						if (x <= con_prob_ee) con_mat_[i][j] = true;
					} else {
						if (x <= con_prob_ei) con_mat_[i][j] = true;
					}
				} else {
					if (types_[j]) {
						if (x <= con_prob_ie) con_mat_[i][j] = true;
					} else {
						if (x <= con_prob_ii) con_mat_[i][j] = true;
					}
				}
				if (con_mat_[i][j]) {
					if (!is_con_) is_con_ = true;
				}
			}
		}
	}
}

void NeuronalNetwork::InitializeSynapticStrength(map<string, string> &m_config) {
	int synaptic_mode = atoi(m_config["SynapticMode"].c_str());
	if (synaptic_mode == 0) {
		vector<vector<double> > s_vals;
		Read2D(m_config["SPath"], s_vals);
		for (size_t i = 0; i < neuron_number_; i ++) {
			for (size_t j = 0; j < neuron_number_; j ++) {
				s_mat_[i][j] = s_vals[i][j];
			}
		}
	} else if (synaptic_mode == 1) {
		double s_ee = atof(m_config["SynapticStrengthEE"].c_str());
		double s_ei = atof(m_config["SynapticStrengthEI"].c_str());
		double s_ie = atof(m_config["SynapticStrengthIE"].c_str());
		double s_ii = atof(m_config["SynapticStrengthII"].c_str());
		for (size_t i = 0; i < neuron_number_; i ++) {
			for (size_t j = 0; j < neuron_number_; j ++) {
				if (types_[i]) {
					if (types_[j]) s_mat_[i][j] = s_ee;
					else s_mat_[i][j] = s_ei;
				} else {
					if (types_[j]) s_mat_[i][j] = s_ie;
					else s_mat_[i][j] = s_ii;
				}
			}
		}
	} else if (synaptic_mode == 2) {
		double s_ee = atof(m_config["SynapticStrengthEE"].c_str());
		double s_ei = atof(m_config["SynapticStrengthEI"].c_str());
		double s_ie = atof(m_config["SynapticStrengthIE"].c_str());
		double s_ii = atof(m_config["SynapticStrengthII"].c_str());
		for (size_t i = 0; i < neuron_number_; i ++) {
			for (size_t j = 0; j < neuron_number_; j ++) {
				if (types_[i]) {
					if (types_[j]) s_mat_[i][j] = s_ee;
					else s_mat_[i][j] = s_ei;
				} else {
					if (types_[j]) s_mat_[i][j] = s_ie;
					else s_mat_[i][j] = s_ii;
				}
			}
		}
		vector<vector<double> > coordinates;
		Read2D(m_config["CoorPath"], coordinates);
		double meta_dis;
		for (int i = 0; i < neuron_number_; i ++) {
			for (int j = 0; j < i; j ++) {
				meta_dis = 0.02 / pow(L2(coordinates[i], coordinates[j]), 2);
				delay_mat_[i][j] *= meta_dis;
				delay_mat_[j][i] *= meta_dis;
			}
		}
	}
}

void NeuronalNetwork::InitializeSynapticDelay(map<string, string> &m_config) {
	if (atoi(m_config["DelayMode"].c_str()) == 0) {
		delay_mat_.clear();
		delay_mat_.resize(neuron_number_, vector<double>(neuron_number_, atof(m_config["HomoSynapticDelay"].c_str())));
	} else if (atoi(m_config["DelayMode"].c_str()) == 1) {
		vector<vector<double> > coordinates;
		Read2D(m_config["CoorPath"], coordinates);
		SetDelay(coordinates, atof(m_config["TransmitSpeed"].c_str()));
	}
}

bool CheckExist(int index, vector<int> &list) {
	for (int i = 0; i < list.size(); i ++) {
		if (list[i] == index) return true;
	}
	return false;
}

void NeuronalNetwork::SetDelay(vector<vector<double> > &coordinates, double speed) {
	double meta_dis;
	for (int i = 0; i < neuron_number_; i ++) {
		for (int j = 0; j < i; j ++) {
			meta_dis = L2(coordinates[i], coordinates[j]) / speed;
			delay_mat_[i][j] = meta_dis;
			delay_mat_[j][i] = meta_dis;
		}
	}
}

double NeuronalNetwork::SortSpikes(vector<int> &update_list, vector<int> &fired_list, double t, double dt, vector<SpikeElement> &T) {
	vector<double> tmp_spikes;
	SpikeElement ADD;	
	// start scanning;
	double id;
	for (int i = 0; i < update_list.size(); i++) {
		id = update_list[i];
		// Check whether id's neuron is in the fired list;
		if (CheckExist(id, fired_list)) {
			for (int j = 1; j < 3; j ++) dym_vals_new_[id][j] = dym_vals_[id][j];
			neurons_[id].UpdateSource(dym_vals_new_[id], t, dt);
		} else {
			for (int j = 0; j < 4; j ++) dym_vals_new_[id][j] = dym_vals_[id][j];
			neurons_[id].UpdateNeuronalState(dym_vals_new_[id], t, dt, ext_inputs_[id], tmp_spikes);
			if (tmp_spikes.size() > 0) {
				ADD.index = id;
				ADD.t = tmp_spikes.front();
				ADD.type = types_[id];
				T.push_back(ADD);
			}
		}
	}
	if (T.empty()) {
		return -1;
	} else if (T.size() == 1) {
		return (T.front()).t;
	} else {
		sort(T.begin(), T.end(), compSpikeElement);
		return (T.front()).t;
	}
}

void NeuronalNetwork::SetRef(double t_ref) {
	for (int i = 0; i < neuron_number_; i ++) { neurons_[i].SetRef(t_ref); }
}

void NeuronalNetwork::InitializeNeuronalType(map<string, string> &m_config) {
	int counter = 0;
	double p = atof(m_config["TypeProbability"].c_str());
	if (atoi(m_config["TypeMode"].c_str()) == 0) {
		counter = floor(neuron_number_*p);
		for (int i = 0; i < counter; i++) types_[i] = true;
	} else if (atoi(m_config["TypeMode"].c_str()) == 1) {
		double x = 0;
		for (int i = 0; i < neuron_number_; i++) {
			x = rand_distribution(rand_gen);
			if (x < p) {
				types_[i] = true;
				counter++;
			}
		}
	} else if (atoi(m_config["TypeMode"].c_str()) == 2) {
		vector<int> type_seq;
		Read1D(m_config["TypePath"], type_seq, 0, 0);
		for (int i = 0; i < neuron_number_; i ++) {
			if ( type_seq[i] ) {
				types_[i] = true;
				counter++;
			}
		}
	}
	printf(">> %d excitatory and %d inhibitory neurons in the network.\n", counter, neuron_number_-counter);
}

void NeuronalNetwork::InitializePoissonGenerator(map<string, string>& m_config) {
	vector<vector<double> > poisson_settings;
	//	poisson_setting: 
	//		[:,0] excitatory Poisson rate;
	//		[:,1] excitatory Poisson strength;
	int driving_mode = atoi(m_config["DrivingMode"].c_str());
	if (driving_mode == 0) {
		double pr = atof(m_config["pr"].c_str());
		double ps = atof(m_config["ps"].c_str());
		poisson_settings.resize(neuron_number_, vector<double>{pr, ps});
	} else if (driving_mode == 1){
		// import the data file of feedforward driving rate:
		Read2D(m_config["PoissonPath"], poisson_settings);
		if (poisson_settings.size() != neuron_number_) {
			cout << "Error inputing length! (Not equal to the number of neurons in the net)";
			return;
		}
	} else throw runtime_error("wrong driving_mode");

	bool poisson_output;
	istringstream(m_config["PoissonOutput"]) >> boolalpha >> poisson_output;
	for (int i = 0; i < neuron_number_; i++) {
		pgs_[i].SetRate(poisson_settings[i][0]);
		pgs_[i].SetStrength(poisson_settings[i][1]);
		if (poisson_output) {
			pgs_[i].SetOuput( m_config["PoissonDir"] + "pg" + to_string(i) + ".csv" );
		}
	}
	istringstream(m_config["PoissonGeneratorMode"]) >> boolalpha >> pg_mode;

	if ( pg_mode ) {
		for (int i = 0; i < neuron_number_; i ++) {
			pgs_[i].GenerateNewPoisson( atof(m_config["MaximumTime"].c_str()), ext_inputs_[i] );
		}
	}
}

// Used in two layer network system;
// TODO: the number of sorting can be reduced;
void NeuronalNetwork::InNewSpikes(vector<vector<Spike> > & data) {
	for (int i = 0; i < neuron_number_; i++) {
		if (!data[i].empty()) {
			for (vector<Spike>::iterator it = data[i].begin(); it != data[i].end(); it++) {
				neurons_[i].InSpike(*it);
			}
		}
	}
}

void NeuronalNetwork::UpdateNetworkState(double t, double dt) {
	if ( !pg_mode ) {
		for (int i = 0; i < neuron_number_; i ++) {
			pgs_[i].GenerateNewPoisson(t + dt, ext_inputs_[i] );
		}
	}

	if (is_con_) {
		vector<SpikeElement> T;
		double newt;
		// Creating updating pool;
		vector<int> update_list, fired_list;
		for (int i = 0; i < neuron_number_; i++) update_list.push_back(i);
		newt = SortSpikes(update_list, fired_list, t, dt, T);
		while (newt > 0) {
			update_list.clear();
			int IND = (T.front()).index;
			fired_list.push_back(IND);
			Spike ADD_mutual;
			ADD_mutual.type = (T.front()).type;
			// erase used spiking events;
			T.erase(T.begin());
			for (int j = 0; j < neuron_number_; j++) {
				if (j == IND) {
					neurons_[j].Fire(t, newt);
				} else {
					if (con_mat_[IND][j]) {
						ADD_mutual.s = s_mat_[IND][j];
						ADD_mutual.t = t + newt + delay_mat_[IND][j];
						neurons_[j].InSpike(ADD_mutual);
						update_list.push_back(j);
						// Check whether this neuron appears in the firing list T;
						for (int k = 0; k < T.size(); k ++) {
							if (j == T[k].index) {
								T.erase(T.begin() + k);
								break;
							}
						}
					}
				}
			}
			newt = SortSpikes(update_list, fired_list, t, dt, T);
		}
		for (int i = 0; i < neuron_number_; i++) {
			neurons_[i].CleanUsedInputs(dym_vals_[i], dym_vals_new_[i], t + dt);
		}
	} else {
		vector<double> new_spikes;
		for (int i = 0; i < neuron_number_; i++) {
			neurons_[i].UpdateNeuronalState(dym_vals_[i], t, dt, ext_inputs_[i], new_spikes);
			if ( new_spikes.size() > 0 ) neurons_[i].Fire(t, new_spikes);
			neurons_[i].CleanUsedInputs(dym_vals_[i], dym_vals_[i], t + dt);
		}
	}
}

void NeuronalNetwork::PrintCycle() {
	for (int i = 0; i < neuron_number_; i++) {
		neurons_[i].GetCycle();
		cout << '\t';
	}
	cout << endl;
}
void NeuronalNetwork::OutPotential(FILEWRITE& file) {
	vector<double> potential(neuron_number_);
	for (int i = 0; i < neuron_number_; i++) {
		potential[i] = neurons_[i].GetPotential(dym_vals_[i]);
	}
	file.Write(potential);
}

void NeuronalNetwork::OutConductance(FILEWRITE& file, bool type) {
	vector<double> conductance(neuron_number_);
	for (int i = 0; i < neuron_number_; i++) {
		conductance[i] = neurons_[i].GetConductance(dym_vals_[i], type);
	}
	file.Write(conductance);
}

void NeuronalNetwork::OutCurrent(FILEWRITE& file) {
	vector<double> current(neuron_number_);
	for (int i = 0; i < neuron_number_; i++) {
		current[i] = neurons_[i].GetCurrent(dym_vals_[i]);
	}
	file.Write(current);
}

void NeuronalNetwork::SaveNeuronType(string neuron_type_file) {
	Print1D(neuron_type_file, types_, "trunc", 0);
}

void NeuronalNetwork::SaveConMat(string connecting_matrix_file) {
	Print2D(connecting_matrix_file, con_mat_, "trunc");
}

int NeuronalNetwork::OutSpikeTrains(vector<vector<double> >& spike_trains) {
	spike_trains.resize(neuron_number_);
	vector<double> add_spike_train;
	int spike_num = 0;
	for (int i = 0; i < neuron_number_; i++) {
		neurons_[i].OutSpikeTrain(add_spike_train);
		spike_trains[i] = add_spike_train;
		spike_num += add_spike_train.size();
	}
	//Print2D(path, spikes, "trunc");
	return spike_num;
}

void NeuronalNetwork::GetNewSpikes(double t, vector<vector<Spike> >& data) {
	data.clear();
	data.resize(neuron_number_);
	vector<Spike> x;
	for (int i = 0; i < neuron_number_; i++) {
		neurons_[i].GetNewSpikes(t, x);
		data[i] = x;
	}
}

int NeuronalNetwork::GetNeuronNumber() {
	return neuron_number_;
}

void NeuronalNetwork::GetConductance(int i, bool type) {
	neurons_[i].GetConductance(dym_vals_[i], type);
}

void NeuronalNetwork::RestoreNeurons() {
	for (int i = 0; i < neuron_number_; i++) {
		neurons_[i].Reset(dym_vals_[i]);
		pgs_[i].Reset();
	}
	ext_inputs_.clear();
	ext_inputs_.resize(neuron_number_);
}
