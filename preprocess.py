#!/usr/bin/env python
import numpy as np
from scipy.signal import butter, lfilter, freqz, medfilt
#from optparse import OptionParser
import argparse
import sys
import signal

def butter_lowpass(cutoff, fs, order):
  nyq = 0.5 * fs
  normal_cutoff = cutoff / nyq
  b, a = butter(order, normal_cutoff, btype='low', analog=False)
  return b, a

def butter_highpass(cutoff, fs, order):
  nyq = 0.5 * fs
  normal_cutoff = cutoff / nyq
  b, a = butter(order, normal_cutoff, btype='high', analog=False)
  return b, a

def butter_lowpass_filter(data, cutoff, fs, order):
  b, a = butter_lowpass(cutoff, fs, order=order)
  y = lfilter(b, a, data)
  return y

def butter_highpass_filter(data, cutoff, fs, order):
  b, a = butter_highpass(cutoff, fs, order=order)
  y = lfilter(b, a, data)
  return y


parser = argparse.ArgumentParser()
parser.add_argument("-c", "--cutoff", dest="cutoff",
                  help="cutoff frequency", metavar="CUTOFF", type=float)
parser.add_argument("-f", "--frequency", dest="fs",
                  help="The rate of incoming data", metavar="RATE", type=float)
#comment the next block for now and compute it straight from input
#parser.add_argument("-n", "--size", dest="vectorSize",
#                  help="The vector size of the incoming data", metavar="VECTOR_SIZE", type=int)
parser.add_argument("-o", "--order", dest="order", default=5,
                  help="The filter's order (optional)", metavar="ORDER", type=int)
parser.add_argument("-t", "--type", dest="filterType", default="LPF",
                  help="The type of filter to use, HPF (High-Pass Filter), or LPF (Low-Pass Filter). The default is set to LPF.", metavar="FILTER_TYPE")


args = parser.parse_args()

#this is a default value in case the user doesn't provide it.
order=5
try: 
  order = int(args.order)
except ValueError:
  print("Wrong value for order entered.")
  sys.exit(1)

#comment the next block for now and compute it straight from input
#try: 
#  vectorSize = args.vectorSize
#except ValueError:
#  print("Wrong value for VECTOR_SIZE entered.")
#  sys.exit()

try: 
  cutoff = args.cutoff
except ValueError:
  print("Wrong value for cutoff frequency entered.")
  sys.exit()

try: 
  fs = args.fs
except ValueError:
  print("Wrong value for RATE entered.")
  sys.exit(1)

filterType = args.filterType
filterType = filterType.upper()

#if condition to check on the data entered, exit if soemthing is wrong or missing
if filterType not in ["LPF","HPF"]:
  print("Wrong type of filter selected.")
  sys.exit(1)

if order < 1:
  print("Invalid value for filter's order.")
  sys.exit(1) 

if fs <= 0:
  print("Invalid value for data frequency.")
  sys.exit(1)

if cutoff <= 0:
  print("Invalid value for cutoff frequency.")
  sys.exit(1)

#load data from stdin as string
try:
#  data = np.loadtxt(sys.stdin, dtype=str)
  data = np.loadtxt(sys.stdin, dtype=bytes).astype(str)
 #loadtxt will throw an exception if the rows are not consistent.
except ValueError:
  print("Invalid input data.")
  sys.exit(1)

#take the label column and place it in the final array
finalData = data[:,0]
#take the remainder in a 2d float array
floats = data[:,1:].astype(np.float)

#compute the vector size from the first line
vectorSize = len(floats[0])

for i in range(0,vectorSize):
  
  #filter a column
  if filterType == "HPF":
    filteredData = butter_highpass_filter(floats[:, i], cutoff, fs, order)
  elif filterType == "LPF":
    filteredData = butter_lowpass_filter(floats[:, i], cutoff, fs, order)

  #stack each newly filtered array to the final array
  finalData = np.column_stack((finalData, filteredData))

#print to stdout
print('\n'.join(['\t'.join([item for item in row]) for row in finalData]))
