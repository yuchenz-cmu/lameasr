#ifndef ALIGN_H
#define ALIGN_H

#include <stdio.h>
#include "topo.h"

void align_write(int *align, int align_len, int total_states, HMMStateMap *state_hmm_map , FILE *fp);

int align_read(int **align, int *total_states, HMMStateMap **state_hmm_map, FILE *fp);

#endif
