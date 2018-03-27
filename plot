#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import matplotlib as mp, os, sys, argparse, fileinput, numpy as np, matplotlib.animation as animation, itertools
from matplotlib import pyplot as plt
from matplotlib import gridspec
from threading import Thread
from time import time,sleep
from queue import Queue
from inspect import isclass
from math import ceil
import sys
from pprint import pprint
mp.style.use("seaborn-dark")

class ScatterPlot:
    def __init__(self,labels,data):
        ndim = len(data[0])
        self.x,self.y = ndim,ndim
        self.sc = [None] * ndim*ndim
        plt.subplot(self.x,self.y,1)
        self(0,labels,data)

    def __call__(self,frameno,labels,data):
        self.labels = list(set(labels))
        self.cm     = mp.cm.get_cmap("cubehelix", len(self.labels)+1)

        markers = list(mp.markers.MarkerStyle.markers.values())
        crosses = [(x,y) for y in reversed(range(self.y)) for x in range(self.x)]
        data    = np.array(data)

        for (x,y),n in zip(crosses,range(len(crosses))):
            li=[self.labels.index(l) for l in labels]
            ax=plt.subplot(self.x,self.y,n+1)

            if self.sc[n] is None:
                self.sc[n] = plt.scatter(data[:,x],data[:,y],color=self.cm(li),alpha=.5)
            else:
                self.sc[n].set_offsets( np.array((data[:,x],data[:,y])).T )
                self.sc[n].set_facecolors( self.cm(li,.5) )
                self.sc[n].set_edgecolors( self.cm(li,.5) )

        return self.sc


class LinePlotWithLabels:
    def __init__(self, labels, data):
        self.vspans = [] # for drawing labels

        # make 2 subplots with fixed ratio
        gs = gridspec.GridSpec(2,1, height_ratios=[16, 1])

        # the line plot
        plt.subplot(gs[0])
        self.arts = plt.plot(data)

        # the (for now) empty label plot
        self.labelAxes = plt.subplot(gs[1])
        self.labelAxes.get_yaxis().set_visible(False)
        self.labelAxes.get_xaxis().set_visible(False)

        # add lines to the arts
        plt.tight_layout()

    def __call__(self,frameno,labels,data):
        # update plot data
        data     = np.array(data)
        xdata    = np.arange(frameno-data.shape[0], frameno)
        labels.append(None)

        # update labels
        self.labels = list(set(labels))
        self.cm  = mp.cm.get_cmap("jet", len(self.labels)+1)

        # append new data
        for art,d in zip(self.arts[:3],data.T):
            art.set_data(xdata,d)

        # update x axis for label plot
        self.labelAxes.set_xlim([0, len(xdata)])

        # remove all vspans and add updated ones
        for span in self.vspans:
            span.remove()

        offset = frameno - data.shape[0]

        # calculate end index of each vspan block
        spans  = [0] + [x+1 for x in range(len(labels)-1) if labels[x]!=labels[x+1]] + [len(labels)-1]

        # span differences (for text centering)
        spanDiffs  = [(s1-s2) for (s1,s2) in zip(spans[:-1], spans[1:])]

        # create list of currently visible labels
        mylabels = [l1 for l1,l2 in zip(labels,labels[1:]) if l1!=l2]

        # collect label indices
        li=[self.labels.index(l) for l in mylabels]

        # create vspans and annotations
        self.vspans = [\
                self.labelAxes.axvspan(offset+x1,offset+x2, alpha=.2, zorder=-1, color=c)\
                for x1,x2,c in zip(spans[:-1],spans[1:],self.cm(li)) ]
        self.vspans += [\
                self.labelAxes.annotate(label, (x1-0.5*sd-len(label)*30,0.5), xycoords=("data", "axes fraction"), rotation=0)\
                for label,x1,sd in zip(mylabels,spans,spanDiffs) ]
        labels.pop()
        return self.arts


