# Modified Makefile for using vmeClass.cpp
# Kolby Kiesling
# kjk15b@acu.edu
# 07 / 15 / 2019

here=$(shell pwd)

.LIBPATTERNS ="'lib%.so lib%.a' "

CXX   = gcc
FC		= g77
F90		= f90
RM		= rm -f
CC		= g++

LD            = gcc
LDFLAGS       = -O
SOFLAGS       = -shared


CXXLIBDIRS	= -L$(HOME)/.local/lib -L$(HOME)/src/teststand/vme_test/OLD
CXXLIBS		= -lxx_usb -lm -lusb -Wl,"-rpath=$(HOME)/.local/lib" -std=c++11
INCLUDEDIRS 	= -I$(HOME)/.local/include -I$(usr)/include/root -I$(HOME)/src/teststand/vme_test/OLD

CXXFLAGS      	= -O -Wall -fPIC -g -fno-rtti -fpermissive $(INCLUDEDIRS)

CFLAGS		+= $(INCLUDEDIRS) $(SOFLAG)
CFLAGS		+= $(CXXLIBDIRS) $(CXXLIBS)
LDFLAGS		= $(CXXFLAGS)

all: xxusb.a vme_simple_test

%.o: %.cpp
	g++ -g -O2 -std=c++11 -I. -c $^ $(CXXLIBS) $(CXXLIBDIRS) $(CXXFLAGS)

xxusb.a: vmeClass.o
	ar rc $@ $^

clean:
	rm -f xxusb.a *.o vme_simple_test *.out

vme_simple_test:
	$(CXX) $(CXXFLAGS) vme_simple_test.cc $(CXXLIBS) $(CXXLIBDIRS) xxusb.a xxusb.a