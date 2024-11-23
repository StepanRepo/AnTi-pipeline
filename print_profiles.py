#! /bin/python

from pathlib import Path
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import numpy as np

import argparse
import os


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

<<<<<<< HEAD
=======
    parser = argparse.ArgumentParser(description = "Plot .prf files from given directory into a pdf file")

    parser.add_argument("directory", type = str, nargs="?", default = ".",
                    help="Directory with profiles")

    parser.add_argument("num", type = int, nargs="?", default = None,
                    help="Number of printed profiles")

    parser.add_argument("-names", action = 'store_true', 
                        help = "Print file names if the parametrer was set")

    args = parser.parse_args()


    wd = Path(args.directory)

    if not os.path.isdir(wd):
        print(f"No such directory; {args.directory}")
        exit(0)

    prf_list = list(wd.glob("*.prf"))

    if (args.num is not None) and (args.num < len(prf_list)):
            prf_list = prf_list[:args.num]

    if args.names:
        names_file = open("./names.txt", "w")

    for prf_file in prf_list:

        if args.names:
            names_file.write(f"{prf_file.name}\n")

>>>>>>> 15fb9b4f166b44dfe1526bde57ca658b806ed6d4
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

            x = (x - x.min()) / (x.max() - x.min())

            plt.figure()
            plt.grid()
            plt.plot(t, x)
<<<<<<< HEAD
=======

            plt.ylim(0, 1)

            plt.title(str(prf_file.name))
            plt.ylabel("Power []")
            plt.xlabel("Time [ms]")
>>>>>>> 15fb9b4f166b44dfe1526bde57ca658b806ed6d4

        #plt.show()
    save_image(os.getcwd() + "/profiles.pdf")

    if args.names:
        names_file.close()

