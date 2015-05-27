% grt-pack
% 
% 

# NANE

 grt-pack - pack video, audio, sensor signals and labels as tracks into .mkv files

# SYNOPSIS
 grt pack [-h|--help] <file> [file] <output-file>
          [-t|--title] <\title\> [title] 

# DESCRIPTION
 This is a simple alias for ffmpeg to pack video/audio/subtitles and audio-encoded sensor data into one file. The actual command line can also be executed directly (and additional information can be added).

 Let's imagine you had a video.mp4 file with audio, and a sensor signal encoded as sensor.flac file and a subtitle file with labels. You can create one .mkv file out of that with the following command line:

    ffmpeg -i video.mp4 -i sensor.flac -i labels.ass
           -map 0:0 -map 0:1 -map 1:0 -map 2:0
           -c:a copy -c:v copy -c:s copy
           output.mkv

 This will copy all input data (instead of re-compressing with a different codec, this is what the -c specifier is for). The -map option is there to specify which stream are to be copied, in this case stream 0 and 1 from the video file (i.e. video and audio track), the sensor track and the subtitle track. All of those will be put into the output.mkv file. grt-pack is a shorthand for that.

# OPTIONS
-h, --help
:   Print a help message. 

-t, --title \<title\> [title]
:   add a title for each added stream, default: filename
 
# EXAMPLES
