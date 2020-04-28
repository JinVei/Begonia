$()

SRCS ?= $(shell find ../Lexer/*.c*) $(shell find ../parser/*.c*) $(shell find ../vm/*.c*) $(shell find ./*.c*) 
HRD  ?= $(shell find ../Lexer/*.h*) $(shell find ../parser/*.h*) $(shell find ../vm/*.h*) $(shell find ./*.h*) 
CXX  ?= g++
INCLUDE ?= -I ./ -I  ../Lexer -I  ../parser -I  ../vm
LIBS    ?= -L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib `llvm-config --cxxflags --ldflags --system-libs --libs all`


all:
	$(CXX) -std=c++2a -stdlib=libc++ $(LIBS)  $(SRCS) $(INCLUDE) -o begonia

