/*
 * Parallel Monte Carlo simulations of SLE curves
 * Howard Cheng
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#include "sle_arg.h"
#include "sle_path.h"
#include "sle_rv.h"

int main(int argc, char* argv[])
{
  int i, thread_i, sample_i = 0;
  FILE *file_p, *file_rv;

  /* Array for independent SLE samples, one for each thread */
  sle_t *sle_arr;

  /* Random variable to compute */
  rand_var rv = &rv_horiz_dist_ratio;
  const char *rv_info = "# Random variable: horizontal distance ratio";

  /* Parse arguments and set parameters */
  sle_arg_t sle_arg;
  sle_parse(argc, argv, &sle_arg);

  /* Initialize threads */
  omp_set_num_threads(sle_arg.threads_total);
  sle_arr = malloc(sizeof(sle_t) * sle_arg.threads_total);
  printf("--------------------------------------------------------------------"
         "----------\n");
  printf("Total number of samples: %d\n", sle_arg.samples_total);
  srand(time(NULL));
  for(thread_i = 0; thread_i < sle_arg.threads_total; ++thread_i){
      sle_init(&sle_arr[thread_i], sle_arg.sle_param);
  }

  if(sle_arg.verbose) {
    sle_print_param(&sle_arr[0], stdout);
  }

  /* Random variable statistics output file */
  remove(sle_arg.args[1]);
  file_rv = fopen(sle_arg.args[1], "a");
  fprintf(file_rv, "%s\n", rv_info);
  sle_print_param(&sle_arr[0], file_rv);

  /* Main loop, generate SLE paths and compute random variables */
  if(sle_arg.verbose) {
    printf("Generating random SLE paths...\n");
  }

  #pragma omp parallel for
  for(i = 0; i < sle_arg.samples_total; ++i){
    int thread_i = omp_get_thread_num();

    sle_gen_path(&sle_arr[thread_i], rand());

    #pragma omp critical
    {
      rv_append_value(&sle_arr[thread_i],
                      rv,
                      file_rv);
      /* Update and print sample_i as opposed to i because
       * concurrency means that i is not executed in sequential
       * order */
      ++sample_i;
      fflush(stdout);
      if(sle_arg.verbose) {
        printf("Computing sample number: %d\r", sample_i);
      }
    }
  }

  if(sle_arg.verbose) {
    printf("\33[2K\r"); /* VT100 escape code (clears line) */
    printf("Done\n");
  }

  printf("Computed random variables written to: %s\n", sle_arg.args[1]);
  fclose(file_rv);

  /* Write last generated SLE path to a file */
  file_p = fopen(sle_arg.args[0], "w");
  sle_print_path(&sle_arr[0], file_p);
  printf("Last sample path written to: %s\n", sle_arg.args[0]);
  fclose(file_p);

  /* Free resources */
  for(thread_i = 0; thread_i < sle_arg.threads_total; ++thread_i){
    sle_free(&sle_arr[thread_i]);
  }
  free(sle_arr);
  printf("--------------------------------------------------------------------"
         "----------\n");
  return 0;
}