class LabelPlot:
    def __init__(self, labels, data):
        self.vspans = [] # for drawing labels

        # make 2 subplots with fixed ratio
        gs = gridspec.GridSpec(2,1, height_ratios=[1, 1])

        self.labelAxes = []
        self.labelAxes.append(plt.subplot(gs[0]))
        self.labelAxes.append(plt.subplot(gs[1],sharex=self.labelAxes[0]))
        self.labelAxes[0].set_ylabel('Ground truth')
        self.labelAxes[0].set_yticks([])
        self.labelAxes[0].set_yticklabels([])
        self.labelAxes[0].grid(False)

        self.labelAxes[1].set_ylabel('Prediction')
        self.labelAxes[1].set_yticks([])
        self.labelAxes[1].set_yticklabels([])
        self.labelAxes[1].grid(False)

        self.labelAxes[0].set_xticklabels([])

        # add lines to the arts
        plt.tight_layout()

    def __call__(self,frameno,gtLabels,dtLabels):
        #dtLabels = gtLabels
        # update plot data
        #gtLabels.append(None)
        #dtLabels.append(None)

        # update labels
        self.labels = list(set(gtLabels).union(dtLabels))
        self.cm = mp.cm.get_cmap("jet", len(self.labels)+1)

        assert len(dtLabels)==len(gtLabels)

        # update x axis for label plot
        self.labelAxes[0].set_xlim([frameno-len(gtLabels), frameno])
        self.labelAxes[1].set_xlim([frameno-len(dtLabels), frameno])

        # remove all vspans and add updated ones
        for span in self.vspans:
            span.remove()
        self.vspans = []

        offset = frameno - len(dtLabels)

        # calculate end index of each vspan block
        gtSpans  = [0] + [x+1 for x in range(len(gtLabels)-1) if gtLabels[x]!=gtLabels[x+1]] + [len(gtLabels)-1]
        dtSpans  = [0] + [x+1 for x in range(len(dtLabels)-1) if dtLabels[x]!=dtLabels[x+1]] + [len(dtLabels)-1]
        gtSpanDiffs  = [(s2-s1) for (s1,s2) in zip(gtSpans[:-1], gtSpans[1:])]
        dtSpanDiffs  = [(s2-s1) for (s1,s2) in zip(dtSpans[:-1], dtSpans[1:])]

        # collect label indices
        myGtlabels = [l1 for l1,l2 in zip(gtLabels,gtLabels[1:]) if l1!=l2]
        myDtlabels = [l1 for l1,l2 in zip(dtLabels,dtLabels[1:]) if l1!=l2]
        gtli=[self.labels.index(l) for l in myGtlabels]
        dtli=[self.labels.index(l) for l in myDtlabels]

        # create vspans and annotations
        self.vspans = [\
                self.labelAxes[0].axvspan(offset+x1,offset+x2, alpha=.2, zorder=-1, color=c)\
                for x1,x2,c in zip(gtSpans[:-1],gtSpans[1:],self.cm(gtli)) ]

        self.vspans += [\
                self.labelAxes[0].annotate(label, (x1+0.5*sd,0.5), xycoords=("data", "axes fraction"), rotation=90)\
                for label,x1,sd in zip(myGtlabels,gtSpans,gtSpanDiffs) ]

        self.vspans += [\
                self.labelAxes[1].axvspan(offset+x1,offset+x2, alpha=.2, zorder=-1, color=c)\
                for x1,x2,c in zip(dtSpans[:-1],dtSpans[1:],self.cm(dtli)) ]

        self.vspans += [\
                self.labelAxes[1].annotate(label, (x1+0.5*sd,0.5), xycoords=("data", "axes fraction"), rotation=90)\
                for label,x1,sd in zip(myDtlabels,dtSpans,dtSpanDiffs) ]

        gtLabels.pop()
        dtLabels.pop()
        return []


