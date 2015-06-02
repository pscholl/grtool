Weird input:

    echo -n "##abc\n(cde) abc def\n (cde) " | grt score -g -s Fbeta

a little less weird:

    echo -n "(a) a b" | grt score -g -s Fbeta

Clearly this should be an insertion and deletion

    echo "smoking NULL
    > NULL smoking" | grt score -n -c
    -------------------------------- ---- ---- ---- ----
                   D                  F    FM   M    C    M    FM   F                  I                
               50.000000             0.00 0.00 0.00 0.00 0.00 0.00 0.00            50.000000            
                   1                  0    0    0    0    0    0    0                  1                
                                                    ---- ---- ---- ---- --------------------------------

same here

    echo "NULL smoking
    > smoking NULL" | grt score -n -c
    -------------------------------- ---- ---- ---- ----
                   D                  F    FM   M    C    M    FM   F                  I                
               50.000000             0.00 0.00 0.00 0.00 0.00 0.00 0.00            50.000000            
                   1                  0    0    0    0    0    0    0                  1                
                                                    ---- ---- ---- ---- --------------------------------
  
and here

    echo "smoking NULL
    > smoking NULL
    > NULL smoking
    > NULL smoking" | grt score -n -c
    -------------------------------- ---- ---- ---- ----
                   D                  F    FM   M    C    M    FM   F                  I                
               50.000000             0.00 0.00 0.00 0.00 0.00 0.00 0.00            50.000000            
                   1                  0    0    0    0    0    0    0                  1                
                                                    ---- ---- ---- ---- --------------------------------
  
 this is fragmented and merged

    echo "smoking smoking
    > smoking smoking
    > smoking smoking
    > smoking smoking
    > smoking smoking
    > smoking smoking
    > smoking NULL   
    > smoking smoking
    > smoking smoking
    > smoking smoking" | grt score -n -c
    ---- ---------------------- ---- ---- ----
     D             F             FM   M    C    M    FM                      F                      I  
    0.00       33.333336        0.00 0.00 0.00 0.00 0.00                 66.666672                 0.00
     0             1             0    0    0    0    0                       2                      0  
                                          ---- ---- ---- ----------------------------------------- ----

    
  

