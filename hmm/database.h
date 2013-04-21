#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>

typedef struct struct_database_record {
    int utt_id;
    char *text;
    char *topo_file; 
    char *feat_file;
} Record;

typedef struct struct_database {
    Record *record;
    int record_size;
} Database;

typedef struct struct_database_record_node {
    Record record;
    struct struct_database_record_node *next;
} RecordNode;

Database* read_database(char *database_file);

#endif
