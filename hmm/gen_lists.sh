#!/bin/bash
digit_dir="mfcc_0-9"
list_dir=list_mix

# train HMM for digits
for num in zero one two three four five six seven eight nine sil ; do
    if [ -f ${list_dir}/${num}.list ]; then
        rm ${list_dir}/${num}.list
    fi

    for f in ${digit_dir}/*${num}* ; do 
        readlink -f $f >> ${list_dir}/${num}.list
    done
done

