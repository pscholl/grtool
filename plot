#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib as mp, os, sys, argparse, fileinput, numpy as np, matplotlib.animation as animation
from matplotlib import pyplot as plt
from threading import Thread
from time import time,sleep
from queue import Queue

cmdline = argparse.ArgumentParser('real-time of data for grt')
cmdline.add_argument('--num-samples', '-n', type=int, default=0,     help="plot the last n samples, 0 keeps all")
cmdline.add_argument('--frame-rate',  '-f', type=float, default=60., help="limit the frame-rate, 0 is unlimited")
cmdline.add_argument('--quiet',       '-q', action="store_true",     help="if given does not copy input to stdout")
cmdline.add_argument('--title',       '-t', type=str, default=None,  help="plot window title")
cmdline.add_argument('files', metavar='FILES', type=str, nargs='*', help="input files or - for stdin")
args = cmdline.parse_args()

class TextLineAnimator(Thread):
    def __init__(self, input_iterator, setup_func=None, loop_func=None, framelimit=None, quiet=False):
        self.loop_func  = loop_func or self.default_loop
        self.setup_func = setup_func or self.default_setup
        self.artists = None
        self.frameno = 0
        self.framelimit = framelimit
        self.paused = False

        #
        # storing the input for drawing
        #
        self.labels = []
        self.data   = []
        self.vspans = [] # for drawing labels

        #
        # transferring the input from stdin
        #
        self.quiet = quiet
        self.queue = Queue(framelimit)
        self.input = input_iterator
        Thread.__init__(self)
        self.start()

    def __call__(self,i):
        #
        # get the current number of elements in the
        # queue and transfer them to the buffer
        #
        qsize = self.queue.qsize()
        self.frameno += qsize

        if qsize == 0:
            return []

        while qsize > 0:
            label,data = self.queue.get()
            qsize -= 1

            self.labels.append(label)
            self.data.append(data)

            if self.framelimit and len(self.data) > self.framelimit:
                self.data.pop(0)
                self.labels.pop(0)

        labels,data = self.labels,self.data
        if self.artists is None and len(self.data) == 0:
            return []

        if self.artists is None:
            self.artists = self.setup_func(labels,data)
            plt.draw()

        self.loop_func(self.frameno,self.artists,labels,data)

        # rescale
        ax = self.artists[0].get_axes()
        ax.relim()
        ax.autoscale_view()

        return self.artists

    def run(self, *args):
        for line in self.input:
            while self.paused: sleep(.01)

            #
            # copy what we got to the next process
            #
            if not self.quiet: sys.stdout.write(line)

            #
            # ignore empty and comment lines
            #
            if len(line.strip()) == 0 or line.strip()[0] == '#':
                continue
            line = line.strip().split()

            #
            # add label and data item to the queue
            #
            try: self.queue.put((None, [float(x) for x in line])); continue
            except ValueError: pass
            try: self.queue.put((line[0], [float(x) for x in line[1:]]))
            except ValueError: pass

        #
        # propagate that we read input completly
        # (yes, sys.stdout.close() does not close the fd)
        #
        os.close(sys.stdout.fileno())

    def default_setup(self,labels,data):
        arts = plt.plot(data)
        plt.tight_layout()
        return arts

    def default_loop(self,frameno,arts,labels,data):
        data  = np.array(data)
        xdata = np.arange(frameno-data.shape[0], frameno) + 1

        for art,d in zip(arts,data.T):
            art.set_data(xdata,d)

        #
        # remove all vspans and add updated ones
        #
        for span in self.vspans:
            span.remove()

        offset = frameno - data.shape[0]
        spans  = [0] + [x for x in range(len(labels)-1) if labels[x]!=labels[x+1]] + [len(labels)]
        labels = [l1 for l1,l2 in zip(labels[:-1],labels[1:]) if l1!=l2]
        self.vspans = [\
                plt.axvspan(offset+x1,offset+x2, alpha=.2, zorder=-1)\
                for x1,x2 in zip(spans[:-1],spans[1:]) ]
        self.vspans += [\
                plt.annotate(label, (offset+x1,0.1), xycoords=("data", "axes fraction"), rotation=30)\
                for label,x1 in zip(labels,spans) ]

        return arts

    def toggle_pause(self):
        self.paused = not self.paused


if __name__=="__main__":
    fig = plt.figure()
    if args.title: fig.canvas.set_window_title(args.title)
    anim = TextLineAnimator(fileinput.input(args.files,bufsize=1),framelimit=args.num_samples,quiet=args.quiet)
    afun = animation.FuncAnimation(fig,anim,interval=1000./args.frame_rate)

    def press(event):
        if event.key == ' ':
            anim.toggle_pause()
            title = fig.canvas.get_window_title()
            if anim.paused: title += " [paused]"
            else: title = title.replace(" [paused]", "")
            fig.canvas.set_window_title(title)
        elif event.key == 'q': os._exit(0) # sys.exit kills only current thread!

    def resize(event):
        try: plt.tight_layout()
        except: pass

    fig.canvas.mpl_connect('key_press_event', press)
    fig.canvas.mpl_connect('resize_event', resize)
    plt.show()
