#include "CodeGen.h"
#include "Parser.h"
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

void sig_handler(int sig) {
  void *array[10];
  size_t size;

  size = backtrace(array, 10);

  fprintf(stderr, "backtrace. signal: %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("need input file\n");
        return 1;
    }
    signal(SIGSEGV, sig_handler);
    printf("compiling %s\n", argv[1]);
    begonia::Parser parser(argv[1]);

    parser.Parse();

    begonia::CodeGen generator;
    int ret_code = generator.initialize();
    if (ret_code != 0) {
        printf("generator. initialize err\n");
        return 1;
    }

    ret_code = generator.generate(parser._ast);
    if (ret_code != 0) {
        printf("generator.generate(parser._ast) error");
        return 1;
    }
    return 0;
}