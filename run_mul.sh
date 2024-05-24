#!/bin/bash

# Compile the program
make lab05_catapang

# Loop through terminals
for i in {1..2}; do
  # Open a new terminal, run your program, redirect the last line of output to a temporary file, and keep the terminal open
  xterm -fn fixed -e "bash -c './lab05_catapang 1 | tee >(tail -n 1 >> slave_tmp.txt); exec bash'" &
done

# Run lab05_catapang with argument 0 and redirect the last line of output to master.txt
./lab05_catapang 0 | tail -n 1 >> master.txt

# Concatenate the outputs into a single line using tabs
tr '\n' '\t' < slave_tmp.txt >> slave.txt

# Clean up the temporary file
rm slave_tmp.txt

# Add a newline character at the end of slave.txt for formatting
echo >> slave.txt