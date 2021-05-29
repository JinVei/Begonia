$()

SRCS ?= $(shell find ./lexer/*.c*) $(shell find ./parser/*.c*) $(shell find ./CodeGenerator/*.c*) $(shell find ./exe/*.c*) 
HRD  ?= $(shell find ./lexer/*.h*) $(shell find ./parser/*.h*) $(shell find ./CodeGenerator/*.h*) $(shell find ./exe/*.h*) 
CXX  ?= g++

INCLUDE ?= -I ./exe -I  ./lexer -I  ./parser -I ./CodeGenerator 
LIBS    ?= `llvm-config --cxxflags --ldflags --system-libs --libs all`


all:
	$(CXX) -std=c++2a   $(SRCS) $(LIBS) $(INCLUDE) -o ./bin/begonia -ggdb

