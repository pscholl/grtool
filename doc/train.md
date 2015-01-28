% grt-train
% 
% 

# NANE

 grt-train - train a trainable machine learning algorithm

# SYNOPSIS
 grt train [TODO]

# DESCRIPTION
 The train command estimates parameters for the various machine learning algorithms contained in the Gesture Recongition Toolkit (GRT).

 The machine learning algorithms include the stages for feature extraction, and actual prediction. To get a full listing use *grt train list*.

# OPTIONS
-h, --help
:   Print a help message.
 
-v, --verbose [level 0-4]
:   Tell the command to be more verbose about its execution.

-t, --type [classification, timeseries, regression, unlabelled]
:   Force the interpretation of the input format to be one of the list.

# CLASSIFIER SPECIFIC OPTIONS

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

## HMM - Hidden Markov Model

## DTW - Dynamic Time Warping
-N, --null-coefficient
:   Treshold for NULL gesture rejection. Every sample with a minimum distance above this threshold will be rejected. Disabled per default.

-R, --rejection-mode
:   Kind of NULL class rejection. One of template, likelihood or treshold_and_likelihood. Defaults to template.

## Finite Sate Machine

## Particle Classifier

# EXAMPLES

