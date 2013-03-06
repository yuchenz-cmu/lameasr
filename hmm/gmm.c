/*
 * =====================================================================================
 *
 *       Filename:  gmm.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/02/2013 12:18:33 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (Yuchen Zhang), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h> 
#include <math.h>
#include <string.h>
#include "gmm.h"
#include "utils.h"

#define PI (3.14159265358)
#define LN_2PI (1.837877066)
#define LINE_BUF_SIZE (256)
#define EPSILON (0.01)

void gmm_clear(GMM *gmm) {
    for (int i = 0; i < gmm->mixture_num; i++) {
        for (int j = 0; j < gmm->feat_dim; j++) {
            gmm->mean[i][j] = 0.0;
            gmm->var[i][j] = 0.0;
            gmm->weight[i] = 0.0;
        }
    }
    gmm->var_const = 0.0;
    gmm->feat_count = 0;
}

/* 
 * Computes the variance constant of the given gmm
 * */
void gmm_var_const(GMM *gmm) {
    // compute the variance constant
    gmm->var_const = -0.5 * gmm->feat_dim * LN_2PI;
    float tmp = 0.0;
    for (int d = 0; d < gmm->feat_dim; d++) {
        tmp += log(gmm->var[0][d]);
    }
    gmm->var_const += -0.5 * tmp;
}

/* 
 * Currently only deal with the case of mixture_num == 1
 * */
int gmm_mean_var(GMM *gmm, float **feat, int feat_size, int feat_dim) {
    assert (gmm->mixture_num > 0);
    assert (gmm->feat_dim == feat_dim);

    float prev_ratio = (float) gmm->feat_count / (float) (gmm->feat_count + feat_size);
    float curr_ratio = (float) feat_size / (float) (gmm->feat_count + feat_size);
    float partial_sum = 0.0;

    // accumulate mean
    // fprintf(stderr, "mean[0]:");
    for (int d = 0; d < feat_dim; d++) {
        gmm->mean[0][d] *= prev_ratio;
        partial_sum = 0.0;
        for (int i = 0; i < feat_size; i++) {
            partial_sum += feat[i][d];
        }
        gmm->mean[0][d] += partial_sum * curr_ratio / (float) feat_size;
        // fprintf(stderr, "%.4f ", gmm->mean[0][d]);
    }
    // fprintf(stderr, "\n");

    // calculate variance
    // fprintf(stderr, "var[0]:");
    for (int d = 0; d < feat_dim; d++) {
        gmm->var[0][d] *= prev_ratio;
        partial_sum = 0.0;
        for (int i = 0; i < feat_size; i++) {
            partial_sum += (feat[i][d] - gmm->mean[0][d]) * (feat[i][d] - gmm->mean[0][d]);
        }
        gmm->var[0][d] += partial_sum * curr_ratio / (float) feat_size;
        // fprintf(stderr, "%.4f ", gmm->var[0][d]);
    }
    // fprintf(stderr, "\n");

    // update other gaussians
    for (int g = 1; g < gmm->mixture_num; g++) {
        for (int d = 0; d < feat_dim; d++) {
            gmm->mean[g][d] = gmm->mean[0][d];
            gmm->var[g][d] = gmm->var[0][d];
        }
    }

    // update the number of reature vectors we've seen so far
    gmm->feat_count += feat_size;

    return 1;
}

/* 
 * Computes the likelihood of a frame given kth gaussian, not multiplying the weight
 */
float gmm_likelihood_idx(GMM *gmm, int k, float *feat, int feat_dim) {
    assert (gmm->feat_dim == feat_dim);

    // for (int d = 0; d < feat_dim; d++) {
    //     fprintf(stderr, "%.4f ", feat[d]);
    // }
    // fprintf(stderr, "\n");

    float ll = -0.5 * feat_dim * LN_2PI;
    float tmp = 0.0;
    // fprintf(stderr, "ll: %f\n", ll);

    for (int d = 0; d < feat_dim; d++) {
        tmp += log(gmm->var[k][d]);
        // fprintf(stderr, "tmp: %f\n", tmp);
    }
    ll += -0.5 * tmp;
    // fprintf(stderr, "ll: %f\n", ll);

    for (int d = 0; d < feat_dim; d++) {
        ll -= (feat[d] - gmm->mean[k][d]) * (feat[d] - gmm->mean[k][d]) / (float) (2 * gmm->var[k][d]);
    }
    // fprintf(stderr, "ll: %f\n", ll);
    return ll;
}

/* 
 * Computes the log-likelihood of a feature vector given GMM
 * */
/*  
float gmm_likelihood(GMM *gmm, float *feat, int feat_dim) {
    assert (gmm->mixture_num == 1);
    return gmm_likelihood_idx(gmm, 0, feat, feat_dim);
}
*/ 

