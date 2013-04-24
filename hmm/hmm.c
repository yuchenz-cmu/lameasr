/*
 * =====================================================================================
 *
 *       Filename:  hmm.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/02/2013 05:13:34 PM
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
#include <float.h>
#include <math.h>
#include <string.h>
#include "gmm.h"
#include "hmm.h"
#include "feat.h"
#include "utils.h"
#include "topo.h"
#include "database.h"

#define MAX_GAUSSIANS (4)

void hmm_align_equal(int feat_size, int slice, int *align) {
    assert (feat_size >= slice);
    int slice_len = feat_size / slice + 1;
    for (int t = 0; t < feat_size; t++) {
        align[t] = t / slice_len;
    }
}

float hmm_decode_viterbi(HMM **hmm_set, int hmm_size, TransMatrix *trans_mat, FeatureStruct *feat_struct, int *align) {
    fprintf(stderr, "hmm_decode_viterbi():\n");
    assert (hmm_size > 0);
    int total_states = trans_mat->total_states;
    int dummy_states = trans_mat->dummy_states;
    float **trans_matrix = trans_mat->trans_matrix;
    HMMStateMap *state_hmm_map = trans_mat->state_hmm_map;
    int feat_size = feat_struct->feat_size;
    int feat_dim = feat_struct->feat_dim;
    float **feat = feat_struct->feat;
    
    // allocate the trellis
    HMMTrellis **trellis = (HMMTrellis **) malloc(sizeof(HMMTrellis *) * feat_size);
    for (int t = 0; t < feat_size; t++) {
        trellis[t] = (HMMTrellis *) malloc(sizeof(HMMTrellis) * total_states);
        for (int s = 0; s < total_states; s++) {
            trellis[t][s].value = -FLT_MAX;
            trellis[t][s].prev.s = -1;
        }
    }

    // start state
    trellis[0][0].value = 0.0;
    for (int s = 0; s < total_states; s++){
        if (trans_matrix[0][s] > -FLT_MAX) {
            int s_hmm_idx = state_hmm_map[s].hmm_id;
            int s_hmm_state_idx = state_hmm_map[s].hmm_state_id;
            float gmm_ll = gmm_likelihood(hmm_set[s_hmm_idx]->states[s_hmm_state_idx].gmm, feat[0], feat_dim);
            trellis[0][s].value = trellis[0][0].value + trans_matrix[0][s] + gmm_ll;
        }
    }
    
    // here we go ... 
    int t = 0;
    int ts_hmm_idx = 0;
    int ds_hmm_idx = 0;
    int ts_hmm_state_idx = 0;
    int ds_hmm_state_idx = 0;
    float gmm_ll = -FLT_MAX;
    float curr_ll = -FLT_MAX;

    while (t < feat_struct->feat_size - 1) {
        // fill next column's regular states with all states from this column
        for (int s = 0; s < total_states; s++) {
            for (int ts = dummy_states; ts < total_states; ts++) {
                if (trans_matrix[s][ts] <= -FLT_MAX) {
                    continue;
                }

                // fill in the next column of trellis
                ts_hmm_idx = state_hmm_map[ts].hmm_id;
                ts_hmm_state_idx = state_hmm_map[ts].hmm_state_id;
                gmm_ll = gmm_likelihood(hmm_set[ts_hmm_idx]->states[ts_hmm_state_idx].gmm, feat[t + 1], feat_dim);
                curr_ll = trellis[t][s].value + trans_matrix[s][ts] + gmm_ll;
                if (curr_ll > trellis[t + 1][ts].value) {
                    trellis[t + 1][ts].value = curr_ll;
                    trellis[t + 1][ts].prev.s = s;
                    trellis[t + 1][ts].prev.t = t;
                }
            }
            
        }

        // fill next column's dummy states with regular states from next column
        for (int s = dummy_states; s < total_states; s++) {
            for (int ds = 0; ds < dummy_states; ds++) {
                if (trans_matrix[s][ds] <= -FLT_MAX) {
                    continue;
                }

                curr_ll = trellis[t + 1][s].value + trans_matrix[s][ds];
                if (curr_ll > trellis[t + 1][ds].value) {
                    trellis[t + 1][ds].value = curr_ll;
                    trellis[t + 1][ds].prev.s = s;
                    trellis[t + 1][ds].prev.t = t;
                }
            }
        }

        t++;
    }

    fprintf(stderr, "Doing back trace ... \n");
    // backtrace
    int end_state = dummy_states - 1;
    float end_ll = trellis[feat_size - 1][end_state].value; 
    int curr_st = end_state;
    int curr_t = feat_size - 1;
    while (curr_t >= 0) {
        // we only keep alignment of regular states, ignore dummy states
        while (curr_st < dummy_states) {
            curr_st = trellis[curr_t][curr_st].prev.s;
        }
        align[curr_t] = curr_st;
        curr_st = trellis[curr_t][curr_st].prev.s;
        curr_t--;
    }

    // free-up memory
    for (int t = 0; t < feat_size; t++) {
        free(trellis[t]);
    }
    free(trellis);

    return end_ll;
}

float hmm_align_dtw(HMM* hmm, float **feat, int feat_size, int feat_dim, int *align) {
    assert (hmm->state_num > 0);
    assert (hmm->states[0].gmm->feat_dim == feat_dim);

    // allocate the trellis
    HMMTrellis **trellis = (HMMTrellis **) malloc(sizeof(HMMTrellis *) * feat_size);
    for (int t = 0; t < feat_size; t++) {
        trellis[t] = (HMMTrellis *) malloc(sizeof(HMMTrellis) * hmm->state_num);
    }

    // DTW here we go ...
    // first column, t==0
    for (int s = 0; s < hmm->state_num; s++) {
        trellis[0][s].value = gmm_likelihood(hmm->states[s].gmm, feat[0], feat_dim);
        // fprintf(stderr, "%f ", trellis[0][s].value);
    }
    // fprintf(stderr, "\n");

    for (int t = 1; t < feat_size; t++) {
        for (int s = 0; s < hmm->state_num; s++) {
            float curr_ll = gmm_likelihood(hmm->states[s].gmm, feat[t], feat_dim);

            float max_prev_ll = trellis[t-1][s].value;
            int max_prev_st = s;

            // test [s-1][t-1] and [s-2][t-1]
            for (int k = 1; k <= 2; k++) {
            // for (int k = 1; k < 2; k++) {
                if (s - k >= 0) {
                    float prev_ll = trellis[t-1][s-k].value;
                    if (prev_ll > max_prev_ll) {
                        max_prev_ll = prev_ll;
                        max_prev_st = s - k;
                    }
                }
            }

            trellis[t][s].value = curr_ll + max_prev_ll;
            trellis[t][s].prev.s = max_prev_st;
            trellis[t][s].prev.t = t - 1;
            // fprintf(stderr, "%f ", trellis[t][s].value);
        }
        // fprintf(stderr, "\n");
    }

    // backtrace
    float max_end = -FLT_MAX;
    int max_end_st = -1;

    for (int s = 0; s < hmm->state_num; s++) {
        if (trellis[feat_size - 1][s].value > max_end) {
            max_end = trellis[feat_size - 1][s].value;
            max_end_st = s;
        }
    }

    int curr_st = max_end_st;
    int curr_t = feat_size - 1;
    while (curr_t >= 0) {
        align[curr_t] = curr_st;
        curr_st = trellis[curr_t][curr_st].prev.s;
        curr_t--;
    }

    // free-up memory
    for (int t = 0; t < feat_size; t++) {
        free(trellis[t]);
    }
    free(trellis);

    return max_end;
}

HMM *hmm_init(int state_num, int gs_num, int feat_dim, char *lex) {
    // initialize HMM
    HMM *hmm = (HMM *) malloc(sizeof(HMM));

    // initialize states
    hmm->states = (HMMState *) malloc(sizeof(HMMState) * state_num);
    hmm->state_num = state_num;

    // initialize GMM for each state
    for (int s = 0; s < state_num; s++) {
        // hmm->states[s].id = s;
        hmm->states[s].id = 0;
        hmm->states[s].gmm = gmm_init(gs_num, feat_dim);
    }
    hmm->lex = lex;

    return hmm;
}

void hmm_clear_gmm(HMM *hmm) {
    for (int s = 0; s < hmm->state_num; s++) {
        gmm_clear(hmm->states[s].gmm);
    }
}

void hmm_update_gmm(HMM *hmm, int hmm_id, FeatureSet *fs, int **alignset, TransMatrix *trans_mat_set) {
    assert (hmm->state_num > 0);

    float *acc_gamma = NULL;
    float **acc_mean = NULL;
    float **acc_var = NULL;
    float *gs_gammas = NULL;
    float gammaD = 0.0;
    int mixture_num = 0;
    float **curr_feat = NULL;
    int curr_feat_size = 0;

    for (int s = 0; s < hmm->state_num; s++) {
        // fprintf(stderr, "Training state %d ... \n", s);
        // initialize all accumulators
        mixture_num = hmm->states[s].gmm->mixture_num;
        acc_gamma = (float *) malloc(sizeof(float) * mixture_num);
        gs_gammas = (float *) malloc(sizeof(float) * mixture_num);
        acc_mean = (float **) malloc(sizeof(float *) * mixture_num);
        acc_var = (float **) malloc(sizeof(float *) * mixture_num);
        for (int g = 0; g < mixture_num; g++) {
            acc_mean[g] = (float *) malloc(sizeof(float) * fs->feat_dim);
            acc_var[g] = (float *) malloc(sizeof(float) * fs->feat_dim);
        }
        gammaD = 0.0;

        // set to zero
        for (int g = 0; g < mixture_num; g++) {
            acc_gamma[g] = 0.0;
            gs_gammas[g] = 0.0;
            for (int d = 0; d < fs->feat_dim; d++) {
                acc_mean[g][d] = 0.0;
                acc_var[g][d] = 0.0;
            }
        }

        // for every utterance
        for (int idx = 0; idx < fs->feat_num; idx++) {
            curr_feat = fs->feat[idx];
            curr_feat_size = fs->feat_sizes[idx];
            HMMStateMap *curr_state_map = trans_mat_set[idx].state_hmm_map;

            // for every frame
            for (int t = 0; t < curr_feat_size; t++) {
                // only update the frames assigned to the state s
                if (curr_state_map[alignset[idx][t]].hmm_id != hmm_id || curr_state_map[alignset[idx][t]].hmm_state_id != s) {
                    continue;
                }
                // fprintf(stderr, "hmm_id: %d, hmm_state_id: %d, align_state_id: %d\n", hmm_id, s, alignset[idx][t]);

                gammaD = 0.0;
                // for every gaussian
                // fprintf(stderr, "gs_gammas[g]: ");
                for (int g = 0; g < mixture_num; g++) {
                    gs_gammas[g] = exp(gmm_likelihood_idx(hmm->states[s].gmm, g, curr_feat[t], fs->feat_dim)) * hmm->states[s].gmm->weight[g];
                    gammaD += gs_gammas[g];
                    // fprintf(stderr, "%f ", gs_gammas[g]);
                }
                // fprintf(stderr, "\n");
                // fprintf(stderr, "gammaD: %f\n", gammaD);

                // for every gaussian
                // fprintf(stderr, "curr_gamma: ");
                for (int g = 0; g < mixture_num; g++) {
                    float curr_gamma = (float) gs_gammas[g] / (float) gammaD;
                    // fprintf(stderr, "%f ", curr_gamma);
                    acc_gamma[g] += curr_gamma;
                    for (int d = 0; d < fs->feat_dim; d++) {
                        acc_mean[g][d] += curr_gamma * curr_feat[t][d];
                        acc_var[g][d] += curr_gamma * curr_feat[t][d] * curr_feat[t][d];
                    }
                }
                // fprintf(stderr, "\n");
            }

        }
        // for every gaussian, update mean, var and weight
        float all_gammas = 0.0;
        for (int g = 0; g < mixture_num; g++) {
            all_gammas += acc_gamma[g];
        }

        for (int g = 0; g < mixture_num; g++) {
            fprintf(stderr, "mean: ");
            for (int d = 0; d < fs->feat_dim; d++) {
                float new_mean = (float) acc_mean[g][d] / (float) acc_gamma[g];
                hmm->states[s].gmm->mean[g][d] = new_mean;
                hmm->states[s].gmm->var[g][d] = (float) acc_var[g][d] / (float) acc_gamma[g] - new_mean * new_mean;
                fprintf(stderr, "%f ", hmm->states[s].gmm->mean[g][d]);
            }
            fprintf(stderr, "\n");
            hmm->states[s].gmm->weight[g] = (float) acc_gamma[g] / (float) all_gammas;
        }

        // free up memory
        for (int g = 0; g < mixture_num; g++) {
            free(acc_mean[g]);
            free(acc_var[g]);
        }
        free(acc_mean);
        free(acc_var);
        free(acc_gamma);
        free(gs_gammas);
        acc_mean = NULL;
        acc_var = NULL;
        acc_gamma = NULL;
        gs_gammas = NULL;
    }
}

void hmm_train_continuous(HMM **hmm_set, int hmm_size, Database *db, int feat_dim, int max_iter, float tolerance) {
    assert (hmm_size > 0);

    // prepare the FeatureSet from database
    fprintf(stderr, "Reading features ... \n");
    FeatureSet *fs = featset_init(feat_dim);
    for (int r = 0; r < db->record_size; r++) {
        if (db->record[r].feat_file[strlen(db->record[r].feat_file) - 1] == '\n') {
            db->record[r].feat_file[strlen(db->record[r].feat_file) - 1] = '\0';
        }
        fprintf(stderr, "Reading from %s ... \n", db->record[r].feat_file);
        featset_read_file(db->record[r].feat_file, fs);
    }
    fprintf(stderr, "done.\n");

    float avg_ll = -FLT_MAX;
    float bestpath_ll = 0.0;
    float curr_ll = 0.0;
    int iter = 0;

    // for every training utterance (feature), prepare a transition matrix
    fprintf(stderr, "Generating topologies for training instances ... \n");
    TransMatrix *trans_mat_set = (TransMatrix *) malloc(sizeof(TransMatrix) * db->record_size);
    for (int t = 0; t < db->record_size; t++) {
        trans_mat_set[t].trans_matrix = NULL;
        trans_mat_set[t].state_hmm_map = NULL;
        trans_mat_set[t].total_states = 0;
        trans_mat_set[t].dummy_states = 0;

        // total_states = topo_gen_transmat(hmm_set, hmm_size, topo_file, &trans_matrix, &state_mapping, &nodes_num, 0.0);
        trans_mat_set[t].total_states = topo_gen_transmat(hmm_set, hmm_size, db->record[t].topo_file, &trans_mat_set[t].trans_matrix, &trans_mat_set[t].state_hmm_map, &trans_mat_set[t].dummy_states, 0.0);
    }

    // for every training utterance (feature), prepare a feature structure
    FeatureStruct *feat_struct_set = (FeatureStruct *) malloc(sizeof(FeatureStruct) * db->record_size);
    for (int f = 0; f < db->record_size; f++) {
        feat_struct_set[f].feat = fs->feat[f];
        feat_struct_set[f].feat_size = fs->feat_sizes[f];
        feat_struct_set[f].feat_dim = fs->feat_dim;
    }

    // prepare the alignment set (each alignment is of length of its feature)
    int **alignset = (int **) malloc(sizeof(int *) * db->record_size);
    for (int idx = 0; idx < fs->feat_num; idx++) {
        alignset[idx] = (int *) malloc(sizeof(int) * fs->feat_sizes[idx]);
    }

    while (iter < max_iter) {
        // for every feature, compute the alignment
        for (int feat_idx = 0; feat_idx < fs->feat_num; feat_idx++) {
            // hmm_decode_viterbi(hmm_set, hmm_size, &trans_mat, &feat_struct, align);
            curr_ll = hmm_decode_viterbi(hmm_set, hmm_size, &trans_mat_set[feat_idx], &feat_struct_set[feat_idx], alignset[feat_idx]);
            bestpath_ll += curr_ll;
            fprintf(stderr, "Likelihood for utterance %d is %f\n", feat_idx, curr_ll);
            fprintf(stderr, "Alignment for '%s' is \n", db->record[feat_idx].text);

            int prev_hmm_id = -1;
            for (int t = 0; t < fs->feat_sizes[feat_idx]; t++) {
                int curr_hmm_id = trans_mat_set[feat_idx].state_hmm_map[alignset[feat_idx][t]].hmm_id;
                if (curr_hmm_id < 0 || curr_hmm_id >= hmm_size) {
                    continue;
                }

                if (curr_hmm_id != prev_hmm_id) {
                    fprintf(stderr, "%d(%s) ", curr_hmm_id,  hmm_set[curr_hmm_id]->lex);
                    prev_hmm_id = curr_hmm_id;
                }
            }
            fprintf(stderr, "\n");

            for (int t = 0; t < fs->feat_sizes[feat_idx]; t++) {
                fprintf(stderr, "%d ", alignset[feat_idx][t]);
            }
            fprintf(stderr, "\n");
        }

        // update the hmm parameters
        // hmm_update_gmm(HMM *hmm, int hmm_id, FeatureSet *fs, int **alignset, HMMStateMap *state_hmm_map)
        for (int h = 0; h < hmm_size; h++) {
            fprintf(stderr, "HMM %d ... \n", h);
            hmm_update_gmm(hmm_set[h], h, fs, alignset, trans_mat_set);
        }

        bestpath_ll /= (float) fs->feat_num;
        fprintf(stderr, "Current likelihood: %f\n", bestpath_ll);

        if (bestpath_ll < avg_ll) {
            fprintf(stderr, "Liklihood decreased, something bad happened.\n");
            break;
        }

        if (bestpath_ll - avg_ll < tolerance) {
            fprintf(stderr, "Likelihood converged.\n");
            break;
        }

        iter++;
        avg_ll = bestpath_ll;
        bestpath_ll = 0.0;
    }

    // free up the memory
    for (int t = 0; t < db->record_size; t++) {
        free(alignset[t]);
    }
    free(trans_mat_set);
    free(feat_struct_set);
    free(alignset);
}

/*
 * Given a FeatureSet, an HMM, updates the HMM GMM's parameters iteratively
 * (First iteration is divided equally, the rest uses DTW alignment)
 * */
