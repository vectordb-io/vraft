#!/bin/bash

for file in `ls remu_*_test`; do
    echo "Running $file..."
    ./"$file" 
done

for file in `ls remu_*_test`; do
    echo "Running $file..."
    ./"$file --enable-pre-vote" 
done