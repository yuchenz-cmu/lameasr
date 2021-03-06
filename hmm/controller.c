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
    char fname_buf[256];

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
    printf("Reading features ... \n");
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

    printf("Training ... \n");
    for (int i = 0; i < 10; i++) {
        // hmm_train_kmeans(models[0], fs_train_one, 50, 0.001);
        fprintf(stderr, "Training %d ... \n", i);
        sprintf(fname_buf, "models/%s.hmm", num2str[i]);
        hmm_train_kmeans(models[i], fs_train[i], 50, 0.001);
        hmm_write(models[i], fname_buf);
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
    fclose(ftest_list);


    // single test
    printf("\nSingle test :\n");
    featset_read_file("recorded_0.mfcc.visual", fs_test);

    int feat_idx =fs_test->feat_num - 1;
    max_prob_idx = -1;
    max_prob = -FLT_MAX;
    curr_num = "eight";
    int *align = (int *) malloc(sizeof(int) * fs_test->feat_sizes[feat_idx]);

    for (int i = 0; i < 10; i++) {
        curr_prob = hmm_align_dtw(models[i], fs_test->feat[feat_idx], fs_test->feat_sizes[feat_idx], feat_dim, align);
        if (curr_prob > max_prob) {
            max_prob = curr_prob;
            max_prob_idx = i;
        }
    }
    free(align);
    fprintf(stdout, "%s\n", num2str[max_prob_idx]);

    return 0;
}
