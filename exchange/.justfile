# .justfile for exchange/ — quick build/run helpers
# Usage:
#   just          # default -> build
#   just build
#   just run      # run the produced binary
#   just clean

build:
	# single-step compile+link (uses clang++ to pull in the C++ runtime)
	clang++ -std=c++17 -g test.cpp OfferExchange.cpp -o exchange_test

run:
	./exchange_test

clean:
	rm -f *.o exchange_test
