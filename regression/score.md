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

    
  
this is a regression

    grt score score-r1.data
    None      NULL   smoking 
    -------- ------ --------- 
    NULL      200      15    
    smoking   181      16    
    -------- ------ --------- 
    
    None           recall          precision           Fbeta              NPV               TNR        
    -------- ----------------- ----------------- ----------------- ----------------- -----------------
    NULL          0.524934          0.930233          0.671141          0.081218          0.516129     
    smoking       0.516129          0.081218          0.140351          0.930233          0.524934     
             0.520532/0.004402 0.505725/0.424507 0.405746/0.265395 0.505725/0.424507 0.520532/0.004402 
    
    ---- ----- ---- ---- -----
     D     F    FM   M     C    M    FM    F                              I                           
    0.00 2.531 0.00 0.00 2.531 0.00 0.00 5.0632                       89.873413                       
     0     2    0    0     2    0    0     4                              71                          
                         ----- ---- ---- ------ ------------------------------------------------------

