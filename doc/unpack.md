% grt-pack
% 
% 

# NANE

 grt-unpack - merge an audio stream and subtitle file into a csv stream for classification

# SYNOPSIS
 grt unpack [-h|--help] \<file\> [file] [-t|--stream] \<streamid\> [streamid]
            [-s|--style] \<style\> [-d|--default] \<label\> [-ss] \<position\>
            [-r|--rate] \<rate\> [-sd] \<duration\>

# DESCRIPTION

 This is a wrapper around ffmpeg for unpacking sensor data for classification with the grt tools. In principle this program merges a subtitle stream with an audio stream, converts it to floats in text format and prints data in "label channel_1 channel_2 ... channel_n" format. 

 Files can be any subtitle file, audio and video file, anything ffmpeg can handle. Audio streams can be selected by their language or name identification tag, if neccesary. Otherwise all audio tracks will be unpacked. If multiple audio tracks exists in the input, they will be resampled to their highest common frequency with the sox audio resampling module. All audio tracks will be merged into one frame. A file can also be left out or specified as '-', in which the standard input is read as a file, which is useful for streaming.

 The matroska .mkv file format is advised to be used, since it can handle all types of streams, while keeping all streams organized.

# OPTIONS

-h, --help
:   Print a help message. 

-t, --stream \<streamid\> [streamid]
:   one or more stream id's matching the language tag to be used

-s, --style \<style\>
:   select only subtitles with this styles, default is to select all

-d, --default \<label\>
:   the label to emit when no label is in the subtitle, default: NULL

-ss \<position\>
:   seek to position before printing values, see ffmpeg-utils(1) Time Duration documentation for format

-r, --rate \<rate\>
:   set the sample rate at which to generate output data, internally the sox resampler of ffmpeg is used to achieve this
 
-sd \<duration\>
:   stop the stream after *duration*, see ffmpeg-utils(1) Time Duration documentation for format


# EXAMPLES

 This example will do a full round-trip using the pack utility, which packs data
into a .mkv file. First we will echo some data into the packing utility, setting
a rate of 2Hz. After this command has completed, unpack is used to unpack this
data again.

    echo "abc 1 1 2
    > abc 2 2 2
    > cde 3 4 5 
    > tfe 6 7 8" > data && grt pack -l data -i data -r 50 -n test test.mkv && grt unpack test.mkv
    abc 1.000000 1.000000 2.000000 
    abc 2.000000 2.000000 2.000000 
    cde 3.000000 4.000000 5.000000 
    tfe 6.000000 7.000000 8.000000 


 Here is an example of duration and stream seeking, which skips the first and
last of the 2Hz sampled signal. Actually the first line's label should be abc
not NULL, however this is an ffmpeg bug when seeking subtitles.

   echo "abc 1 1 2
   > abc 2 2 2
   > cde 3 4 5 
   > tfe 6 7 8" > data && grt pack -l data -i data -r 50 -n test test.mkv && grt unpack -ss 0.5 -sd 1 test.mkv
   NULL 2.000000 2.000000 2.000000 
   cde 3.000000 4.000000 5.000000 
