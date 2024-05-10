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
output_file="${3:-output.csv}"  # Use third argument if provided, otherwise default to "output.csv"

# Check if the output file exists, if not, create it with header
if [ ! -f "$output_file" ]; then
    echo -e "INPUT\tRun 1\tRun 2\tRun 3" > "$output_file"
fi

# Loop through each line in the input file
while IFS= read -r line; do
   # Remove trailing whitespace from the input line
    rline=$(echo "$line" | sed -e 's/[[:space:]]*$//')

    # Run the program three times with the input line and store the results
    echo "Running with input: $line"
    # Run the program three times with the input line and store the results
    echo "1st run"
    result1=$(echo "$line" | ./"$1")
    echo "2nd run"
    result2=$(echo "$line" | ./"$1")
    echo "3rd run"
    result3=$(echo "$line" | ./"$1")
    
    # Write input and results to the output file as CSV format
    echo -e "$rline\t$result1\t$result2\t$result3" >> "$output_file"
done < "$2"

echo "CSV output appended to $output_file"
