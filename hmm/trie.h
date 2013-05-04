#ifndef LEXICON_H
#define LEXICON_H

#define LEX_LEN (64)
#define LETTER_LEN (8)

typedef struct struct_trie_node {
    struct struct_trie_node** next;
    int alphabet_size;
    char lex[LEX_LEN];
    short is_word;

} TrieNode;

typedef struct struct_trie {
    char** alphabet;
    int alphabet_size;
    TrieNode *head_node;
} Trie;

#endif
