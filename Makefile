# -*- Makefile -*-

INC_DIR := -I"$(OCL_ROOT)/include"
CXX := g++
CXXFLAGS := -std=c++14 -g $(INC_DIR) -Wno-ignored-attributes
LIB_DIR := -L"$(OCL_ROOT)/lib/x86_64"
LDFLAGS := $(LIB_DIR) -lopencl 

main: main.o
	$(CXX) $? $(LDFLAGS) -o $@


.PHONY: clean

clean:
	rm *.o main
