
CXXFLAGS = -std=c++11 -W -Wall -O3 -march=native -ffast-math
CXX = clang++ -stdlib=libc++
#CXX = g++

sma: sma.cc

.PHONY: clean test

test: sma
	./sma > /dev/null

clean:
	rm -f sma

