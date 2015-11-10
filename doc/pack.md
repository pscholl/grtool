% grt-pack
% 
% 

# NANE

 grt-pack - pack video, audio, sensor signals and labels as tracks into .mkv files

# SYNOPSIS
 grt pack [-h|--help] [-i|--input] <file> [-n|--name] <\name\>
          [-r|--rate] <\rate\> [-m|--media] <\media\> [-v|--verbose]
          [-ss|--seek] <\offset\> [-l|--label] <\subtitle\>
          <output-file>

# DESCRIPTION

 This is a wrapper for ffmpeg to encode data files from the GRT File Format into a subtitle and audio stream. This can be used for compression and storage of data files. grt-pack will take multiple -i switches, accompanied by -r and -n switches which define the name tag and recording rate of the sensor data. A subtitle stream will be created from the given labels, and an audio stream from the data fields. Those will be encoded into an .mkv file.

 This can also be done manually with ffmpeg. Let's imagine you had a video.mp4 file with audio, and a sensor signal encoded as sensor.flac file and a subtitle file with labels. You can create one .mkv file out of that with the following command line:

   ffmpeg -i video.mp4 -i sensor.flac -i labels.ass
          -map 0:0 -map 0:1 -map 1:0 -map 2:0
          -c:a copy -c:v copy -c:s copy
          output.mkv

 Which is useful if your data is already encoded as an audio track.

# LIMITATIONS

 The maximum number of data fields is 6, since they are encoded as channels in audio stream and ffmpeg cannot handle more than that.

# OPTIONS

-h, --help
:   Print a help message. 

-i INPUT, --input INPUT
:   grt input file to convert to wavpak/ass file, multiple can be given, if none given defaults to stdin

-n NAME, --name NAME
:   names for the input files to be put into the mkv file

-r RATE, --rate RATE
:   a rate (in Hz) must be given for each input

-m MEDIA, --media MEDIA
:   additional file to add, multiple can be given

-ss OFFSET, --seek OFFSET
:   seek media before adding

-l LABEL, --label LABEL
:   addtional subtitle file to be added to ffmpeg

-v, --verbose
:   more verbose output
 
# EXAMPLES

    echo "abc 1 1 2
    > abc 2 2 2
    > cde 3 4 5
    > tfe 6 7 8" | grt pack -r 2 -n acceleration test.mkv
    
