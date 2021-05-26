$()

SRCS ?= $(shell find ../lexer/*.c*) $(shell find ../parser/*.c*) $(shell find ../CodeGenerator/*.c*) $(shell find ./*.c*) 
HRD  ?= $(shell find ../lexer/*.h*) $(shell find ../parser/*.h*) $(shell find ../CodeGenerator/*.h*) $(shell find ./*.h*) 
CXX  ?= g++

INCLUDE ?= -I ./ -I  ../lexer -I  ../parser -I ../CodeGenerator 
LIBS    ?= `llvm-config --cxxflags --ldflags --system-libs --libs all`


all:
	$(CXX) -std=c++2a   $(SRCS) $(LIBS) $(INCLUDE) -o begonia

