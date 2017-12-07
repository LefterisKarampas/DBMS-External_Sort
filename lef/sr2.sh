rm log.txt sorted_* unsorted_data.db temp
make
./build/sr_main1 > /dev/null
./build/sr_main2 > log.txt