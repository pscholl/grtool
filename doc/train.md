% grt-train
%
%

# NAME

 grt-train - train a machine learning algorithm

# SYNOPSIS
 grt train [-h|--help] [-v|--verbose \<level\>] [-o|--output \<file\>]
           [-n|--trainset \<n|file\>] \<algorithm\> [input-data]

 grt train list

# DESCRIPTION
 The train command estimates parameters for the various machine learning algorithms contained in the Gesture Recongition Toolkit (GRT), i.e. it trains the algorithm for specific data. Listing the available algorithms can be achieved by providing the 'list' command. Algorithm-specifc paramaters can be listed by providing the algorithm's name on the command line and the '-h' command.

 This program can pass through the feature vectors for prediction directly after training, i.e. for building a cross-validation pipe. Per default all data is consumed in training and the trained algorithm written to the file provided by the '-o' switch. You can use the '-n' switch to limit the data used for training or reading training data from a file different than standard input, the remaining input data will then be printed on the standard output. The argument supplied to this option can either range from 0 to 1, in which case it will interpreted as a percentage. Any other number will be interpreted as an absolute number. When giving a percentage as input data, the whole input needs to be kept in memory. See the examples below for more details. Alternatively an input file (-i) can be given for reading training data from. Both option are mutually exclusive.

 Depending on the classifier you chose, input is handled differently. Normal classifiers work on line-by-line basis. That means one line is read and classified/trained on. Timeseries compatible ones (e.g. HMM, listed when using grt train list) read lines until an empty line is encountered and classify/train on that block!

# OPTIONS
-h, --help
:   Print a help message.

-v, --verbose [level 0-4]
:   Print a lot of details about the current execution.

-o, --output <file>
:   Store the trained classifier in <file>.

-n, --train-set <float|file>
:   Specifies the dataset used for training. Can either specify a random split when given as a number between (0,1]. When a floating point number greater than one is given, it is interpreted as one instance of a K-fold split. The fraction part is interpreted as K, and the integral part as the n-th split of this K folds. If a file is given, it will be completly read and used for training. If -1, will use the whole input for training. Defaults to -1.

# CLASSIFIER SPECIFIC OPTIONS

 Once an algorithm has been chosen, more command line parameters become available. Not all of them can be used with all inputs, for example KNN can only be used for classification (i.e. fixed sizes sample frames).

## SVM - Support Vector Machines

 Classifier based on the infamouse LibSVM implementation.

-K, --kernel
:   kernel type. Must be one of (linear,poly,rbf,sigmoid,precomputed, defaults to linear)

-T, --type
:   SVM type. Must be one of (C_SVC,NU_SVC,ONE_CLASS,EPSILONS_VR,NU_SVR, defaults to C_CVS)

-G, --gamma
:   set to 0 for auto-calculation (default: 0)

-D, --degree
:   svm degree (default: 3)

-O, --coef0
:   coef0 paramater (default: 0)

-M, --nu
:   nu parameter (default: 0.5)

-C, --C
:   C parameter (default: 1)


## KNN - K-Nearest Neighbors

A classifier that just stores all samples during training. For classification the major vote of the K-nearest neighbors (i.e. the K training samples with lowest distance to the classification sample) is used.

-N, --null-coefficient
:   Treshold for NULL gesture rejection. Every sample with a minimum distance above this threshold will be rejected.

-d, --distance
:   The norm to use for calculating the distance between two samples. Can be one of euclidean, cosine or manhattan distance.

-K, --K-neighbors
:   The number of neighbors to check when predicting a class. Setting this to zero (the default) will enable a search for the optimum K value, which can be controlled with the min-K and max-K options.

--min-K
:   The minimum K value when doing a search for K

--max-K
:   The maximum K value when doing a search for K

## HMM - Discrete Hidden Markov Model

## cHMM - Continuous Hidden Markov Model

## DTW - Dynamic Time Warping
-N, --null-coefficient
:   Treshold for NULL gesture rejection. Every sample with a minimum distance above this threshold will be rejected. Disabled per default.

-R, --rejection-mode
:   Kind of NULL class rejection. One of template, likelihood or treshold_and_likelihood. Defaults to template.

## Finite Sate Machine

## Particle Classifier

# EXAMPLES

## Training a discrete HMM
 In this example we will train a discrete Hidden Markov Model on a time-series input. Switching to time-series mode is achieved here by providing the according command line switch. We will only use 50% of the data, the data will be split in a stratified and random way. HMMs only work on time-series, which is why we need to provide one, and since we do a 50%-split we need to provide two samples. One of those samples (chosen at random) will be printed on the standard output while the other is consumed during training.

    echo "abc 1
    > abc 1
    >
    > abc 1
    > abc 1" | grt train HMM -S 2 -T ergodic -o test.hmm -n .5
    abc 1
    abc 1

 The output of this command can then be used as the test-set, which is also the reason for the comment line at to the top. Which instructs the next command (most probably the predict command) that a time-series is about to follow.

## Training a dynamic time warping algorithm
 Let us do the same as we did in the last example, but this time with a DTW.

    echo "abc 1
    > abc 1
    >
    > cde 2
    >
    > abc 1
    > abc 1" | grt train DTW -o test.crf -n 2.0
    abc 1
    abc 1

## Making a non-random split
 In case you do not want to use stratified random split from your data as the training sequence (for example when your data is too large to fit into memory) you can use the N-first samples of your input sequence for training. Achieved by supplying a number greater than 1 to the -n option:

    echo "abc 1
    > abc 1
    >
    > cde 2
    >
    > abc 1
    > abc 1" | grt train DTW -o test.crf -n 2.0
    abc 1
    abc 1

## Doing a Cross-Validation with separate training input
 This is especially useful for doing leave-one-out cross-validations. Instead of giving the length or percentage of input that is to be used for training, you can also open a second file to read the training data from. In bash you can also use a so-called here-doc for this, which is what we use here:

    echo "cde 2
    >
    > abc 1
    > abc 1" | grt train DTW -o test.crf -n <(echo -n "abc 1\nabc 1")
