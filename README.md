Key-Value Map Implementations in C and Development Framework
============================================================

This project contains a framework for developing Key-Value Maps or
Key-Value Sets. The current version only contains an implementation
of the Skiplist data-structure but more implementations will come.


## Build and Test ##

### Requirements ###

The following tools are required to test and build:

* git
* gcc
* [Scons](http://www.scons.org/)

Run something like the following on Debian and Ubuntu systems:

`sudo apt-get install git scons gcc`

### Step by Step Instructions ###

1. `git clone git://github.com/kjellwinblad/c_maps.git`
2. `cd c_maps`
3. `scons`
4. `./bin/test_concurrent_skiplist`

If everything is alright the message *"SKIPLIST TESTS COMPLETED!"*
will be printed to standard output.

## Usage  ##

The best way to see how the data-structure and the framework can be
used is currently to look in the file `test_skiplist.c` and
`test_kvset.c`.
