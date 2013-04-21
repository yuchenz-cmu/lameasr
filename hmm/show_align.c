/*
 * =====================================================================================
 *
 *       Filename:  show_align.c
 *
 *    Description:  Prints the alignment
 *
 *        Version:  1.0
 *        Created:  04/20/2013 03:57:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yuchen Zhang (), yuchenz@cs.cmu.edu
 *   Organization:  Carnegie Mellon University
 *
 * =====================================================================================
 */
#include <stdio.h>
#include "align.h"
#include "topo.h"

void print_usage(char *msg) {
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }
    fprintf(stderr, "Usage: ./show_align <align_file>\n");
}

int main(int argc, char** argv) {
    char *align_file = NULL;
    if (argc != 2) {
        print_usage("Invalid number of arguments.");
        return 2;
    }

    align_file = argv[1];
    int *alignment = NULL;
    int align_len = 0;
    HMMStateMap *state_hmm_map = NULL;
    int total_states = 0;

    FILE *fp = fopen(align_file, "rb");
    align_len = align_read(&alignment, &total_states, &state_hmm_map, fp); 
    fclose(fp);

    printf("Frame number: %d\n", align_len);
    for (int i = 0; i < align_len; i++) {
        printf("%d ", alignment[i]);
    }
    printf("\n");

    printf ("Total states: %d\n", total_states);
    for (int i = 0; i < align_len; i++) {
        printf("%d(%d) ", state_hmm_map[alignment[i]].hmm_id, state_hmm_map[alignment[i]].hmm_state_id);
    }
    printf("\n");

    return 0;
}

