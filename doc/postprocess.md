% grt-postprocess
%
%
 
# NAME

 grt-postprocess - postprocess data

# SYNOPSIS

 grt postprocess [-h|--help] [-v|--verbose \<level\>]
                \<algorithm\> [input-data]

 grt postprocess list

# DESCRIPTION

 The post-process command takes a list of labels and its prediction and can apply several additional steps to smoothen the prediction.
 This can be used to emulate event detection and de-overlapping segmentation windows.

# OPTIONS

-h, --help
:   Print a help message.
 
-v, --verbose [level 0-4]
:   Print a lot of details about the current execution.


# POSTPROCESSOR DESCRIPTIONS AND OPTIONS

## change

 This filter only prints a frame when either the ground-truth, prediction or both labels changed.

## majority

 This filter operates on window (-W|--window) and (-O|--overlap) of frames and returns only the majority of labels in that window [@Minnen2007].

-W, --window [int of frames]
:   The number of frames to consider during one vote.

-O, --overlap [int of frames]
:   The overlap in percent of frames to consider.

## confidence

 Use label with maximum confidence [@Minnen2007]. I.e. the one with a P(label|x), if multiple maxima exists, the first one is chosen.

-W, --window [int of frames]
:   The number of frames to consider during one vote.

-O, --overlap [int of frames]
:   The overlap in percent of frames to consider.

## score

 Sum all label scores over all classes and select the one with the highest probability [@Minnen2007].

-W, --window [int of frames]
:   The number of frames to consider during one vote.

-O, --overlap [int of frames]
:   The overlap in percent of frames to consider.

# EXAMPLES

 This an example of the majority filter, which will ouput the major label over an N-sized window with an overlap. Note that the default overlap is 50%, which is why the last line is repeated.

    echo "NULL NULL
    > label label
    > label some_prediction
    > some_prediction some_prediction
    > some_prediction some_prediction
    > " | grt postprocess majority -W 2
    NULL NULL
    label label
    label some_prediction
    some_prediction some_prediction
    some_prediction some_prediction

 And this is an example of a post-processing using predicition probabilities, which need to be printed if they are used. You can switch the prediction tool to do so with the -p switch.

    echo "NULL NULL .5
    > NULL label .9
    > label NULL .5
    > label label 1
    > " | grt postprocess confidence -W 2 -O 0
    NULL label
    label label

 Here a window of two frames (lines) with zero overlap was selected. The first two lines are merged, the groundtruth label NULL is selected and the prediction of label is selected since it was classified with higher confidence. For the next two frames the 'label' is selected as ground truth and 'label' as the prediction since it had the highest confidence. We can also use the sum over confidence to estimate the prediction:

    echo "NULL NULL .5
    > NULL label .9
    > label NULL .5
    > label label 1
    > " | grt postprocess score -W 4 -O 0
    label label

 The 'NULL' was chosen for ground-truth, since the larger four frames time window overlaps multiple label and the first is chosen accordingly. As summed confidence of the 'label' prediction is larger than the sum of the 'NULL' prediction, the 'label' is chosen as output for this frame.


# REFERENCES

[@Minnen2007]: Minnen, D., Westeyn, T., Presti, P., Ashbrook, D., & Starner, T. (2007). Recognizing Soldier Activities in the Field. Proceedings of International IEEE Workshop on Wearable and Implantable Body Sensor Networks (BSN), 13, 236–241. doi:10.1007/978-3-540-70994-7_40