/* 
 * Computes the log-likelihood of a feature vector given GMM
 * */
float gmm_likelihood(GMM *gmm, float *feat, int feat_dim) {
    assert (gmm->mixture_num > 0);

    // fprintf(stderr, "gmm_ll: ");
    float ll = gmm_likelihood_idx(gmm, 0, feat, feat_dim) + log(gmm->weight[0]);
    // fprintf(stderr, "%f ", ll);
    for (int g = 1; g < gmm->mixture_num; g++) {
        ll = utils_log_add(gmm_likelihood_idx(gmm, g, feat, feat_dim) + log(gmm->weight[g]), ll);
        // fprintf(stderr, "%f ", ll);
    }
    // fprintf(stderr, "\n");
    return ll;
}

/* 
 * Given a GMM, split the gaussian with largest var
 * Returns the current mixture number
 * */
int gmm_split_max_var(GMM *gmm) {
    assert (gmm->mixture_num > 0);

    int max_var_idx = 0;
    float max_var = 0.0;

    // find the gaussian with largest variance
    for (int g = 0; g < gmm->mixture_num; g++) {
        float curr_var = 0.0;
        for (int d = 0; d < gmm->feat_dim; d++) {
            curr_var += gmm->var[g][d] * gmm->var[g][d];
        }
        if (curr_var > max_var) {
            max_var = curr_var;
            max_var_idx = g;
        }
    }

    // grow the mean, var and weight array
    float **copy_mean = (float **) malloc(sizeof(float *) * (gmm->mixture_num + 1));
    float **copy_var = (float **) malloc(sizeof(float *) * (gmm->mixture_num + 1));
    float *copy_weight = (float *) malloc(sizeof(float) * (gmm->mixture_num + 1));

    for (int g = 0; g < gmm->mixture_num + 1; g++) {
        copy_mean[g] = (float *) malloc(sizeof(float) * gmm->feat_dim);
        copy_var[g] = (float *) malloc(sizeof(float) * gmm->feat_dim);
    }

    // copy the elements over
    for (int g = 0; g < gmm->mixture_num; g++) {
        for (int d = 0; d < gmm->feat_dim; d++) {
            copy_mean[g][d] = gmm->mean[g][d];
            copy_var[g][d] = gmm->var[g][d];
        }
        copy_weight[g] = gmm->weight[g];
    }

    // assign weight to the one more gaussian in the end
    for (int d = 0; d < gmm->feat_dim; d++) {
        copy_mean[gmm->mixture_num][d] = gmm->mean[max_var_idx][d] + EPSILON;
        copy_var[gmm->mixture_num][d] = gmm->var[max_var_idx][d];
        gmm->mean[max_var_idx][d] -= EPSILON;
    }
    copy_weight[gmm->mixture_num] = gmm->weight[max_var_idx];

    // free up and reassign
    for (int g = 0; g < gmm->mixture_num; g++) {
        free(gmm->mean[g]);
        free(gmm->var[g]);
    }
    free(gmm->mean);
    free(gmm->var);
    free(gmm->weight);
    gmm->mean = copy_mean;
    gmm->var = copy_var;
    gmm->weight = copy_weight;

    gmm_normalize_weight(gmm);
    gmm->mixture_num++;

    return gmm->mixture_num;
}

/*  
 * Initialize the GMM with mixture number and feature dimension
 * */
GMM* gmm_init(int mixture_num, int feat_dim) {
    GMM *gmm = (GMM *) malloc(sizeof(GMM));
    gmm->feat_dim = feat_dim;
    gmm->mixture_num = mixture_num;
    gmm->mean = (float **) malloc(sizeof(float *) * mixture_num);
    gmm->var = (float **) malloc(sizeof(float *) * mixture_num);

    for (int i = 0; i < mixture_num; i++) {
        gmm->mean[i] = (float *) malloc(sizeof(float) * feat_dim);
        gmm->var[i] = (float *) malloc(sizeof(float) * feat_dim);
    }

    gmm->weight = (float *) malloc(sizeof(float) * mixture_num);
    gmm_clear(gmm);
    return gmm;
}

/* 
 * Normalize the weight of the gmm to sum up to 1
 * */
void gmm_normalize_weight(GMM *gmm) {
    assert (gmm->mixture_num > 0);

    float wght_sum = 0.0;
    for (int g = 0; g < gmm->mixture_num; g++) {
        wght_sum += gmm->weight[g];
    }
    for (int g = 0; g < gmm->mixture_num; g++) {
        gmm->weight[g] = (float) gmm->weight[g] / (float) wght_sum;
    }
}



