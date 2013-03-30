#ifndef HMM_H
#define HMM_H

#include "gmm.h"
#include "feat.h"

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

void hmm_update_gmm();
void hmm_clear_gmm(HMM *hmm);

/*
 * Fills the align array with equal alignment, assume align is of length feat_size
 */
void hmm_align_equal(int feat_size, int slice, int *align);
void hmm_train_kmeans(HMM *hmm, FeatureSet *fs, int max_iter, float tolerance);

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
 * Given a set of HMMs, assign a unique id for every state in every HMM
 * */
void hmm_assign_stateid(HMM **hmm_set, int hmm_size);

#endif
