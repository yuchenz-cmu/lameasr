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

/*
 * Mark everything in GMM to zero
 */
void gmm_clear(GMM *gmm);

/*  
 * Computes the variance constant of the given gmm
 */
void gmm_var_const(GMM *gmm);

/*  
 * Computes the likelihood of a frame given kth gaussian, not multiplying the weight
 */
float gmm_likelihood_idx(GMM *gmm, int k, float *feat, int feat_dim);

/*  
 * Computes the log-likelihood of a feature vector given GMM
 */
float gmm_likelihood(GMM *gmm, float *feat, int feat_dim);

/*   
 * Initialize the GMM with mixture number and feature dimension
 */
GMM* gmm_init(int mixture_num, int feat_dim);
int gmm_read_feat(FILE *fp, float ***feat, int init_capacity, int feat_dim);

/*  
 * Currently only deal with the case of mixture_num == 1
 */
int gmm_mean_var(GMM *gmm, float **feat, int feat_size, int feat_dim);

/*  
 * Normalize the weight of the gmm to sum up to 1
 */
void gmm_normalize_weight(GMM *gmm);

/*  
 * Given a GMM, split the gaussian with largest var
 * Returns the current mixture number
 */
int gmm_split_max_var(GMM *gmm);

#endif
