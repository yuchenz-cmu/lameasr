/*
 * =====================================================================================
 *
 *       Filename:  unit_test.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/03/2013 03:14:51 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "uthash.h"

void gen(float ***matrix) {
    int w = 10;
    int h = 20;
    float **grid = (float **) malloc(sizeof(float *) * w); 
    printf("ppp\n");
    for (int i = 0; i < w; i++) {
        grid[i] = (float *) malloc(sizeof(float) * h);
        for (int j = 0; j < h; j++) {
            grid[i][j] = i + j;
        }
    }

    printf("daaaa\n");
    *matrix = grid;
}

typedef struct struct_lex_hmm_hash {
    char *lex;
    int hmm_id;
    UT_hash_handle hh;
} LexHmmHash;

int main(int argc, char **argv) {
    LexHmmHash *lex_hmm_dict = NULL;
    LexHmmHash hash_item;

    hash_item.lex = "one";
    hash_item.hmm_id = 1;
    HASH_ADD_STR(lex_hmm_dict, lex, &hash_item);

    HASH_FIND_STR(lex_hmm_dict, hash_item.lex, 
}

/*  
int main(int argc, char **argv) {

    // fprintf(stderr, "%f\n", FLT_MIN);
    int w = 10;
    int h = 20;
    float **matrix = (float **) malloc(sizeof(float *) * w);
    for (int i = 0; i < w; i++) {
        matrix[i] = (float *) malloc(sizeof(float) * h);
        for (int j = 0; j < h; j++) {
            matrix[i][j] = i + j;
        }
    }

    FILE *fp = fopen("test.bin", "wb");
    // write
    for (int i = 0; i < w; i++) {
        fwrite(matrix[i], sizeof(float), h, fp);
    }
    fclose(fp);

    // read
    float **grid = (float **) malloc(sizeof(float *) * w);
    for (int i = 0; i < w; i++) {
        grid[i] = (float *) malloc(sizeof(float) * h);
    }
    int inum = 0;
    float fnum = 0;
    char mystr[256];
    fp = fopen("nine.hmm", "rb");

    fread(&inum, sizeof(int), 1, fp);
    fprintf(stderr, "HMM state number: %d\n", inum);

    fread(&inum, sizeof(int), 1, fp);
    fprintf(stderr, "HMM lex length: %d\n", inum);

    // hmm_lex = (char *) malloc(sizeof(char) * hmm_lex_len);
    fread(mystr, sizeof(char), inum, fp);
    fprintf(stderr, "HMM lex: %s\n", mystr);

    fread(&inum, sizeof(int), 1, fp);
    fprintf(stderr, "feat_dim: %d\n", inum);
    fread(&inum, sizeof(int), 1, fp);
    fprintf(stderr, "mixture_num: %d\n", inum);

    for (int idx = 0; idx < 5; idx++) {
        // state id 
        fread(&inum, sizeof(int), 1, fp);
        fprintf(stderr, "state_id: %d\n", inum);
        fread(&inum, sizeof(int), 1, fp);
        fread(&inum, sizeof(int), 1, fp);
        fread(&inum, sizeof(float), 1, fp);
        fread(&inum, sizeof(long), 1, fp);
        fread(&inum, sizeof(float), 1, fp);

        float line[13];
        for (int i = 0; i < 14; i++) {
            fread(line, sizeof(float), 13, fp);
            for (int p = 0; p < 13; p++) {
                printf("%f ", line[p]);
            }
            printf("\n");
        }
        fread(line, sizeof(float), 7, fp);

        for (int i = 0; i < 7; i++) {
            printf("%f ", line[i]);
        }
        printf("\n");
    }

 
    fread(&inum, sizeof(int), 1, fp);
    fread(&inum, sizeof(int), 1, fp);
    fread(&inum, sizeof(int), 1, fp);
    fread(&inum, sizeof(float), 1, fp);
    fread(&inum, sizeof(long), 1, fp);
    fread(&inum, sizeof(float), 1, fp);

    for (int i = 0; i < 14; i++) {
        fread(line, sizeof(float), 13, fp);
    }
    for (int i = 0; i < 13; i++) {
        printf("%f ", line[i]);
    }
    printf("\n");

    fclose(fp);

}
*/ 

