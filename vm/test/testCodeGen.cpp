#include "CodeGen.h"
#include "Parser.h"
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef STDERR_FILENO
#define STDERR_FILENO 0
#endif

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main() {
    signal(SIGSEGV, handler);
    begonia::Parser parser("./test_code.bga");
    parser.Parse();

    begonia::CodeGen generator;
    int ret_code = generator.initialize();
    if (ret_code != 0) {
        printf("generator.initialize err\n");
        return 1;
    }
    generator.generate(parser._ast);

    int retcode = system("ld -o test_code ./output.o  -lSystem -macosx_version_min 10.14");
    if (retcode != 0) 
        return 1;
    return 0;
}