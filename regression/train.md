do not fail on empty input

    echo "" | grt train HMM -o abc

we'd also like n-dimensional input

    echo "#timeseries
    > inverting 1 1
    > inverting 1 1 
    > inverting 1 1 
    > 
    > inverting 2 2
    > inverting 2 2
    > inverting 2 2" | grt train cHMM -o test.hmm

we'd also like n-dimensional input and DTW

    echo "#timeseries
    > inverting 1 1
    > inverting 1 1 
    > inverting 1 1 
    > 
    > inverting 2 2
    > inverting 2 2
    > inverting 2 2" | grt train DTW -o test.DTW

this is also "bad" input

    echo "NULL 0
    > NULL 0
    > NULL 0" | grt train RandomForests -o test.rf
