% grt-info
% 
% 

# NAME

 grt-info - print information about a data sequence

# SYNOPSIS
 grt info [-h|--help] [-v|--verbose \<level\>] [-t, --type <classification,timeseries,regression,unlabelled>] [input-file]

# DESCRIPTION
 This programs prints various statistics about the supplied data sequence. If no input-file is given, data is read from standard input. Statistics include the class label mapping, number of samples in each class, length of samples, dimension and data ranges.

 The type of input can be switched, per default it will be interpreted as classification input or the first comment line will be used, for more details see the INPUT section of the grt manpage.

# OPTIONS
-h, --help
:   Print a help message.
 
-v, --verbose [level 0-4]
:   Tell the command to be more verbose about its execution.

-t, --type [classification, timeseries, regression, unlabelled]
:   Force the interpretation of the input format to be one of the list.

# EXAMPLES

 Forcing the input type to be be a timeseries:

    echo "abc 1
    > cde 2
    >
    > cde 1
    > cde 2" | grt info -t timeseries
    DatasetName:	NOT_SET
    DatasetInfo:	
    Number of Dimensions:	1
    Number of Samples:	2
    Number of Classes:	1
    ClassStats:
    ClassLabel:	1	Number of Samples:	2	ClassName:	cde
    Dataset Ranges:
    [1] Min:	1	Max: 2
    Timeseries Lengths:
    ClassLabel: 1 Length:	2
    ClassLabel: 1 Length:	2

 or interpreted as the default classification type:

    echo "abc 1
    > cde 2
    >
    > cde 1
    > cde 2" | grt info -t timeseries

 or switched by a comment line

    echo "# timeseries
    > abc 1
    > cde 2
    >
    > cde 1
    > cde 2" | grt info 
