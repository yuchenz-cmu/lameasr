#ifndef TOPO_H
#define TOPO_H

#define MAX_LEX_LEN (32)

typedef struct struct_topo_list_node {
    int start_node;
    int end_node;
    char lex[MAX_LEX_LEN];
    struct struct_topo_list_node *next_node;
} TopoListNode;

typedef struct struct_trans_matrix {
    float **trans_matrix;
    int *state_hmm_map;
    int total_states;
    int dummy_states;
} TransMatrix;

/*
 * Generates the transition matrix from a topology set, based on the HMMs we have.
 * Returns the total number of unique HMM states (including dummy state) defined by the topology
 * */
int topo_gen_transmat(HMM **hmm_set, int hmm_size, char *topofile, float ***matrix, int **state_mapping, int *total_dummy_nodes);

/*
 * Given a set of HMMs, assign each state a unique state-id starting from 0
 * Returns the total number of states assigned
 * */
// int topo_assign_hmm_stateid(HMM **hmm_set, int hmm_size);

#endif
