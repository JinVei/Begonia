$()

SRCS ?= $(shell find ../../Lexer/*.c*) $(shell find ../*.c*) $(shell find ./*.c*)
HRD  ?= $(shell find ../../Lexer/*.h*) $(shell find ../*.h*) $(shell find ./*.h*)
CXX  ?= g++
INCLUDE ?= -I ./ -I  ../../Lexer -I  ../
LIBS    ?= 


all:
	$(CXX) -std=c++2a -stdlib=libc++ $(LIBS)  $(SRCS) $(INCLUDE) -o TestParser

