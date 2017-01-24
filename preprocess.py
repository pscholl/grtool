#!/usr/bin/env python
import numpy as np
from scipy.signal import butter, lfilter, freqz, medfilt
from optparse import OptionParser
import sys
import signal

def butter_lowpass(cutoff, fs, order=5):
  nyq = 0.5 * fs
  normal_cutoff = cutoff / nyq
  b, a = butter(order, normal_cutoff, btype='low', analog=False)
  return b, a

def butter_highpass(cutoff, fs, order=5):
  nyq = 0.5 * fs
  normal_cutoff = cutoff / nyq
  b, a = butter(order, normal_cutoff, btype='high', analog=False)
  return b, a

def butter_lowpass_filter(data, cutoff, fs, order=5):
  b, a = butter_lowpass(cutoff, fs, order=order)
  y = lfilter(b, a, data)
  return y

def butter_highpass_filter(data, cutoff, fs, order=5):
  b, a = butter_highpass(cutoff, fs, order=order)
  y = lfilter(b, a, data)
  return y


parser = OptionParser()
parser.add_option("-c", "--cutoff", dest="cutoff",
                  help="cutoff frequency", metavar="CUTOFF")
parser.add_option("-f", "--frequency", dest="fs",
                  help="The rate of incoming data", metavar="RATE")
parser.add_option("-n", "--size", dest="vectorSize",
                  help="The vector size of the incoming data", metavar="VECTOR_SIZE")
parser.add_option("-o", "--order", dest="order", default=5,
                  help="The filter's order", metavar="ORDER")
parser.add_option("-t", "--type", dest="filterType", default="HPF",
                  help="The type of filter to use, HPF (High-Pass Filter), or LPF (Low-Pass Filter).", metavar="FILTER_TYPE")


(options, args) = parser.parse_args()

try: 
  order = int(float(options.order))
except ValueError:
  print("Wrong value for order entered.")
  sys.exit()

try: 
  vectorSize = int(float(options.vectorSize))
except ValueError:
  print("Wrong value for VECTOR_SIZE entered.")
  sys.exit()

try: 
  cutoff = float(options.cutoff)
except ValueError:
  print("Wrong value for cutoff frequency entered.")
  sys.exit()

try: 
  fs = float(options.fs)
except ValueError:
  print("Wrong value for RATE entered.")
  sys.exit()

filterType = options.filterType
filterType = filterType.upper()

#if condition to check on the data entered, exit if soemthing is wrong or missing
if filterType not in ["LPF","HPF"]:
  print("Wrong type of filter selected.")
  sys.exit()

if order < 1:
  print("Invalid value for filter's order.")
  sys.exit() 


#load data from stdin as string
data = np.loadtxt(sys.stdin, dtype=str)

for i in range(0,vectorSize):
  
  #filter a column while skipping 1 (because of the preceding label) after converting it to float
  if filterType == "HPF":
    filteredData = butter_highpass_filter(data[:, i + 1].astype(np.float), cutoff, fs, order)
  elif filterType == "LPF":
    filteredData = butter_lowpass_filter(data[:, i + 1].astype(np.float), cutoff, fs, order)
  
  #replace the column used as input to the filter with the filtered values
  data[:, i + 1] = filteredData 

#print to stdout
print('\n'.join(['\t'.join([item for item in row]) for row in data]))
