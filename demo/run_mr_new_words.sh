#!/usr/bin/env bash

INPUT_PATH=$1
TMP_PATH=$2
OUT_PATH=$3

NUM_ARTICLE=$(ls -1q cache/bizmsg/inp/* | wc -l)

# map => generate ngrams for each files
for filename in $INPUT_PATH/*
do
    python main.py -generate_ngrams < $filename | sort -t'|' > $TMP_PATH/gram_$(basename $filename)
done

# reduce
python main.py -merge_and_split_ngrams -inp_prefix $TMP_PATH/gram_ -outp_prefix $TMP_PATH/gram2_ -n_splits $NUM_ARTICLE

# map => compute solidity and entropy for each files

for filename in $TMP_PATH/gram2_*
do
    python main.py -selecting_candidates -inp $filename > $TMP_PATH/score_$(basename $filename)
done

python main.py -get_final_results -inp_prefix $TMP_PATH/score_gram2_ -n_splits $NUM_ARTICLE > $OUT_PATH