#!/bin/bash
make
echo " "
echo "first task - useless: (output redirected to ./useless/out.txt)"
./build/useless <<< "./useless/test.txt" > "./useless/out.txt"

echo " "
echo "second task - backup: (test with gzip)"
./build/backup "./backup/first" "./backup/second" 

echo " "
echo "second task - backup: (test with handmade gzip)"
./build/backup "./backup/first" "./backup/second_empty" "2"

echo " "
echo "third task - runsim: (test with argument = 1)"
./build/runsim "1" < "./runsim/test1.txt"  

sleep 2
echo " "
echo "third task - runsim: (test with argument = 3)"
./build/runsim "3" < "./runsim/test3.txt" 

ipcrm --all=msg