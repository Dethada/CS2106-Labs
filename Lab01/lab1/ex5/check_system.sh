#!/bin/bash

####################
# Lab 1 Exercise 5
# Name: David Zhu, Tan Yuan Wei
# Student No: A0253167A, A0235417H
# Lab Group: B01, B14
####################

# Fill the below up
hostname=$(uname -n)

# -s to get OS
# -m to get hardware architecture
machine_hardware=$(echo "$(uname -s) $(uname -m)")

max_process_id=$(cat /proc/sys/kernel/pid_max)

# x to include background processes
# o user to reduce unnessary output
user_process_count=$(ps -xo uid --no-headers|wc -l)

user_with_most_processes=$(ps -eo user --sort uid --no-headers|uniq -c|sort -nr|awk 'NR==1 {print $2}')

mem_free_percentage=$(free|awk 'NR==2 {print $4/$2*100}')

echo "Hostname: $hostname"
echo "Machine Hardware: $machine_hardware"
echo "Max Process ID: $max_process_id"
echo "User Processes: $user_process_count"
echo "User With Most Processes: $user_with_most_processes"
echo "Memory Free (%): $mem_free_percentage"
