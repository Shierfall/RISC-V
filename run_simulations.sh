#!/bin/bash

# Script: run_simulations.sh
INPUT_DIR="./bin_tests"
OUTPUT_DIR="./res_outputs"

# Override default directories if arguments are provided
if [ $# -ge 1 ]; then
    INPUT_DIR="$1"
fi

if [ $# -ge 2 ]; then
    OUTPUT_DIR="$2"
fi
if [ ! -d "$INPUT_DIR" ]; then
    echo "Error: Input directory '$INPUT_DIR' does not exist."
    exit 1
fi

# Create the output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Counter for processed files
count=0

# Iterate over each .bin file in the input directory
for bin_file in "$INPUT_DIR"/*.bin; do
    # Check if any .bin files exist
    if [ ! -e "$bin_file" ]; then
        echo "No .bin files found in '$INPUT_DIR'."
        break
    fi

    # Extract the base name without extension
    base_name=$(basename "$bin_file" .bin)

    echo "------------------------------------------------------------"
    echo "Processing: $base_name.bin"

    # Run the simulator with the current .bin file
    ./riscv_simulator "$bin_file"

    # Check if the simulator executed successfully
    if [ $? -ne 0 ]; then
        echo "Error: Simulator failed for '$base_name.bin'. Skipping..."
        continue
    fi

    # Check if register_dump.res was generated
    if [ ! -f "register_dump.res" ]; then
        echo "Error: 'register_dump.res' not found after running simulator for '$base_name.bin'."
        continue
    fi

    # Define the new .res filename
    res_file="$OUTPUT_DIR/$base_name.res"

    # Move and rename the register_dump.res to the output directory
    mv "register_dump.res" "$res_file"

    # Verify that the move was successful
    if [ $? -eq 0 ]; then
        echo "Success: Saved register dump to '$res_file'."
        count=$((count + 1))
    else
        echo "Error: Failed to move 'register_dump.res' to '$res_file'."
    fi
done

echo "------------------------------------------------------------"
echo "Simulation complete. $count .res files saved in '$OUTPUT_DIR'."
