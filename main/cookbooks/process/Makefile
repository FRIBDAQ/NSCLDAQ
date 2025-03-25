##
# Makefile for process
#
#   Assumes an daqsetup.bash has been sourced so that DAQROOT etc.
#   are defined:


CXX = g++

CXXFLAGS = -Wall -O2 -g -I. -I$(DAQINC)
LDFLAGS = -L$(DAQLIB) -Wl,-rpath=$(DAQLIB)
LDLIBS = -ldataformat -ldaqio -lException

all: process

process: process.o processor.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -c $< -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f process *.o
