# wkt-parsing-benchmark

WKT geometry parsing benchmarks

## Depends

 - libmapnik
 - libgeos_c

## Running

To build and run the benchmarks simply do:

    make

## Results

### 1) OS X 

    $ clang++ -v
    Apple LLVM version 4.2 (clang-425.0.28) (based on LLVM 3.2svn)
    Target: x86_64-apple-darwin12.3.0
    Thread model: posix
    
    $ geos-config --version
    3.3.8
    
    $ mapnik-config --git-describe
    v2.1.0-1112-g401ca20

### mapnik compiled against libc++, using -std=c++11

    $ ./run
    1) threaded -> mapnik: 640 milliseconds
    2) threaded -> geos: 2470 milliseconds
    3) mapnik: 2800 milliseconds
    4) geos: 9150 milliseconds

### mapnik compiled against libstdc++, using -ansi

    $ ./run
    1) threaded -> mapnik: 880 milliseconds
    2) threaded -> geos: 2520 milliseconds
    3) mapnik: 3440 milliseconds
    4) geos: 9200 milliseconds

