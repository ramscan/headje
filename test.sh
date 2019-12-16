#!/bin/sh
echo > test.txt "test text file"
sleep 2; echo "Line 2" | cat - test.txt > test2.txt && mv test2.txt test.txt
sleep 2; echo "Line 3" | cat - test.txt > test2.txt && mv test2.txt test.txt
sleep 2; echo "Line 4" | cat - test.txt > test2.txt && mv test2.txt test.txt
sleep 2; echo "press q to quit" >> test.txt
