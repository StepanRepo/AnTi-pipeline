#! /bin/python

from pathlib import Path
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import numpy as np
from scipy import signal
from scipy import interpolate, optimize
from scipy.ndimage import gaussian_filter1d as gaussian_filter
norm = np.random.normal

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


def continous_max(vec):
    c_max = vec.argmax()

    x = np.linspace(-2, 2, 5)
    y = vec[c_max-2:c_max+3]

    interp = interpolate.interp1d(x, y, 3)
    
    a = optimize.fmin(lambda x: -interp(x), 0)[0]
    print(f"{a:.16f}")

    return c_max + a





if __name__ == "__main__":


    wd = Path("./out")
    prf_list = list(wd.glob("*.ccf"))

    
    for k, prf_file in enumerate(prf_list):
        with open(str(prf_file)) as prf:

            data = prf.readlines()

            channels = len(data)
            obs_window = len(data[0].split())

            ccf = np.empty(shape = (channels, obs_window))

            for i, line in enumerate(data):
                s = line.split()

                for j, num in enumerate(s):
                    ccf[i][j] = np.float16(num)



            v_sum = np.empty(obs_window)
            for i in range(obs_window):
                v_sum[i] = np.mean(ccf[:, i])

            h_sum = np.empty(channels)
            for i in range(channels):
                h_sum[i] = np.mean(ccf[i, :])


            fig = plt.figure(figsize = (8, 7))    
            fig.suptitle(str(prf_file.name))
            fig.text(0.8, .9, f"max = {v_sum.argmax()}")


            gs = fig.add_gridspec(2, 2,  width_ratios = (4, 1), height_ratios=(1, 4),
                      left=0.1, right=0.9, bottom=0.1, top=0.9,
                      wspace=0.05, hspace=0.05)
            ax = fig.add_subplot(gs[1, 0])
            ax_histx = fig.add_subplot(gs[0, 0], sharex = ax)
            ax_histy = fig.add_subplot(gs[1, 1], sharey = ax)

            ax.imshow(ccf, interpolation = "none", origin = "lower", aspect = 2)

            ax.set_xlabel("Time")
            ax.set_ylabel("Frequency channel")


            ax_histx.plot(v_sum)
            ax_histx.grid(True)
            ax_histx.tick_params(axis = "x", labelbottom = False)

            y = np.arange(channels)
            h_sum [h_sum == 0] = np.NaN
            ax_histy.plot(h_sum, y)
            ax_histy.grid(True)
            ax_histy.tick_params(axis = "y", labelleft=False)


#    plt.figure()
#    plt.grid()
#    plt.plot(v_sum)
#    plt.title("vertical sum")
#    plt.xlabel(f"max = {continous_max(v_sum) - 579:.3f}")
#
#    plt.figure()
#    plt.grid()
#    plt.plot(h_sum)
#    plt.title("horizontal sum")
#    
#    if (v_sum.argmax() > 600):
#        v_sum = v_sum[:559] + v_sum[600:]
#
#
#    plt.figure()
#    plt.grid()
#    plt.plot(v_sum)
#    plt.title("vertical sum")
#    plt.xlabel(f"max = {continous_max(v_sum) - 579:.3f}")
    

    save_image("correlations.pdf")
