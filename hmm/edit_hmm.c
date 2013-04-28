/*
 * =====================================================================================
 *
 *       Filename:  edit_hmm.c
 *
 *    Description:  Edit information for HMM
 *
 *        Version:  1.0
 *        Created:  04/28/2013 03:11:29 PM
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

int main(int argc, char **argv) {
    if (argc != 3 && argc != 2) {
        fprintf(stderr, "edit_hmm: change HMM's lex\n");
        fprintf(stderr, "Usage: ./edit_hmm <model.hmm> [new lex]\n");
        return 2;
    }

    char *hmm_filename = argv[1];
    char *new_lex = NULL;
    if (argc == 3) {
        new_lex = argv[2];
    }

    HMM *model = hmm_read(hmm_filename);
    fprintf(stderr, "Original lex: %s\n", model->lex);

    if (new_lex != NULL) {
        model->lex = new_lex;
        fprintf(stderr, "Changed to lex: %s\n", model->lex);
        hmm_write(model, hmm_filename);
    }

    return 0;
}
