#ifndef __TEST_KVSET_H__
#define __TEST_KVSET_H__

#include "kvset.h"

void test_general_kvset_properties(KVSet * (*create_kvset_fun)());

void test_ordered_kvset_properties(KVSet * (*create_kvset_fun)());

void test_unordered_kvset_properties(KVSet * (*create_kvset_fun)());

#endif
