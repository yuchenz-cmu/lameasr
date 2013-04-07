/*
 * =====================================================================================
 *
 *       Filename:  train_single_word.c
 *
 *    Description:  Train a single word model from a list of features
 *
 *        Version:  1.0
 *        Created:  04/06/2013 12:44:58 AM
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
#include "feat.h"
#include "hmm.h"

#define MAX_TRAIN_ITER (50)
#define TRAIN_EPSILON (0.001)
#define STATES_PER_HMM (5)
#define BUF_LEN (512)

void print_usage(char *msg) {
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }
    fprintf(stderr, "Usage: ./train_single_word --feat-list <feat list file> --lex <lex> --model <output model file> --dim <feature dimension>\n");
}

int train_single_word(char *feat_list_file, char *lex, char *model_file, int feat_dim) {
    char line_buf[BUF_LEN];
    FeatureSet *fs_train = featset_init(feat_dim);
    HMM *model = hmm_init(STATES_PER_HMM, 1, feat_dim, lex);

    // read in features
    printf("Reading features ... \n");
    FILE *ftrain_list = fopen(feat_list_file, "r");
    while (fgets(line_buf, BUF_LEN, ftrain_list) != NULL) {
        line_buf[strlen(line_buf) - 1] = 0;
        fprintf(stderr, "Reading feature from %s ... \n", line_buf);
        featset_read_file(line_buf, fs_train);
    }
    fclose(ftrain_list);

    // train HMM on the features
    fprintf(stderr, "Training HMM for %s ... ", lex);
    hmm_train_kmeans(model, fs_train, MAX_TRAIN_ITER, TRAIN_EPSILON); 
    fprintf(stderr, "done.\n");

    // write to disk
    hmm_write(model, model_file);

    return 0;
}

int main(int argc, char **argv) {
    char *feat_list_file = NULL;
    char *lex = NULL; 
    char *model_file = NULL;
    int feat_dim = 0;
    int arg_idx = 1;
    int ret;
        
    while (arg_idx < argc) {
        if (!strcmp(argv[arg_idx], "--feat-list")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            feat_list_file = argv[arg_idx + 1];
            arg_idx += 2;
        } else if (!strcmp(argv[arg_idx], "--lex")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            lex = argv[arg_idx + 1];
            arg_idx += 2;
        } else if (!strcmp(argv[arg_idx], "--model")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            model_file = argv[arg_idx + 1];
            arg_idx += 2;
        } else if (!strcmp(argv[arg_idx], "--dim")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            feat_dim = atoi(argv[arg_idx + 1]);
            arg_idx += 2;
        } else {
            print_usage("Invalid argument.");
            return 3;
        }
    }

    if (!feat_list_file || !lex || !model_file || !feat_dim) {
        print_usage("Missing argument.");
        return 4;
    }

    fprintf(stderr, "Using feature dimension: %d\n", feat_dim);

    // training ... 
    ret = train_single_word(feat_list_file, lex, model_file, feat_dim);

    return 0;
}
