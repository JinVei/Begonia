$()

SRCS ?= $(shell find ../../Lexer/*.c*) $(shell find ../*.c*) $(shell find ./*.c*)
HRD  ?= $(shell find ../../Lexer/*.h*) $(shell find ../*.h*) $(shell find ./*.h*)
CXX  ?= g++
INCLUDE ?= -I./ -I../../Lexer -I../ -I/usr/local/opt/llvm/include
LIBS    ?=  -L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib `llvm-config --cxxflags --ldflags --system-libs --libs all`


all:
	$(CXX) -std=c++2a -stdlib=libc++ $(LIBS)  $(SRCS) $(INCLUDE) -o TestParser

