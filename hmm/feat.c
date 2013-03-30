/*
 * =====================================================================================
 *
 *       Filename:  feat.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/02/2013 05:41:04 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "feat.h"

#define INIT_CAPACITY 32
#define LINE_BUF_SIZE (256)

/*
 * Grow the feature by twice
 * */
int featset_feat_grow(float ***feat, int feat_size, int feat_dim) {
    int curr_size = feat_size;
    float **feat_copy = (float **) malloc(sizeof(float *) * curr_size * 2);
    for (int i = 0; i < curr_size * 2; i++) {
        feat_copy[i] = (float *) malloc(sizeof(float) * feat_dim);
    }
    for (int i = 0; i < curr_size; i++) {
        for (int d = 0; d < feat_dim; d++) {
            feat_copy[i][d] = (*feat)[i][d];
        }
    }
    curr_size *= 2;
    free(*feat);
    *feat = feat_copy;
    return curr_size;
}

/*
 * Grow the FeatureSet by one feature. (a feature is a two-dimensional array)
 * */
int featset_grow(FeatureSet *fs) {
    // prepare the empty feature
    float **feat = (float **) malloc(sizeof(float *) * INIT_CAPACITY);
    for (int i = 0; i < INIT_CAPACITY; i++) {
        feat[i] = (float *) malloc(sizeof(float) * fs->feat_dim);
    }

    // grow the FeatureSet
    float ***new_feat = (float ***) malloc(sizeof(float **) * (fs->feat_num + 1));
    for (int idx = 0; idx < fs->feat_num; idx++) {
        new_feat[idx] = (float **) malloc(sizeof(float *) * (fs->feat_sizes[idx]));
        for (int i = 0; i < fs->feat_sizes[idx]; i++) {
            new_feat[idx][i] = malloc(sizeof(float) * fs->feat_dim);
        }
    }

    int *new_feat_sizes = (int *) malloc(sizeof(int) * (fs->feat_num + 1));

    fprintf(stderr, "Growing FeatureSet from %d to %d ... \n", fs->feat_num, fs->feat_num + 1);
    for (int idx = 0; idx < fs->feat_num; idx++) {
        for (int i = 0; i < fs->feat_sizes[idx]; i++) {
            for (int d = 0; d < fs->feat_dim; d++) {
                new_feat[idx][i][d] = fs->feat[idx][i][d];
            }
        }
        new_feat_sizes[idx] = fs->feat_sizes[idx];
    }
    fs->feat_num++;

    // clean up mem
    if (fs->feat != NULL) {
        free(fs->feat);
        free(fs->feat_sizes);
    }

    // put new feature in
    fs->feat = new_feat;
    fs->feat_sizes = new_feat_sizes;

    fs->feat[fs->feat_num - 1] = feat;
    fs->feat_sizes[fs->feat_num - 1] = INIT_CAPACITY;

    return fs->feat_num;
}

FeatureSet *featset_init(int feat_dim) {
    FeatureSet *fs = (FeatureSet *) malloc(sizeof(FeatureSet));
    fs->feat_dim = feat_dim;
    fs->feat_num = 0;
    fs->feat_sizes = NULL;
    fs->feat = NULL;
    return fs;
}

/*
 * Reads the features from file, returns the number of feature vectores read.
 * */
int featset_read_file(char *filename, FeatureSet *fs) {
    char buf[LINE_BUF_SIZE];
    char *one_feat = NULL;
    int dim_idx = 0;
    FILE *fp = fopen(filename, "r");

    // grow the FeatureSet by 1
    featset_grow(fs);
    float ***curr_feat = &fs->feat[fs->feat_num - 1];
    int curr_capacity = fs->feat_sizes[fs->feat_num - 1];
    int feat_size = 0;

    while (fgets(buf, LINE_BUF_SIZE, fp) != NULL) {
        if (feat_size == curr_capacity) {
            // grow
            fprintf(stderr, "Growing from %d to %d ... \n", curr_capacity, curr_capacity * 2);
            curr_capacity = featset_feat_grow(curr_feat, feat_size, fs->feat_dim);
        }

        one_feat = strtok(buf, " ");
        dim_idx = 0;
        while (one_feat != NULL) {
            assert (dim_idx < fs->feat_dim);
            (*curr_feat)[feat_size][dim_idx] = atof(one_feat);

            // fprintf(stderr, "%.4f ", (*curr_feat)[feat_size][dim_idx]);

            one_feat = strtok(NULL, " ");
            dim_idx++;
        }

        // fprintf(stderr, "\n");
        feat_size++;
    }
    fs->feat_sizes[fs->feat_num - 1] = feat_size;

    fclose(fp);
    return feat_size;
}

void featset_print(FeatureSet *fs) {
    fprintf(stderr, "Number of features: %d\n", fs->feat_num);
    fprintf(stderr, "Feature dimension: %d\n", fs->feat_dim);
    for (int idx = 0; idx < fs->feat_num; idx++) {
        fprintf(stderr, "Feature %d\tnumber of features: %d\n", idx, fs->feat_sizes[idx]);
        for (int i = 0; i < fs->feat_sizes[idx]; i++) {
            for (int d = 0; d < fs->feat_dim; d++) {
                fprintf(stderr, "%.4f ", fs->feat[idx][i][d]);
            }
            fprintf(stderr, "\n");
        }
    }
}

