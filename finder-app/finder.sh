#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

if [ "$#" -ne 2 ]; then
  echo "Error: Two arguments required - <directory> <search_string>"
  exit 1
fi

filesdir=$1
searchstr=$2

if [ ! -d "$filesdir" ]; then
  echo "Error: $filesdir does not represent a valid directory."
  exit 1
fi

grep -r "$searchstr" "$filesdir"

number_of_lines=$(grep -r "$searchstr" "$filesdir" | wc -l)
number_of_files=$(find "$filesdir" -type f | wc -l)



# Print the result
echo "The number of files are $number_of_files and the number of matching lines are $number_of_lines"


exit 0