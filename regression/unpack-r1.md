This is an example where unpack failed to unpack the whole data, randomly stopping to early!

    grt unpack unpack-r1.mkv unpack-r1.srt | wc -l
    27411

and this should be double the amount

    grt unpack unpack-r1.mkv unpack-r1.srt unpack-r1.mkv unpack-r1.srt | wc -l
    54822
