% grt-score
% 
% 

# NANE

 grt-score - score the output of a prediction

# SYNOPSIS
 grt score [-h|--help] [-c|--no-confusion] [-n|--no-score] [-F|--F-score <beta>]
           [-g|--group] [-q|--quiet] [-i|--intermediate] [-f||--flat]
           [-s|--sort <F1|recall|precision|NPV|TNR|disabled>] [input-file]

# DESCRIPTION
 Use this program to evaluate trained models. Given a list of prediction and ground truth labels it can calculate the confusion matrix, recall (TP/[TP+FN]), precision (TP/[TP+FP]), Fbeta score ([1+beta^2]*[precision*recall]/[[beta^2]*precision+recall]), the true negative rate (TNR, TN/[TN+FN]) and negative predictive value (NPV, TN/[FN+TN]) of the prediction. See https://en.wikipedia.org/wiki/Positive_and_negative_predictive_values for a detailed explanation of these values. Additionally an Event Analysis Diagram[1] most useful for continous activity recognition can be printed. 

 Input can be given in two modes: tagged and untagged. In tagged each line must adhere to the following specification:

 (tag) label prediction

tag, prediction and label can be a string designating a group tag (which can also contain whitespace), a ground truth label and a predicted label. In tagged mode, switched via -g, all prediction with the same tag will be aggregated. This mode can be used for running multiple repetitions or folds in parallel. In untagged mode each line must adhere to this specification:

 label prediction

