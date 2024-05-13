#!/bin/bash

# Loop through terminals
for i in {1..8}; do
    # Open a new terminal and run your program
    xterm -fn fixed -e "./lab04_catapang 1" &
done

# Run lab04_catapang with argument 0 and redirect output to output.txt
./lab04_catapang 0 >> output.txt
