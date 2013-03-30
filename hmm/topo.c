/*
 * =====================================================================================
 *
 *       Filename:  topo.c
 *
 *    Description:  Topology of the grammar
 *
 *        Version:  1.0
 *        Created:  03/29/2013 02:06:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LINE_BUF_SIZE (256)

int topo_assign_hmm_stateid(HMM **hmm_set, int hmm_size) {
    int idx = 0
    for (int h = 0; h < hmm_size; h++) {
        for (int s = 0; s < hmm_set[h].state_num; s++) {
            hmm_set[h].state[s].id = idx;
            idx++;
        }
    }
    return idx;
}

int topo_gen_transmat(HMM **hmm_set, int hmm_size, char *topofile, float ***matrix) {
    FILE *fp = fopen(topofile, "r");
    char buf[LINE_BUF_SIZE];
    int lc = 0; // line count
    int node_num = 0;
    int start_node = 0;
    int end_node = 0;
    char *onetok = NULL;

    while (fgets(buf, LINE_BUF_SIZE, fp) != NULL) {
        if (lc == 0) {
            // total number of nodes
            node_num = atoi(buf); 
        } else if (lc == 1) {
            // start node
            start_node = atoi(buf);
        } else if (lc == 2) {
            // end node
            end_node = atoi(buf);
        } else {

        }

        lc++;
    }

    fclose(fp);
}
