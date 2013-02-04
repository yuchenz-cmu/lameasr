#!/bin/bash
sample_rate=16000
bits=16
for f in ./*.raw ; do
    echo "Converting $f ... "
    sox -t raw -b ${bits} -r ${sample_rate} -e signed-integer -c 1 $f $f.wav
done
