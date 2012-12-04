#set term x11
#set output
#set term post eps
#set output 'benchmark.eps'
set terminal png
set output 'benchmark.png'
set title "Benchmark"
set xlabel "Number of Elements"
set ylabel "Time per operation"
#set key below
set logscale x;
#f(x)= (log(x) / log(2))
#f(x) title 'log2',
plot "benchmark_skiplist_lookup.dat" using 1:2 title 'lookup' with linespoints,\
     "benchmark_skiplist_put.dat" using 1:2 title 'put' with linespoints,\
     "benchmark_skiplist_remove.dat" using 1:2 title 'remove' with linespoints

