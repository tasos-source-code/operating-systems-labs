#!/bin/bash

loads=(10 20 30 40 50 100)
pages=(10 100 1000 10000 100000)

echo "---------------------------------------------------------------------------------"
echo "The test checks if the all the accessed pages have young_bit = 1,"
echo "The test checks if the all the pages have pressnet bit = 1,"
echo "The test if all the written pages have dirty bit = 1 (50% of the accessed pages)."
echo "for different load and number of pages"
echo "---------------------------------------------------------------------------------"

for page in "${pages[@]}"
do
    echo "------------------------------------------"
    echo "Testing Page Group: Number of pages=$page"
    echo "------------------------------------------"
    for load in "${loads[@]}"
    do
        # echo ">> Executing: ./wset_monitor $load $page"
        ./wset_monitor $load $page
    done
done

echo "All tests completed."