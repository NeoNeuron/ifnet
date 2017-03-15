//***************
//	Copyright: Kyle Chen
//	Author: Kyle Chen
//	DATE: 2017-02-21 16:04:45
//	Description: define class ConnectivityMatrix; Connectivity matrix is used 
// 	to describe network structure of one dimensional network;
//***************
#ifndef _MULTI_NETWORK_SIMULATION_CONNECTIVITY_MATRIX_H_ 
#define _MULTI_NETWORK_SIMULATION_CONNECTIVITY_MATRIX_H_ 

#include<iostream>
#include<fstream>
#include<cstdlib>
#include<vector> 

using namespace std;

class ConnectivityMatrix {
private:
	int neuron_number_;
	int connecting_density_; // number of connection on one side of each neuron;
	vector<vector<int> > matrix_; // binary connectivity matrix; posible value is 1 or 0;
	vector<vector<int> > path_matrix_; // least path between each neuronal pair;
	vector<vector<int> > mediate_mode_matrix_; // indices of mediate neurons between the pole of neuron pairs;
	vector<double> clustering_coefficient_; // clustering coefficient of each neuron;

	//	Functions:

	//	Find given elements of given array and output their indices.
	void Scan(int target_value, int row_index_, vector<int> &output_indices);

	//	Find the least path between each pair of neurons; Store results in the path_matrix_;
	void FindLeastPath();

	//	Calculate the path clustering coefficient of each neuron; Store results in clustering_coefficenct_;
	void CalculateClusteringCoefficient();

public:
	//	INPUTS:

	// Set size of network: 
	void SetNeuronNumber(int neuron_number);

	//	Set connectivity density of network;
	void SetConnectingDensity(int connecting_density);

	//	Load onnectivity matrix from external matrix; 
	void LoadMatrix(vector<vector<int> > &matrix);

	//	Rewire network according to certain probability;
	void Rewire(double p, int seed, bool OutputOption);

	//	Read the value of matrix element in ith row, jth column;
	int ReadMatrix(int i, int j);

	//	Output the average value of least path among all neuronal pairs;
	double OutputMeanPath();

	//	Output the average value of clustering coefficient among all neurons;
	double OutputMeanClusteringCoefficient();

	//	Output matrix_ to external file.
	void OutputMatrix(ofstream & file);
};

#endif // _MULTI_NETWORK_SIMULATION_CONNECTIVITY_MATRIX_H_ 