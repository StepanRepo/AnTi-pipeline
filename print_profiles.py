#! /bin/python

from pathlib import Path
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


if __name__ == "__main__":

    wd = Path("./out")
    prf_list = list(wd.glob("*.prf"))

    for k, prf_file in enumerate(prf_list):
        with open(str(prf_file)) as prf:

            numpar = int(prf.readline().split()[1])

            data = prf.readlines()
            data = data[numpar-1:]

            channels = len(data)
            obs_window = len(data[0].split())

            x = np.empty(shape = (channels, obs_window))

            for i, line in enumerate(data):
                s = line.split()

                for j, num in enumerate(s):
                    x[i][j] = np.float16(num)

            plt.figure()
            plt.imshow(x, interpolation = "none", origin = "lower", norm = colors.LogNorm())

            plt.xlabel("Time")
            plt.ylabel("Frequency channel")
            plt.title(str(prf_file.name))
            plt.colorbar()


    prf_list = list(wd.glob("*.tpl"))

    for k, prf_file in enumerate(prf_list):
        with open(str(prf_file)) as prf:

            data = prf.readlines()
            data = data[3:]

            channels = len(data)
            obs_window = len(data[0].split())

            x = np.empty(shape = (channels, obs_window))

            for i, line in enumerate(data):
                s = line.split()

                for j, num in enumerate(s):
                    x[i][j] = np.float16(num)

            plt.figure()
            plt.imshow(x, interpolation = "none", origin = "lower", norm = colors.LogNorm())

            plt.xlabel("Time")
            plt.ylabel("Frequency channel")
            plt.title(str(prf_file.name))
            plt.colorbar()
    save_image("profiles.pdf")
