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
     train[tr] - trains a prediction model
     predict[pr] - predict from unseen data
     score[s] - calculate classifcation score for prediction
     extract[e] - extract features from a data sequence
     preprocess[pp] - preprocess data sequence

 Executing for example 'grt help score', will retrieve documentation about the score sub-command. You can also use this command to execute any other command and get its usage documentation.

    grt info -h
    usage: info [options] ... [filename]...
    options:
      -t, --type       force classification, regression or timeseries input (string [=])
      -v, --verbose    verbosity level: 0-4 (int [=0])
      -h, --help       print this message

[1]: http://www.december.com/unix/tutor/pipesfilters.html
[2]: http://www.linfo.org/pipes.html

