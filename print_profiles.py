#! /bin/python

from pathlib import Path
import matplotlib.pyplot as plt
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

            t = np.empty(len(data))
            x = np.empty(len(data))

            for i, line in enumerate(data):
                s = line.split()

                t[i] = np.float64(s[0])
                x[i] = np.float16(s[1])

            plt.figure()
            plt.plot(t, x)

        #plt.show()
    save_image("out/profiles.pdf")







