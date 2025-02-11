# Makefile for the pcthreads project

# Compiler and flags
pcthreads: proj3.cpp
	g++ -g -Wall -o pcthreads proj3.cpp

# Clean target to remove generated files
clean:
	rm -f pcthreads
