#ifndef BENCH_H
#define BENCH_H
#include "tst.h"

double tvgetf();


int bench_test(const tst_node *root,
               char *out_file,
               const int max);

#endif
