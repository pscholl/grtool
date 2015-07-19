% grt-extract
% 
% 

# NANE

 grt-extract - extract features from a data sequence

# SYNOPSIS
 grt extract [-v|--verbose \<1..4\>] [-h|--help] [-q|--no-header] 
             \<feature-vector\> [input-file]

 grt extract list

# DESCRIPTION
 This program implements common feature extraction methods found in the literature. Supplying 'list' to the extract program will list all available extractors. Extract will work on the standard input format of the grtool suite. Each line encompasses a feature, made of a label and n sensor readings. An empty line designates the end of a fragment. Each feature extractor that you choose will be applied to a fragment. Since all extracctors are aggregating functions, only a single line will be returned, labelled with the first label in the fragment.

-h, --help
:   Print a help message. If an extractor is specified, the option for this extractor will be printed also.
 
-v, --verbose \<0..4\>
:   Tell the command to be more verbose about its execution.

-q, --no-header
:   Do not print the optional header line in the output

# EXAMPLES

 For starters let's list all available extraction modules:
    
    grt extract list
    usage: extract [options] ... <feature-extractor>
    options:
      -v, --verbose        verbosity level: 0-4 (int [=0])
      -h, --help           print this message
      -q, --no-header      do not print the header
      -z, --z-normalize    z-normalize ( (x-mean(x))/std(x) ) all samples
      -o, --o-normalize    o-normalize, compute x_i - x_0, i.e. remove the first component from each sample
    
    Available Extractors:
    
     mean (m): compute mean/average of each axis
     range (r): compute range (min/max and their difference
     variance (v): compute variance of each axis
     median (e): compute median of each axis
     zcr (z): zero-crossing rate
     rms (s): root-mean squared over each and all axis
     time (t): shorthand for all time-domain features: mean,variance,range,median

    

 We could then decide to extract mean and range from our input. This would result in the following command line:

    echo "inverting 1 1
    > inverting 0 0
    > pipetting 2 3
    > pipetting 2 2" | grt extract "m range"
    # mean	range	
    pipetting	1.25	1.5	2	0	2	3	0	3	


 For each input dimension, the feature will be calculated and printed on the output. The mean extractor returns two dimensions, while the range extractor gives the maximum, minimum and the span of each input axis. This results in an eight-dimensional feature vector as output.

 So, this aggregates multiple segments into one frame. Frames are separated by empty lines into segments. The following examples will aggregate the two segments into two frames!

    echo "inverting 1 1
    > inverting 0 0
    >
    > pipetting 2 3
    > pipetting 2 2 " | grt e m
    # mean	
    inverting	0.5	0.5	
    pipetting	2	2.5	
