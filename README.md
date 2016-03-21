# grtool - command line interface to the Gesture Recognition Toolkit

[![Build Status](https://travis-ci.org/pscholl/grtool.svg)](https://travis-ci.org/pscholl/grtool)

 A command line utility similar to [mlpack](mlpack.org), [weka](http://www.cs.waikato.ac.nz/ml/weka/) and [scikit-learn](scikit-learn.org) for doing gesture recognition. Contains a number of utility which can be put together with a standard unix pipeline to handle the task of training and running machine learning algorithms. The data format is meant for enabling quick trials.

 All data frames are given in textual format, in which each line presents a frame for classification and needs to be prepended with a label:

    NULL 1 2 3
    abc  1.2 3 4
    NULL 0 0 0
    ...

 This designates three frames for classification labelled differently, each contained a three-dimensional feature vector. This can be piped directly for training:

    echo "NULL 1 2 3
    abc  1.2 3 4
    NULL 0 0 0" | grt train RandomForests -o tt.rf

 This command trains a randomforest on this dataset and puts the learned model into tt.rf. See the [wiki](https://github.com/pscholl/grtool/wiki) and man-pages for more information.

## Building

grtool requires of other projects to run properly.

  - ffmpeg
  - python3
  - a recent version of gcc
  - Gesture Recognition Toolkit, please use this https://github.com/pscholl/grt

After downloading the source code install with

    make PREFIX=/usr install

and install the documentation (needs pandoc) with

    make PREFIX=/usr install-doc

## Testing

 You can run the doc-test on your machine with

   make DESTDIR=test test

 
## Android

http://kevinboone.net/android_native.html