void hmm_train_kmeans(HMM *hmm, FeatureSet *fs, int max_iter, float tolerance) {
    assert (fs->feat_num > 0);

    // clears the GMMs and set global mean, var and equal weight
    hmm_clear_gmm(hmm);
    for (int s = 0; s < hmm->state_num; s++) {
        gmm_mean_var(hmm->states[s].gmm, fs->feat[0], fs->feat_sizes[0], fs->feat_dim);

        float eq_wght = 1.0 / (float) hmm->states[s].gmm->mixture_num;
        for (int g = 0; g < hmm->states[s].gmm->mixture_num; g++) {
            hmm->states[s].gmm->weight[g] = eq_wght;
        }
    }

    // create the alignment tables
    int **alignset = (int **) malloc(sizeof(int *) * fs->feat_num);
    for (int idx = 0; idx < fs->feat_num; idx++) {
        alignset[idx] = (int *) malloc(sizeof(int) * fs->feat_sizes[idx]);

        // initial equal alignment
        hmm_align_equal(fs->feat_sizes[idx], hmm->state_num, alignset[idx]);
    }

    // keep track of likelihoods of the previous run
    float avg_ll = -FLT_MAX;
    float bestpath_ll = 0.0;
    float curr_ll = 0.0;
    int iter = 0;
    int curr_gs_num = 1;

    while (iter < max_iter) {

        // splitting gaussians
        if (iter % 4 == 3 && curr_gs_num < MAX_GAUSSIANS) {
        // if (0) {
            fprintf(stderr, "Splitting gaussians ... \n");
            for (int s = 0; s < hmm->state_num; s++) {
                gmm_split_max_var(hmm->states[s].gmm);
            }
            float avg_ll = -FLT_MAX;
            curr_gs_num++;
        }

        fprintf(stderr, "Iteration: %d\n", iter);
        fprintf(stderr, "weights: ");
        for (int g = 0; g < hmm->states[0].gmm->mixture_num; g++) {
            fprintf(stderr, "%f ", hmm->states[0].gmm->weight[g]);
        }
        fprintf(stderr, "\n");

        // clears the hmm
        // hmm_clear_gmm(hmm);

        // update the model
        /*
        for (int idx = 0; idx < fs->feat_num; idx++) {
            hmm_update_gmm(hmm, fs->feat[idx], fs->feat_sizes[idx], fs->feat_dim, alignset[idx]);
        }
        */
        // hmm_update_gmm(hmm, fs, alignset);

        // compute the alignment
        for (int idx = 0; idx < fs->feat_num; idx++) {
            // for (int t = 0; t < fs->feat_sizes[idx]; t++) {
            //     fprintf(stderr, "%d ", alignset[idx][t]);
            // }
            // fprintf(stderr, "\n");

            curr_ll = hmm_align_dtw(hmm, fs->feat[idx], fs->feat_sizes[idx], fs->feat_dim, alignset[idx]) / (float) fs->feat_sizes[idx];
            bestpath_ll += curr_ll;
            fprintf(stderr, "idx: %d, curr_ll: %f\n", idx, curr_ll);
        }

        bestpath_ll /= (float) fs->feat_num;
        if (bestpath_ll < avg_ll) {
            fprintf(stderr, "Liklihood decreased, something bad happened.\n");
            break;
        }

        if (bestpath_ll - avg_ll < tolerance) {
            fprintf(stderr, "Likelihood converged.\n");
            break;
        }

        iter++;
        avg_ll = bestpath_ll;
        bestpath_ll = 0.0;
    }

    for (int idx = 0; idx < fs->feat_num; idx++) {
        free(alignset[idx]);
    }
    free(alignset);
}