A ground truth label separated by whitespace from a prediction needs to be given on each line. This is the default behaviour. Lines starting with a pound sign (#) will be ignored, as well as lines that contain only whitespace.


[1]: Ward, J., Lukowicz, P., & Gellersen, H. (2011). Performance metrics for activity recognition, 2(1), 1–23. doi:10.1145/1889681.1889

# OPTIONS
-h, --help
:   Print a help message.

-f, --flat
:   flat output instead of table-based, implies --no-confusion, useful for piping into grt plot

-q, --quiet
:   Do not print warning messages.
 
-c, --no-confusion
:   Do not report the confusion matrix.

-n, --no-score
:   Do not report any score.

-e, --no-ead
:   Suppress Event Analysis Diagram (EAD) output.

-F, --F-score <beta>
:   Specifies the beta value of the F-score, defaults to 1.

-g, --group
:   Turn on tag-based aggregation, all input lines must be prefixed with (<tag>), where tag can be any string. In this mode all lines with the same tag are aggregated and reported as belonging to the same group. This is useful for scoring multiple predictions in the same run (see the examples section). If turned off, all lines are assumed to belong to the same group. Defaults to off.

-s, --sort <F1|recall|precision|NPV|TNR|disabled>
:   Report results for all groups (needs to be enabled with -g) in ascending order. Ordering parameter is the mean number of scoring parameter.

-i, --intermdiate
:   Report intermediate results, useful for piped operation, where the actual calculation takes a long time.

# EXAMPLES

## Single-Run Scoring

These are a few example on the input and output data of the scoring. Let's start with a simple example, which scores a single prediction. The output of a prediction is assumed to be made of a ground truth label and a prediction, which can be piped directly into the score program. Let's imagine that the output is coming from a swipe detector and that someone was observing ground truth labels, and that we have one prediction per line:

    echo "right_swipe right_swipe
    > right_swipe left_swipe
    > left_swipe  left_swipe
    > left_swipe  left_swipe" | grt score -e
    None          right_swipe   left_swipe 
    ------------ ------------- ------------ 
    right_swipe        1                   
    left_swipe         1            2      
    ------------ ------------- ------------ 
    
    None               recall          precision           Fbeta              NPV               TNR        
    ------------ ----------------- ----------------- ----------------- ----------------- -----------------
    right_swipe       0.500000          1.000000          0.666667          0.500000          1.000000     
    left_swipe        1.000000          0.666667          0.800000          1.000000          0.500000     
                     0.75/0.25       0.833333/0.166    0.733333/0.066      0.75/0.25         0.75/0.25     

We have four lines of input, i.e. four predictions in total that need to be scored. They are made of a left_swipe, right_swipe label. The left hand side of the input is the ground truth label, which is gathered by a wizard-of-oz like system, and the right hand side is the prediction of the model. You can see that we have one misclassification on line two, which is reflected in the recall, precision and F-score, as well as in the confusion matrix. Reporting of the overall scores (and leaving only the confusion matrix) can be turned off.

    echo "right_swipe right_swipe
    > right_swipe left_swipe
    > left_swipe  left_swipe
    > left_swipe  left_swipe" | grt score -n -e
    None          right_swipe   left_swipe 
    ------------ ------------- ------------ 
    right_swipe        1                   
    left_swipe         1            2      
    ------------ ------------- ------------ 

In the last example, only the confusion matrix is reported.

## Aggregating Input Groups

Most often you want to test your machine learning model on multiple parameter and aggregate the output parameters (for example when doing cross-fold validation). grt-score supports this by allowing *grouped* input. For this each prediction can be prefixed with a tag by supplying a string in parenthesis at the start of a line. grt-score will then report the chosen scores for each group. Group tags can be given in any order.

    echo "(participant 0) right_swipe right_swipe
    > (participant 1) right_swipe left_swipe
    > (participant 1) right_swipe right_swipe
    > (participant 1) right_swipe right_swipe
    > (participant 0) left_swipe  left_swipe
    > (participant 0) left_swipe  left_swipe" | grt score -g -c -e
    participant 1        recall          precision           Fbeta              NPV               TNR        
    -------------- ----------------- ----------------- ----------------- ----------------- -----------------
    left_swipe                            0.000000                            1.000000          0.750000     
    right_swipe         0.666667          1.000000          0.800000          0.000000                       
                     0.333333/0.333       0.5/0.5           0.4/0.4           0.5/0.5         0.375/0.375    
    
    participant 0        recall          precision           Fbeta              NPV               TNR        
    -------------- ----------------- ----------------- ----------------- ----------------- -----------------
    right_swipe         1.000000          1.000000          1.000000          1.000000          1.000000     
    left_swipe          1.000000          1.000000          1.000000          1.000000          1.000000     
                          1/0               1/0               1/0               1/0               1/0        

As you can see, results are now reported for each input tag after the complete input has been read. Also notice that the class-mean and standard deviation is reported at the bottom of each table.

## Sorting By Best Performing Group

 In most cases you like to select the best performing parameter set. For this you can switch on grouping with -g and sorting by mean recall, precision or Fbeta score. This can be done with the -s switch. For example grouping and sorting our example on its mean Fbeta score:

    echo "(participant 0) right_swipe right_swipe
    > (participant 1) right_swipe left_swipe
    > (participant 1) right_swipe right_swipe
    > (participant 1) right_swipe right_swipe
    > (participant 0) left_swipe  left_swipe
    > (participant 0) left_swipe  left_swipe" | grt score -g -s Fbeta -c -e
    participant 1        recall          precision           Fbeta              NPV               TNR        
    -------------- ----------------- ----------------- ----------------- ----------------- -----------------
    left_swipe                            0.000000                            1.000000          0.750000     
    right_swipe         0.666667          1.000000          0.800000          0.000000                       
                     0.333333/0.333       0.5/0.5           0.4/0.4           0.5/0.5         0.375/0.375    
    
    participant 0        recall          precision           Fbeta              NPV               TNR        
    -------------- ----------------- ----------------- ----------------- ----------------- -----------------
    right_swipe         1.000000          1.000000          1.000000          1.000000          1.000000     
    left_swipe          1.000000          1.000000          1.000000          1.000000          1.000000     
                          1/0               1/0               1/0               1/0               1/0        

When reading input from a pipe, intermediate scores will also be reported, i.e. whenever the order changes it will be reported. When reading input from a file no intermediate scores will be reported.

## Using the Event Analysis Diagram

 For continous AR the classical statistic measure can be misleading. Ward et.al. proposed additional error measures which allow for a clearer picture of error on an event instead of frame level. These errors can include fragmentation, i.e. when multiple predictions (separated by NULL predictions) split the same ground truth label sequence:

    echo "NULL NULL
    > label NULL
    > label label
    > label NULL
    > label label
    > label NULL
    > label label
    > NULL  label
    > NULL  NULL" | grt score -n -c
    deletions: 0
    ev_fragmented: 1
    ev_fragmerged: 0
    ev_merged: 0
    correct: 0
    re_merged: 0
    re_fragmerged: 0
    re_fragmented: 3
    insertions: 0

 Another possible source of non-serious error, are timing errors. These describe cases where the boundaries of ground truth labels were missed slightly.

    echo "NULL NULL
    > label NULL
    > label label
    > label NULL
    > NULL  label" | grt score -n -c
    deletions: 0
    ev_fragmented: 0
    ev_fragmerged: 0
    ev_merged: 0
    correct: 1
    re_merged: 0
    re_fragmerged: 0
    re_fragmented: 0
    insertions: 1

 These are also knows as under- and overfills.

    echo "NULL label
    > label label" | grt score -n -c
    deletions: 0
    ev_fragmented: 0
    ev_fragmerged: 0
    ev_merged: 0
    correct: 1
    re_merged: 0
    re_fragmerged: 0
    re_fragmented: 0
    insertions: 0

  Deletions are errors, where the ground truth event was not detected at all.

    echo "NULL NULL
    > label NULL
    > NULL NULL" | grt score -n -c 
    deletions: 1
    ev_fragmented: 0
    ev_fragmerged: 0
    ev_merged: 0
    correct: 0
    re_merged: 0
    re_fragmerged: 0
    re_fragmented: 0
    insertions: 0
 
 A further source of errors are merges, denoting cases in which two or more ground truth events where predicted as only one event. Put it a different way, the predictor did not correctly separate the ground truth event. These events are counted separately for ground truth and prediction.

    echo "label label
    > NULL label
    > label label" | grt score -n -c
    deletions: 0
    ev_fragmented: 0
    ev_fragmerged: 0
    ev_merged: 2
    correct: 0
    re_merged: 1
    re_fragmerged: 0
    re_fragmented: 0
    insertions: 0

 And last but not least there is a combination of fragmentation and merging errors.

    echo "label label
    > label label
    > NULL  label
    > label label
    > label NULL
    > label label
    > label NULL
    > label label" | grt score -n -c 
    deletions: 0
    ev_fragmented: 0
    ev_fragmerged: 2
    ev_merged: 0
    correct: 0
    re_merged: 0
    re_fragmerged: 3
    re_fragmented: 0
    insertions: 0


