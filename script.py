#!/usr/bin/python
import subprocess
import os
import matplotlib.pyplot as plt
import numpy as np
import random

# compile *.cpp files
def Compile():
	os.system("g++ ./multi-network/*.cpp -o ./multi-network/two-network-system.out")
	os.system("g++ ./lfp/*.cpp -o ./lfp/calculate-lfp.out")
	os.system("g++ ./tdmi/*.cpp -o ./tdmi/calculate-tdmi.out")

def MakeTitle(saving_filename):
	# split filename into subunit and reassumble them into acceptable title string;
	# sub_unit[0] = 'tdmi'
	# sub_unit[1] = index of neuron
	# sub_unit[2] = connecting order
	# sub_unit[3] = time range
	# sub_unit[4] = str labels of classification
	# sub_unit[5] = expected occupancy for historgram in mutual information calculation
	# sub_unit[6] = timing step for MI
	# sub_unit[7] = maximum negative time delay
	# sub_unit[8] = maximum positive time delay
	sub_unit = saving_filename.split('-')
	title = sub_unit[0].upper()
	title += ' #' + sub_unit[1]
	if sub_unit[2] == '1':
		title += ' $1^{st}$ '
	elif sub_unit[2] == '2':
		title += ' $2^{nd}$ '
	sub_sub_unit = sub_unit[3].split('_')
	title += sub_sub_unit[0] + '~' + sub_sub_unit[1] + ' ms '
	title += sub_unit[4] + '\n'
	title += 'expected occupancy = ' + sub_unit[5]
	sub_sub_unit = sub_unit[6].split('_')
	title += ' dt = ' + sub_sub_unit[0] + '.' + sub_sub_unit[1] + ' ms'
	title += ' NTD = ' + sub_unit[7]
	title += ' PTD = ' + sub_unit[8]
	return title

# Create text in TDMI plot, including type of target neuron and the number of neuron it connected as well as their type;
def CreateText(dir, neuron_index, order, classification, num):
	# loading files;
	pre_net = np.loadtxt(dir + 'preNeuron.txt')
	post_net = np.loadtxt(dir + 'postNeuron.txt')
	con_mat = np.loadtxt(dir + 'conMat.txt')
	# create text variable
	text = '#' + str(neuron_index) + ' neuron '
	if abs(pre_net[neuron_index, 0] - 1) < 1e-6:
		text += 'is excitatory\n'
	else:
		text += 'is inhibitory\n'
	# consider the order of connection
	neuron_list_1 = [i for i in range(np.size(con_mat[neuron_index, :])) if abs(con_mat[neuron_index, i] - 1) < 1e-6]
	if order == 1:
		neuron_list_all = neuron_list_1
	else:
		neuron_list_2 = []
		for ind in neuron_list_1:
			neuron_list_2 += [j for j in range(np.size(con_mat[neuron_index, :])) if abs(con_mat[ind, j] - 1) < 1e-6]
		del ind
		neuron_list_2 = np.unique(neuron_list_2)
		neuron_list_all = np.setdiff1d(neuron_list_2, neuron_list_1)
	# consider the classification of connection
	if classification == 'exc':
		neuron_list = [ind for ind in neuron_list_all if abs(post_net[ind, 0] - 1) < 1e-6]
	elif classification == 'inh':
		neuron_list = [ind for ind in neuron_list_all if abs(post_net[ind, 0]) < 1e-6]
	else:
		neuron_list = neuron_list_all
	# select num neurons from neuron pool above
	if np.size(neuron_list) > num and num > 0:
		neuron_list = random.sample(neuron_list, num)
	else:
		pass	

	text += 'LFP is generated by ' + str(np.size(neuron_list)) + ' neurons\n'
	# classify neuronal type of all this neurons
	counter = 0
	for ind in neuron_list:
		if abs(post_net[ind,0] - 1) < 1e-6:
			counter += 1
	text += 'including ' + str(counter) + ' excitatroy and ' + str(np.size(neuron_list) - counter) + ' inhibitory neurons'
	return text

