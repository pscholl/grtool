% grt-score
% 
% 

# NANE

 grt-score - score the output of a prediction

# SYNOPSIS
 grt score [-h|--help] [-c|--no-confusion] [-p|--no-precision] [-r|--no-recall]
           [-F|--F-score <beta>] [-g|--group] [-q|--quiet]
           [-t|--top-score <F1|recall|precision|disabled>] [input-file]

# DESCRIPTION
 Use this program to evaluate trained models. Given a list of prediction and ground truth labels it can calculate the confusion matrix, recall, prediction and Fbeta score of the prediction.

 Input can be given in two modes: tagged and untagged. In tagged each line must adhere to the following specification:

 (tag) label prediction

tag, prediction and label can be a string designating a group tag (which can also contain whitespace), a ground truth label and a predicted label. In tagged mode, switched via -g, all prediction with the same tag will be aggregated. This mode can be used for running multiple repetitions or folds in parallel. In untagged mode each line must adhere to this specification:

 label prediction

A ground truth label separated by whitespace from a prediction needs to be given on each line. This is the default behaviour. Lines starting with a pound sign (#) will be ignored, as well as lines that contain only whitespace.


# OPTIONS
-h, --help
:   Print a help message.

-q, --quiet
:   Do not print warning messages.
 
-c, --no-confusion
:   Do not report the confusion matrix.

-p, --no-precision
:   Do not report the precision score for each label, and its mean and standard deviation.

-r, --no-recall
:   Same as --no-precision but for the recall score.

-F, --F-score <beta>
:   Report the weighted mean of recall and precision. The weighting factor beta can be supplied and defaults to 1, supplying 0 disables reporting of the F-score.

-g, --group
:   Turn on tag-based aggregation, all input lines must be prefixed with (<tag>), where tag can be any string. In this mode all lines with the same tag are aggregated and reported as belonging to the same group. This is useful for scoring multiple predictions in the same run (see the examples section). If turned off, all lines are assumed to belong to the same group. Defaults to off.

-s, --sort <F1|recall|precision|disabled>
:   Report results for all groups (needs to be enabled with -g) in ascending order. Ordering parameter is the mean number of scoring parameter.

-i, --intermdiate
:   Report intermediate results, useful for piped operation, where the actual calculation takes a long time.

# EXAMPLES

## Single-Run Scoring

These are a few example on the input and output data of the scoring. Let's start with a simple example, which scores a single prediction. The output of a prediction is assumed to be made of a ground truth label and a prediction, which can be piped directly into the score program. Let's imagine that the output is coming from a swipe detector and that someone was observing ground truth labels, and that we have one prediction per line:

    echo "right_swipe right_swipe
    > right_swipe left_swipe
    > left_swipe  left_swipe
    > left_swipe  left_swipe" | grt score 
    None          right_swipe   left_swipe 
    ------------ ------------- ------------ 
    right_swipe        1                   
    left_swipe         1            2      
    ------------ ------------- ------------ 
    
    None               recall          precision           Fbeta       
    ------------ ----------------- ----------------- -----------------
    right_swipe       0.500000          1.000000          0.666667     
    left_swipe        1.000000          0.666667          0.800000     
                     0.75/0.25       0.833333/0.166    0.733333/0.066  

We have four lines of input, i.e. four predictions in total that need to be scored. They are made of a left_swipe, right_swipe label. The left hand side of the input is the ground truth label, which is gathered by a wizard-of-oz like system, and the right hand side is the prediction of the model. You can see that we have one misclassification on line two, which is reflected in the recall, precision and F-score, as well as in the confusion matrix. Reporting of some scores can be turned off by providing the corresponding parameter:

    echo "right_swipe right_swipe
    > right_swipe left_swipe
    > left_swipe  left_swipe
    > left_swipe  left_swipe" | grt score -r -p -F 0
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
    > (participant 0) left_swipe  left_swipe" | grt score -g -c
    participant 1        recall          precision           Fbeta       
    -------------- ----------------- ----------------- -----------------
    left_swipe                            0.000000                       
    right_swipe         0.666667          1.000000          0.800000     
                        0.666667          0.5/0.5           0.800000     
    
    participant 0        recall          precision           Fbeta       
    -------------- ----------------- ----------------- -----------------
    right_swipe         1.000000          1.000000          1.000000     
    left_swipe          1.000000          1.000000          1.000000     
                          1/0               1/0               1/0        

As you can see, results are now reported for each input tag after the complete input has been read. Also notice that the class-mean and standard deviation is reported at the bottom of each table.

## Sorting By Best Performing Group

 In most cases you like to select the best performing parameter set. For this you can switch on grouping with -g and sorting by mean recall, precision or Fbeta score. This can be done with the -s switch. For example grouping and sorting our example on its mean Fbeta score:

    echo "(participant 0) right_swipe right_swipe
    > (participant 1) right_swipe left_swipe
    > (participant 1) right_swipe right_swipe
    > (participant 1) right_swipe right_swipe
    > (participant 0) left_swipe  left_swipe
    > (participant 0) left_swipe  left_swipe" | grt score -g -s Fbeta -c
    participant 1        recall          precision           Fbeta       
    -------------- ----------------- ----------------- -----------------
    left_swipe                            0.000000                       
    right_swipe         0.666667          1.000000          0.800000     
                        0.666667          0.5/0.5           0.800000     
    
    participant 0        recall          precision           Fbeta       
    -------------- ----------------- ----------------- -----------------
    right_swipe         1.000000          1.000000          1.000000     
    left_swipe          1.000000          1.000000          1.000000     
                          1/0               1/0               1/0        

When reading input from a pipe, intermediate scores will also be reported, i.e. whenever the order changes it will be reported. When reading input from a file no intermediate scores will be reported.
