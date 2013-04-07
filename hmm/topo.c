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
#include <assert.h>
#include <math.h>
#include <float.h>
#include "topo.h"
#include "hmm.h"

#define LINE_BUF_SIZE (256)

int topo_gen_transmat(HMM **hmm_set, int hmm_size, char *topofile, float ***matrix, HMMStateMap **state_mapping, int *total_dummy_nodes) {
    assert(hmm_size > 0); 
    for (int h = 1; h < hmm_size; h++) {
        assert(hmm_set[h - 1]->state_num == hmm_set[h]->state_num);
    }

    FILE *fp = fopen(topofile, "r");
    char buf[LINE_BUF_SIZE];
    int lc = 0; // line count
    int node_num = 0;
    int start_node = 0;
    int end_node = 0;
    float **grid = NULL;
    int total_states = 0;
    TopoListNode *head_node = (TopoListNode *) malloc(sizeof(TopoListNode));
    TopoListNode *curr_node = head_node;
    TopoListNode *prev_node = NULL;
    HMMStateMap *hmm_state_map = NULL;
    int hmm_states_num = hmm_set[0]->state_num;

    while (fgets(buf, LINE_BUF_SIZE, fp) != NULL) {
        buf[strlen(buf) - 1] = '\0';
        if (lc == 0) {
            // total number of nodes
            node_num = atoi(buf); 

            // increase the total number of states with the number of dummy states
            // the first N states will be the dummy states
            total_states += node_num;
        } else if (lc == 1) {
            // start node
            start_node = atoi(buf);
        } else if (lc == 2) {
            // end node
            end_node = atoi(buf);
        } else {
            curr_node->start_node = atoi(strtok(buf, " "));
            curr_node->end_node = atoi(strtok(NULL, " "));
            // curr_node->lex = strtok(NULL, " ");
            strncpy(curr_node->lex, strtok(NULL, " "), MAX_LEX_LEN);

            curr_node->next_node = (TopoListNode *) malloc(sizeof(TopoListNode));
            prev_node = curr_node;
            curr_node = curr_node->next_node;
            total_states += hmm_states_num;
        }

        lc++;
    }
    fclose(fp);
    free(curr_node);
    prev_node->next_node = NULL;

    // builds the grid
    grid = (float **) malloc(sizeof(float *) * total_states);
    for (int s = 0; s < total_states; s++) {
        grid[s] = (float *) malloc(sizeof(float) * total_states);
        for (int p = 0; p < total_states; p++) {
            grid[s][p] = -FLT_MAX;
        }
    }

    // builds the mapping between state_id and hmm_id
    hmm_state_map = (HMMStateMap *) malloc(sizeof(HMMStateMap) * total_states);
    for (int s = 0; s < node_num; s++) {
        hmm_state_map[s].hmm_id = -1;  // -1 means dummy state
        hmm_state_map[s].hmm_state_id = -1;
    }

    // go through the linked list and fill in the grid
    int curr_st_idx = node_num;
    curr_node = head_node; 
    while (curr_node != NULL) {
        // for each state, find which HMM it corresponds to by iterating
        // TODO: This is stupid, should use a hash table
        
        int hmm_id = -1;
        fprintf(stderr, "node_lex: %s\n", curr_node->lex);
        for (int h = 0; h < hmm_size; h++) {
            // fprintf(stderr, "%s\n", hmm_set[h]->lex);
            if (!strcmp(hmm_set[h]->lex, curr_node->lex)) {
                hmm_id = h;
                break;
            }
        }

        // not found, fail
        assert(hmm_id >= 0);

        for (int s = 0; s < hmm_states_num; s++) {
            hmm_set[hmm_id]->states[s].id = curr_st_idx;
            hmm_state_map[curr_st_idx].hmm_id = hmm_id;
            hmm_state_map[curr_st_idx].hmm_state_id = s;
            // grid[curr_st_idx][curr_st_idx + 1] = 1.0;
            curr_st_idx++;
        }

        // start dummy node ==> first state
        grid[curr_node->start_node][hmm_set[hmm_id]->states[0].id] = 0.0;   // log(1.0)
        fprintf(stderr, "*[%d] ==> [%d]\n", curr_node->start_node, hmm_set[hmm_id]->states[0].id);

        // s[k] ==> s[k + 1], and s[k] ==> s[k]
        for (int s = 0; s < hmm_states_num - 1 ; s++) {
            grid[hmm_set[hmm_id]->states[s].id][hmm_set[hmm_id]->states[s + 1].id] = log(0.5);
            grid[hmm_set[hmm_id]->states[s].id][hmm_set[hmm_id]->states[s].id] = log(0.5);
            fprintf(stderr, "[%d] ==> [%d]\n", hmm_set[hmm_id]->states[s].id, hmm_set[hmm_id]->states[s + 1].id);
        }

        // last state ==> end dummy node
        grid[hmm_set[hmm_id]->states[hmm_states_num - 1].id][curr_node->end_node] = 0.0;    // log 1.0
        fprintf(stderr, "[%d] ==> [%d]*\n", hmm_set[hmm_id]->states[hmm_states_num - 1].id, curr_node->end_node);

        // free up memory
        head_node = curr_node->next_node;
        // curr_node = curr_node->next_node;
        free(curr_node);
        curr_node = head_node;
    }

    // int topo_gen_transmat(HMM **hmm_set, int hmm_size, char *topofile, float ***matrix, int **state_mapping) {
    assert(total_states == curr_st_idx);
    
    *matrix = grid;
    *state_mapping = hmm_state_map;
    *total_dummy_nodes = node_num;

    return total_states;
}

