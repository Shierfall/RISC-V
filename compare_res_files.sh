#!/bin/bash

# compare_res_files.sh
# Usage: ./compare_res_files.sh <directory1> <directory2>
# Description: Compares .res files with matching names in two directories using 02155_check_output.sh

# Check if exactly two arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <directory1> <directory2>"
    exit 1
fi

DIR1="$1"
DIR2="$2"

# Check if the first directory exists and is a directory
if [ ! -d "$DIR1" ]; then
    echo "Error: Directory '$DIR1' does not exist."
    exit 1
fi

# Check if the second directory exists and is a directory
if [ ! -d "$DIR2" ]; then
    echo "Error: Directory '$DIR2' does not exist."
    exit 1
fi

# Iterate over each .res file in the first directory
for FILE1 in "$DIR1"/*.res; do
    # Check if there are no .res files in DIR1
    if [ ! -e "$FILE1" ]; then
        echo "No .res files found in '$DIR1'."
        break
    fi

    BASENAME=$(basename "$FILE1")
    FILE2="$DIR2/$BASENAME"

    if [ -e "$FILE2" ]; then
        echo "Comparing '$BASENAME'..."
        # Invoke the comparison script
        ./02155_check_output.sh "$FILE1" "$FILE2"
    else
        echo "Warning: '$BASENAME' does not exist in '$DIR2'."
    fi
done

# Optionally, check for .res files in DIR2 that are not in DIR1
for FILE2 in "$DIR2"/*.res; do
    # Check if there are no .res files in DIR2
    if [ ! -e "$FILE2" ]; then
        echo "No .res files found in '$DIR2'."
        break
    fi

    BASENAME=$(basename "$FILE2")
    FILE1="$DIR1/$BASENAME"

    if [ ! -e "$FILE1" ]; then
        echo "Warning: '$BASENAME' does not exist in '$DIR1'."
    fi
done
