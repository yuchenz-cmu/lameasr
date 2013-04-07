#!/bin/bash
if [ $# -eq 0 ]; then
    echo 
    # ./decoder --topo ./topos/telephone.topo --model-list ./all_models.list --feat ./mfccs/2_1_5.mfcc --dim 13
    # ./decoder --topo ./topos/telephone.topo --model-list ./all_models.list --feat ./mfccs/1_2_3.mfcc --dim 13
    # ./decoder --topo topos/sil_test.topo --model-list ./all_models.list --feat ./mfccs/1_2_3.mfcc --dim 13
    # ./decoder --topo topos/sil_test.topo --model-list ./all_models.list --feat ./mfccs/2_1_5.mfcc --dim 13
    # ./decoder --topo topos/numbers.topo --model-list ./all_models.list --feat ./mfccs/1_2_3.mfcc --dim 13
    # ./decoder --topo topos/sil_test.topo --model-list ./all_models.list --feat ./mfccs/2_1_5.mfcc --dim 13
elif [ $# -eq 1 ]; then
    topo_file=$1
    ./decoder --topo ${topo_file} --model-list ./all_models.list --feat ./mfccs/1_2_3.mfcc --dim 13
fi
