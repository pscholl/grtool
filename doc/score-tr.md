% grt-score-tr
%
%

# NAME

grt-score-tr

# SYNOPSIS
grt-score-tr [-h] [-d DEFAULT] [-m N] [--transition STR]  
             [--non-binary | -u] [--insert STR] [--delete STR] [-t N]  
             [--groundtruth-hyst N] [-ph] [-i] [-iu] [-it] [-ih]  
             [PRED]

Scoring for transitions in a timeline of activities  
Apply hysteresis on predictions  
Print various infos on sample sets

Default output: binary transition check

# DESCRIPTION
A collection of tools and functions for analyzing the output of classifiers for transitions, e.g. for piping to the **_score_** module. Hysteresis smoothing can be applied to the input predictions before analyzing for transitions. The output is a binary transition stream indicating if transitions in the input are matched, inserted or deleted.

Note: The module was previously called **_score-tl_**.

# OPTIONS
positional arguments:
-  PRED                  predictions sample stream (default: -)

optional arguments:
-  -h, --help            show this help message and exit
-  -d DEFAULT, --default DEFAULT default class label (default: NULL)

transition check arguments:
-  -m N, --margin N      margin around transition
  - 0-1: percentage of whole set
  - int: num samples  
(default: 0)  
-  --transition STR      transition label string (default: trans)
-  --non-binary          non-binary mode:
  - print actual label instead of transition label
  - print insert / delete string where applicable  
(default: False)
-  -u, --unsupervised    ignore labels and just look for transitions (default: False)
-  --insert STR          output for predicted label transition without groundtruth match (non-binary mode) (default: INSERT)
-  --delete STR          output for groundtruth label transition without prediction match (non-binary mode) (default: DELETE)

hysteresis arguments:
-  -t N, --threshold N   number of equal-label samples indicating a hysteresis label change
  - 0-1: percentage of whole set
  - int: num samples  
(default: 0)
-  --groundtruth-hyst N  hystresis parameter on groundtruth, same as -t (default: 0)
-  -ph, --print-hyst     print predictions stream after hysteresis and quit (default: False)

info arguments:
-  -i, --info            (= -iu -it -ih) (default: False)
-  -iu, --info-unique    print info on the unique labels in the loaded set and quit (default: False)
-  -it, --info-timeline  print info on the timeline labels in the loaded set and quit (default: False)
-  -ih, --info-hyst      use hysteresis set instead of original predictions for info (default: False)

# EXAMPLES

## unsupervised transitions
The -u/--unsupervised option directs the transition parser to disregard *equality* of groundtruth and prediction labels and just looks for label *changes*. In the example, a stream of groundtruth labels and cluster indices is analyzed, with labels and indices being different:

    echo "abc 1
    > abc 1
    > def 0
    > def 0
    > def 0
    > abc 1
    > def 0" | grt score-tr -u

    # samples: 7 threshold: 0 margin: 0
    trans	trans
    NULL	NULL
    trans	trans
    NULL	NULL
    trans	trans
    NULL	NULL
    trans	trans
    NULL	NULL

## hysteresis
The -t/--threshold option sets a hysteresis threshold, which can be used for smoothing the input with regard to transitions, i.e. prediction changes of less samples than the set threshold are disregarded. In the example hysteresis is applied to a short sample stream, note the prediction change for the second sample is ignored. The -ph/--print-hyst option prints the resulting output without analyzing for transitions:

    echo "abc 1
    > abc 0
    > abc 1
    > abc 1
    > def 0
    > def 0
    > def 0" | grt score-tr -t 2 -ph

    abc	1
    abc	1
    abc	1
    abc	1
    def	0
    def	0
    def	0

## margin
When analyzing the input stream for transitions, the matching of transitions can be relaxed by setting the -m/--margin option. For each transition in either groundtruth or prediction, a transition in the respective other stream then still counts as a match if it happens within the margin. In the example, the predicted transition is one sample late, and so is not counted with a margin of 0 samples; A transition deletion and insertion are detected instead:

    echo "abc 1
    > abc 1
    > def 1
    > def 0
    > def 0" | grt score-tr -u -t 1 -m 0

    # samples: 5 threshold: 1 margin: 0
    trans	trans
    NULL	NULL
    trans	NULL
    NULL	NULL
    NULL	trans
    NULL	NULL

Now with a margin of 1 sample. The transitions now match:

    echo "abc 1
    > abc 1
    > def 1
    > def 0
    > def 0" | grt score-tr -u -t 1 -m 1

    # samples: 5 threshold: 1 margin: 1
    trans	trans
    NULL	NULL
    trans	trans
    NULL	NULL
