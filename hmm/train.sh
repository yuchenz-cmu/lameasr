#!/bin/bash

model_dir="models"
list_dir="list_mix"

rm ${model_dir}/*
for num in zero one two three four five six seven eight nine sil sil_2 ; do
    ./train_single_word --feat-list ${list_dir}/${num}.list --lex ${num} --dim 13 --model ${model_dir}/${num}.hmm 
done
