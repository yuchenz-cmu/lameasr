/*
 * =====================================================================================
 *
 *       Filename:  align.c
 *
 *    Description:  Alignment
 *
 *        Version:  1.0
 *        Created:  04/20/2013 02:25:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yuchen Zhang (), yuchenz@cs.cmu.edu
 *   Organization:  Carnegie Mellon University
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "topo.h"

void align_write(int *align, int align_len, int total_states, HMMStateMap *state_hmm_map , FILE *fp) {
    fwrite(&align_len, sizeof(int), 1, fp);
    fwrite(align, sizeof(int), align_len, fp);
    fwrite(&total_states, sizeof(int), 1, fp);
    fwrite(state_hmm_map, sizeof(HMMStateMap), total_states, fp);
}

int align_read(int **_align, int *total_states, HMMStateMap **_state_hmm_map, FILE *fp) {
    int align_len = 0;
    int map_size = 0;
    int *align = NULL;
    HMMStateMap *state_hmm_map = NULL;

    fread(&align_len, sizeof(int), 1, fp);
    
    align = (int *) malloc(sizeof(int) * align_len);
    fread(align, sizeof(int), align_len, fp);

    fread(&map_size, sizeof(int), 1, fp);

    state_hmm_map = (HMMStateMap *) malloc(sizeof(HMMStateMap) * map_size);
    fread(state_hmm_map, sizeof(HMMStateMap), map_size, fp);

    *total_states = map_size;
    *_state_hmm_map = state_hmm_map;
    *_align = align;
    return align_len;
}
