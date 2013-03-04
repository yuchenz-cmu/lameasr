/*
 * =====================================================================================
 *
 *       Filename:  controller.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/02/2013 11:45:14 PM
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
#include <assert.h>
#include "gmm.h"
#include "feat.h"
#include "hmm.h"

int main(int argc, char **argv) {
    FeatureSet *fs_train_one = featset_init(13);
    FeatureSet *fs_train_two = featset_init(13);
    FeatureSet *fs_train_three = featset_init(13);

    FeatureSet *fs_test_one = featset_init(13);
    FeatureSet *fs_test_two = featset_init(13);
    FeatureSet *fs_test_three = featset_init(13);

    featset_read_file("./mfcc_0-9/one_0.mat", fs_train_one);
    featset_read_file("./mfcc_0-9/one_1.mat", fs_train_one);
    featset_read_file("./mfcc_0-9/one_2.mat", fs_train_one);
    featset_read_file("./mfcc_0-9/one_3.mat", fs_train_one);
    featset_read_file("./mfcc_0-9/one_4.mat", fs_train_one);

    featset_read_file("./mfcc_0-9/two_0.mat", fs_train_two);
    featset_read_file("./mfcc_0-9/two_1.mat", fs_train_two);
    featset_read_file("./mfcc_0-9/two_2.mat", fs_train_two);
    featset_read_file("./mfcc_0-9/two_3.mat", fs_train_two);
    featset_read_file("./mfcc_0-9/two_4.mat", fs_train_two);

    featset_read_file("./mfcc_0-9/three_0.mat", fs_train_three);
    featset_read_file("./mfcc_0-9/three_1.mat", fs_train_three);
    featset_read_file("./mfcc_0-9/three_2.mat", fs_train_three);
    featset_read_file("./mfcc_0-9/three_3.mat", fs_train_three);
    featset_read_file("./mfcc_0-9/three_4.mat", fs_train_three);


    featset_read_file("./mfcc_0-9/one_5.mat", fs_test_one);
    featset_read_file("./mfcc_0-9/two_8.mat", fs_test_two);
    featset_read_file("./mfcc_0-9/three_5.mat", fs_test_three);

    // HMM *hmm = hmm_init(5, 13);
    HMM **models = (HMM **) malloc(sizeof(HMM *) * 3);
    models[0] = hmm_init(5, 13, "one");
    models[1] = hmm_init(5, 13, "two");
    models[2] = hmm_init(5, 13, "three");

    hmm_train_kmeans(models[0], fs_train_one, 50, 0.001);
    hmm_train_kmeans(models[1], fs_train_two, 50, 0.001);
    hmm_train_kmeans(models[2], fs_train_three, 50, 0.001);

    FeatureSet *fs_test = fs_test_two;

    int *align = (int *) malloc(sizeof(int) * fs_test->feat_sizes[0]);
    fprintf(stderr, "testing 1 \n");
    float model_one_ll = hmm_align_dtw(models[0], fs_test->feat[0], fs_test->feat_sizes[0], fs_test->feat_dim, align);

    fprintf(stderr, "testing 2 \n");
    float model_two_ll = hmm_align_dtw(models[1], fs_test->feat[0], fs_test->feat_sizes[0], fs_test->feat_dim, align);

    fprintf(stderr, "testing 3 \n");
    float model_three_ll = hmm_align_dtw(models[2], fs_test->feat[0], fs_test->feat_sizes[0], fs_test->feat_dim, align);

    fprintf(stderr, "model_one_ll: %f\n", model_one_ll);
    fprintf(stderr, "model_two_ll: %f\n", model_two_ll);
    fprintf(stderr, "model_three_ll: %f\n", model_three_ll);

    return 0;
}
