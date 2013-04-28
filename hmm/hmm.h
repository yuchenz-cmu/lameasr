#ifndef HMM_H
#define HMM_H

#include "gmm.h"
#include "feat.h"
#include "topo.h"
#include "database.h"

typedef struct struct_hmm_state {
    GMM *gmm;
    int id;
} HMMState;

typedef struct struct_hmm_point {
    int s;
    int t;
} HMMPoint;

typedef struct struct_hmm_trellis {
    float value;
    HMMPoint prev;

} HMMTrellis;

typedef struct struct_hmm {
    HMMState *states;
    int state_num;
    char *lex;
} HMM;

/*
 * Given an HMM and a set of frames, computes the alignment.
 * Fills the array of length feat_size, of which state a feature is in.
 * Returns the likelihood of the best path
 */
float hmm_align_dtw(HMM* hmm, float **feat, int feat_size, int feat_dim, int *align);

/*
 * Initialize HMM, all of its states, and each state's GMM
 */
HMM *hmm_init(int state_num, int gs_num, int feat_dim, char *lex);

/*
 * Given an HMM, a FeatureSet, and a set of alignment of being at which state at time t,
 * update the GMM's parameters
 * Return: number of total frames used in accmulation
 */
int hmm_update_gmm(HMM *hmm, int hmm_id, FeatureSet *fs, int **alignset, TransMatrix *trans_mat_set);

void hmm_clear_gmm(HMM *hmm);

/*
 * Fills the align array with equal alignment, assume align is of length feat_size
 */
void hmm_align_equal(int feat_size, int slice, int *align);

// void hmm_train_kmeans(HMM *hmm, FeatureSet *fs, int max_iter, float tolerance);

/*
 * Outputs an HMM to file.
 * Returns 1 for success, 0 for failure.
 */
int hmm_write(HMM *hmm, char *filename);

/*
 * Reads an HMM from disk.
 * Returns the HMM when success, NULL if failed.
 */
HMM* hmm_read(char *filename);

/*
 * Given a set of HMMs, a transition matrix, and a feature, compute the Viterbi alignment
 * Returns the Viterbi likelihood
 */
float hmm_decode_viterbi(HMM **hmm_set, int hmm_size, TransMatrix *trans_mat, FeatureStruct *feat_struct, int *align);

/*  
 * Train a set of HMMs from a set of features and a set of topologies
 */
void hmm_train_continuous(HMM **hmm_set, int hmm_size, Database *db, int feat_dim, int max_iter, float tolerance);

#endif
