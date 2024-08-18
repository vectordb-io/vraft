#!/bin/sh

# Loop through all files in the current directory
for file in *; do
    # Check if the file name ends with "_test"
    case "$file" in
        *_test)
            # Check if the file name does not start with "remu_"
            case "$file" in
                remu_*) ;;
                *)
                    # Execute the file
                    ./"$file"
                    # Check the return value and exit if not zero
                    if [ $? -ne 0 ]; then
                        echo "Execution of $file failed with exit code $?. Exiting."
                        exit 1
                    fi
                    ;;
            esac
            ;;
    esac
done
