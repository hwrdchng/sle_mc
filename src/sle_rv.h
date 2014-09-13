/*
 * This contains functions for computing random variables
 * associated with an SLE path
 *
 */

#ifndef SLE_RAND_VAR_H
#define SLE_RAND_VAR_H

#include "sle_path.h"

typedef double (*rand_var)(const sle_t *sle);

void rv_append_value(const sle_t *sle, const rand_var rv, FILE *fp);

/* Examples of random variables to compute are here.
 * Add your own and see sle_mc for usage. */
double rv_horiz_dist_ratio(const sle_t *sle);
double rv_num_steps_greater_than_eps(const sle_t *sle);

#endif /* SLE_RAND_VAR_H */