class LinePlot:
    def __init__(self, labels, data):
        self.vspans = [] # for drawing labels
        self.arts = plt.plot(data, '+')
        plt.tight_layout()

    def __call__(self,frameno,labels,data):
        data     = np.array(data)
        xdata    = np.arange(frameno-data.shape[0], frameno)
        labels.append(None)

        self.labels = list(set(labels))
        self.cm  = mp.cm.get_cmap("jet", len(self.labels)+1)

        for art,d in zip(self.arts,data.T):
            art.set_data(xdata,d)

        #
        # remove all vspans and add updated ones
        #
        for span in self.vspans:
            span.remove()

        offset = frameno - data.shape[0]
        spans  = [0] + [x+1 for x in range(len(labels)-1) if labels[x]!=labels[x+1]] + [len(labels)-1]
        mylabels = [l1 for l1,l2 in zip(labels,labels[1:]) if l1!=l2]
        li=[self.labels.index(l) for l in mylabels]
        self.vspans = [\
                plt.axvspan(offset+x1,offset+x2, alpha=.2, zorder=-1, color=c)\
                for x1,x2,c in zip(spans[:-1],spans[1:],self.cm(li)) ]
        self.vspans += [\
                plt.annotate(label, (offset+x1,0), xycoords=("data", "axes fraction"), rotation=30, rotation_mode="anchor")\
                for label,x1 in zip(mylabels,spans) ]
        labels.pop()
        return self.arts


class XYPlot:
    def __init__(self, labels, data):
        self.annotations,self.arts = [],[]
        data = np.array(data).T
        for i,x,y,m in zip(range(len(data[::2])),data[::2],data[1::2],itertools.cycle('>^*o')):
            s=np.argsort(x)
            try: a = plt.plot(y[s],x[s],m,label=str(i))[0]
            except ValueError: sys.stderr.write("input dims do not fit\n"); sys.exit(-1)
            self.arts.append(a)
            for (l,point) in zip(labels,zip(x,y)):
                a = plt.annotate(l,xy=point,xycoords="data")
                a.set_visible(False)
                self.annotations.append(a)

        #
        # Limit axes
        #
        plt.xlim((0,1))
        plt.ylim((0,1))
        plt.tight_layout()
        plt.legend(loc=2)

        #
        # add mouse-over annotations
        #
        def on_move(ev):
            if (ev.xdata is None):
                return

            for a in self.annotations:
                x,y = a.xy
                a.set_visible( (x - ev.xdata)**2 + (y - ev.ydata)**2 <= .00005 )

        plt.gcf().canvas.mpl_connect('motion_notify_event',on_move)

    def __call__(self,frameno,labels,data):
        data = np.array(data).T
        for art,x,y in zip(self.arts,data[::2],data[1::2]):
            s=np.argsort(x)
            try: art.set_data(x[s],y[s])
            except ValueError: sys.stderr.write("input dims do not fit\n"); sys.exit(-1)
            for (l,point) in zip(labels,zip(x,y)):
                a = plt.annotate(l,xy=point,xycoords="data")
                a.set_visible(False)
                self.annotations.append(a)
        return self.arts


plotters = {
        'xy'            : XYPlot,
        'line'          : LinePlot,
        'labels'        : LabelPlot,
        'lineLabels'    : LinePlotWithLabels,
        'scatter'       : ScatterPlot, }

cmdline = argparse.ArgumentParser('real-time of data for grt')
cmdline.add_argument('--plot-type',   '-p', type=str, default="line",help="type of plot", choices=plotters.keys())
cmdline.add_argument('--num-samples', '-n', type=int, default=0,     help="plot the last n samples, 0 keeps all")
cmdline.add_argument('--frame-rate',  '-f', type=float, default=60., help="limit the frame-rate, 0 is unlimited")
cmdline.add_argument('--quiet',       '-q', action="store_true",     help="if given does not copy input to stdout")
cmdline.add_argument('--title',       '-t', type=str, default=None,  help="plot window title")
cmdline.add_argument('--plot-font-size', '-pf', type=int, default=None,  help="plot font size")
cmdline.add_argument('--plot-x-label', '-pxl', type=str, default=None,  help="plot xaxis label")
cmdline.add_argument('--plot-y-label', '-pyl', type=str, default=None,  help="plot yaxis label")
cmdline.add_argument('--output',      '-o', type=str, default=None,  help="if given plot into movie file instead of screen")
cmdline.add_argument('files', metavar='FILES', type=str, nargs='*',  help="input files or - for stdin")
args = cmdline.parse_args()

