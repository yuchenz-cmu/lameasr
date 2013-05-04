/*
 * =====================================================================================
 *
 *       Filename:  trie.c
 *
 *    Description:  Trie based on Trie structure
 *
 *        Version:  1.0
 *        Created:  05/04/2013 12:39:56 PM
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
#include <assert.h>
#include "trie.h"

#define BUF_LEN (128)

Trie* trie_init(char *alphabet_file) {
    FILE *falphabet = fopen(alphabet_file, "r");
    char line[BUF_LEN];
    int lc = 0;

    Trie *trie = (Trie *) malloc(sizeof(Trie));

    // see how many lines we have
    while (fgets(line, BUF_LEN, falphabet) != NULL) {
        if (strlen(line) > 0) {
            lc++;
        }
    }

    // allocate space and read again
    trie->alphabet = (char **) malloc(sizeof(char *) * lc);
    trie->alphabet_size = lc;
    trie->head_node = NULL;

    // rewind
    rewind(falphabet);

    // read again
    fprintf(stderr, "Read alphabet: ");
    for (int i = 0; i < trie->alphabet_size; i++) {
        fgets(line, BUF_LEN, falphabet);
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0'; 
        }
        trie->alphabet[i] = (char *) malloc(sizeof(char) * strlen(line) + 1);
        strncpy(trie->alphabet[i], line, strlen(line));
        fprintf(stderr, "%s ", trie->alphabet[i]);
    }
    fprintf(stderr, "\n");

    fclose(falphabet);
    return trie;
}

void free_trie_node(TrieNode *head) {
    if (head == NULL) {
        return;
    }

    for (int i = 0; i < head->alphabet_size; i++) {
        if (head->next[i] != NULL) {
            free_trie_node(head->next[i]);
            head->next[i] = NULL;
        }
    }

    free(head->next);
    free(head);
}

TrieNode* init_trie_node(int alphabet_size) {
    TrieNode *trie_node = (TrieNode *) malloc(sizeof(TrieNode));
    trie_node->next = (TrieNode **) malloc(sizeof(TrieNode *) * alphabet_size);
    for (int i = 0; i < alphabet_size; i++) {
        trie_node->next[i] = NULL;
    }
    trie_node->alphabet_size = alphabet_size;
    trie_node->is_word = 0;

    return trie_node;
}

void trie_add_word(Trie *trie, char* target, char* lex) {
    // assume word is separated by space
    if (trie->head_node == NULL) {
        trie->head_node = init_trie_node(trie->alphabet_size);
    }

    char word[LEX_LEN];
    strncpy(word, target, LEX_LEN);
    char curr_letter[LETTER_LEN];
    int aidx = 0;
    TrieNode *curr_node = trie->head_node;
    TrieNode *prev_node = NULL;
    char *token = strtok(word, " ");

    while (token != NULL) {
        // fprintf(stderr, "%s\n", token);
        aidx = 0;
        while (aidx < trie->alphabet_size) {
            if (!strcmp(token, trie->alphabet[aidx])) {
                break;
            }
            aidx++;
        }
        assert(aidx < trie->alphabet_size);

        if (curr_node->next[aidx] == NULL) {
            curr_node->next[aidx] = init_trie_node(trie->alphabet_size);
        }

        prev_node = curr_node;
        curr_node = curr_node->next[aidx];
        token = strtok(NULL, " ");
    }

    prev_node->is_word = 1;
    strncpy(prev_node->lex, lex, LEX_LEN);
    fprintf(stderr, "Added %s ... \n", prev_node->lex);
}

char* trie_find_word(Trie *trie, char* target) {
    char word[LEX_LEN];
    strncpy(word, target, LEX_LEN);
    char *token = strtok(word, " ");
    int aidx = 0;
    TrieNode *curr_node = trie->head_node;
    TrieNode *prev_node = NULL;
    
    while (token != NULL) {
        aidx = 0;
        while (aidx < trie->alphabet_size) {
            if (!strcmp(token, trie->alphabet[aidx])) {
                break;
            }
            aidx++;
        }
        if (aidx == trie->alphabet_size) {
            // not in alphabet
            return NULL;
        }

        if (curr_node->next[aidx] == NULL) {
            // not found
            return NULL;
        }

        prev_node = curr_node;
        curr_node = curr_node->next[aidx]; 
        token = strtok(NULL, " ");
    }

    if (prev_node != NULL && prev_node->is_word) {
        return prev_node->lex;
    } else {
        // not a word
        return NULL;
    }
}

int main() {
    // Trie *trie = trie_init("phones/letters.txt");
    // trie_add_word(trie, "o n e", "W AX N");
    // trie_add_word(trie, "t w o", "T UW");
    // trie_add_word(trie, "t h r e e", "TH R IY");
    // trie_add_word(trie, "f o u r", "F OW R");
    // char *target_word = "f o u r";
    
    Trie *trie = trie_init("phones/phones.txt");
    trie_add_word(trie, "W AX N", "one");
    trie_add_word(trie, "T UW", "two");
    trie_add_word(trie, "TH R IY", "three");
    trie_add_word(trie, "F OW R", "four");

    char *target_word = "F OW R";

    char *result = trie_find_word(trie, target_word);
    if (result == NULL) {
        fprintf(stderr, "Not found\n");
    } else {
        fprintf(stderr, "%s\n", result);
    }
}
