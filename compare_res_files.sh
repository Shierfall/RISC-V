#!/bin/bash

# Usage: ./compare_res_files.sh <directory1> <directory2>

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <directory1> <directory2>"
    exit 1
fi
DIR1="$1"
DIR2="$2"
if [ ! -d "$DIR1" ]; then
    echo "Error: Directory '$DIR1' does not exist."
    exit 1
fi
if [ ! -d "$DIR2" ]; then
    echo "Error: Directory '$DIR2' does not exist."
    exit 1
fi
for FILE1 in "$DIR1"/*.res; do
    if [ ! -e "$FILE1" ]; then
        echo "No .res files found in '$DIR1'."
        break
    fi
    BASENAME=$(basename "$FILE1")
    FILE2="$DIR2/$BASENAME"
    if [ -e "$FILE2" ]; then
        echo "Comparing '$BASENAME'..."
        ./02155_check_output.sh "$FILE1" "$FILE2"
    else
        echo "Warning: '$BASENAME' does not exist in '$DIR2'."
    fi
done
for FILE2 in "$DIR2"/*.res; do
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
