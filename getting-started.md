# Build

Ubuntu 16.10:

    ./configure

Development build with `gcc -O0 -ggdb3`:

    make

Production build with `gcc -O3 -g0`:

    make clean
    make G=0 O=3

Speedup due to `-O3` is dramatic (~10x).

Run:

    ./main.out
