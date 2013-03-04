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

#define PI (3.14159265358)
#define LN_2PI (1.837877066)
#define LINE_BUF_SIZE (256)

void gmm_clear(GMM *gmm) {
    for (int i = 0; i < gmm->mixture_num; i++) {
        for (int j = 0; j < gmm->feat_dim; j++) {
            gmm->mean[i][j] = 0.0;
            gmm->var[i][j] = 0.0;
            // gmm->weight[i][j] = 0.0;
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
    assert (gmm->mixture_num == 1);
    assert (gmm->feat_dim == feat_dim);

    float prev_ratio = (float) gmm->feat_count / (float) (gmm->feat_count + feat_size);
    float curr_ratio = (float) feat_size / (float) (gmm->feat_count + feat_size);
    float partial_sum = 0.0;

    // accumulate mean
    for (int d = 0; d < feat_dim; d++) {
        gmm->mean[0][d] *= prev_ratio;
        partial_sum = 0.0;
        for (int i = 0; i < feat_size; i++) {
            partial_sum += feat[i][d];
        }
        gmm->mean[0][d] += partial_sum * curr_ratio / (float) feat_size;
    }

    // calculate variance
    for (int d = 0; d < feat_dim; d++) {
        gmm->var[0][d] *= prev_ratio;
        partial_sum = 0.0;
        for (int i = 0; i < feat_size; i++) {
            partial_sum += (feat[i][d] - gmm->mean[0][d]) * (feat[i][d] - gmm->mean[0][d]);
        }
        gmm->var[0][d] += partial_sum * curr_ratio / (float) feat_size;
    }

    // update the number of reature vectors we've seen so far
    gmm->feat_count += feat_size;

    // computes the variance constant
    gmm_var_const(gmm);

    return 1;
}

/* 
 * Computes the log-likelihood of a feature vector given GMM
 * */
float gmm_likelihood(GMM *gmm, float *feat, int feat_dim) {
    assert (gmm->mixture_num == 1);
    assert (gmm->feat_dim == feat_dim);

    // for (int d = 0; d < feat_dim; d++) {
    //     fprintf(stderr, "%.4f ", feat[d]);
    // }
    // fprintf(stderr, "\n");

    // float ll = 0.0;
    float ll = -0.5 * feat_dim * LN_2PI;
    float tmp = 0.0;
    // fprintf(stderr, "ll: %f\n", ll);

    for (int d = 0; d < feat_dim; d++) {
        tmp += log(gmm->var[0][d]);
        // fprintf(stderr, "tmp: %f\n", tmp);
    }
    ll += -0.5 * tmp;
    // fprintf(stderr, "ll: %f\n", ll);

    for (int d = 0; d < feat_dim; d++) {
        ll -= (feat[d] - gmm->mean[0][d]) * (feat[d] - gmm->mean[0][d]) / (float) (2 * gmm->var[0][d]);
    }
    // fprintf(stderr, "ll: %f\n", ll);
    // return (gmm->var_const + ll);
    return ll;
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

