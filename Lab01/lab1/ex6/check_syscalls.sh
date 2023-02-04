#!/bin/bash

####################
# Lab 1 Exercise 6
# Name: David Zhu, Tan Yuan Wei
# Student No: E0958755, E0727417
# Lab Group: B01, B14
####################

echo "Printing system call report"

# Compile file
gcc -std=c99 pid_checker.c -o ex6

# Use strace to get report
strace -c ./ex6
