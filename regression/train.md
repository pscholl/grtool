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

Unseen symbols during the training sequence should not lead to a critical
failure in the CRF model during prediction:

    echo "# timeseries
    > inverting 1
    > inverting 1
    >
    > inverting 2
    > testing 1
    > 
    > inverting 1
    > inverting 4
    >
    > inverting 1
    > inverting 1" | grt train CRF -n 2 -o test.crf