def PlotTdmi(saving_filename, figure_text):
	data_ordered = np.loadtxt("./tdmi/file-dat/tdmi_ordered.dat")
	data_rand = np.loadtxt("./tdmi/file-dat/tdmi_rand.dat")
	# basic plot
	fig = plt.figure(0, figsize=(10,8), dpi=60)
	plt.plot(data_ordered[:,0], data_ordered[:,1], label = "tdmi-original")
	plt.plot(data_rand[:,0], data_rand[:,1], label = "tdmi-swapped")
	# setting axis range;
	x_max = np.max(data_ordered[:,0])
	x_min = np.min(data_ordered[:,0])
	if np.max(data_ordered[:,1]) > np.max(data_rand[:,1]):
		y_max = np.max(data_ordered[:,1])
	else:
		y_max = np.max(data_rand[:,1])
	if np.min(data_ordered[:,1] < np.min(data_rand[:,1])):
		y_min = np.min(data_ordered[:,1])
	else:
		y_min = np.min(data_rand[:,1])
	abs_diff = y_max - y_min;
	y_min -= abs_diff * 0.1
	y_max += abs_diff * 0.1
	plt.axis([x_min, x_max, y_min, y_max])
	# setting labels and title
	plt.xlabel("Time-delay(ms)")
	plt.ylabel("Mutual Information(bits)")
	title = MakeTitle(saving_filename = saving_filename)
	# title = saving_filename
	plt.title(title)
	plt.legend()
	plt.grid(True)
	# add text
	x_text_pos = (x_max - x_min) * 0.05 + x_min
	y_text_pos = (y_max - y_min) * 0.9 + y_min
	plt.text(x_text_pos, y_text_pos, figure_text, weight = 'light')
	saving_dir = "./tdmi/figure-eps/"
	plt.savefig(saving_dir + saving_filename + '.eps')
	plt.close(0)
	del data_rand
	del data_ordered

def main():
	# Update to newest code version;
	compile_updated = False;
	if compile_updated == False:
		Compile()
	# setting preliminary parameters
	loading_dir = "/media/kyle/Drive/ResearchData/Mar15/"
	simulation_accomplish = True
	time_lb = 1000
	time_ub = 10000
	expected_occupancy = 50
	negative_time_delay = 60
	positive_time_delay = 100
	
	# Generating neruonal data based on settings above;
	if simulation_accomplish == False:
		subprocess.call(["./multi-network/two-network-system.out", loading_dir])
	# Setting loops for local field potentials;
	#all_neuron = np.linspace(0, 99 ,100)
	#target_neuron_indice_list = random.sample(all_neuron, 20)
	target_neuron_indice_list = [20]
	order_options = [1]
	classification_options = ['exc']
	# number of neurons in given classification;
	#num_list = range(0, 8)
	num = 8
	# Setting loops for time-delayed mutual information;
	timing_step_list = [0.25]
	# Start loops
	for ind in target_neuron_indice_list:
		for order in order_options:
			for classification in classification_options:
				# for num in num_list:
				subprocess.call(["./lfp/calculate-lfp.out", loading_dir, str(int(ind)), str(order), str(time_lb), str(time_ub), classification, str(num)])
				for dt in timing_step_list:
					subprocess.call(['./tdmi/calculate-tdmi.out', str(expected_occupancy), str(dt), str(negative_time_delay), str(positive_time_delay)])
					
					# create a saving filename
					saving_filename = 'tdmi-' + str(int(ind)) + '-' + str(order) + '-' + str(time_lb) + '_' + str(time_ub) + '-' + classification + '-' + str(expected_occupancy) + '-';
					str_dt = str(dt).split('.')
					saving_filename += str_dt[0] + '_' + str_dt[1] + '-' + str(negative_time_delay) + '-' + str(positive_time_delay)
					# saving_filename = 'tdmi-20-' + str(num)
					# create figure_text
					figure_text = CreateText(dir = loading_dir, neuron_index = int(ind), order = order, classification = classification, num = num)
					print figure_text
					PlotTdmi(saving_filename = saving_filename, figure_text = figure_text)
					print '=================================================='



main()