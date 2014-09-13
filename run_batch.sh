#!/bin/bash

# Script for submitting batch jobs
# Sample usage:
# nohup nice -10 ./run_batch &

PATH_BASE="path"
RV_BASE="rv"
DATA_DIR="./data/"

if [ ! -f ./sle_mc ]; then
  echo "File sle_mc not found. Run make and try again"
  exit 1
fi

if [ ! -d "$DATA_DIR" ]; then
  mkdir "$DATA_DIR"
fi

for i in `seq 1 1000`
do
  NOW=$(date +"%H_%M_%S-%m_%d_%Y")
  ./sle_mc "./data/$PATH_BASE-$NOW.csv" "./data/$RV_BASE-$NOW.csv"
done
