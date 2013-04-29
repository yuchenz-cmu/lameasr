/*
 * =====================================================================================
 *
 *       Filename:  decoder.c
 *
 *    Description:  Single-word decoder
 *
 *        Version:  1.0
 *        Created:  04/06/2013 04:30:54 PM
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
#include "hmm.h"
#include "topo.h"
#include "align.h"

#define BUF_LEN (512)

void print_usage(char *msg) {
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }
    fprintf(stderr, "Usage: ./decoder --topo <topology file> --model-list <model file list> --feat <feature file> --dim <feature dimension> --sil-word <sil_2> --align-file <align_file>\n");
}

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
    char *topo_file = NULL;
    char *model_list_file = NULL;
    char *feat_file = NULL;
    char *sil_word = "sil_2";
    char *align_file = NULL;
    int feat_dim = 0;
    int arg_idx = 1;

    while (arg_idx < argc) {
        if (!strcmp(argv[arg_idx], "--topo")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            topo_file = argv[arg_idx + 1];
            arg_idx += 2;
        } else if (!strcmp(argv[arg_idx], "--model-list")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            model_list_file = argv[arg_idx + 1];
            arg_idx += 2;
        } else if (!strcmp(argv[arg_idx], "--feat")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            feat_file = argv[arg_idx + 1];
            arg_idx += 2;
        } else if (!strcmp(argv[arg_idx], "--dim")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            feat_dim = atoi(argv[arg_idx + 1]);
            arg_idx += 2;
        } else if (!strcmp(argv[arg_idx], "--sil-word")) {
            if (arg_idx + 1 >= argc) {
                print_usage("Missing argument.");
                return 2;
            }
            sil_word = argv[arg_idx + 1];
            arg_idx += 2;
        } else if (!strcmp(argv[arg_idx], "--align-file")) {
            if (arg_idx + 1 > argc) {
                print_usage("Missing argument.");
                return 2;
            }
            align_file = argv[arg_idx + 1];
            arg_idx += 2;
        } else {
            print_usage("Invalid argument.");
            return 3;
        }
    }

    if (!topo_file || !model_list_file || !feat_file || !feat_dim) {
        print_usage("Missing argument.");
        return 4;
    }
    fprintf(stderr, "Using silent word %s ... \n", sil_word);

    // reads in models
    HMM **hmm_set = NULL;
    FILE *fmodel_list = fopen(model_list_file, "r");
    int hmm_size = read_models(fmodel_list, &hmm_set);
    fclose(fmodel_list);
    fprintf(stderr, "Loaded %d models ... \n", hmm_size);

    // generate transition matrix
    float **trans_matrix = NULL;
    HMMStateMap *state_mapping = NULL;
    int total_states = 0;
    int nodes_num = 0;

    total_states = topo_gen_transmat((void **)hmm_set, hmm_size, topo_file, &trans_matrix, &state_mapping, &nodes_num, 0.0);
    fprintf(stderr, "Loaded topology for HMM.\n");

    // prepare for decoding
    TransMatrix trans_mat;
    FeatureStruct feat_struct;
    FeatureSet *decode_fs = featset_init(feat_dim, 16); 

    trans_mat.trans_matrix = trans_matrix;
    trans_mat.state_hmm_map = state_mapping;
    trans_mat.total_states = total_states;
    trans_mat.dummy_states = nodes_num;

    // read features for decoding
    featset_read_file(feat_file, decode_fs);

    feat_struct.feat = decode_fs->feat[0];
    feat_struct.feat_size = decode_fs->feat_sizes[0];
    feat_struct.feat_dim = decode_fs->feat_dim;

    int *align = (int *) malloc(sizeof(int) * decode_fs->feat_sizes[0]);
    hmm_decode_viterbi(hmm_set, hmm_size, &trans_mat, &feat_struct, align);

    int curr_hmm_id = 0;
    int prev_hmm_id = -1;
    for (int t = 0; t < feat_struct.feat_size; t++) {
        // fprintf(stderr, "%d ", align[t]);
        curr_hmm_id = state_mapping[align[t]].hmm_id;
        // fprintf(stderr, "%d ", curr_hmm_id);
        if (curr_hmm_id != prev_hmm_id) { 
            if (curr_hmm_id >= 0 && strcmp(hmm_set[curr_hmm_id]->lex, sil_word)) {
                printf("%s ", hmm_set[curr_hmm_id]->lex);
            }
        }
        prev_hmm_id = curr_hmm_id;
    }
    printf("\n");

    if (align_file != NULL) {
        fprintf(stderr, "Writing alignment ... ");
        FILE *fp = fopen(align_file, "wb");  
        align_write(align, decode_fs->feat_sizes[0], total_states, state_mapping, fp);
        fclose(fp);
        fprintf(stderr, "done\n");
    }

    return 0;
}
