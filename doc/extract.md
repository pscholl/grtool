% grt-extract
% 
% 

# NANE

 grt-extract - extract features from a data sequence

# SYNOPSIS
 grt extract [TODO options] \<feature-extractor or file-name\> [input-file]

 grt extract list

# DESCRIPTION
 This program uses the feature extraction modules of the GRT to transform the input sequence with the given feature extractor. Giving 'list' as the extractor will list all available alogrithms. The 'feature-extractor' can either be one of this list or an already saved instance.

 Saving a feature extractor is useful for extractor that need to be trained (for example the KMeansQuantizer). All input data will be passed through after the training has been accomplished, so piping is possible. The number of sample used for training can be controlled with the '-n' option.

# OPTIONS
-h, --help
:   Print a help message.
 
-v, --verbose [level 0-4]
:   Tell the command to be more verbose about its execution.

-t, --type [classification, timeseries, regression, unlabelled]
:   Force the interpretation of the input format to be one of the list.

TODO

# CLASSIFIER SPECIFIC OPTIONS

## KMeansQuantizer 

TODO

# EXAMPLES

 This example trains a KMeanQuantizer and store the instance into a file called 'kmeans.grt' for later usage:

    $ grt extract KMeanQuantizer -o kmeans.grt kmeans.dataset
    inverting 0
    pipetting 1
    ...

 Now we can load this *trained* extractor again to quantize another dataset with the same values:

    $ cat kmeans2.dataset | grt extract kmeans.grt
    pipetting 1
    inverting 0
    ...

 We can also build a full pipe with the extractor. We will use the first 100 samples to *train* the extractor, which will then pass these (and all following) samples to the next program in the pipe:

    $ grt extract KMeanQuantizer -n 100 kmeans.dataset | grt train HMM -m hmm.grt
    ...
