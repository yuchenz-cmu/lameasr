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
#include <string.h>
#include <float.h>
#include "gmm.h"
#include "feat.h"
#include "hmm.h"

#define BUF_LEN (512)

void print_usage() {
    fprintf(stderr, "Usage: ./controller <feature dimension> <train_file> <test_file>\n");
}

int str2num(char *strin) {
    if (!strcmp(strin, "one")) {
        return 1;
    } else if (!strcmp(strin, "two")) {
        return 2;
    } else if (!strcmp(strin, "three")) {
        return 3;
    } else if (!strcmp(strin, "four")) {
        return 4;
    } else if (!strcmp(strin, "five")) {
        return 5;
    } else if (!strcmp(strin, "six")) {
        return 6;
    } else if (!strcmp(strin, "seven")) {
        return 7;
    } else if (!strcmp(strin, "eight")) {
        return 8;
    } else if (!strcmp(strin, "nine")) {
        return 9;
    } else if (!strcmp(strin, "zero")) {
        return 0;
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        print_usage();
        return 2;
    }

    int feat_dim = atoi(argv[1]);
    char *train_file = argv[2];
    char *test_file = argv[3];
    char line_buf[BUF_LEN];
    char *num2str[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    char *curr_num = NULL;
    int curr_num_int;
    char *curr_feat_file = NULL;

    fprintf(stderr, "Feature dimension: %d\n", feat_dim);

    HMM **models = (HMM **) malloc(sizeof(HMM *) * 10);
    FeatureSet **fs_train = (FeatureSet **) malloc(sizeof(FeatureSet *) * 10);
    // FeatureSet **fs_test = (FeatureSet **) malloc(sizeof(FeatureSet *) * 10);
    FeatureSet *fs_test = featset_init(feat_dim);

    for (int i = 0; i < 10; i++) {
        models[i] = hmm_init(5, 1, feat_dim, num2str[i]);
        fs_train[i] = featset_init(feat_dim);
    }

    // do training
    FILE *ftrain_list = fopen(train_file, "r");
    while (fgets(line_buf, BUF_LEN, ftrain_list) != NULL) {
        line_buf[strlen(line_buf) - 1] = 0;
        fprintf(stderr, "Reading feature from %s ... \n", line_buf);
        curr_num = strtok(line_buf, " ");
        curr_num_int = str2num(curr_num);
        curr_feat_file = strtok(NULL, " ");
        featset_read_file(curr_feat_file, fs_train[curr_num_int]);

    }
    fclose(ftrain_list);

    for (int i = 0; i < 10; i++) {
        // hmm_train_kmeans(models[0], fs_train_one, 50, 0.001);
        fprintf(stderr, "Training %d ... \n", i);
        hmm_train_kmeans(models[i], fs_train[i], 50, 0.001);
    }

    // do testing
    int max_prob_idx = -1;
    float max_prob = 0.0;
    float curr_prob = 0.0;

    FILE *ftest_list = fopen(test_file, "r");

    while (fgets(line_buf, BUF_LEN, ftest_list) != NULL) {
        line_buf[strlen(line_buf) - 1] = 0;
        curr_num = strtok(line_buf, " ");
        curr_num_int = str2num(curr_num);
        curr_feat_file = strtok(NULL, " ");
        featset_read_file(curr_feat_file, fs_test);
        
        fprintf(stderr, "Testing on %d %s ...\n", curr_num_int, curr_feat_file);
        
        int feat_idx = fs_test->feat_num - 1;
        max_prob_idx = -1;
        max_prob = -FLT_MAX;
        // float model_three_ll = hmm_align_dtw(models[2], fs_test->feat[0], fs_test->feat_sizes[0], fs_test->feat_dim, align);
        int *align = (int *) malloc(sizeof(int) * fs_test->feat_sizes[feat_idx]);

        for (int i = 0; i < 10; i++) {
            curr_prob = hmm_align_dtw(models[i], fs_test->feat[feat_idx], fs_test->feat_sizes[feat_idx], feat_dim, align);
            if (curr_prob > max_prob) {
                max_prob = curr_prob;
                max_prob_idx = i;
            }
        }

        free(align);
        fprintf(stdout, "%s: %s\n", curr_num, num2str[max_prob_idx]);
    }

    return 0;
}

/* 
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
    models[0] = hmm_init(5, 1, 13, "one");
    models[1] = hmm_init(5, 1, 13, "two");
    models[2] = hmm_init(5, 1, 13, "three");

    fprintf(stderr, "Training ... \n");
    hmm_train_kmeans(models[0], fs_train_one, 50, 0.001);
    hmm_train_kmeans(models[1], fs_train_two, 50, 0.001);
    hmm_train_kmeans(models[2], fs_train_three, 50, 0.001);

    FeatureSet *fs_test = fs_test_three;

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
*/ 
