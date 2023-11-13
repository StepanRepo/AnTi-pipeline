#! /bin/python

import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.backends.backend_pdf import PdfPages
import numpy as np

def save_image(filename):
	# PdfPages is a wrapper around pdf
	# file so there is no clash and create
	# files with no error.
    with PdfPages(filename) as p:

        # get_fignums Return list of existing
        # figure numbers
        fig_nums = plt.get_fignums()
        figs = [plt.figure(n) for n in fig_nums]

        # iterating over the numbers in list
        for fig in figs:

            # and saving the files
            #fig.savefig(p, format='pdf')
            fig.savefig(p, format='pdf')


#p = 714.46991959
#tau = 0.2048
#obs_window = p//tau

p = 1_187.8767516
tau = 0.2048
obs_window = p//tau

print(p/tau, obs_window)

data = []

with open("123") as file:
    for line in file:
        data.append(np.float16(line))

data = np.array(data)

for obs_window in [5795, 5796, 5797, 5798, 5799]:
    prf = np.zeros(obs_window)

    for i in range(len(data)//obs_window):
        leap = (p/tau - obs_window)*i

        if (leap - int(leap) > 0.5):
            leap += 1

        leap = int(leap)
        leap = 0

        prf += data[(i*obs_window - leap) : ((i+1)*obs_window - leap)]

    plt.figure()
    plt.grid()
    plt.text(0, 0, str(obs_window))
    plt.plot(prf)

save_image("profiles.pdf")


