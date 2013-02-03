#!/bin/bash
sample_rate=16000
bits=16
sox -t raw -b ${bits} -r ${sample_rate} -e signed-integer -c 1 ./recorded.raw ./recorded.wav
