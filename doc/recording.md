# Recording and Archiving Datasets

 Arguably one important step for every machine learning and activity recognition tasks is the recording of a dataset. Multiple media stream, including audio, video, motion, sensor and label data need to be recorded simultaneoulsy or merged post-experiment. This problem is not to be underestimated, as the format and clarity of such recordings have a direct influence on the quality of your research results, their re-usability and -producability and your own productivity as a developer. Luckily the MPEG-7 and Matroska standardization teams had these problem already in mind, which is why now ready-made tools can be used to organize your work with multi-media streams.

 One such tool is (ffmpeg)[https://ffmpeg.org], which we can use for the purpose of live recording datasets including various data streams. Merging them post-experiment if necessary and for adding labels to our data. All these datastreams can be merged into a single container file. This file can be streamed with multi-media protocol through a network, via pipes or simply stored on disk. One container format which can contain arbitrarily encoded data is the (matroska)[https://matroska.org] format. In this example we will look at each step separately. But first we will have to look at how to record arbitrary datastreams with ffmpeg.

## Recording Sensor Data with FFmpeg

 FFmpeg used on the command line supports input of data via pipes. This allows one or multiple processes to generate data and write to a pipe. FFmpeg can then pick each dataframe, encode it and transport it further. All we need todo is to tell ffmpeg where to pick up this data, which format it is in and at which rate it is being generated. To do this we will encode all data as an audio stream with multiple channels. This way we can use lossless audio compression utilities to save space.

 Let's assume we have a process that reads some sensor and prints the readings as a number of floats on the standard output of the program. We can then pipe this output into ffmpeg for further encoding:

   ./my-sensor-data | ffmpeg -f f32le -ac 2.0 -ar 50 -i pipe:0 test.mkv

## Capturing Video Simultaneously

   ./my-sensor-data | ffmpeg -f f32le -ac 2.0 -ar 50 -i pipe:0 -f v4l2 -i /dev/video0 -map 0:0 -map 1:0 test.mkv -map 1:0 -f xv window

## Recording, Streaming and Merging with Multiple Machines
