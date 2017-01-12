% grt-predict-skl
%
%

# NAME

grt-predict-skl

# SYNOPSIS
grt-predict-skl [-h] [-g] [-i] [-r X Y] [ESTIMATOR] [SAMPLES]

unsupervised clustering algorithms (estimators) from scikit-learn  
[predict module]

Default output: prediction stream with labels and predictions

# DESCRIPTION
Counterpart to the **_train-skl_** module, mostly for the use with GMM models. Other clusterers are supported as well but usage of this module is redundant since **_train-skl_** has the -p/--prediction option.

# OPTIONS
positional arguments:
-  ESTIMATOR            estimator dump, filename or stdin (default: -)
-  SAMPLES              sample stream, format: [label] [[features]] (default: -)

optional arguments:
-  -h, --help           show this help message and exit
-  -g, --graph          graph results of estimator fitting (default: False)
-  -i, --info           print estimator info and quit (default: False)
-  -r X Y, --retry X Y  retry loading model X times in Y-second intervals before giving up (default: (10, 1))

# EXAMPLES
