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
    // struct struct_hmm_trellis *prev;
    HMMPoint prev;

} HMMTrellis;

typedef struct struct_hmm {
    HMMState *states;
    int state_num;
    char *lex;
} HMM;

float hmm_align_dtw(HMM* hmm, float **feat, int feat_size, int feat_dim, int *align);
HMM *hmm_init(int state_num, int feat_dim, char *lex);
void hmm_update_gmm();
void hmm_clear_gmm(HMM *hmm);
void hmm_align_equal(int feat_size, int slice, int *align);
void hmm_train_kmeans(HMM *hmm, FeatureSet *fs, int max_iter, float tolerance);

#endif