int hmm_write(HMM *hmm, char *filename) {
    /*
    [state_num][lex_len][lex ... ][feat_dim][mixture_num]
    [state_id 1]
    [gmm_feat_dim][gmm_mixture_num][gmm_var_const][gmm_feat_count][gmm_gamma_count]
    gmm_mean[0][d1], gmm_mean[0][d2] ... gmm_mean[0][d13]
    gmm_mean[1][d1], ...
    ...
    gmm_var[0][d1], gmm_var[0][d2] ... gmm_var[0][d13]
    ...
    gmm_weight[0], gmm_weight[1] ...

    */
    FILE *fp = fopen(filename, "wb");
    int hmm_lex_len = strlen(hmm->lex) + 1;
    HMMState *curr_st = NULL;
    GMM *curr_gmm = NULL;

    // write the top level structures
    fwrite(&hmm->state_num, sizeof(int),1, fp);
    fwrite(&hmm_lex_len, sizeof(int), 1, fp);
    fwrite(hmm->lex, sizeof(char), hmm_lex_len, fp);
    fwrite(&hmm->states[0].gmm->feat_dim, sizeof(int), 1, fp);
    fwrite(&hmm->states[0].gmm->mixture_num, sizeof(int), 1, fp);

    // write the states
    for (int s = 0; s < hmm->state_num; s++) {
        curr_st = &hmm->states[s];
        curr_gmm = curr_st->gmm;

        // write the state id
        fwrite(&curr_st->id, sizeof(int), 1, fp);

        // write the GMM parameters
        fwrite(&curr_gmm->feat_dim, sizeof(int), 1, fp);
        fwrite(&curr_gmm->mixture_num, sizeof(int), 1, fp);
        fwrite(&curr_gmm->var_const, sizeof(float), 1, fp);
        fwrite(&curr_gmm->feat_count, sizeof(long), 1, fp);
        fwrite(&curr_gmm->gamma_count, sizeof(float), 1, fp);

        // write the GMM mean matrix
        fprintf(stderr, "writing mean: \n");
        for (int g = 0; g < curr_gmm->mixture_num; g++) {
            fwrite(curr_gmm->mean[g], sizeof(float), curr_gmm->feat_dim, fp);
            for (int d = 0; d < curr_gmm->feat_dim; d++) {
                fprintf(stderr, "%f ", curr_gmm->mean[g][d]);
            }
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");

        fprintf(stderr, "writing var: \n");
        // write the variance matrix
        for (int g = 0; g < curr_gmm->mixture_num; g++) {
            fwrite(curr_gmm->var[g], sizeof(float), curr_gmm->feat_dim, fp);
            for (int d = 0; d < curr_gmm->feat_dim; d++) {
                fprintf(stderr, "%f ", curr_gmm->var[g][d]);
            }
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");

        fprintf(stderr, "Writing weights: \n");
        for (int d = 0; d < curr_gmm->mixture_num; d++) {
            fprintf(stderr, "%f ", curr_gmm->weight[d]);
        }
        fprintf(stderr, "\n");
        // writes the weights
        fwrite(curr_gmm->weight, sizeof(float), curr_gmm->mixture_num, fp);
    }

    fclose(fp);
    return 0;
}

HMM* hmm_read(char *filename) {
    FILE *fp = fopen(filename, "rb");
    int hmm_state_num = 0;
    int hmm_lex_len = 0;
    char *hmm_lex = NULL;
    // char hmm_lex[256];
    HMM *hmm = NULL;
    GMM *gmm = NULL;
    int gmm_feat_dim = 0;
    int gmm_mixture_num = 0;

    // reads in the HMM top level structures
    fread(&hmm_state_num, sizeof(int), 1, fp);
    fprintf(stderr, "HMM state number: %d\n", hmm_state_num);

    fread(&hmm_lex_len, sizeof(int), 1, fp);
    // fprintf(stderr, "HMM lex length: %d\n", hmm_lex_len);

    hmm_lex = (char *) malloc(sizeof(char) * hmm_lex_len);
    fread(hmm_lex, sizeof(char), hmm_lex_len, fp);
    fprintf(stderr, "HMM lex: %s\n", hmm_lex);

    fread(&gmm_feat_dim, sizeof(int), 1, fp);
    fprintf(stderr, "HMM feature dimension: %d\n", gmm_feat_dim);
    fread(&gmm_mixture_num, sizeof(int), 1, fp);
    fprintf(stderr, "Number of Gaussians: %d\n", gmm_mixture_num);

    // initialize the HMM
    hmm = hmm_init(hmm_state_num, gmm_mixture_num, gmm_feat_dim, hmm_lex);

    // reads in all the GMMs
    for (int s = 0; s < hmm->state_num; s++) {
        gmm = hmm->states[s].gmm;
        fread(&hmm->states[s].id, sizeof(int), 1, fp);
        fread(&gmm->feat_dim, sizeof(int), 1, fp);
        fread(&gmm->mixture_num, sizeof(int), 1, fp);
        fread(&gmm->var_const, sizeof(float), 1, fp);
        fread(&gmm->feat_count, sizeof(long), 1, fp);
        fread(&gmm->gamma_count, sizeof(float), 1, fp);
        // fprintf(stderr, "state id: %d\n", hmm->states[s].id);
        // fprintf(stderr, "gmm mixture_num: %d\n", gmm->mixture_num);
        // fprintf(stderr, "gmm feat_dim: %d\n", gmm->feat_dim);

        // reads in the mean
        for (int g = 0; g < gmm->mixture_num; g++) {
            fread(gmm->mean[g], sizeof(float), gmm->feat_dim, fp);
        }
        // fprintf(stderr, "Read means ... \n");

        // reads in the var
        for (int g = 0; g < gmm->mixture_num; g++) {
            fread(gmm->var[g], sizeof(float), gmm->feat_dim, fp);
        }
        // fprintf(stderr, "Read vars ... \n");

        // reads in the weights
        fread(gmm->weight, sizeof(float), gmm->mixture_num, fp);
        // fprintf(stderr, "Read weights ... \n");

        // fprintf(stderr, "Means: \n");
        // for (int g = 0; g < gmm->mixture_num; g++) {
        //     for (int d = 0; d < gmm->feat_dim; d++) {
        //         fprintf(stderr, "%f ", gmm->mean[g][d]);
        //     }
        //     fprintf(stderr, "\n");
        // }

        // fprintf(stderr, "Vars: \n");
        // for (int g = 0; g < gmm->mixture_num; g++) {
        //     for (int d = 0; d < gmm->feat_dim; d++) {
        //         fprintf(stderr, "%f ", gmm->var[g][d]);
        //     }
        //     fprintf(stderr, "\n");
        // }

        // fprintf(stderr, "Weights: \n");
        // for (int g = 0; g < gmm->mixture_num; g++) {
        //     fprintf(stderr, "%f ", gmm->weight[g]);
        // }
        // fprintf(stderr, "\n");
    }

    fclose(fp);

    return hmm;
}

