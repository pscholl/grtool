Weird input:

    echo -n "##abc\n(cde) abc def\n (cde) " | grt score -g -t Fbeta

a little less weird:

    echo -n "(a) a b" | grt score -g -t Fbeta
