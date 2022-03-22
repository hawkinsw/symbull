#!/bin/env bash

executable_directory="/home/hawkinsw/code/coreutils/install/bin"

output_directory="output"

mkdir ${output_directory} > /dev/null 2>&1

for i in `ls ${executable_directory}`; do
				build/learn_elf --outfile="${output_directory}/${i}.out" --l=30 ${executable_directory}/$i
done
