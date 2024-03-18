#!/bin/bash

# Check if the program binary exists
if [ ! -f "lab02_catapang" ]; then
    echo "Error: Program binary not found."
    exit 1
fi

# Check if the input file exists
if [ ! -f "lab02_input.txt" ]; then
    echo "Error: Input file not found."
    exit 1
fi

# Loop through each line in the input file and run the program with that line as input
while IFS= read -r line; do
    echo "Running with input: $line"
    echo "$line" | ./lab02_catapang >> lab02_output.txt  # Redirect output to a file
done < "lab02_input.txt"

echo "All runs completed."
