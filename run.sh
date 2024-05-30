#!/bin/bash

# Define the executable name and configuration file
exename="lab04_catapang"
config="config.txt"
slaves=2

# Compile the program
make "$exename"

# Function to run the program in an xterm and keep the window open
xterm_slave() {
    xterm -fn fixed -e "bash -c './$exename 1 $config'; exec bash" &
}

# Function to run the program in an xterm, redirect output to a temporary file, and keep the window open
xterm_master() {
    xterm -fn fixed -e "bash -c 'stdbuf -oL ./$exename 0 $config | tee master.tmp'; tail -n 1 master.tmp > master.txt; rm master.tmp; exec bash" &
}

# Loop through terminals for slave processes
for ((i = 0; i < slaves; i++)); do
    echo "Running slave $i"
    xterm_slave
done

# Run the executable with argument 0 and capture the output in master.txt
echo "Running master"
xterm_master
