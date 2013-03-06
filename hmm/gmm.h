#ifndef GMM_H
#define GMM_H

typedef struct struct_gmm {
    float **mean;
    float **var;
    float *weight;
    int feat_dim;
    int mixture_num; 
    float var_const;
    long feat_count; // how many feature vectors we've seen so far
    float gamma_count;  // same thing as above, but is gamma
} GMM;

void gmm_clear(GMM *gmm);
void gmm_var_const(GMM *gmm);
float gmm_likelihood_idx(GMM *gmm, int k, float *feat, int feat_dim);
float gmm_likelihood(GMM *gmm, float *feat, int feat_dim);
GMM* gmm_init(int mixture_num, int feat_dim);
int gmm_read_feat(FILE *fp, float ***feat, int init_capacity, int feat_dim);
int gmm_mean_var(GMM *gmm, float **feat, int feat_size, int feat_dim);
int gmm_update(GMM *gmm, float **feat, int feat_size, int feat_dim);
void gmm_normalize_weight(GMM *gmm);
int gmm_split_max_var(GMM *gmm);

#endif
