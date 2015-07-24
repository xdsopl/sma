
CXXFLAGS = -stdlib=libc++ -std=c++11 -W -Wall -O3 -march=native
CXX = clang++

sma: sma.cc

.PHONY: clean test

test: sma
	./sma > /dev/null

clean:
	rm -f sma

