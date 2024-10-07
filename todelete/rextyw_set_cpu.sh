#!/bin/bash

# Prompt user for the name of the process to look up
read -p "Enter the name of the process you want to query: " process_name

# Use pgrep to find the PID of that process name
pid=$(pgrep -f "$process_name")

# Check if the PID was found
if [ -z "$pid" ]; then
    echo "No process found for $process_name"
    exit 1
fi

# Display the found PID
echo "Found PID for the process: $pid"

# Prompt user for the CPU number they wish to assign the process to
read -p "Enter the CPU number you wish to assign to this process (e.g.: 0,1,2...): " cpu_num

# Use taskset to assign the process to a specific CPU
taskset -cp $cpu_num $pid

# Output result
if [ $? -eq 0 ]; then
    echo "Successfully assigned process $pid to CPU $cpu_num"
else
    echo "Failed to assign process to CPU"
fi

