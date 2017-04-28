#!/usr/bin/python
import subprocess
import os
import matplotlib.pyplot as plt
import numpy as np
import random
from scipy.optimize	import curve_fit
import pandas as pd
import mylib

# Update to newest code version;
compile_updated = True;
if compile_updated == False:
	mylib.Compile()
# setting preliminary parameters
loading_dir = "/media/kyle/Drive/ResearchData/Apr22/t7/"
total_neuron_number = 100
simulation_accomplish = False
time_lb = 1000
time_ub = 10000
expected_occupancy = 50
negative_time_delay = 60
positive_time_delay = 100

# Generating neruonal data based on settings above;
if simulation_accomplish == False:
	subprocess.call(["./multi-network/two-network-system.out", loading_dir])
	os.system("cp ./multi-network/config.ini " + loading_dir)
# Setting loops for local field potentials;
# all_neuron = range(0, 100)
# target_neuron_indice_list = random.sample(all_neuron, 1)
target_neuron_indice_list = range(0, total_neuron_number)
order_options = [1]
# prepare lists:
conMat = mylib.load_matrix(loading_dir = loading_dir, filename = 'conMat.txt')

lists = []
for i in target_neuron_indice_list:
	ll = np.nonzero(conMat[i,:])[0]
	lists.append(ll)

# classification_options = ['all']
# Setting loops for time-delayed mutual information;
timing_step_list = [0.25]
# preparing storage for data;
data_dic = {'index':np.zeros(total_neuron_number).astype(int),'type':np.zeros(total_neuron_number).astype(int), 'mean firing rate':np.zeros(total_neuron_number), 'number of connection':np.zeros(total_neuron_number).astype(int), 'number of excitatory connection':np.zeros(total_neuron_number).astype(int), 'number of inhibitory connection':np.zeros(total_neuron_number).astype(int), 'signal noise ratio':np.zeros(total_neuron_number), 'peak time':np.zeros(total_neuron_number), 'time constant':np.zeros(total_neuron_number)}
data_out = pd.DataFrame(data_dic) 

# Start loops
for i in range(total_neuron_number)
	ind = target_neuron_indice_list[i]
	ll = lists[i]
	# transform list to str
	list_str = [str(nn) for nn in ll]
	list_str = ','.join(list_str)
	# for num in num_list:
	subprocess.call(["./lfp/calculate-lfp.out", loading_dir, str(int(ind)), list_str, str(time_lb), str(time_ub), str(total_neuron_number)])
	for dt in timing_step_list:
		subprocess.call(['./tdmi/calculate-tdmi.out', str(expected_occupancy), str(dt), str(negative_time_delay), str(positive_time_delay)])
		
		# # create a saving filename

		# saving_filename = 'tdmi-' + str(int(ind)) + '-' + str(order) + '-' + str(time_lb) + '_' + str(time_ub) + '-' + classification + '-' + str(expected_occupancy) + '-';
		# str_dt = str(dt).split('.')
		# saving_filename += str_dt[0] + '_' + str_dt[1] + '-' + str(negative_time_delay) + '-' + str(positive_time_delay)
		# # saving_filename = 'tdmi-20-' + str(num)
		# # create figure_text
		# figure_text = mylib.CreateText(loading_dir = loading_dir, neuron_index = int(ind), order = order, classification = classification, num = num)
		# print figure_text
		# mylib.PlotTdmi(saving_filename = saving_filename, figure_text = figure_text)
		data_out.ix[ind]['index'] = ind
		pre_net_types = np.genfromtxt(loading_dir + 'preNeuron.txt', dtype = int, usecols = 0)
		data_out.ix[ind]['type'] = pre_net_types[ind]
		data_out.ix[ind]['mean firing rate'] = mylib.mean_rate(loading_dir = loading_dir, filename = 'rasterPre.txt', index = neuron_index, tmax = 10)
		post_net_types = np.genfromtxt(loading_dir + 'postNeuron.txt', dtype = int, usecols = 0)
		data_out.ix[ind]['number of connection'] = len(ll)
		data_out.ix[ind]['number of excitatory connection'], data_out.ix[ind]['number of excitatory connection'] = mylib.DivideNeuronalFunction(neuron_types = post_net_types, neuron_list = ll)
		time_series, signal_order, signal_rand = mylib.import_tdmi()
		data_out.ix[ind]['signal noise ratio'],	data_out.ix[ind]['peak time'] = info[7], data_out.ix[ind]['time constant'] = mylib.tdmi_parameters(time_series, signal_order, signal_rand)
		print '=================================================='

# print data_2d
data_out.to_csv(loading_dir + "pre-net-data.csv", float_format = '%.4f', index  = False, columns = ['index','type', 'mean firing rate', 'number of connection', 'number of excitatory connection', 'number of inhibitory connection', 'signal noise ratio', 'peak time', 'time constant'])