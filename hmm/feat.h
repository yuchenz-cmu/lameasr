#ifndef FEAT_H
#define FEAT_H

typedef struct struct_feat_set {
    float ***feat;
    int *feat_sizes;
    int feat_num;
    int feat_dim;
    int curr_capacity;
} FeatureSet;

typedef struct struct_feat_struct {
    float **feat;
    int feat_size;
    int feat_dim;
} FeatureStruct;

void featset_print(FeatureSet *fs);
int featset_read_file(char *filename, FeatureSet *fs);
FeatureSet *featset_init(int feat_dim, int init_capacity);
int featset_grow(FeatureSet *fs);
int featset_feat_grow(float ***feat, int feat_size, int feat_dim);

#endif
