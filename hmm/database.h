#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>

#define FILENAME_LEN (512)
#define TEXT_LEN (1024)

typedef struct struct_database_record {
    int utt_id;
    char text[TEXT_LEN];
    char topo_file[FILENAME_LEN]; 
    char feat_file[FILENAME_LEN];
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
