#ifndef TOPO_H
#define TOPO_H

/*
 * Generates the transition matrix from a topology set, based on the HMMs we have.
 * Returns the total number of unique HMM states (including dummy state) defined by the topology
 * */
int topo_gen_transmat(HMM *hmm_set, int hmm_size, char *topofile, float ***matrix);

/*
 * Given a set of HMMs, assign each state a unique state-id starting from 0
 * Returns the total number of states assigned
 * */
int topo_assign_hmm_stateid(HMM **hmm_set, int hmm_size);

#endif
