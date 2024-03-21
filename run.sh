#!/bin/bash

# Check if the program binary exists
if [ ! -f "$1" ]; then
    echo "Error: Program binary not found."
    exit 1
fi

# Check if the input file exists
if [ ! -f "$2" ]; then
    echo "Error: Input file not found."
    exit 1
fi

# Set output file
output_file="${3:-output.txt}"  # Use third argument if provided, otherwise default to "lab02_output.txt"

# Loop through each line in the input file and run the program with that line as input
while IFS= read -r line; do
    echo "Running with input: $line"
    echo "$line" | ./"$1" >> "$output_file"  # Redirect output to the specified file
done < "$2"

echo "All runs completed."
