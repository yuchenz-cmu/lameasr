/*
 * =====================================================================================
 *
 *       Filename:  utils.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/05/2013 10:29:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <math.h>
#include <stdio.h>

#ifndef M_LN2
#define M_LN2 (0.69314718055994530942)
#endif

/* 
 * calculate log(exp(left) + exp(right)) more accurately
 * based on http://www.cs.cmu.edu/~roni/11761-s12/assignments/log_add.c
 * */
float utils_log_add(float left, float right) {
    if (right < left) {
        return left + log1p(exp(right - left));
    } else if (right > left) {
        return right + log1p(exp(left - right));
    } else {
        return left + M_LN2;
    }
}
 
/* 
int main(int argc, char **argv) {
    float result = utils_log_add(-1.897119985, -1.46967597);
    printf("%f\n", result);
    return 0;
}
*/
