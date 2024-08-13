#!/bin/sh
awk '{
    # split the line by space into an array
    split($0, arr, " ")
    # remove "->" from the second element
    gsub(/->/, "", arr[2])
    # remove the first ":" from the third element
    sub(":", "", arr[3])
    # combine everything together with spaces
    output = arr[1] " " arr[2] " " arr[3]
    for(i=4; i<=NF; i++) {
        output = output " " $i
    }
    print output
}' 

