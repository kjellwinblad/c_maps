Key-Value Map Implementations in C and Development Framework
============================================================

This project contains a framework for developing Key-Value Maps or
Key-Value Sets. The current version only contains an implementation
of the Skiplist data-structure but more implementations will come.


## Build and Test ##

### Requirements ###

The following tools are required to test and build:

* git
* GNU Make
* gcc
* valgrind

Run something like the following on Debian and Ubuntu systems:

`sudo apt-get install git make gcc valgrind`

### Step by Step Instructions ###

1. `git clone git://github.com/kjellwinblad/c_maps.git`
2. `cd c_skiplist`
3. `make run_test_skiplist`

If everything is alright the message *"SKIPLIST TESTS COMPLETED!"*
will be printed to standard output.

## Run Benchmark ##

1. Follow the instructions in section *Build and Test*
2. `make run_benchmark_skiplist`

## Usage  ##

The best way to see how the data-structure and the framework can be
used is currently to look in the file `test_skiplist.c` and
`test_kvset.c`.
