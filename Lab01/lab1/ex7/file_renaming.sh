#!/bin/bash

####################
# Lab 1 Exercise 7
# Name: David Zhu, Tan Yuan Wei
# Student No: A0253167A, A0235417H
# Lab Group: B01, B14
####################

####################
# Strings that you may need for prompt (not in order)
####################
# Enter $N numbers:
# Enter NEW prefix (only alphabets):
# Enter prefix (only alphabets):
# INVALID
# Number of files to create:
# Please enter a valid number [0-9]:
# Please enter a valid prefix [a-z A-Z]:

re_alpha='^[a-zA-Z]+$'
re_numeric='^[0-9]+$'
result=''

read_alpha () {
    read result

    if ! [[ $result =~ $re_alpha ]] ; then
        echo "INVALID"
        echo "Please enter a valid prefix [a-z A-Z]:"
        read result
    fi
}

read_numeric () {
    read result

    if ! [[ $result =~ $re_numeric ]] ; then
        echo "INVALID"
        echo "Please enter a valid number [0-9]:"
        read result
    fi
}

#################### Steps ####################

# Fill in the code to request user for the prefix
echo "Enter prefix (only alphabets):"
read_alpha
prefix=$result

# Enter numbers and create files #
echo "Number of files to create:"
read_numeric
num_files=$result

echo "Enter $num_files numbers:"
i=0
while [[ $i -lt $num_files ]];
do
    read_numeric
    suffix=$result
    touch "${prefix}_${suffix}.txt"
    let i=i+1
done

echo ""
echo "Files Created"
ls "${prefix}_"*.txt
echo ""

# Fill in the code to request user for the new prefix
echo "Enter NEW prefix (only alphabets):"
read_alpha
new_prefix=$result

# Renaming to new prefix #
for f in "${prefix}_"*.txt;
do
    mv "$f" "$(printf '%s\n' "$f" | sed "s/$prefix/$new_prefix/")"
done

echo ""
echo "Files Renamed"
ls "${new_prefix}_"*.txt
echo ""
