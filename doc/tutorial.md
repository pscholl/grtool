% grt tutorial
% 
% 

 This tutorial introduces some of the concepts of grtool. The goal of grtool is to provide a unix tool for gesture recognition, or other machine learning tasks. Currently all algorithms are based on the ones found in the Gesture Recognition Toolkit (TODO). For that grtool provides commands for basic gesture recognition tasks and one general command to find and exexute those commands. This tutorial assumes that you have basic knowledge of the unix commandline and piping mechanism, if not you should checkout documentation on those first [1,2]. Let's dive in.

# General command
 The command we will use most often is called 'grt', which can used to list all available commands and retrieve documentation:

    grt
    usage: grt <cmd> [options]
    available commands: 
     help[h] - prints this message or the help for the specified command
     info[i] - print stats about a dataset file
     train[t] - trains a prediction model
     train-dlib[td] - trains a prediction model, uses dlib multiclass machine learning trainers
     predict[p] - predict from unseen data
     predict-dlib[pd] - predict from unseen data, processes trainers created from train-dlib
     score[s] - calculate classifcation score for prediction
     extract[e] - extract features from a data sequence
     preprocess[pp] - preprocess data sequence
     postprocess[pop] - postprocess label streams
     plot[pl] - python based stream plotter
     montage[m] - python based montage plot
     segment[sg] - segments a list of samples into multiple timeseries
     pack[pa] - pack multiple streams into an .mkv file (using ffmpeg)
     unpack[u] - unpack first audio and subtitle stream from .mkv file (using ffmpeg)


 Executing for example 'grt help score', will retrieve documentation about the score sub-command. You can also use this command to execute any other command and get its usage documentation.

    grt info -h
    usage: info [options] ... [filename]...
    options:
      -t, --type       force input type (string [=classification]{classification,regression,timeseries,auto})
      -v, --verbose    verbosity level: 0-4 (int [=0])
      -h, --help       print this message

[1]: http://www.december.com/unix/tutor/pipesfilters.html
[2]: http://www.linfo.org/pipes.html


# NULL rejection for gesture recognition

 Having a classifier that is able to predict the possible gesture that has been performed is only the first step in building a gesture recognition systems.
 The problem of suppressing non-gestures or rather finding possible gestures in a stream of data is called gesture spotting.
 One solution for this problem is using classifiers that supports something called NULL rejection, that is rejecting data which does not contain any detectable gesture.
 A hidden markov model can eb exploited for that.
 By additionally learning the minimum probablity of a gesture during classification, a treshold can be set which rejects un-classifisable data.
 This is supported by a special label in the data stream, which is called "NULL".
 This label will not be learned as a classification output, but will be used to set the rejection treshold.

    echo "# just some random data
    > testing 1 
    > testing 1 
    > testing 1 
    > testing 1 
    > 
    > NULL 2
    > NULL 2
    > 
    > NULL 2
    > NULL 2
    > 
    > testing 1
    > testing 1" | grt train HMM -S 1 -n 2.0 | grt predict
    testing	testing
    NULL	NULL
    
