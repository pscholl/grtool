% grt-preprocess
%
%
 
# NAME

 grt-preprocess - preprocess data

# SYNOPSIS

 grt preprocess [-h|--help] [-v|--verbose \<level\>] [-t,--type \<classification,regression,unlabelled,timseries\>]
                \<algorithm\> [input-data]

 grt preprocess list

# DESCRIPTION

 The pre-process command, as its name suggests, allows to pre-process incoming data. Several filter to smoothen and filter out frequency bands are available. You can use grt preprocess list to get a listing of those. Each filter is described in more detail in the following sections.

# OPTIONS

-h, --help
:   Print a help message.
 
-v, --verbose [level 0-4]
:   Print a lot of details about the current execution.

-t, --type [classification, timeseries, regression, unlabelled]
:   Force the interpretation of the input format to be one of the list. (default: classification)

# PREPROCESSOR DESCRIPTIONS AND OPTIONS

## LowPassFilter

## HighPassFilter

## MedianFilter

 Calculates the median over a running window of K samples, K must be an integer greater than one.

-F, --filter-size \<K\>
:   The size of the running window over which to calculate the Median.

## MovingAverageFilter

 Calculates the mean over a running window of K samples, K must be an integer greater than one.

-F, --filter-size \<K\>
:   The size of the running window over which to calculate the Mean.

## DoubleMovingAverageFilter

 Calculates the mean *twice* over a running window of K samples, K must be an integer greater than one.

-F, --filter-size \<K\>
:   The size of the running window over which to calculate the Mean.


# EXAMPLES

    grt 
