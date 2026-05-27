#!/bin/bash

IN_FOLDER=$1
OUT_FOLDER=$2
TODO=$3

while read file; do
    # echo $file
    FILE_NAME=${file/$IN_FOLDER}
    OUT_FILE=${FILE_NAME/".smt2"}.npz
    timeout 30m python3 treewalker.py --file $file --out ${OUT_FOLDER}${OUT_FILE}
done < $TODO
