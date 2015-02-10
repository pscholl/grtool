Weird input:

    echo -n "##abc\n(cde) abc def\n (cde) " | grt score -g -s Fbeta

a little less weird:

    echo -n "(a) a b" | grt score -g -s Fbeta
