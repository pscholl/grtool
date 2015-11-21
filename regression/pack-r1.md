This should not hang the pack program

    timeout 15. grt pack -i pack-r1.grt -n test -r 50 pack-r1.mkv

this should also work:

    cat pack-r1.grt | awk -e '{print $2" "$3" "$4}'| grt pack -n accl -r 50  test.mkv

