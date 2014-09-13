/*
 * Wrapper for argument parsing using GNU argp
 */

#ifndef SLE_ARG_H
#define SLE_ARG_H

#include <argp.h>

#include "sle_path.h"

/* sle_mc arguments */
typedef struct sle_arg {
  /* File output: {path_output_file, rv_output_file} */
  char *args[2];

  /* SLE path parameters */
  sle_param_t sle_param;

  /* Monte Carlo parameters */
  int samples_total;
  int threads_total;

  /* Verbose output */
  int verbose;
} sle_arg_t;

error_t sle_parse(int argc, char *argv[], sle_arg_t *sle_arg);

#endif /* SLE_ARG_H*/
