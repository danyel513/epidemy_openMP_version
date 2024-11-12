#!/bin/bash

# library compilation
gcc -Wall -c epidemy.c -o epidemy.o

# compile and link for serial
gcc -Wall serial_epidemy.c epidemy.o -o serial_epidemy

# compile and link for parallel
gcc -Wall parallel_epidemy.c epidemy.o -o parallel_epidemy

# validating outputs
gcc -Wall validate_outputs.c -o validate_outputs

# test serial program
echo "Values: population = {10k, 20k, 50k, 100k, 500k},
      simulation time = {50, 100, 150, 200, 500}, threads = {5, 10} "


# define params
ITERATIONS=(50 100 150 200 500)
FILES=("epidemics10K.txt" "epidemics20K.txt" "epidemics50K.txt" "epidemics100K.txt")
THREADS=5

# running function - automate testing
run_test()
{
    iteration=$1
    file=$2

    echo "Running serial_epidemy with $iteration iterations on $file"
    ./serial_epidemy "$iteration" "$file" "$THREADS"

    echo "Running parallel_epidemy with $iteration iterations on $file"
    ./parallel_epidemy "$iteration" "$file" "$THREADS"

    echo "Validating outputs..."
    ./validate_outputs f_serial_out.txt f_parallel_out.txt
}

# iterate through the files and values
for iteration in "${ITERATIONS[@]}"; do
    for file in "${FILES[@]}"; do
        run_test "$iteration" "$file"
    done
done

