Not so much a regression but rather checking the output format of the subtitle conversion

    echo "abc 1 2 3
    > NULL 1 2 3
    > abc 1 2 3
    > NULL 1 2 3" | grt pack -r 50 -n test pack-r2.mkv && ffprobe pack-r2.mkv -of json -show_streams
    {
        "streams": [
            {
                "index": 0,
                "codec_name": "wavpack",
                "codec_long_name": "WavPack",
                "codec_type": "audio",
                "codec_time_base": "1/50",
                "codec_tag_string": "[0][0][0][0]",
                "codec_tag": "0x0000",
                "sample_fmt": "fltp",
                "sample_rate": "50",
                "channels": 3,
                "channel_layout": "3.0",
                "bits_per_sample": 0,
                "r_frame_rate": "0/0",
                "avg_frame_rate": "0/0",
                "time_base": "1/1000",
                "start_pts": 0,
                "start_time": "0.000000",
                "disposition": {
                    "default": 1,
                    "dub": 0,
                    "original": 0,
                    "comment": 0,
                    "lyrics": 0,
                    "karaoke": 0,
                    "forced": 0,
                    "hearing_impaired": 0,
                    "visual_impaired": 0,
                    "clean_effects": 0,
                    "attached_pic": 0
                },
                "tags": {
                    "NAME": "test",
                    "ENCODER": "Lavc56.60.100 wavpack",
                    "DURATION": "00:00:00.080000000"
                }
            },
            {
                "index": 1,
                "codec_name": "srt",
                "codec_long_name": "SubRip subtitle",
                "codec_type": "subtitle",
                "codec_time_base": "1/1000",
                "codec_tag_string": "[0][0][0][0]",
                "codec_tag": "0x0000",
                "r_frame_rate": "0/0",
                "avg_frame_rate": "0/0",
                "time_base": "1/1000",
                "start_pts": 0,
                "start_time": "0.000000",
                "duration_ts": 80,
                "duration": "0.080000",
                "disposition": {
                    "default": 1,
                    "dub": 0,
                    "original": 0,
                    "comment": 0,
                    "lyrics": 0,
                    "karaoke": 0,
                    "forced": 0,
                    "hearing_impaired": 0,
                    "visual_impaired": 0,
                    "clean_effects": 0,
                    "attached_pic": 0
                },
                "tags": {
                    "NAME": "test",
                    "ENCODER": "Lavc56.60.100 srt",
                    "DURATION": "00:00:00.060000000"
                }
            }
        ]
    }


 But also the srt should be formatted correctly

    echo "abc 1 2 3
    > NULL 1 2 3
    > abc 1 2 3
    > NULL 1 2 3" | grt pack -r 50 -n test pack-r2.mkv && ffmpeg -i pack-r2.mkv -f srt -
    1
    00:00:00,000 --> 00:00:00,020
    abc
    
    2
    00:00:00,020 --> 00:00:00,040
    NULL
    
    3
    00:00:00,040 --> 00:00:00,060
    abc
    
