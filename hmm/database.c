/*
 * =====================================================================================
 *
 *       Filename:  database.c
 *
 *    Description:  Database used in training and testing
 *
 *        Version:  1.0
 *        Created:  04/21/2013 12:47:16 AM
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
#include "database.h"

#define LINE_BUF (1024)

Database* read_database(char *database_file) {
    
    char line_buf[LINE_BUF];
    RecordNode *head_node = (RecordNode *) malloc(sizeof(RecordNode));
    RecordNode *curr_node = head_node;
    int node_num = 0;
    char *tokstr;

    FILE *fp = fopen(database_file, "r");

    while (fgets(line_buf, LINE_BUF, fp) != NULL) {
        curr_node->record.utt_id = atoi(strtok(line_buf, "\t"));
        strncpy(curr_node->record.text, strtok(NULL, "\t"), TEXT_LEN);
        strncpy(curr_node->record.topo_file, strtok(NULL, "\t"), FILENAME_LEN);
        strncpy(curr_node->record.feat_file, strtok(NULL, "\t"), FILENAME_LEN);

        curr_node->next = (RecordNode *) malloc(sizeof(RecordNode));
        curr_node->next->record.utt_id = -1;    // marks the end
        curr_node = curr_node->next;
        curr_node->next = NULL;
        node_num++;
    }
     
    fclose(fp);

    // go through linked-list
    Database *db = (Database *) malloc(sizeof(Database));
    db->record = (Record *) malloc(sizeof(Record) * node_num);
    db->record_size = node_num;
    curr_node = head_node;
    int record_idx = 0;

    while (curr_node->record.utt_id >= 0) {
        memcpy(&db->record[record_idx], &curr_node->record, sizeof(Record));
        curr_node = curr_node->next;
        record_idx++;
    }


    // free-up memory
    curr_node = head_node;
    RecordNode *pnode = NULL;
    while (curr_node->next != NULL) {
        pnode = curr_node->next;
        free(curr_node);
        curr_node = pnode;
    }

    return db;
}

/* 
int main(int argc, char **argv) {
    Database *db = read_database("train.db");    
    for (int i = 0; i < db->record_size; i++) {
        printf("%d\t%s\t%s\t%s\n", db->record[i].utt_id, db->record[i].text, db->record[i].topo_file, db->record[i].feat_file);
    }
}
*/
