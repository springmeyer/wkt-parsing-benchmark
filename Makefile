MAPNIK_CXXFLAGS=$(shell mapnik-config --cflags)
MAPNIK_LDFLAGS=$(shell mapnik-config --libs --ldflags) -licuuc -lboost_system -lboost_thread
# NOTE: add -F/ -framework CoreFoundation to benchmark in OS X instruments
GEOS_CXXFLAGS=$(shell geos-config --cflags)
GEOS_LDFLAGS=$(shell geos-config --ldflags ) -lgeos_c
CXXFLAGS := $(CXXFLAGS) # inherit from env
LDFLAGS := $(LDFLAGS) # inherit from env

all: test

run: test.cpp
	$(CXX) -o ./run test.cpp $(CXXFLAGS) $(MAPNIK_CXXFLAGS) $(GEOS_CXXFLAGS) $(LDFLAGS) $(MAPNIK_LDFLAGS) $(GEOS_LDFLAGS) 

test: run
	./run

clean:
	rm -r run


.PHONY: test
