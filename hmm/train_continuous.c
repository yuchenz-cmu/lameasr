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

int main(int argc, char **argv) {
    char *model_list = "all_models.list";
    char *database_file = "train.db";
    HMM **hmm_set = NULL;
    int max_iter = 100;
    float tolerance = 0.001;
    char *result_dir = "train_result";
    char filename_buf[512];

    FILE *fmodel_list = fopen(model_list, "r");
    int hmm_size = read_models(fmodel_list, &hmm_set);
    fclose(fmodel_list); 

    Database *train_db = read_database(database_file);

    // void hmm_train_continuous(HMM **hmm_set, int hmm_size, Database *db, int feat_dim, int max_iter, float tolerance)     
    
    hmm_train_continuous(hmm_set, hmm_size, train_db, 13, max_iter, tolerance);

    for (int h = 0; h < hmm_size; h++) {
        sprintf(filename_buf, "%s/%s.hmm", result_dir, hmm_set[h]->lex);
        fprintf(stderr, "Writing model for %s\n", filename_buf);
        hmm_write(hmm_set[h], filename_buf);
    }
    
    return 0;
}