class TextLineAnimator(Thread):
    def __init__(self, input_iterator, plotter=LinePlot, loop_func=None, framelimit=None, quiet=False):
        self.plotter = plotter
        self.frameno = 0
        self.framelimit = framelimit
        self.paused = False

        #
        # storing the input for drawing
        #
        self.labels = []
        self.data   = []

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
            return None

        while qsize > 0:
            label,data = self.queue.get()
            qsize -= 1

            self.labels.append(label)
            self.data.append(data)

            if self.framelimit and len(self.data) > self.framelimit:
                self.data.pop(0)
                self.labels.pop(0)

        labels,data = self.labels,self.data

        if isclass(self.plotter) and len(self.data) == 0:
            return []

        if isclass(self.plotter):
            self.plotter = self.plotter(labels,data)
            plt.draw()

        arts = self.plotter(self.frameno,labels,data)

        if arts is not None and len(arts) > 0:
            # rescale
            ax = arts[0].axes
            ax.relim()
            ax.autoscale_view(True,True,True)

        return arts

    def run(self, *args):
        for line in self.input:
            while self.paused: sleep(.01)

            #
            # copy what we got to the next process
            #
            if not self.quiet: sys.stdout.write(line); sys.stdout.flush()

            #
            # ignore empty and comment lines
            #
            if len(line.strip()) == 0 or line.strip()[0] == '#':
                continue
            line = line.strip().split()

            #
            # add label and data item to the queue, make sure to "continue"
            # once we have parsed the input in one of the possible formats
            #
            try: self.queue.put((None, [float(x) for x in line])); continue
            except ValueError: pass
            try: self.queue.put((line[0], [float(x) for x in line[1:]])); continue
            except ValueError: pass
            try: self.queue.put((line[0], line[1]))
            except ValueError: pass


        #
        # propagate that we read input completly
        # (yes, sys.stdout.close() does not close the fd)
        # but delay this until all data has been consumed!
        #
        while self.queue.qsize() > 0:
            sleep(.1)

        os.close(sys.stdout.fileno())
        sys.stdout.close()

    def toggle_pause(self):
        self.paused = not self.paused


class MyFuncAnimation(animation.FuncAnimation):
    #
    # This is a hack to stop the animation, to avoid python busy-looping
    # when there is no more data to read. The window will still persist,
    # however as soon as stdout is closed we can be more or less sure
    # that all data has been read and drawn.
    #
    def _step(self,*args):
        animation.FuncAnimation._step(self,*args)
        if sys.stdout.closed:
            plt.gcf().canvas.set_window_title(plt.gcf().canvas.get_window_title()+" [done]")
        return not sys.stdout.closed

if __name__=="__main__":
    fig = plt.figure()
    if args.title: fig.canvas.set_window_title(args.title)
    elif len(args.files) > 0: fig.canvas.set_window_title(" ".join(args.files))

    if args.plot_font_size: mp.rcParams.update({'font.size': args.plot_font_size})

    anim = TextLineAnimator(fileinput.input(args.files,bufsize=1),framelimit=args.num_samples,quiet=args.quiet,plotter=plotters[args.plot_type])
    afun = MyFuncAnimation(fig,anim,interval=1000./args.frame_rate)

    if args.plot_x_label: plt.xlabel(args.plot_x_label)
    if args.plot_y_label: plt.ylabel(args.plot_y_label)

    if args.output is not None:
        writer = animation.writers['ffmpeg']
        writer = writer(fps=args.frame_rate,metadata={'title':args.title})
        afun.save(args.output, writer=writer)
        sys.exit(0)

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
