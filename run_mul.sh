#!/bin/bash

exename="lab5_int"
config="config.txt"
slaves=2

# Compile the program
make "$exename"

xterm_master() {
  echo "running master"
  xterm -sb -fn fixed -e "bash -c 'stdbuf -oL ./$exename 0 $config | tee master.tmp'; tail -n 1 master.tmp >> master.txt; rm master.tmp;" &
  # ./$exename 0 $config | tail -n 1 >> master.txt &
} 

for ((i = 0; i < slaves; i++)); do
  echo "running slave $i"
  xterm -sb -fn fixed -e "bash -c './$exename 1 $config | tee >(tail -n 1 >> slave_tmp.txt)'" &
  # ./$exename 1 $config | tail -n 1 >> slave_tmp.txt &
done

xterm_master

echo "waiting to finish all instances"

wait

echo "copying time elapsed to txt file"
if [ -f slave_tmp.txt ]; then
  tr '\n' '\t' < slave_tmp.txt >> slave.txt
  echo >> slave.txt
  rm slave_tmp.txt
else
  echo "No output from slaves. Check for errors."
fi
