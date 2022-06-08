#!/bin/sh
#(
#echo "Examine Red Leather Chair"
#echo "quit"
#echo "q"
#echo "y"
#) | ./dMagnetic -ini dMagnetic.ini corruption >corruption_zwei.txt
#cat corrupt.txt | ./dMagnetic -ini dMagnetic.ini corruption >corruption_zwei.txt
#cat fish.txt | ./dMagnetic -ini dMagnetic.ini fish | tee fish_zwei.txt

cat wonderland3.txt | ./dMagnetic -ini dMagnetic.ini wonderland | tee wonderland_zwei.txt

