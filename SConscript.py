from collections import OrderedDict
import multiprocessing

import sys

############
#Definitions
############

Import('mode')
std_cc_flags = ['-std=gnu99',
                '-Wall',
                '-pthread']

std_link_flags = ['-pthread']

debug_flags = ['-O1',
               '-g']

optimization_flags = ['-O3']

profile_flags = ['-fno-omit-frame-pointer -O2']

if(mode=='debug'):
    std_cc_flags = std_cc_flags + debug_flags
    std_link_flags = std_link_flags + debug_flags
elif(mode=='profile'):
    std_cc_flags = std_cc_flags + profile_flags
    std_link_flags = std_link_flags + profile_flags
else:
    std_cc_flags = std_cc_flags + optimization_flags
    std_link_flags = std_link_flags + optimization_flags

num_of_cores_str=str(multiprocessing.cpu_count())

env = Environment(
    CCFLAGS = ' '.join(std_cc_flags),
    LINKFLAGS = ' '.join(std_link_flags),
    CPPPATH = ['.',
               'src/utils',
               'src/maps',
               'src/tests'])

test_kvset_object = env.Object(source = 'src/tests/test_kvset.c',
                               CPPDEFINES = [('NUMBER_OF_THREADS', num_of_cores_str)])

env.Program(target='test_skiplist',
            source=['src/maps/skiplist.c',
                    'src/tests/test_skiplist.c',
                    test_kvset_object])


env.Program(target='test_concurrent_skiplist',
            source=['src/maps/concurrent_skiplist.c',
                    'src/tests/test_concurrent_skiplist.c',
                    'src/utils/hazard_pointers.c',
                    test_kvset_object])
