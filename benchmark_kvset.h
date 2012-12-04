#ifndef __BENCHMARK_KVSET_H__
#define __BENCHMARK_KVSET_H__

#include "kvset.h"

void run_benchmark(KVSet * (*create_kvset_fun)(),
                   int max_num_of_operations,
                   char benchmark_identifier[]);

#endif

