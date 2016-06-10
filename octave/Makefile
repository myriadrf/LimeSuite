# Makefile for Octave extension LimeSuite
SHELL := /bin/bash

.phony: all
all: LimeSuite.oct

.phony: clean
clean:
	rm -f LimeSuite.oct *.o

.phony: distclean
distclean: clean
	rm -f *~ octave-core core

.phony: test
test: LimeSuite_oct.oct
	octave --silent LimeInitialize.m

%.oct: %.cc	
#-LD:/projects/lms7suite/octave
	mkoctfile -v -I/usr/local/include/lime -L. -lLimeSuite $<
