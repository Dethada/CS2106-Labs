#!/bin/bash

####################
# Lab 1 Exercise 6
# Name: David Zhu, Tan Yuan Wei
# Student No: A0253167A, A0235417H
# Lab Group: B01, B14
####################

echo "Printing system call report"

# Compile file
gcc -std=c99 pid_checker.c -o ex6

# Use strace to get report
strace -c ./ex6
