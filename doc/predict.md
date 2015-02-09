% grt-predict
% 
% 

# NANE

 grt-predict - predict the class of a data sequence

# SYNOPSIS
 grt predict [-h] [-v|--verbose \<level\>] [-t|--type \<input-type\>] \<classification-model\> [input-file]

# DESCRIPTION
 This program predict the class label of unseen data according to the model stored in the classification model file. The output will be a tab-separated list of the label given in the input file and the predicted label. If the input data is unlabelled the output is undefined.

 The output of this file can be directly piped to the *grt score* command for further examination.

# OPTIONS
-h, --help
:   Print a help message.
 
-v, --verbose [level 0-4]
:   Tell the command to be more verbose about its execution.

-t, --type [classification, timeseries, regression, unlabelled]
:   Force the interpretation of the input format to be one of the list.

# EXAMPLES

    echo "" | grt predict my-model.hmm
    ...
