#!/bin/bash
make clean
make
for f in *.in;
do
    echo "Testing with $f"
    ./ex3 "$f" | diff -q -b --strip-trailing-cr "${f/.in/}.out" -
    if [ $? -ne 0 ]; then
        echo "Failed";
    fi
done
