% grt-extract
% 
% 

# NANE

 grt-extract - extract features from a data sequence

# SYNOPSIS
 grt extract [-v|--verbose \<1..4\>] [-h|--help] [-n|--num-samples \<n\>]
             [-o|--output \<file\>]
              \<feature-extractor or file-name\> [input-file]

 grt extract list

# DESCRIPTION
 This program uses the feature extraction modules of the GRT to transform the input sequence with the given feature extractor. Giving 'list' as the extractor will list all available alogrithms. The 'feature-extractor' can either be one of this list or an already saved instance. In the latter case a filename needs to be provided. Storing an extractor can be achieved with the '-o' option. Storing a feature extractor is useful for extractors that need to be trained (for example the KMeansQuantizer). All input data will be passed through after the training has been accomplished, so piping is possible. The number of samples used for training can be controlled with the '-n' option, if not specified the whole input will be cached in memory before passing through.

 Some of the extractors (those taking a sample length or window parameter) exhibit a filter lag, i.e. they need to collect enough samples before giving any output. This is known as filter lag, and can be remedied by padding or mirroring values at the front or by cutting of the first n samples from the sequence. Extract only supports the latter, so depending or your window or sample size the first n sample will be removed.

 Training is supported only by the Quantizer extractors. Per default extract will load the whole sequene into memory and train on that sequence. You can control this behaviour with the -n switch. Supplying an integer will read only the specified samples into memory and train on that.

# OPTIONS
-h, --help
:   Print a help message. If an extractor is specified, the option for this extractor will be printed also.
 
-v, --verbose \<0..4\>
:   Tell the command to be more verbose about its execution.

-n, --num-samples \<n\>
:   The number of samples that are used for training the extractor. Per default the whole input will be read (and cached in memory). This option can therefore limit the amount of data that needs to be hold in memory.

-o, --output \<file\>
:   Store the trained extractor. Useful if you do not want to train the extractor for each run.

# EXTRACTOR SPECIFIC OPTIONS

 Each extractor has additional parameters which are exposed through the command line interface. When runnign a command like `grt extract KMeansQuantizer -h`, these additional parameters will be printed.

## TimeDomainFeatures

 Extract the mean, standard deviation, euclidean norm and RMS of the data series. The series can be offset by the first value, useful for removin static offsets. Two windows are involved in this extraction: the computation and reporting window. The computation window gives the size of the input window for calculation of features, while the reporting window gives the number of time-lagged computations that are reported. For example, using a compuation of size 10 uses a sliding window of 10 frames to compute features. Setting the reporting window to 2, for example, reports the last two compuation for each frame.

-K, --samples
:   number of sample used in compuation (default: 100)

-F, --frames
:   number of reported samples in each frame (default: 10)

-O, --offset
:   offset input by first input value

-M, --no-mean
:   do not compute mean

-S, --no-stddev
:   do not compute standard deviation

-E, --no-euclidean
:   do not compute euclidean norm

-R, --no-rms
:   do not compute root mean squared


# EXAMPLES

 For starters let's list all available extraction modules:
    
    grt extract list
    usage: extract [options] ... <feature-extractor or input file name> [<filename>] 
    options:
      -v, --verbose        verbosity level: 0-4 (int [=0])
      -h, --help           print this message
      -n, --num-samples    number of input samples to use for training (int [=0])
      -o, --output         if given store the model in file, for later usage (string [=])
    
    FFT
    FFTFeatures
    FrequencyDomainFeatures
    KMeansFeatures
    KMeansQuantizer
    MovementIndex
    MovementTrajectoryFeatures
    RBMQuantizer
    SOMQuantizer
    TimeDomainFeatures
    TimeseriesBuffer
    ZeroCrossingCounter

 We could then decide to use a KMeansQuantizer, which we will train on the whole dataset and store the trained quantizer in the kmeans.grt file. The default number of cluster is set to 10, which are more clusters than samples in our dataset. We limit the number of clusters to 4 because of this:

    echo "inverting 1 1 1
    > inverting 0 0 0
    > pipetting 2 2 2
    > pipetting 2 2 2 " | grt extract KMeansQuantizer -K 4 -o kmeans.grt
    inverting	0
    inverting	1
    pipetting	3
    pipetting	3

 We can also build a full pipe with the extractor. We will use the first 100 samples to *train* the extractor, which will then pass these (and all following) samples to the next program in the pipe:

    echo "inverting 1 1 1
    > inverting 0 0 0
    > pipetting 2 2 2
    > pipetting 2 2 2 " | grt extract KMeansQuantizer -K 4 -o kmeans.grt
    inverting	0
    inverting	1
    pipetting	3
    pipetting	3


