#!/usr/bin/bash

INPUT_DIR=$1
OUTPUT_DIR=$2
EXE_PATH=$3

if [ -z $3 ] 
then
    EXE_PATH=./
fi

if [[ ! -e $OUTPUT_DIR ]]; then
    mkdir $OUTPUT_DIR
fi

for filepath in "$INPUT_DIR"/*
do
  if [ -f "$filepath" ];then
    echo "Hanlding `basename $filepath txt` ...";
    $EXE_PATH/fastnewwords < $filepath > $OUTPUT_DIR/"`basename $filepath txt`.words.txt" &
  fi
done

wait
echo "done"
exit 0