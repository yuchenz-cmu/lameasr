/*
 * =====================================================================================
 *
 *       Filename:  malloc_test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/02/2013 01:14:15 PM
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

int main(int argc, char** argv) {
    int *ptr;
    
    ptr = (int *) malloc(1000000000000000 * sizeof(int));
    sleep(1);
    free(ptr);

    return 0;
}

