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

## majority

 This filter operates on window (-W|--window) and (-O|--overlap) of frames and returns only the majority of labels in that window [@Minnen2007].

## change

 This filter only prints a frame when a label changed.

## confidence

 Use label with maximum confidence [@Minnen2007].

## score

 Use summed label score/confidence for final decision [@Minnen2007]

# EXAMPLES

 This an example of using the ClassLabelChangeFilter.

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
    
    grt 
