% grt-predict
% 
% 

# NANE

 grt-predict - predict the class of a data sequence

# SYNOPSIS
 grt predict [-h] [-v|--verbose \<level\>] [-l|--likelihood] [-n|--null]
             [classification-model] [input-file]

# DESCRIPTION
 This program predicts the class label of unseen data according to the model stored in the classification model file. The output will be a tab-separated list of the labels given in the input file and the predicted label. If the input data is unlabelled the output is undefined.

 The output of this file can be directly piped to the *grt score* command for further examination.

# OPTIONS

-h, --help
:   Print a help message.
 
-v, --verbose [level 0-4]
:   Tell the command to be more verbose about its execution.

-n, --null
:   Randomly select a label for each line. This is useful to check how your system reacts to random classification, e.g. as a baseline measure.

-l, --likelihood
:   additionally print the likelihood of each prediction.

# EXAMPLES

    
    
