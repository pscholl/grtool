 We can also just pack in a subtitle, but need to write this into an intermediate file

    echo "abc 1 2 3
    > NULL 1 2 3
    > abc 1 2 3
    > NULL 1 2 3" > labels &&  grt pack -l labels -r 50 -n test pack-r2.mkv && ffmpeg -i pack-r2.mkv -f srt -
    1
    00:00:00,000 --> 00:00:00,020
    abc
    
    2
    00:00:00,020 --> 00:00:00,040
    NULL
    
    3
    00:00:00,040 --> 00:00:00,060
    abc
    
    4
    00:00:00,060 --> 00:00:00,080
    NULL
    

 And we can make sure that empty lines are handled correctly

    echo "abc 1 
    > abc 2
    >    
    > cde 3" > labels && grt pack -l labels -r 50 -n test -i labels -r 50 -n test pack-r2.mkv && grt unpack pack-r2.mkv
    abc 1.000000 
    abc 2.000000 
    cde 3.000000 


 Ah this should fail in a reasonable way:

   echo "1 2
   > 2 1" | grt pack -i - - | grt unpack -
