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

int main(int argc, char **argv) {
    FeatureSet *fs = featset_init(13);

    featset_read_file("./feats/recorded_0/recorded_0.mfcc.visual", fs);
    featset_read_file("./feats/recorded_1/recorded_0.mfcc.visual", fs);
    featset_read_file("./feats/recorded_2/recorded_0.mfcc.visual", fs);
    featset_read_file("./feats/recorded_3/recorded_0.mfcc.visual", fs);
    featset_read_file("./feats/recorded_4/recorded_0.mfcc.visual", fs);
    featset_print(fs);

    GMM *gmm = gmm_init(1, 13);

    for (int idx = 0; idx < fs->feat_num; idx++) {
        gmm_mean_var(gmm, fs->feat[idx], fs->feat_sizes[idx], fs->feat_dim, 1);
        for (int d = 0; d < gmm->feat_dim; d++) {
            fprintf(stderr, "d: %d\tmean: %.4f\tvar: %.4f \n", d, gmm->mean[0][d], gmm->var[0][d]);
        }
        fprintf(stderr, "gmm->feat_count: %d\n", gmm->feat_count);
    }

    return 0;
}
