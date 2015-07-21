% grt-predict
% 
% 

# NANE

 grt-predict - predict the class of a data sequence

# SYNOPSIS
 grt predict [-h] [-v|--verbose \<level\>] [-t|--type \<input-type\>] \<classification-model\> 
             [-s|--segment \<N\>] [-p|--passthrough] [input-file]

# DESCRIPTION
 This program predict the class label of unseen data according to the model stored in the classification model file. The output will be a tab-separated list of the label given in the input file and the predicted label. If the input data is unlabelled the output is undefined.

 The output of this file can be directly piped to the *grt score* command for further examination.

# OPTIONS

-h, --help
:   Print a help message.
 
-v, --verbose [level 0-4]
:   Tell the command to be more verbose about its execution.

-s, --segment [N]
:   greedy confidence segmentation, capture a window of at least N samples and consume samples until the classifier rejects the window as NULL, defaults to 0 which disables this

-p, --passthrough
:   enable pass-though mode for greedy segmentation mode, labels are a combination of groundtruth and current prediction printed with each line of data

# EXAMPLES

    
    
