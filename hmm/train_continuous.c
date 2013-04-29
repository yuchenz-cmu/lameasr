/*
 * =====================================================================================
 *
 *       Filename:  train_continuous.c
 *
 *    Description:  Training models from continuous recordings
 *
 *        Version:  1.0
 *        Created:  04/24/2013 12:46:11 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yuchen Zhang (), yuchenz@cs.cmu.edu
 *   Organization:  Carnegie Mellon University
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmm.h"
#include "database.h"
#define BUF_LEN (512)

int read_models(FILE *fmodel_list, HMM ***hmm_set) {
    char line_buf[BUF_LEN];
    int model_num = 0;

    // first read through to see how many lines do we have
        while (fgets(line_buf, BUF_LEN, fmodel_list) != NULL) {
        model_num++;
    }

    // read in the actual HMMs
    rewind(fmodel_list);

    HMM **hmm_list = (HMM **) malloc(sizeof(HMM *) * model_num);
    for (int m = 0; m < model_num; m++) {
        fgets(line_buf, BUF_LEN, fmodel_list);
        line_buf[strlen(line_buf) - 1] = '\0';
        hmm_list[m] = hmm_read(line_buf);
        fprintf(stderr, "Loaded model for %s ... \n", hmm_list[m]->lex);
    }

    *hmm_set = hmm_list;
    return model_num;
}

void print_usage(char *msg) {
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }
    fprintf(stderr, "Usage: ./train_continuous --model-list <initial model list> --db <database file> --result-dir <result dir> --max-iter <maximum iteration> --feat-dim <feature dimension> --init-capacity [initial feature capacity]\n");
}

int main(int argc, char **argv) {
    char *model_list = NULL; // "all_models.list";
    char *database_file = NULL; // "train.db";
    // char *database_file = "train_10.db";
    // char *database_file = "train_1.db";
    HMM **hmm_set = NULL;
    int max_iter = 20;
    float tolerance = 0.00001;
    char *result_dir = NULL; // "train_result";
    char filename_buf[512];
    int feat_dim = 13;
    int i = 1;
    int init_feat_capacity = 64;

    while (i < argc) {
        if (!strcmp(argv[i], "--model-list")) {
            if (i + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            model_list = argv[i + 1];
            i += 2;
        } else if (!strcmp(argv[i], "--db")) {
            if (i + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            database_file = argv[i + 1];
            i += 2;
        } else if (!strcmp(argv[i], "--result-dir")) {
            if (i + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            result_dir = argv[i + 1];
            i += 2;
        } else if (!strcmp(argv[i], "--max-iter")) {
            if (i + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            max_iter = atoi(argv[i + 1]);
            i += 2;
        } else if (!strcmp(argv[i], "--feat-dim")) {
            if (i + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            feat_dim = atoi(argv[i + 1]);
            i += 2;
        } else if (!strcmp(argv[i], "--init-capacity")) {
            if (i + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            init_feat_capacity = atoi(argv[i + 1]);
            i += 2;
        } else {
            fprintf(stderr, "Invalid argument: %s\n", argv[i]);
            print_usage(NULL);
            return 3;
        }
    }

    if (database_file == NULL || max_iter < 1 || result_dir == NULL || feat_dim < 1 || model_list == NULL) {
        print_usage("Missing argument.");
        return 4;
    }

    FILE *fmodel_list = fopen(model_list, "r");
    int hmm_size = read_models(fmodel_list, &hmm_set);
    fclose(fmodel_list);

    Database *train_db = read_database(database_file);

    // void hmm_train_continuous(HMM **hmm_set, int hmm_size, Database *db, int feat_dim, int max_iter, float tolerance)

    hmm_train_continuous(hmm_set, hmm_size, train_db, feat_dim, max_iter, tolerance, init_feat_capacity);

    for (int h = 0; h < hmm_size; h++) {
        sprintf(filename_buf, "%s/%s.hmm", result_dir, hmm_set[h]->lex);
        fprintf(stderr, "Writing model for %s\n", filename_buf);
        hmm_write(hmm_set[h], filename_buf);
    }

    return 0;
}

